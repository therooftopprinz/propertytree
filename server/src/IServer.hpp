#ifndef __ISERVER_HPP__
#define __ISERVER_HPP__

namespace propertytree
{

struct IServer
{
    virtual void onDisconnect(int pFd) = 0; 
};

} // propertytree

#endif // __ISERVER_HPP__
