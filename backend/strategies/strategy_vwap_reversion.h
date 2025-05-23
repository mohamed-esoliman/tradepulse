#pragma once

#include "strategy_base.h"
#include <deque>
#include <map>

class VwapReversionStrategy : public IStrategy
{
public:
    explicit VwapReversionStrategy(OrderBook &order_book) : order_book_(order_book) {}
    void onMarketTick(const MarketTick &tick) override;
    void setLookback(int lookback) override { lookback_ = lookback; }
    void setOrderQuantity(int quantity) override { order_qty_ = quantity; }
    const char *name() const override { return "vwap_reversion"; }

private:
    struct Accum
    {
        double pv{0.0};
        double v{0.0};
    };
    OrderBook &order_book_;
    std::map<std::string, std::deque<std::pair<double, double>>> window_; // price, size
    int lookback_{50};
    int order_qty_{100};
    int order_counter_{0};
};
