#pragma once

#include "data_source.h"
#include <string>
#include <atomic>
#include <thread>

class ReplayFeed : public IDataSource
{
public:
    explicit ReplayFeed(const std::string &file_path, double speed = 1.0);
    ~ReplayFeed();
    void start(std::function<void(const MarketTick &)> on_tick) override;
    void stop() override;

private:
    void run();
    std::string file_path_;
    double speed_;
    std::atomic<bool> running_{false};
    std::thread thread_;
    std::function<void(const MarketTick &)> on_tick_;
};
