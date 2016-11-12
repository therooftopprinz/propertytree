#include <iostream>
#include <cctype>
#include "MessageEssential.hpp"

// int main()
// {
//     CreateRequest cr;

//     cr.header.size = 42;
//     cr.header.type.x = 'D';
//     cr.path = std::string("reqt!");

//     Encoder en;
//     cr >> en;

//     Buffer endata = en.data();

//     CreateRequest newcr;
//     Decoder de(endata.data(), endata.data()+endata.size());
//     newcr << de;

//     std::cout << "ncr.header.size " << cr.header.size << std::endl;
//     std::cout << "ncr.header.type " << cr.header.type.x << std::endl;
//     std::cout << "ncr.path " << (std::string)cr.path << std::endl;

//     return 0;
// }
