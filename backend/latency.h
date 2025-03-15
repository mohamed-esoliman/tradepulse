#pragma once

#include <string>
#include <map>
#include <chrono>
#include <functional>
#include <thread>
#include <queue>
#include <mutex>
#include <atomic>

struct LatencyEvent
{
    std::string venue;
    double latency_ms;
    std::chrono::system_clock::time_point timestamp;
    std::string order_id;
};

struct DelayedOrder
{
    std::string order_id;
    std::string venue;
    std::chrono::system_clock::time_point execute_time;
    std::function<void()> callback;
};

class LatencySimulator
{
public:
    LatencySimulator();
    ~LatencySimulator();

    void start();
    void stop();

    void addOrderDelay(const std::string &order_id, const std::string &venue, std::function<void()> callback);
    void setLatencyCallback(std::function<void(const LatencyEvent &)> callback);

    // Latency configuration
    void setVenueLatency(const std::string &venue, double latency_ms);
    double getVenueLatency(const std::string &venue) const;

private:
    void processDelayedItems();

    std::map<std::string, double> venue_latencies_;
    std::queue<DelayedOrder> delayed_queue_;
    std::mutex queue_mutex_;

    std::function<void(const LatencyEvent &)> latency_callback_;

    std::atomic<bool> running_;
    std::thread processor_thread_;

    static constexpr int PROCESSING_INTERVAL_MS = 1;
};