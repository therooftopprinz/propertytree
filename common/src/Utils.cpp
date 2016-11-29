#include <sstream>
#include <cctype>
#include "Utils.hpp"
#include <common/src/Logger.hpp>
namespace ptree
{
namespace utils
{

void printRaw(const void *begin, uint32_t size)
{
    const static logger::Logger log("PrintRaw");
    std::ostringstream a;
    std::ostringstream b;
    uint8_t *x = (uint8_t*) begin;
    for(uint32_t i = 0; i < size; i++)
    {
        if (std::isprint((int)x[i]))
        {
            a << (char) x[i];
        }
        else
        {
            a << ".";
        }

        b << std::hex <<  (uint32_t)x[i] << " ";
    }

    log << logger::DEBUG << "Buffer(" << size << "): " << b.str();
    log << logger::DEBUG << "BufTxt(" << size << "): " << a.str();
}

void printRawAscii(const void *begin, uint32_t size)
{
    const static logger::Logger log("PrintRaw");
    std::ostringstream a;
    uint8_t *x = (uint8_t*) begin;
    for(uint32_t i = 0; i < size; i++)
    {
        if (std::isprint((int)x[i]))
        {
            a << (char) x[i];
        }
        else
        {
            a << ".";
        }

    }

    log << logger::DEBUG << "Buffer(" << size << "): " << a.str();
}


std::pair<std::string, std::string> getParentAndChildNames(std::string path)
{
    const static  logger::Logger log("getParentAndChildNames");
    std::string::iterator it = path.end();

    while (it != path.begin() && *it != '/')
    {
        it--;
    }

    std::string parent(path.begin(), it);
    std::string children(it+1, path.end());

    parent = parent == "" ? "/" : parent;
    log << logger::DEBUG << parent << " " << children;

    return std::make_pair(parent, children);
}

} // namespace utils
} // namespace ptree 