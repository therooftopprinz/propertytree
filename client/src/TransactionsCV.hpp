#ifndef CLIENT_TRANSACTIONCV_HPP_
#define CLIENT_TRANSACTIONCV_HPP_

#include <common/src/Logger.hpp>
#include "Types.hpp"

namespace ptree
{
namespace client
{

class TransactionsCV;
class TransactionCV
{
public:
    TransactionCV():
        condition(false)
    {}
    Buffer& getBuffer()
    {
        return value;
    }
private:
    std::mutex mutex;
    std::condition_variable cv;
    std::atomic<bool> condition;
    Buffer value;
    friend TransactionsCV;
};

class TransactionsCV
{
public:
    TransactionsCV();
    ~TransactionsCV() = default;

    std::shared_ptr<TransactionCV> addTransactionCV(uint32_t transactionId);
    bool waitTransactionCV(uint32_t transactionId);
    std::shared_ptr<TransactionCV> getTransactionCV(uint32_t);
    void notifyTransactionCV(uint32_t transactionId, Buffer& value);
private:
    typedef std::map<uint32_t, std::shared_ptr<TransactionCV>> TrCVMap;
    MutexedObject<TrCVMap> transactionIdCV;
    logger::Logger log;
};


}
}
#endif  // CLIENT_TRANSACTIONCV_HPP_