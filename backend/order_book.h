#pragma once

#include <string>
#include <queue>
#include <map>
#include <chrono>
#include <functional>
#include <vector>

enum class OrderSide
{
    BUY,
    SELL
};

struct Order
{
    std::string id;
    std::string venue;
    std::string symbol;
    OrderSide side;
    double price;
    int quantity;
    std::chrono::system_clock::time_point timestamp;
    int64_t exchange_recv_ts_ms{-1};
    int64_t ingest_ts_ms{-1};
};

struct Trade
{
    std::string id;
    std::string venue;
    OrderSide side;
    double price;
    int quantity;
    std::chrono::system_clock::time_point timestamp;
    double pnl;
    double size;
    std::string symbol;
    int64_t order_created_ts_ms;
    int64_t order_executed_ts_ms;
    int64_t server_broadcast_ts_ms;
    int64_t exchange_recv_ts_ms;
    int64_t ingest_ts_ms;
    double modelled_latency_ms;
};

class OrderBook
{
public:
    OrderBook();
    ~OrderBook();

    void submitOrder(const Order &order);
    void setTradeCallback(std::function<void(const Trade &)> callback);

    double getTotalPnL() const;
    std::vector<Trade> getRecentTrades(int count = 10) const;

private:
    void processOrder(const Order &order);
    std::string generateTradeId();

    std::map<std::string, double> last_prices_;
    std::vector<Trade> trades_;
    std::function<void(const Trade &)> trade_callback_;

    double total_pnl_;
    int trade_counter_;

    // Simple position tracking
    std::map<std::string, int> positions_;
    std::map<std::string, double> avg_prices_;
};