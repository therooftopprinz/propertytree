#ifndef __VALUE_CLIENT_HPP__
#define __VALUE_CLIENT_HPP__

#include <condition_variable>
#include <unordered_map>
#include <functional>
#include <stdexcept>
#include <atomic>

#include <bfc/configuration_parser.hpp>
#include <bfc/epoll_reactor.hpp>
#include <bfc/socket.hpp>
#include <bfc/socket.hpp>

#include <logless/logger.hpp>
#include <propertytree/protocol_ng.hpp>
#include <propertytree/protocol_udp.hpp>

namespace propertytree
{

using reactor_t = bfc::epoll_reactor<std::function<void()>>;
using buffer = std::vector<std::byte>;

class value_client;

class value
{
public:
    value(uint64_t id, value_client& client);

    using cb_t = std::function<void(const bfc::const_buffer_view&)>;

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

    value& operator=(buffer);

    size_t size();
    void fetch();
    unsigned subscribe(cb_t);
    void unsubscribe(unsigned);
    unsigned stream_subscribe(cb_t);
    void stream_unsubscribe(unsigned);

private:
    void commit();

    friend class value_client;

    std::mutex mutex;
    uint64_t id;
    buffer data;
    std::atomic<unsigned> m_watchers_id;
    std::map<unsigned, cb_t> watchers;
    std::map<unsigned, cb_t> stream_watchers;
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
        bool logful = true;
    };

    value_client(const config_s*, reactor_t* reactor);
    value_ptr get(uint64_t);

    logless::logger& get_logger();

private:
    static constexpr size_t ENCODE_SIZE = 1024*16;
    friend class value;

    // tcp
    enum tcp_read_state_e {WAIT_HEADER, WAIT_REMAINING};
    void tcp_read_server();
    void tcp_disconnect();
    void tcp_handle(cum::update&&);
    void tcp_handle(cum::acknowledge&&);
    size_t encode(const cum::protocol_value_server& msg, std::byte* data, size_t size);

    // udp
    void udp_read_server();
    void udp_handle_ack         (const header_s&, const acknowledge_s&);
    void udp_handle_value       (const header_s&, const key_sn_s&, const bfc::const_buffer_view&);
    void udp_handle             (const bfc::const_buffer_view&&);

    // api
    void stream(uint64_t, const buffer&);
    void set_value(uint64_t, const buffer&);
    buffer get_value(uint64_t);
    void subscribe(uint64_t);
    void unsubscribe(uint64_t);
    void stream_subscribe(uint64_t);
    void stream_unsubscribe(uint64_t);

    std::mutex m_value_map_mutex;
    std::unordered_map<uint64_t, value_ptr> m_value_map;
    logless::logger m_logger;
    reactor_t* m_reactor;

    // tcp
    struct tcp_transaction_s
    {
        cum::protocol_value_client rsp;
        bool satisfied = false;
        std::condition_variable cv;
        std::mutex mutex;
    };

    bfc::socket m_tcp_server_socket;
    reactor_t::context m_tcp_server_socket_ctx;
    std::byte m_tcp_read_buffer[1024*16];
    tcp_read_state_e m_tcp_read_state = WAIT_HEADER;
    size_t m_tcp_expected_read_size = 2;
    size_t m_tcp_read_buffer_idx = 0;
    std::atomic_uint16_t m_tcp_transaction_id = 0;
    std::mutex m_tcp_transaction_map_mutex;
    std::unordered_map<uint16_t, std::shared_ptr<tcp_transaction_s>> m_tcp_transaction_map;

    // udp
    struct udp_transaction_s
    {
        std::byte snd_msg[ENCODE_SIZE];
        std::byte rcv_msg[ENCODE_SIZE];
        bool satisfied = false;
        std::condition_variable cv;
        std::mutex mutex;
    };

    sockaddr_in m_udp_server_addr;
    bfc::socket m_udp_server_socket;
    reactor_t::context m_udp_server_socket_ctx;
    std::byte m_udp_read_buffer[1024*16];
    std::atomic_uint16_t m_udp_transaction_id = 0;
    std::mutex m_udp_transaction_map_mutex;
    std::unordered_map<uint16_t, std::shared_ptr<udp_transaction_s>> m_udp_transaction_map;
};

} // propertytree

#endif // __VALUE_CLIENT_HPP__
