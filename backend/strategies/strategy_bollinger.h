#pragma once

#include "strategy_base.h"
#include <deque>
#include <map>

class BollingerStrategy : public IStrategy
{
public:
    explicit BollingerStrategy(OrderBook &order_book) : order_book_(order_book) {}
    void onMarketTick(const MarketTick &tick) override;
    void setLookback(int lookback) override { period_ = lookback > 0 ? lookback : period_; }
    void setOrderQuantity(int quantity) override { order_qty_ = quantity; }
    const char *name() const override { return "bollinger"; }

private:
    OrderBook &order_book_;
    std::map<std::string, std::deque<double>> prices_;
    int period_{20};
    double k_{2.0};
    int order_qty_{100};
    int order_counter_{0};
};
