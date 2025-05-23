#pragma once

#include "strategies/strategy_base.h"
#include "order_book.h"
#include <deque>
#include <map>

class MomentumStrategy : public IStrategy
{
public:
    explicit MomentumStrategy(OrderBook &order_book) : order_book_(order_book) {}
    ~MomentumStrategy() = default;

    void onMarketTick(const MarketTick &tick) override;
    void setLookback(int threshold) override { tick_threshold_ = threshold; }
    void setOrderQuantity(int quantity) override { order_quantity_ = quantity; }
    const char *name() const override { return "momentum"; }

private:
    void checkMomentum(const std::string &venue, const MarketTick &tick);
    bool isUpwardMomentum(const std::string &venue) const;
    bool isDownwardMomentum(const std::string &venue) const;

    OrderBook &order_book_;
    std::map<std::string, std::deque<double>> price_history_;
    int tick_threshold_{3};
    int order_quantity_{100};
    int order_counter_{0};
    static constexpr int MAX_PRICE_HISTORY = 10;
};
