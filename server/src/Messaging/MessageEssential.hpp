#ifndef SERVER_MESSAGING_MESSAGEESSENTIAL_HPP_
#define SERVER_MESSAGING_MESSAGEESSENTIAL_HPP_

#include <vector>
#include <cstring>  

namespace ptree
{
namespace server
{

using Buffer = std::vector<uint8_t>;

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

    inline Buffer generate()
    {
        // std::cout << __PRETTY_FUNCTION__ << std::endl;
        Buffer x(sizeof(uint32_t));
        *(uint32_t*)(x.data()) = values.size();

        for (auto& i : values)
        {
            auto v = i.generate();
            x.insert(x.end(), v.begin(), v.end());
        }

        return x;
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

private:
    std::vector<T> values;
};

class String
{
public:
    String(){}

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

    std::string& operator *()
    {
        return value;
    }

    operator std::string& ()
    {
        return value;
    }

    inline Buffer generate()
    {
        // std::cout << __PRETTY_FUNCTION__ << std::endl;
        Buffer x(value.size()+1);
        std::memcpy(x.data(), value.c_str(), value.size()+1);
        return x;
    }

    inline uint8_t* parse(uint8_t *start, uint8_t *limit)
    {
        uint8_t *nullf=start;
        while (nullf<limit && *(nullf++));
        value = std::string((char*)start, nullf-start);
        return nullf;
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

    inline Buffer generate()
    {
        // std::cout << __PRETTY_FUNCTION__ << std::endl;
        Buffer x(sizeof(uint32_t)+value.size());
        *(uint32_t*)(x.data()) = value.size();
        std::memcpy(x.data()+sizeof(uint32_t), value.data(), value.size());
        return x;
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
        return start + size;
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
        value = t;
        return value;
    }

    operator T&()
    {
        return value;
    }

    inline Buffer generate()
    {
        // std::cout << __PRETTY_FUNCTION__ << std::endl;
        Buffer x(sizeof(T));
        *(T*)(x.data()) = value;
        return x;
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

private:
    T value;
};

class Encoder
{
public:
    Encoder(){}
    template <typename T, typename... Tt>
    void translate(T& head)
    {
        // std::cout << __PRETTY_FUNCTION__ << std::endl;
        auto v = head.generate();
        // std::cout << "adding:" << std::endl;
        // for (const auto& i : v)
        // {
        //     std::cout << (int)i << " ";
        // }
        // std::cout << "\n\n";

        codedData.insert(codedData.end(), v.begin(), v.end());
    }

    template <typename T, typename... Tt>
    void translate(T& head, Tt&... tail)
    {
        // std::cout << __PRETTY_FUNCTION__ << std::endl;
        translate(head);
        translate(tail...);
    }

    Buffer& data()
    {
        return codedData;
    }

private:
    Buffer codedData;
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
private:
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
inline Buffer generate()\
{\
    Encoder en;\
    *this >> en;\
    return en.data();\
}\
inline uint8_t* parse(uint8_t* start, uint8_t* limit)\
{\
    Decoder de(start, limit);\
    *this << de;\
    return de.getNext();\
}
// END THUG

} // namespace server
} // namespace ptree

#endif  // SERVER_MESSAGING_MESSAGEESSENTIAL_HPP_