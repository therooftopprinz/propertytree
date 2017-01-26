#include "TransactionsCV.hpp"

namespace ptree
{
namespace client
{
TransactionsCV::TransactionsCV():
    log("TransactionsCV")
{}

std::shared_ptr<TransactionCV> TransactionsCV::addTransactionCV(uint32_t transactionId)
{
    std::lock_guard<std::mutex> guard(transactionIdCV.mutex);
    // TODO: emplace
    transactionIdCV.object[transactionId] = std::make_shared<TransactionCV>();
    return transactionIdCV.object[transactionId];
}

void TransactionsCV::notifyTransactionCV(uint32_t transactionId, Buffer& value)
{
    std::shared_ptr<TransactionCV> tcv;
    {
        std::lock_guard<std::mutex> guard(transactionIdCV.mutex);
        auto it = transactionIdCV.object.find(transactionId);
        if (it == transactionIdCV.object.end())
        {
            log << logger::ERROR << "transactionId not found in CV list.";
            return;
        }
        tcv = it->second;
    }

    log << logger::ERROR << "unlocking cv for tid=" << transactionId;
    tcv->condition = true;

    {
        std::lock_guard<std::mutex> guard(tcv->mutex);
        tcv->value = std::move(value);
        tcv->cv.notify_all();
    }

    {
        std::lock_guard<std::mutex> guard(transactionIdCV.mutex);
        auto it = transactionIdCV.object.find(transactionId);
        if (it == transactionIdCV.object.end())
        {
            log << logger::ERROR << "transactionId not found in CV list.";
            return;
        }
        transactionIdCV.object.erase(it);
    }
}

bool TransactionsCV::waitTransactionCV(uint32_t transactionId)
{
    std::shared_ptr<TransactionCV> tcv;
    {
        std::lock_guard<std::mutex> guard(transactionIdCV.mutex);
        auto it = transactionIdCV.object.find(transactionId);
        if (it == transactionIdCV.object.end())
        {
            log << logger::ERROR << "transactionId not found in CV list.";
            return false;
        }
        tcv = it->second;
    }

    {
        std::unique_lock<std::mutex> guard(tcv->mutex);
        using namespace std::chrono_literals;
        tcv->cv.wait_for(guard, 1s,[&tcv](){return bool(tcv->condition);});
        return tcv->condition;
    }
}
}
}
