#include "market_feed.h"
#include <iostream>
#include <thread>
#include <chrono>

MarketFeed::MarketFeed() : running_(false), symbol_("BTC-USD"), tick_interval_ms_(100)
{
    venues_ = {"SYNTH"};
    current_prices_["SYNTH"] = 100.0;
    std::random_device rd;
    generators_["SYNTH"] = std::mt19937(rd());
    distributions_["SYNTH"] = std::normal_distribution<double>(0.0, 0.1);
}

MarketFeed::~MarketFeed()
{
    stop();
}

void MarketFeed::start(std::function<void(const MarketTick &)> on_tick)
{
    if (running_)
        return;

    on_tick_ = on_tick;
    running_ = true;
    feed_thread_ = std::thread(&MarketFeed::generateTicks, this);
}

void MarketFeed::stop()
{
    if (!running_)
        return;

    running_ = false;
    if (feed_thread_.joinable())
    {
        feed_thread_.join();
    }
}

void MarketFeed::setSymbol(const std::string &symbol) { symbol_ = symbol; }
void MarketFeed::setTickIntervalMs(int interval_ms) { tick_interval_ms_ = interval_ms; }

double MarketFeed::getCurrentPrice(const std::string &venue) const
{
    auto it = current_prices_.find(venue);
    return (it != current_prices_.end()) ? it->second : 0.0;
}

std::vector<std::string> MarketFeed::getVenues() const
{
    return venues_;
}

void MarketFeed::generateTicks()
{
    while (running_)
    {
        // Generate price movement using random walk
        double price_change = distributions_["SYNTH"](generators_["SYNTH"]);
        current_prices_["SYNTH"] = std::max(1.0, current_prices_["SYNTH"] + price_change);

        MarketTick tick;
        tick.venue = "SYNTH";
        tick.symbol = symbol_;
        tick.price = current_prices_["SYNTH"];
        tick.size = 0.0;
        tick.exchange_recv_ts_ms = -1;
        auto now = std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::system_clock::now());
        tick.ingest_ts_ms = now.time_since_epoch().count();

        if (on_tick_)
        {
            on_tick_(tick);
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(tick_interval_ms_));
    }
}