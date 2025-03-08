#pragma once

#include <string>
#include <vector>
#include <random>
#include <chrono>
#include <functional>
#include <map>
#include <atomic>
#include <thread>

struct MarketTick
{
    std::string venue;
    double price;
    std::chrono::system_clock::time_point timestamp;
    double volume;
};

class MarketFeed
{
public:
    MarketFeed();
    ~MarketFeed();

    void start();
    void stop();

    void setTickCallback(std::function<void(const MarketTick &)> callback);

    double getCurrentPrice(const std::string &venue) const;
    std::vector<std::string> getVenues() const;

private:
    void generateTicks();

    std::vector<std::string> venues_;
    std::map<std::string, double> current_prices_;
    std::map<std::string, std::mt19937> generators_;
    std::map<std::string, std::normal_distribution<double>> distributions_;

    std::function<void(const MarketTick &)> tick_callback_;
    std::atomic<bool> running_;
    std::thread feed_thread_;

    static constexpr int TICK_INTERVAL_MS = 100;
};