#pragma once

#include "strategies/strategy_base.h"
#include <deque>
#include <map>

class MeanReversionStrategy : public IStrategy
{
public:
    explicit MeanReversionStrategy(OrderBook &order_book) : order_book_(order_book) {}
    void onMarketTick(const MarketTick &tick) override;
    void setLookback(int lookback) override { lookback_ = lookback; }
    void setOrderQuantity(int quantity) override { order_quantity_ = quantity; }
    const char *name() const override { return "mean_reversion"; }

private:
    OrderBook &order_book_;
    std::map<std::string, std::deque<double>> price_history_;
    int lookback_{10};
    int order_quantity_{100};
    int order_counter_{0};
};
