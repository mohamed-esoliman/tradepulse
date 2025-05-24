#pragma once

#include <string>
#include <vector>
#include <random>
#include <chrono>
#include <functional>
#include <map>
#include <atomic>
#include <thread>
#include "data_source.h"

class MarketFeed : public IDataSource
{
public:
    MarketFeed();
    ~MarketFeed();

    void start(std::function<void(const MarketTick &)> on_tick) override;
    void stop() override;

    void setSymbol(const std::string &symbol);
    void setTickIntervalMs(int interval_ms);
    double getCurrentPrice(const std::string &venue) const;
    std::vector<std::string> getVenues() const;

private:
    void generateTicks();

    std::vector<std::string> venues_;
    std::map<std::string, double> current_prices_;
    std::map<std::string, std::mt19937> generators_;
    std::map<std::string, std::normal_distribution<double>> distributions_;

    std::function<void(const MarketTick &)> on_tick_;
    std::atomic<bool> running_;
    std::thread feed_thread_;

    std::string symbol_;
    int tick_interval_ms_;
};