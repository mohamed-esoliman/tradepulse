#pragma once

#include "strategies/strategy_base.h"
#include "order_book.h"
#include <vector>
#include <map>
#include <deque>
#include <functional>

class MomentumStrategy : public IStrategy
{
public:
    MomentumStrategy(OrderBook &order_book);
    ~MomentumStrategy();

    void onMarketTick(const MarketTick &tick) override;
    void setLookback(int threshold) override;
    void setOrderQuantity(int quantity) override;
    const char *name() const override { return "momentum"; }

    // Strategy parameters
    void setTickThreshold(int threshold);
    void setOrderQuantityInternal(int quantity);

private:
    void checkMomentum(const std::string &venue, const MarketTick &tick);
    bool isUpwardMomentum(const std::string &venue) const;
    bool isDownwardMomentum(const std::string &venue) const;

    OrderBook &order_book_;

    // Price history for momentum calculation
    std::map<std::string, std::deque<double>> price_history_;

    // Strategy parameters
    int tick_threshold_;
    int order_quantity_;

    // Order tracking
    int order_counter_;

    static constexpr int MAX_PRICE_HISTORY = 10;
};