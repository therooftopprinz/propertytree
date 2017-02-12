#ifndef INTERFACE_NEWMESSAGECODEC_HPP_
#define INTERFACE_NEWMESSAGECODEC_HPP_

#include <cstdint>
#include <vector>
#include <cstring>

namespace ptree
{
namespace protocol
{

struct Buffxr
{
    Buffxr() = default;
    Buffxr(uint32_t size) :
        mData(new uint8_t[size]),
        mSize(size)
    {
    }

    Buffxr(Buffxr&) = delete;
    Buffxr(Buffxr&& other)
    {
        deallocate();
        move(other);
    }

    Buffxr& operator= (Buffxr&& other)
    {
        deallocate();
        move(other);
        return *this;
    }

    ~Buffxr()
    {
        deallocate();
    }

    uint8_t* data()
    {
        return mData;
    }

    uint32_t size()
    {
        return mSize;
    }

private:
    void move(Buffxr& other)
    {
        mData = other.mData;
        mSize = other.mSize;
        other.mData = nullptr;
        other.mSize = 0;
    }

    void deallocate()
    {
        if (mData)
        {
            delete[] mData;
            mData = nullptr;
            mSize = 0;
        }
    }

    uint8_t* mData = nullptr;
    uint32_t mSize = 0;
};

class String
{
public:

    char *get()
    {
        return mStr;
    }
    void fill(uint8_t** offset)
    {
        mStr = (char*)(*offset);
        *offset += mSize;
    }
    void loadFrom(uint8_t** offset)
    {
        mStr = (char*)(*offset);
        mSize = std::strlen(mStr);
        *offset += mSize;
    }
    uint32_t size()
    {
        return mSize;
    }
    void setSize(uint32_t sz)
    {
        mSize = sz;
    }
private:
    char* mStr;
    uint32_t mSize;
};

template<typename T>
class Simple
{
public:
    T& get()
    {
        return *v;
    }
    void fill(uint8_t** offset)
    {
        v = (T*)(*offset);
        *offset += size();
    }

    void loadFrom(uint8_t** offset)
    {
        fill(offset);
    }

    constexpr uint32_t size()
    {
        return sizeof(T);
    }
private:
    T* v;
};

template<typename T,typename IndexSize = uint32_t>
class SimpleArray
{
public:
    std::vector<T>& get()
    {
        return mValues;
    }
    void fill(uint8_t** offset)
    {
        *(IndexSize*)(**offset) = mSize;
        *offset += sizeof(IndexSize);
        mValues = *(IndexSize*)(**offset);
        *offset += sizeof(IndexSize)*mSize;
    }

    void loadFrom(uint8_t** offset)
    {
        IndexSize mSize = *(IndexSize*)(**offset);
        *offset += sizeof(IndexSize);
        mValues = *(IndexSize*)(**offset);
        *offset += sizeof(IndexSize)*mSize;
    }

    void setSize(uint32_t sz)
    {
        mSize = sz;
    }

    uint32_t size()
    {
        return mSize;
    }
private:
    T *mValues;
    uint32_t mSize;
};

template<typename T>
struct Complex
{
public:
    T& get()
    {
        return v;
    }

    void fill(uint8_t** offset)
    {
        v.fill(offset);
    }

    void loadFrom(uint8_t** offset)
    {
        v.loadFrom(offset);
    }

    uint32_t size()
    {
        return v.size();
    }
private:
    T v;
};

template<typename T, typename IndexSize = uint16_t>
class ComplexArray
{
public:
    std::vector<T>& get()
    {
        return mValues;
    }
    void fill(uint8_t** offset)
    {
        *(IndexSize*)(*offset) = mValues.size();
        *offset += sizeof(IndexSize);
        for (auto& i : mValues)
        {
            i.fill(offset);
        }
    }

    void loadFrom(uint8_t** offset)
    {
        IndexSize nElems = *(IndexSize*)(*offset);
        *offset += sizeof(IndexSize);
        for (IndexSize i = 0; i<nElems; i++)
        {
            T e;
            e.loadFrom(offset);
            // mValues.emplace_back(e);
        }
    }

    uint32_t size()
    {
        uint32_t sz = sizeof(IndexSize);
        for (auto& i : mValues)
        {
            sz += i.size();
        }
        return sz;
    }
private:
    std::vector<T> mValues;
};

class PointerFiller
{
public:
    PointerFiller(uint8_t** offset):
        offset(offset)
    {
    }

    void fill()
    {
    }

    template<typename T, typename... Ts>
    void fill(T& head, Ts&... tail)
    {
        head.fill(offset);
        fill(tail...);
    }

    void loadFrom()
    {
    }

    template<typename T, typename... Ts>
    void loadFrom(T& head, Ts&... tail)
    {
        head.loadFrom(offset);
        loadFrom(tail...);
    }
private:
    uint8_t** offset;
};

class Sizer
{
public:
    Sizer(uint32_t& size):
        accSz(size)
    {
    }

    void size()
    {
    }

    template<typename T, typename... Ts>
    void size(T& head, Ts&... tail)
    {
        accSz += head.size();
        size(tail...);
    }
private:
    uint32_t& accSz;
};

#define MESSAGE_FIELDS_2(...) \
void allocate(){ \
    uint32_t sz = size(); \
    data =std::move (Buffxr(sz)); \
    fill(); \
} \
uint32_t size(){ \
    uint32_t sz = 0; \
    Sizer sizer(sz); \
    sizer.size(__VA_ARGS__); \
    return sz; \
} \
void fill() { \
    uint8_t* offset = data.data(); \
    PointerFiller filler(&offset); \
    filler.fill(__VA_ARGS__); \
} \
void fill(uint8_t** offset) { \
    PointerFiller filler(offset); \
    filler.fill(__VA_ARGS__); \
} \
void loadFrom() { \
    uint8_t* offset = data.data(); \
    PointerFiller filler(&offset); \
    filler.loadFrom(__VA_ARGS__); \
} \
void loadFrom(uint8_t** offset) { \
    PointerFiller filler(offset); \
    filler.loadFrom(__VA_ARGS__); \
} \
void loadFrom(Buffxr d) { \
    data = std::move(d); \
    loadFrom(); \
} \
Buffxr data;

} // namespace protocol
} // namespace ptree

#endif  // INTERFACE_MESSAGECODEC_HPP_