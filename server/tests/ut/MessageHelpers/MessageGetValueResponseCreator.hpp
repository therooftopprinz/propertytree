#ifndef SERVER_MESSAGE_HELPER_MESSAGE_GETVALUERESPONSE_CREATOR_HPP_
#define SERVER_MESSAGE_HELPER_MESSAGE_GETVALUERESPONSE_CREATOR_HPP_

#include "MessageCreator.hpp"

namespace ptree
{
namespace server
{

/** ------------------------------------------------------------------- MessageGetValueResponseCreator
*
*   HEADER  | DATA
*              []
*/

class MessageGetValueResponseCreator : MessageCreator
{
public:
    MessageGetValueResponseCreator(uint32_t transactionId):
        MessageCreator(transactionId)
    {}

    Buffer create();
    void setValue(BufferPtr value);
    
private:
    BufferPtr value;
};

} // namespace server
} // namespace ptree

#endif  // SERVER_MESSAGE_HELPER_MESSAGE_GETVALUERESPONSE_CREATOR_HPP_