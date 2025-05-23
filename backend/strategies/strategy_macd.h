#pragma once

#include "strategy_base.h"
#include <deque>
#include <map>

class MacdStrategy : public IStrategy
{
public:
    explicit MacdStrategy(OrderBook &order_book) : order_book_(order_book) {}
    void onMarketTick(const MarketTick &tick) override;
    void setLookback(int lookback) override { long_window_ = lookback > 0 ? lookback : long_window_; }
    void setOrderQuantity(int quantity) override { order_qty_ = quantity; }
    const char *name() const override { return "macd"; }

private:
    OrderBook &order_book_;
    std::map<std::string, std::deque<double>> prices_;
    int short_window_{12};
    int long_window_{26};
    int signal_window_{9};
    int order_qty_{100};
    int order_counter_{0};
};
