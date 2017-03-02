#include "TransactionsCV.hpp"
#include <common/src/Utils.hpp>

namespace ptree
{
namespace client
{
TransactionsCV::TransactionsCV():
    log("TransactionsCV")
{}

TransactionsCV::~TransactionsCV()
{
    log << logger::DEBUG << "~TransactionsCV";
}

std::shared_ptr<TransactionCV> TransactionsCV::addTransactionCV(uint32_t transactionId)
{
    log << logger::DEBUG << "addTransactionCV: adding: " << transactionId;
    std::lock_guard<std::mutex> guard(transactionIdCV.mutex);
    transactionIdCV.object.emplace(transactionId, std::make_shared<TransactionCV>());
    return transactionIdCV.object[transactionId];
}

void TransactionsCV::notifyTransactionCV(uint32_t transactionId, Buffer& value)
{
    log << logger::DEBUG << "notifyTransactionCV for tid=" << transactionId;
    utils::printRaw(value.data(), value.size());
    std::shared_ptr<TransactionCV> tcv;
    {
        std::lock_guard<std::mutex> guard(transactionIdCV.mutex);
        auto it = transactionIdCV.object.find(transactionId);
        if (it == transactionIdCV.object.end())
        {
            log << logger::ERROR << "notifyTransactionCV: trigger: transactionId not found in CV list, tid=" << transactionId;
            return;
        }
        tcv = it->second;
    }

    {
        std::lock_guard<std::mutex> guard(tcv->mutex);
        tcv->value = std::move(value);
        tcv->condition = true;
        log << logger::DEBUG << "unlocking cv for tid=" << transactionId;
        utils::printRaw(tcv->value.data(), tcv->value.size());
        tcv->cv.notify_all();
    }
}

bool TransactionsCV::waitTransactionCV(uint32_t transactionId)
{
    log << logger::DEBUG << "waitTransactionCV for: " << transactionId;
    std::shared_ptr<TransactionCV> tcv;
    {
        std::lock_guard<std::mutex> guard(transactionIdCV.mutex);
        auto it = transactionIdCV.object.find(transactionId);
        if (it == transactionIdCV.object.end())
        {
            log << logger::ERROR << "waitTransactionCV: wait: transactionId not found in CV list, tid=" << transactionId;
            return false;
        }
        tcv = it->second;
    }

    {
        std::unique_lock<std::mutex> guard(tcv->mutex);
        using namespace std::chrono_literals;
        tcv->cv.wait_for(guard, 1s,[&tcv](){return bool(tcv->condition);});
    }

    log << logger::DEBUG << "waiting complete for tid=" << transactionId;
    utils::printRaw(tcv->value.data(), tcv->value.size());

    {
        std::lock_guard<std::mutex> guard(transactionIdCV.mutex);
        log << logger::DEBUG << "waitTransactionCV: erase: tcv";
        auto it = transactionIdCV.object.find(transactionId);
        transactionIdCV.object.erase(it);
    }
    return tcv->condition;
}
}
}
