#ifndef INTERFACE_MESSAGECODEC_HPP_
#define INTERFACE_MESSAGECODEC_HPP_

#include <vector>
#include <cstring>
#include <exception>
#include <iostream>
#include <sstream>
#include "MessageCoDecHelpers.hpp"

namespace ptree
{
namespace protocol
{

class SizeReader
{
public:
    SizeReader():mCurrentSize(0) {}

    void translate()
    {
    }

    template <typename T, typename... Tt>
    void translate(T& head, Tt&... tail)
    {
        sizeRead(head, mCurrentSize);
        translate(tail...);
    }

    uint32_t size()
    {
        return mCurrentSize;
    }
private:
    uint32_t mCurrentSize;
};

class Encoder
{
public:
    Encoder(BufferView& codedData):
        mEncodeCursor(codedData)
    {}

    void translate()
    {
    }

    template <typename T, typename... Tt>
    void translate(T& head, Tt&... tail)
    {
        encode(head, mEncodeCursor);
        translate(tail...);
    }

private:
    BufferView& mEncodeCursor;
};

class Decoder
{
public:
    Decoder(BufferView& bv):
        mDecodeCursor(bv)
    {}

    void translate()
    {
    }

    template <typename T, typename... Tt>
    void translate(T& head, Tt&... tail)
    {
        decode(head, mDecodeCursor);
        translate(tail...);
    }
private:
    BufferView& mDecodeCursor;
};

class FieldPrinter
{
public:
    FieldPrinter(std::stringstream& outputStringStream):
        mCurrentNameIndex(0),
        mOutputStringStream(outputStringStream)
    {}

    void registerFieldNames(const char * names)
    {
        for (size_t cur = 0; true; cur++)
        {
            if (names[cur]==',')
            {
                mNames.emplace_back(names, cur);
                names = &names[cur+1];
                cur = 0;
            }
            if (!names[cur])
            {
                mNames.emplace_back(names, cur);
                break;
            }
        }
    }

    void translate()
    {
    }

    template <typename T, typename... Tt>
    void translate(T& head, Tt&... tail)
    {
        printField(head, mNames[mCurrentNameIndex], mOutputStringStream);
        mOutputStringStream << ", ";
        ++mCurrentNameIndex;
        translate(tail...);
    }
private:
    std::vector<std::string> mNames;
    size_t mCurrentNameIndex;
    std::stringstream& mOutputStringStream;
};

// THUG CPP
#define MESSAGE_FIELDS(...)\
template<typename T>\
void serialize(T& codec)\
{\
    codec.translate(__VA_ARGS__);\
}\
void generate(BufferView& data)\
{\
    Encoder en(data);\
    this->serialize(en);\
}\
void parse(BufferView& data)\
{\
    Decoder de(data);\
    this->serialize(de);\
}\
uint32_t size()\
{\
    SizeReader sr;\
    sr.translate(__VA_ARGS__);\
    return sr.size();\
}\
std::vector<uint8_t> getPacked()\
{\
    Buffer e(this->size());\
    BufferView v(e);\
    Encoder en(v);\
    this->serialize(en);\
    return e;\
}\
bool unpackFrom(std::vector<uint8_t>& message)\
{\
    BufferView bv(message);\
    Decoder de(bv);\
    this->serialize(de);\
    return true;\
}\
void print(std::stringstream& output)\
{\
    FieldPrinter fp(output);\
    fp.registerFieldNames(#__VA_ARGS__);\
    fp.translate(__VA_ARGS__);\
}\
std::string toString()\
{\
    std::stringstream ss;\
    print(ss);\
    return ss.str();\
}


// END THUG

} // namespace protocol
} // namespace ptree

#endif  // INTERFACE_MESSAGECODEC_HPP_