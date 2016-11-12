#ifndef SERVER_MESSAGE_HELPER_MESSAGE_METAUPDATENOTIFICATION_SENDER_HPP_
#define SERVER_MESSAGE_HELPER_MESSAGE_METAUPDATENOTIFICATION_SENDER_HPP_

#include "MessageSender.hpp"

namespace ptree
{
namespace server
{

/** ---------------------------------------------------------   MessageMetaUpdateNotificationSender
*
*   HEADER | ( CREATE_OBJECT |     UUID     | PROPERTY_TYPE | PATH | NULL )...
*                (uint8_t)      (uint32_t)      (uint8_t)      []     0
*          , ( DELETE_OBJECT |    UUID     )...
*                (uint8_t)       (uint32_t)
*/

class MessageMetaUpdateNotificationSender : public MessageSender
{
public:
    MessageMetaUpdateNotificationSender(const ClientServer::UuidActionTypeAndPathMap&
        uuidActionTypeAndPathMap, IEndPointPtr endpoint):
        MessageSender((uint32_t)-1, endpoint),
        uuidActionTypeAndPathMap(uuidActionTypeAndPathMap)
    {}

    void send();

private:
    uint32_t getMessageSize();
    const ClientServer::UuidActionTypeAndPathMap& uuidActionTypeAndPathMap;
};


} // namespace server
} // namespace ptree

#endif  // SERVER_MESSAGE_HELPER_MESSAGE_METAUPDATENOTIFICATION_SENDER_HPP_