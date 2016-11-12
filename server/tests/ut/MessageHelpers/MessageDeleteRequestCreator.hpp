#ifndef SERVER_MESSAGE_HELPER_MESSAGE_DELETERQUEST_CREATOR_HPP_
#define SERVER_MESSAGE_HELPER_MESSAGE_DELETERQUEST_CREATOR_HPP_

#include "MessageCreator.hpp"

namespace ptree
{
namespace server
{

/** -------------------------------------------------------------------- MessageDeleteRequestCreator
*
*   HEADER  | PATH | NULL
*              []     0
*/

class MessageDeleteRequestCreator : public MessageCreator
{
public:
    MessageDeleteRequestCreator(uint32_t transactionId):
        MessageCreator(transactionId)
    {}

    Buffer create();
    void setPath(std::string path);
    
private:
    std::string path;
};

} // namespace server
} // namespace ptree

#endif  // SERVER_MESSAGE_HELPER_MESSAGE_DELETERQUEST_CREATOR_HPP_