#ifndef __VALUE_CLIENT_HPP__
#define __VALUE_CLIENT_HPP__

#include <condition_variable>
#include <unordered_map>
#include <functional>
#include <stdexcept>
#include <atomic>

#include <bfc/epoll_reactor.hpp>
#include <bfc/configuration_parser.hpp>
#include <bfc/socket.hpp>
#include <bfc/socket.hpp>
#include <protocol_ng.hpp>

namespace propertytree
{

using reactor_t = bfc::epoll_reactor<std::function<void()>>;
using buffer = std::vector<uint8_t>;

class value_client;

class value
{
public:
    value(uint64_t id, value_client& client);

    using cb_t = std::function<void(const buffer&)>;

    template <typename T> std::optional<T> as();
    buffer raw();

    template <typename T> value& operator=(T&);
    value& operator=(buffer);

    size_t size();

    void fetch();

    unsigned subscribe(cb_t);
    void unsubscribe(unsigned);

private:
    void commit();

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
        std::string ip;
        uint16_t port;
    };

    value_client(const config_s&, reactor_t& reactor);
    value_ptr get(uint64_t);
private:
    static constexpr size_t ENCODE_SIZE = 1024*64;
    friend class value;
    enum read_state_e {WAIT_HEADER, WAIT_REMAINING};

    void disconnect();
    void handle(cum::get_value_response&&);
    void handle(cum::update&&);

    void read_server();

    size_t encode(const cum::protocol_value_server& msg, std::byte* data, size_t size);

    void set_value(uint64_t, const buffer&);
    buffer get_value(uint64_t);
    void subscribe(uint64_t);
    void unsubscribe(uint64_t);

    reactor_t& m_reactor;
    bfc::socket m_server_socket;
    reactor_t::context m_server_socket_ctx;

    std::byte m_read_buffer[512];
    read_state_e m_read_state = WAIT_HEADER;
    size_t m_expected_read_size = 2;
    size_t m_read_buffer_idx = 0;

    std::mutex m_value_map_mutex;
    std::unordered_map<uint64_t, value_ptr> m_value_map;

    struct transaction_s
    {
        bool satisfied = false;
        cum::protocol_value_client message;
        std::condition_variable cv;
        std::mutex mutex;
    };

    std::atomic_uint64_t m_transaction_id = 0;
    std::mutex m_transaction_map_mutex;
    std::unordered_map<uint64_t, std::shared_ptr<transaction_s>> m_transaction_map;
};

} // propertytree

#endif // __VALUE_CLIENT_HPP__
