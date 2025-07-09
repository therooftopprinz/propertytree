#ifndef __VALUE_CLIENT_HPP__
#define __VALUE_CLIENT_HPP__

#include <condition_variable>
#include <unordered_map>
#include <functional>
#include <stdexcept>
#include <atomic>

#include <netinet/tcp.h>

#include <bfc/configuration_parser.hpp>
#include <bfc/epoll_reactor.hpp>
#include <bfc/socket.hpp>
#include <bfc/socket.hpp>

#include <logless/logger.hpp>

#include <propertytree/protocol_ng.hpp>
#include <propertytree/logger.hpp>

namespace propertytree
{

using reactor_t = bfc::epoll_reactor<std::function<void()>>;
using buffer = std::vector<std::byte>;

class value_client;

class value
{
public:
    value(uint64_t id, value_client& client);

    using cb_t = std::function<void(const buffer&)>;

    template <typename T> std::optional<T> as()
    {
        if (size() != sizeof(T))
        {
            return {};
        }

        T rv;
        std::memcpy(&rv, data.data(), size());
        return rv;
    }

    buffer raw();

    template <typename T>
    value& operator=(T&& t)
    {
        if (sizeof(T) != size())
        {
            data.resize(sizeof(T));
        }

        std::memcpy(data.data(), &t, sizeof(T));
        commit();
        return *this;
    }

    template <typename T>
    value& fast_set(T&& t)
    {
        if (sizeof(T) != size())
        {
            data.resize(sizeof(T));
        }

        std::memcpy(data.data(), &t, sizeof(T));
        commit(false);
        return *this;
    }


    value& operator=(buffer);

    size_t size();

    void fetch();

    unsigned subscribe(cb_t);
    void unsubscribe(unsigned);

private:
    void commit(bool=true);

    friend class value_client;

    std::mutex mutex;
    uint64_t id;
    buffer data;
    std::atomic<unsigned> m_watchers_id;
    std::map<unsigned, cb_t> watchers;
    value_client& client;
};

using value_ptr = std::shared_ptr<value>;

class value_client
{
public:
    struct config_s
    {
        std::string ip = "127.0.0.1";
        uint16_t port = 15000;
        std::string log = "value_client.blog";
        bool no_delay = false;
        bool logful = true;
    };

    value_client(const config_s&, reactor_t& reactor);
    value_ptr get(uint64_t);

    logless::logger& get_logger();

private:
    static constexpr size_t ENCODE_SIZE = 1024*64;
    friend class value;
    enum read_state_e {WAIT_HEADER, WAIT_REMAINING};
    struct transaction_s
    {
        uint16_t id = 0xFF;
        bool satisfied = false;
        cum::protocol_value_client message;
        std::condition_variable cv;
        std::mutex mutex;
    };

    std::shared_ptr<transaction_s> create_transaction();
    bool wait_transaction(transaction_s&);

    void disconnect();

    template <typename T>
    void handle_transactional(T&& rsp)
    {
        std::unique_lock lg(m_transaction_map_mutex);
        auto it = m_transaction_map.find(rsp.transaction_id);
        if (m_transaction_map.end() == it)
        {
            PTLOG_ERR("value_client | fd=%3d; | protocol_error=unknown_response trid=%u;", m_server_socket.fd(), rsp.transaction_id);
            disconnect();
            return;
        }
        auto transaction = it->second;
        lg.unlock();

        std::unique_lock lg2(transaction->mutex);
        transaction->message = std::move(rsp);
        transaction->satisfied = true;
        lg2.unlock();
        transaction->cv.notify_all();
    }

    void handle(cum::acknowledge&&);
    void handle(cum::get_value_response&&);
    void handle(cum::update&&);

    void read_server();

    size_t encode(const cum::protocol_value_server& msg, std::byte* data, size_t size);

    void set_value(uint64_t, const buffer&);
    void set_value_nowait(uint64_t, const buffer&);
    buffer get_value(uint64_t);
    void subscribe(uint64_t);
    void unsubscribe(uint64_t);

    logless::logger m_logger;
    reactor_t& m_reactor;
    bfc::socket m_server_socket;
    reactor_t::context m_server_socket_ctx;

    std::byte m_read_buffer[512];
    read_state_e m_read_state = WAIT_HEADER;
    size_t m_expected_read_size = 2;
    size_t m_read_buffer_idx = 0;

    std::mutex m_value_map_mutex;
    std::unordered_map<uint64_t, value_ptr> m_value_map;

    std::mutex m_transaction_map_mutex;
    std::uint16_t m_transaction_id = 0;
    std::unordered_map<uint16_t, std::shared_ptr<transaction_s>> m_transaction_map;
};

} // propertytree

#endif // __VALUE_CLIENT_HPP__
