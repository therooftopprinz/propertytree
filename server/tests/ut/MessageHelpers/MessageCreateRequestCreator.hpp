#ifndef SERVER_MESSAGE_HELPER_MESSAGE_CREATEREQUEST_CREATOR_HPP_
#define SERVER_MESSAGE_HELPER_MESSAGE_CREATEREQUEST_CREATOR_HPP_

#include "MessageCreator.hpp"

namespace ptree
{
namespace server
{

/** ------------------------------------------------------------------   MessageCreateRequestCreator
*
*    HEADER | VALUE_SIZE | VALUE_TYPE | VALUE | PATH | NULL
*             (uint32_t)    (uint8_t)     []     []     0
*/

class MessageCreateRequestCreator : public MessageCreator
{
public:
    MessageCreateRequestCreator(uint32_t transactionId):
        MessageCreator(transactionId)
    {}

    Buffer create();
    void setPath(const std::string& path);
    void setValue(BufferPtr bufferptr);
    void setType(protocol::PropertyType type);
    
private:
    std::string path;
    BufferPtr value;
    protocol::PropertyType type;
};

} // namespace server
} // namespace ptree

#endif  // SERVER_MESSAGE_HELPER_MESSAGE_CREATEREQUEST_CREATOR_HPP_