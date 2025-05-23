#include "live_feed_coinbase.h"
#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <boost/beast/ssl.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/beast/websocket/ssl.hpp>
#include <iostream>
#include <chrono>
#include <thread>

using tcp = boost::asio::ip::tcp;
namespace ssl = boost::asio::ssl;
namespace websocket = boost::beast::websocket;

LiveFeedCoinbase::LiveFeedCoinbase(const std::string &symbol) : symbol_(symbol) {}

LiveFeedCoinbase::~LiveFeedCoinbase() { stop(); }

void LiveFeedCoinbase::start(std::function<void(const MarketTick &)> on_tick)
{
    if (running_)
        return;
    running_ = true;
    on_tick_ = on_tick;
    thread_ = std::thread(&LiveFeedCoinbase::run, this);
}

void LiveFeedCoinbase::stop()
{
    if (!running_)
        return;
    running_ = false;
    if (thread_.joinable())
        thread_.join();
}

static std::string build_subscribe_message(const std::string &product_id)
{
    // Coinbase Advanced public trades stream subscription (text protocol)
    // {"type":"subscribe","channel":"market_trades","product_ids":["BTC-USD"]}
    std::ostringstream oss;
    oss << "{\"type\":\"subscribe\",\"channel\":\"market_trades\",\"product_ids\":[\"" << product_id << "\"]}";
    return oss.str();
}

void LiveFeedCoinbase::run()
{
    try
    {
        boost::asio::io_context ioc;
        ssl::context ctx(ssl::context::tlsv12_client);
        tcp::resolver resolver(ioc);
        auto const host = std::string("advanced-trade-ws.coinbase.com");
        auto const port = std::string("443");
        auto const results = resolver.resolve(host, port);
        boost::beast::ssl_stream<boost::beast::tcp_stream> tls_stream(ioc, ctx);
        boost::beast::get_lowest_layer(tls_stream).connect(results);
        if (!SSL_set_tlsext_host_name(tls_stream.native_handle(), host.c_str()))
        {
            return;
        }
        tls_stream.handshake(ssl::stream_base::client);
        websocket::stream<boost::beast::ssl_stream<boost::beast::tcp_stream>> ws(std::move(tls_stream));
        ws.set_option(websocket::stream_base::decorator([](websocket::request_type &req)
                                                        { req.set(boost::beast::http::field::user_agent, std::string("TradePulse/1.0")); }));
        ws.handshake(host, "/");

        auto sub = build_subscribe_message(symbol_);
        ws.write(boost::asio::buffer(sub));

        boost::beast::flat_buffer buffer;
        while (running_)
        {
            buffer.clear();
            ws.read(buffer);
            auto msg = boost::beast::buffers_to_string(buffer.data());
            auto now_ms = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
            if (msg.find("\"channel\":\"market_trades\"") == std::string::npos)
                continue;
            size_t pos = 0;
            int emitted = 0;
            while (true)
            {
                size_t ppos = msg.find("\"price\":\"", pos);
                if (ppos == std::string::npos)
                    break;
                size_t pend = msg.find('"', ppos + 9);
                if (pend == std::string::npos)
                    break;
                std::string price_str = msg.substr(ppos + 9, pend - (ppos + 9));
                size_t spos = msg.find("\"size\":\"", pend);
                if (spos == std::string::npos)
                {
                    pos = pend + 1;
                    continue;
                }
                size_t send = msg.find('"', spos + 8);
                if (send == std::string::npos)
                {
                    pos = spos + 1;
                    continue;
                }
                std::string size_str = msg.substr(spos + 8, send - (spos + 8));
                // try to extract exchange nanosecond timestamp and convert to ms
                int64_t exch_ms = -1;
                size_t tpos = msg.find("\"time\":\"", send);
                if (tpos != std::string::npos)
                {
                    size_t tend = msg.find('"', tpos + 8);
                    if (tend != std::string::npos)
                    {
                        std::string tstr = msg.substr(tpos + 8, tend - (tpos + 8));
                        if (tstr.size() >= 13)
                        {
                            exch_ms = std::atoll(tstr.substr(0, 13).c_str());
                        }
                    }
                }
                double price = std::atof(price_str.c_str());
                double size = std::atof(size_str.c_str());
                if (price > 0.0)
                {
                    MarketTick tick{};
                    tick.venue = "COINBASE";
                    tick.symbol = symbol_;
                    tick.price = price;
                    tick.size = size;
                    tick.exchange_recv_ts_ms = exch_ms;
                    tick.ingest_ts_ms = now_ms;
                    if (on_tick_)
                        on_tick_(tick);
                    emitted++;
                }
                pos = send + 1;
                if (emitted >= 10)
                    break;
            }
        }
        ws.close(websocket::close_code::normal);
    }
    catch (...)
    {
    }
}
