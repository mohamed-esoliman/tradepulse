#pragma once

#include "strategy_base.h"
#include <deque>
#include <map>

class RsiStrategy : public IStrategy
{
public:
    explicit RsiStrategy(OrderBook &order_book) : order_book_(order_book) {}
    void onMarketTick(const MarketTick &tick) override;
    void setLookback(int lookback) override { period_ = lookback > 0 ? lookback : period_; }
    void setOrderQuantity(int quantity) override { order_qty_ = quantity; }
    const char *name() const override { return "rsi"; }

private:
    OrderBook &order_book_;
    std::map<std::string, std::deque<double>> prices_;
    int period_{14};
    int order_qty_{100};
    int order_counter_{0};
};
