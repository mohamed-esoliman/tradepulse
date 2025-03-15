#include "latency.h"
#include <iostream>

LatencySimulator::LatencySimulator() : running_(false)
{
    // Set default latencies for different venues
    venue_latencies_["NASDAQ"] = 20.0; // 20ms
    venue_latencies_["LSE"] = 70.0;    // 70ms
    venue_latencies_["NYSE"] = 15.0;   // 15ms
    venue_latencies_["CBOE"] = 25.0;   // 25ms
}

LatencySimulator::~LatencySimulator()
{
    stop();
}

void LatencySimulator::start()
{
    if (running_)
        return;

    running_ = true;
    processor_thread_ = std::thread(&LatencySimulator::processDelayedItems, this);
}

void LatencySimulator::stop()
{
    if (!running_)
        return;

    running_ = false;
    if (processor_thread_.joinable())
    {
        processor_thread_.join();
    }
}

void LatencySimulator::addOrderDelay(const std::string &order_id, const std::string &venue, std::function<void()> callback)
{
    double latency_ms = getVenueLatency(venue);

    DelayedOrder delayed_order;
    delayed_order.order_id = order_id;
    delayed_order.venue = venue;
    delayed_order.execute_time = std::chrono::system_clock::now() +
                                 std::chrono::milliseconds(static_cast<int>(latency_ms));
    delayed_order.callback = callback;

    {
        std::lock_guard<std::mutex> lock(queue_mutex_);
        delayed_queue_.push(delayed_order);
    }

    // Emit latency event
    if (latency_callback_)
    {
        LatencyEvent event;
        event.venue = venue;
        event.latency_ms = latency_ms;
        event.timestamp = std::chrono::system_clock::now();
        event.order_id = order_id;
        latency_callback_(event);
    }
}

void LatencySimulator::setLatencyCallback(std::function<void(const LatencyEvent &)> callback)
{
    latency_callback_ = callback;
}

void LatencySimulator::setVenueLatency(const std::string &venue, double latency_ms)
{
    venue_latencies_[venue] = latency_ms;
}

double LatencySimulator::getVenueLatency(const std::string &venue) const
{
    auto it = venue_latencies_.find(venue);
    return (it != venue_latencies_.end()) ? it->second : 50.0; // Default 50ms
}

void LatencySimulator::processDelayedItems()
{
    while (running_)
    {
        std::queue<DelayedOrder> ready_orders;
        auto now = std::chrono::system_clock::now();

        // Check for orders ready to execute
        {
            std::lock_guard<std::mutex> lock(queue_mutex_);
            std::queue<DelayedOrder> temp_queue;

            while (!delayed_queue_.empty())
            {
                DelayedOrder order = delayed_queue_.front();
                delayed_queue_.pop();

                if (order.execute_time <= now)
                {
                    ready_orders.push(order);
                }
                else
                {
                    temp_queue.push(order);
                }
            }

            delayed_queue_ = temp_queue;
        }

        // Execute ready orders
        while (!ready_orders.empty())
        {
            DelayedOrder order = ready_orders.front();
            ready_orders.pop();

            if (order.callback)
            {
                order.callback();
            }
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(PROCESSING_INTERVAL_MS));
    }
}