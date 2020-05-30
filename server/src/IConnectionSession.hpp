#ifndef __ICONNECTION_SESSION_HPP__
#define __ICONNECTION_SESSION_HPP__

#include <memory>

#include <bfc/Buffer.hpp>

#include <interface/protocol.hpp>


namespace propertytree
{

struct IConnectionSession
{
    virtual ~IConnectionSession() {}
    virtual void send(const bfc::ConstBufferView&) = 0;
    
};

} // propertytree

#endif // __ICONNECTION_SESSION_HPP__
