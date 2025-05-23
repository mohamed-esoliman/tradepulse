#include "strategy_macd.h"
#include <chrono>

static double ema(const std::deque<double> &x, int period)
{
    if (period <= 1 || x.empty())
        return x.back();
    double k = 2.0 / (period + 1);
    double ema = x.front();
    for (size_t i = 1; i < x.size(); ++i)
        ema = x[i] * k + ema * (1 - k);
    return ema;
}

void MacdStrategy::onMarketTick(const MarketTick &tick)
{
    auto &px = prices_[tick.venue];
    px.push_back(tick.price);
    if (px.size() > static_cast<size_t>(std::max(long_window_, std::max(short_window_, signal_window_))))
        px.pop_front();
    if (px.size() < static_cast<size_t>(long_window_))
        return;

    double ema_short = ema(px, short_window_);
    double ema_long = ema(px, long_window_);
    double macd = ema_short - ema_long;
    // rough signal line from macd history; reuse px deque for brevity
    std::deque<double> macd_series = px;
    macd_series.back() = macd;
    double signal = ema(macd_series, signal_window_);
    double hist = macd - signal;

    if (hist > 0)
    {
        Order o;
        o.id = "O" + std::to_string(++order_counter_);
        o.venue = tick.venue;
        o.symbol = tick.symbol;
        o.side = OrderSide::BUY;
        o.price = tick.price;
        o.quantity = order_qty_;
        o.timestamp = std::chrono::system_clock::now();
        o.exchange_recv_ts_ms = tick.exchange_recv_ts_ms;
        o.ingest_ts_ms = tick.ingest_ts_ms;
        if (on_order)
            on_order(o);
    }
    else if (hist < 0)
    {
        Order o;
        o.id = "O" + std::to_string(++order_counter_);
        o.venue = tick.venue;
        o.symbol = tick.symbol;
        o.side = OrderSide::SELL;
        o.price = tick.price;
        o.quantity = order_qty_;
        o.timestamp = std::chrono::system_clock::now();
        o.exchange_recv_ts_ms = tick.exchange_recv_ts_ms;
        o.ingest_ts_ms = tick.ingest_ts_ms;
        if (on_order)
            on_order(o);
    }
}
