#pragma once

#include "strategy_base.h"
#include <deque>
#include <map>

class BreakoutStrategy : public IStrategy
{
public:
    explicit BreakoutStrategy(OrderBook &order_book) : order_book_(order_book) {}
    void onMarketTick(const MarketTick &tick) override;
    void setLookback(int lookback) override { lookback_ = lookback; }
    void setOrderQuantity(int quantity) override { order_qty_ = quantity; }
    const char *name() const override { return "breakout"; }

private:
    OrderBook &order_book_;
    std::map<std::string, std::deque<double>> window_;
    int lookback_{20};
    int order_qty_{100};
    int order_counter_{0};
};
