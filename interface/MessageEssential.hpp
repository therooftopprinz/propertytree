#ifndef INTERFACE_MESSAGEESSENTIAL_HPP_
#define INTERFACE_MESSAGEESSENTIAL_HPP_

#include <vector>
#include <cstring>  

namespace ptree
{

namespace protocol
{

using Buffer = std::vector<uint8_t>;

struct BufferView
{
    BufferView(){}
    BufferView(Buffer& buffer):
        start(buffer.data()),
        limit((uint8_t*)buffer.data()+buffer.size())
    {}
    BufferView(uint8_t* start, uint8_t* limit):
        start(start),
        limit(limit)
    {}

    uint8_t* start;
    uint8_t* limit;
};

template <typename T>
class BlockArray
{
public:
    BlockArray(){}

    std::vector<T>& operator*()
    {
        return values;
    }

    std::vector<T>* operator->()
    {
        return &values;
    }

    T& operator[](int32_t index)
    {
        return values[index];
    }

    void generate(BufferView& data)
    {
        // std::cout << __PRETTY_FUNCTION__ << std::endl;
        if (data.start+sizeof(uint32_t)>data.limit)
            return;
        *(uint32_t*)(data.start) = values.size();
        data.start += sizeof(uint32_t);

        for (auto& i : values)
        {
            i.generate(data);
        }
    }

    inline uint8_t* parse(uint8_t *start, uint8_t *limit)
    {
        uint32_t size = *(uint32_t*)(start);
        if (start >= limit)
        {
            return start;
        }

        start+=sizeof(uint32_t);
        values.resize(size);
        for (uint32_t i=0; i<size; i++)
        {
            start = values[i].parse(start, limit);
            if (start>=limit)
            {
                break;
            }
        }
        return start;
    }

    uint32_t size()
    {
        uint32_t sz = sizeof(uint32_t);
        for (auto& i : values)
        {
            sz += i.size();
        }
        return sz;
    }

private:
    std::vector<T> values;
};

class String
{
public:
    String(){}

    String(const char* cpvalue):
        value(cpvalue)
    {}

    String(std::string& value):
        value(value)
    {}

    String(std::string&& value):
        value(std::move(value))
    {}

    String& operator =(std::string& t)
    {
        value = t;
        return *this;
    }

    String& operator =(std::string&& t)
    {
        value = std::move(t);
        return *this;
    }

    String& operator =(const char* cpvalue)
    {
        value = cpvalue;
        return *this;
    }

    std::string& operator *()
    {
        return value;
    }

    std::string* operator->()
    {
        return &value;
    }


    operator std::string& ()
    {
        return value;
    }

    void generate(BufferView& data)
    {
        // std::cout << __PRETTY_FUNCTION__ << std::endl;
        if (data.start+value.size()+1>data.limit)
            return;
        std::memcpy(data.start, value.c_str(), value.size()+1);
        data.start += value.size()+1;
    }

    inline uint8_t* parse(uint8_t *start, uint8_t *limit)
    {
        uint8_t *nullf=start;
        while (nullf<limit && *(nullf++));
        value = std::string((char*)start, nullf-1-start);
        return nullf;
    }

    uint32_t size()
    {
        return value.size()+1;
    }

private:
    std::string value;
};

class BufferBlock
{
public:
    BufferBlock(){}

    BufferBlock(Buffer& value):
        value(value)
    {}

    BufferBlock(Buffer&& value):
        value(std::move(value))
    {}

    Buffer& operator*()
    {
        return value;
    }

    Buffer* operator->()
    {
        return &value;
    }

    void generate(BufferView& data)
    {
        if (data.start+value.size()+sizeof(uint32_t)>data.limit)
            return;

        *((uint32_t*)(data.start)) = value.size();
        data.start += sizeof(uint32_t);
        std::memcpy(data.start, value.data(), value.size());
        data.start += value.size();
    }

    inline uint8_t* parse(uint8_t* start, uint8_t* limit)
    {
        uint32_t size = *(uint32_t*)(start);
        if (start+size > limit)
        {
            return limit;
        }
        value.resize(size);
        std::memcpy(value.data(), start+sizeof(uint32_t), size);
        return start + sizeof(uint32_t) + size;
    }

    uint32_t size()
    {
        return sizeof(uint32_t)+value.size();
    }

private:
    Buffer value;
};

template <typename T>
class Simple
{
public:
    Simple(){}

    Simple(T& value):
        value(value)
    {}

    Simple(T&& value):
        value(std::move(value))
    {}

    T& operator*()
    {
        return value;
    }

    T* operator->()
    {
        return &value;
    }

    T& operator=(T& t)
    {
        value = t;
        return value;
    }

    T& operator=(T&& t)
    {
        value = std::move(t);
        return value;
    }

    operator T&()
    {
        return value;
    }

    void generate(BufferView& data)
    {
        // std::cout << __PRETTY_FUNCTION__ << std::endl;
        if (data.start+sizeof(T)>data.limit)
            return;
        *((T*)(data.start)) = value;
        data.start += sizeof(T);
    }

    inline uint8_t* parse(uint8_t* start, uint8_t* limit)
    {
        if (start+sizeof(T) > limit)
        {
            return limit;
        }
        value = *((T*)start);
        return start + sizeof(T);
    }

    uint32_t size()
    {
        return sizeof(T);
    }

private:
    T value;
};

class SizeReader
{
public:
    SizeReader():currentsize(0) {}

    template <typename T>
    void translate(T& head)
    {
        currentsize += head.size();
    }
    template <typename T, typename... Tt>
    void translate(T& head, Tt&... tail)
    {
        translate(head);
        translate(tail...);
    }
    uint32_t size()
    {
        return currentsize;
    }
private:
    uint32_t currentsize;
};

class Encoder
{
public:
    Encoder(BufferView& codedData):
        encodeCursor(codedData)
    {}

    template <typename T>
    void translate(T& head)
    {
        // std::cout << __PRETTY_FUNCTION__ << std::endl;
        head.generate(encodeCursor);
    }
    template <typename T, typename... Tt>
    void translate(T& head, Tt&... tail)
    {
        // std::cout << __PRETTY_FUNCTION__ << std::endl;
        translate(head);
        translate(tail...);
    }

    BufferView& encodeCursor;
};

class Decoder
{
public:
    Decoder(uint8_t* start, uint8_t* limit):
        start(start),
        limit(limit)
    {}
    template <typename T, typename... Tt>
    void translate(T& head)
    {
        // std::cout << __PRETTY_FUNCTION__ << std::endl;
        start = head.parse(start, limit);
    }
    template <typename T, typename... Tt>
    void translate(T& head, Tt&... tail)
    {
        // std::cout << __PRETTY_FUNCTION__ << std::endl;
        translate(head);
        translate(tail...);
    }
    uint8_t* getNext()
    {
        return start;
    }

    uint8_t* start;
    uint8_t* limit;
};

// THUG CPP
#define MESSAGE_FIELDS(...)\
template<typename T>\
inline void operate(T& codec)\
{\
    codec.translate(__VA_ARGS__);\
}\
inline void operator >> (Encoder& en)\
{\
    operate(en);\
}\
inline void operator << (Decoder& de)\
{\
    operate(de);\
}\
void generate(BufferView& data)\
{\
    Encoder en(data);\
    *this >> en;\
}\
inline uint8_t* parse(uint8_t* start, uint8_t* limit)\
{\
    Decoder de(start, limit);\
    *this << de;\
    return de.getNext();\
}\
uint32_t size()\
{\
    SizeReader sr;\
    sr.translate(__VA_ARGS__);\
    return sr.size();\
}

#define TVUNIONBEGIN \
inline uint8_t* multiDoFuntion(ptree::protocol::BufferView& data, uint8_t* start, uint8_t* limit, uint32_t isTo) {

#define UNION(id, variable) \
if (type == id){ \
    if (isTo == 0) /** encode **/ { \
        variable.generate(data); \
    } else if (isTo == 1) /** decode **/ { \
        return variable.parse(start,limit); \
    } else /** size */{ \
        return reinterpret_cast<uint8_t*>(variable.size()); \
    }}

#define TVUNIONEND \
    return nullptr; \
} void setType(uint8_t t) { \
    type = t; \
} void generate(ptree::protocol::BufferView& data) { \
    if (data.start+sizeof(uint8_t)>data.limit) \
        return; \
    *((uint8_t*)(data.start)) = type; \
    data.start += sizeof(uint8_t); \
    multiDoFuntion(data, nullptr, nullptr, 0); \
} inline uint8_t* parse(uint8_t* start, uint8_t* limit) { \
    ptree::protocol::BufferView nuller; \
    type = *start; \
    start += sizeof(uint8_t); \
    return multiDoFuntion(nuller, start, limit, 1); \
} uint8_t type; uint32_t size() { \
    ptree::protocol::BufferView nuller; \
    return (uintptr_t)multiDoFuntion(nuller, nullptr, nullptr, 2) + sizeof(uint8_t); \
}

#define TVENCODERBEGIN \
inline void operator >> (Encoder& en) { \
    if (en.encodeCursor.start + sizeof(uint8_t) > en.encodeCursor.limit) return; \
    *(en.encodeCursor.start) = type; \
    en.encodeCursor.start += sizeof(uint8_t);

#define TVENCODEREND }

#define TVDECODERBEGIN \
inline void operator << (Decoder& de) { \
    if (de.start + sizeof(uint8_t) > de.limit) return; \
    type = *de.start; \
    de.start += sizeof(uint8_t);

#define TVDECODEREND }

#define ENCODE_STEP(ntype, variable) \
if (type == ntype) en.translate(variable);

#define DECODE_STEP(ntype, variable) \
if (type == ntype) de.translate(variable);
// END THUG

} // namespace protocol
} // namespace ptree

#endif  // SERVER_MESSAGING_MESSAGEESSENTIAL_HPP_
