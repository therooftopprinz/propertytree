#ifndef SERVER_MESSAGE_HELPER_MESSAGE_SETVALUEINDICATION_CREATOR_HPP_
#define SERVER_MESSAGE_HELPER_MESSAGE_SETVALUEINDICATION_CREATOR_HPP_

#include "MessageCreator.hpp"

namespace ptree
{
namespace server
{

/** --------------------------------------------------------------- MessageSetValueIndicationCreator
*   HEADER  |    UUID    |   []   | NULL  
*             (uint32_t)    DATA     0
*/

class MessageSetValueIndicationCreator : public MessageCreator
{
public:
    MessageSetValueIndicationCreator(uint32_t transactionId):
        MessageCreator(transactionId)
    {}

    Buffer create();
    void setUuid(uint32_t uuid);
    void setValue(core::ValueContainer value);
    
private:
    uint32_t uuid = 0;
    core::ValueContainer value;
};


} // namespace server
} // namespace ptree

#endif  // SERVER_MESSAGE_HELPER_MESSAGE_SETVALUEINDICATION_CREATOR_HPP_