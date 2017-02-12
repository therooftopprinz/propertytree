#ifndef INTERFACE_MESSAGECODECHELPERS_HPP_
#define INTERFACE_MESSAGECODECHELPERS_HPP_

#include <vector>
#include <cstring>
#include <exception>
#include <iostream>

namespace ptree
{
namespace protocol
{

using Buffer = std::vector<uint8_t>;

struct BufferView
{
    BufferView(){}
    BufferView(Buffer& buffer):
        mStart(buffer.data()),
        mLimit(buffer.data()+buffer.size())
    {}
    BufferView(uint8_t* start, uint8_t* limit):
        mStart(start),
        mLimit(limit)
    {}

    uint8_t* mStart;
    uint8_t* mLimit;
};

struct BadData : public std::exception
{
    BadData(const char * msg):
        msg_(msg)
    {}

    BadData(std::string msg):
        msg_(msg)
    {}

    virtual ~BadData() throw (){}

    virtual const char* what() const throw (){
       return msg_.c_str();
    }

protected:
    std::string msg_;
};

struct BlockBase
{
    virtual void generate(BufferView& data) = 0;
    virtual void parse(BufferView& data) = 0;
    virtual uint32_t size() = 0;
};

inline void codecError(std::string fn, std::string file, int ln)
{
    std::string err;
    err += "Cannot translate in ";
    err += fn;
    err += "\nfile: ";
    err += file;
    err += ":" + std::to_string(ln);
    throw BadData(err);
}

inline void errorOnLimit(BufferView &data, uint32_t size, std::string fn, std::string file, int ln)
{
    if (data.mStart+size > data.mLimit)
    {
        codecError(fn, file, ln);
    }
}

/***  BlockBase CODEC ***/
inline void sizeRead(BlockBase& head, uint32_t& mCurrentSize)
{
    mCurrentSize += head.size();
}

inline void encode(BlockBase& head, BufferView& mEncodeCursor)
{
    head.generate(mEncodeCursor);
}

inline void decode(BlockBase& head, BufferView& mDecodeCursor)
{
    head.parse(mDecodeCursor);
}

/***  GENERIC ARRAY CODEC ***/
template <typename T, size_t N>
void sizeRead(T (&)[N], uint32_t& mCurrentSize)
{
    mCurrentSize += sizeof(T)*N;
}

template <typename T, size_t N>
void encode(T (&value)[N], BufferView& data)
{
    uint32_t size = sizeof(T)*N;
    errorOnLimit(data, size, __PRETTY_FUNCTION__, __FILE__, __LINE__);
    std::memcpy(data.mStart, value, sizeof(T)*N);
    data.mStart += sizeof(T)*N;
}

template <typename T, size_t N>
void decode(T (&value)[N], BufferView& data)
{
    uint32_t size = sizeof(T)*N;
    errorOnLimit(data, size, __PRETTY_FUNCTION__, __FILE__, __LINE__);
    std::memcpy(value, data.mStart, size);
    data.mStart += size;
}

/***  GENERIC CODEC ***/
template <typename T>
void sizeRead(T&, uint32_t& mCurrentSize)
{
    mCurrentSize += sizeof(T);
}

template<typename T>
void encode(T& value, BufferView& data)
{
    errorOnLimit(data, sizeof(T), __PRETTY_FUNCTION__, __FILE__, __LINE__);
    *(T*)(data.mStart) = value;
    data.mStart += sizeof(T);
}

template<typename T>
void decode(T& value, BufferView& data)
{
    errorOnLimit(data, sizeof(T), __PRETTY_FUNCTION__, __FILE__, __LINE__);
    value = *(T*)(data.mStart);
    data.mStart += sizeof(T);
}

/***  std::string CODEC ***/
inline void sizeRead(std::string& value, uint32_t& mCurrentSize)
{
    mCurrentSize += value.size()+1;
}

inline void decode(std::string& value, BufferView& data)
{
    uint32_t size = value.size()+1;
    errorOnLimit(data, size, __PRETTY_FUNCTION__, __FILE__, __LINE__);
    uint8_t *nullf = data.mStart;
    while (nullf<data.mLimit && *(nullf++));
    value = std::string((char*)data.mStart, nullf-1-data.mStart);
    data.mStart = nullf;
}

inline void encode(std::string& value, BufferView& data)
{
    uint32_t size = value.size()+1;
    errorOnLimit(data, size, __PRETTY_FUNCTION__, __FILE__, __LINE__);
    std::memcpy(data.mStart, value.c_str(), size);
    data.mStart += size;
}

/***  std::vector CODEC ***/
template<typename T>
inline void sizeRead(std::vector<T>& values, uint32_t& mCurrentSize)
{
    typedef uint32_t IndexType;
    /** vector codec has fix type for L which is u16 **/
    mCurrentSize += sizeof(IndexType);
    for (auto& i : values)
    {
        sizeRead(i, mCurrentSize);
    }
}

template<typename T>
inline void decode(std::vector<T>& values, BufferView& data)
{
    typedef uint32_t IndexType;
    errorOnLimit(data, sizeof(IndexType), __PRETTY_FUNCTION__, __FILE__, __LINE__);
    uint32_t size = *(IndexType*)(data.mStart);
    data.mStart += sizeof(IndexType);
    values.resize(size);
    for (uint32_t i=0; i < size; i++)
    {
        decode(values[i], data);
    }
}

template<typename T>
inline void encode(std::vector<T>& values, BufferView& data)
{
    typedef uint32_t IndexType;
    errorOnLimit(data, sizeof(IndexType), __PRETTY_FUNCTION__, __FILE__, __LINE__);
    *(IndexType*)(data.mStart) = values.size();
    data.mStart += sizeof(IndexType);
    for (auto& i : values)
    {
        encode(i, data);
    }
}

/***  BlockArray CODEC ***/
template <typename T, typename IndexType = uint16_t>
struct BlockArray : public BlockBase
{
public:
    void generate(BufferView& data)
    {
        errorOnLimit(data, sizeof(IndexType), __PRETTY_FUNCTION__, __FILE__, __LINE__);
        *(IndexType*)(data.mStart) = values.size();
        data.mStart += sizeof(IndexType);

        for (auto& i : values)
        {
            i.generate(data);
        }
    }
    void parse(BufferView& data)
    {
        errorOnLimit(data, sizeof(IndexType), __PRETTY_FUNCTION__, __FILE__, __LINE__);
        uint32_t size = *(IndexType*)(data.mStart);
        data.mStart += sizeof(IndexType);
        values.resize(size);
        for (uint32_t i=0; i<size; i++)
        {
            values[i].parse(data);
            if (data.mStart == data.mLimit)
            {
                break;
            }
            else if(data.mStart > data.mLimit)
            {
                codecError(__PRETTY_FUNCTION__, __FILE__, __LINE__);
            }
        }
    }
    uint32_t size()
    {
        uint32_t sz = sizeof(IndexType);
        for (auto& i : values)
        {
            sz += i.size();
        }
        return sz;
    }

    std::vector<T>& get()
    {
        return values;
    }

private:
    std::vector<T> values;
};

#define BLOCK (BlockBase&)

} // namespace protocol
} // namespace ptree

#endif  // INTERFACE_MESSAGECODECHELPERS_HPP_
