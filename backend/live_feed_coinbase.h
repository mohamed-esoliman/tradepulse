#pragma once

#include "data_source.h"
#include <string>
#include <atomic>
#include <thread>

class LiveFeedCoinbase : public IDataSource
{
public:
    explicit LiveFeedCoinbase(const std::string &symbol);
    ~LiveFeedCoinbase();
    void start(std::function<void(const MarketTick &)> on_tick) override;
    void stop() override;

private:
    void run();
    std::string symbol_;
    std::atomic<bool> running_{false};
    std::thread thread_;
    std::function<void(const MarketTick &)> on_tick_;
};
