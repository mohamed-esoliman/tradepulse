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
    OrderSide side;
    double price;
    int quantity;
    std::chrono::system_clock::time_point timestamp;
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