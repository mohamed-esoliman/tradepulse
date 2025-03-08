#include "market_feed.h"
#include <iostream>
#include <thread>

MarketFeed::MarketFeed() : running_(false)
{
    venues_ = {"NASDAQ", "LSE", "NYSE", "CBOE"};

    // Initialize starting prices and random generators
    current_prices_["NASDAQ"] = 125.50;
    current_prices_["LSE"] = 98.75;
    current_prices_["NYSE"] = 142.25;
    current_prices_["CBOE"] = 87.90;

    // Setup random number generators with different seeds
    std::random_device rd;
    for (const auto &venue : venues_)
    {
        generators_[venue] = std::mt19937(rd());
        distributions_[venue] = std::normal_distribution<double>(0.0, 0.1);
    }
}

MarketFeed::~MarketFeed()
{
    stop();
}

void MarketFeed::start()
{
    if (running_)
        return;

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

void MarketFeed::setTickCallback(std::function<void(const MarketTick &)> callback)
{
    tick_callback_ = callback;
}

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
        for (const auto &venue : venues_)
        {
            // Generate price movement using random walk
            double price_change = distributions_[venue](generators_[venue]);
            current_prices_[venue] = std::max(1.0, current_prices_[venue] + price_change);

            // Create market tick
            MarketTick tick;
            tick.venue = venue;
            tick.price = current_prices_[venue];
            tick.timestamp = std::chrono::system_clock::now();
            tick.volume = 100 + (generators_[venue]() % 900); // Random volume 100-1000

            // Call the callback if set
            if (tick_callback_)
            {
                tick_callback_(tick);
            }
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(TICK_INTERVAL_MS));
    }
}