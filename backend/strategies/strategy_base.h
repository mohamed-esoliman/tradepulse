#pragma once

#include "data_source.h"
#include "order_book.h"
#include <functional>

class IStrategy
{
public:
    virtual ~IStrategy() = default;
    std::function<void(const Order &)> on_order;
    virtual void onMarketTick(const MarketTick &tick) = 0;
    virtual void setLookback(int lookback) = 0;
    virtual void setOrderQuantity(int quantity) = 0;
    virtual const char *name() const = 0;
};
