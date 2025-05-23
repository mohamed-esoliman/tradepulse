#include "replay_feed.h"
#include <fstream>
#include <sstream>
#include <chrono>

ReplayFeed::ReplayFeed(const std::string &file_path, double speed)
    : file_path_(file_path), speed_(speed)
{
}

ReplayFeed::~ReplayFeed()
{
    stop();
}

void ReplayFeed::start(std::function<void(const MarketTick &)> on_tick)
{
    if (running_)
        return;
    running_ = true;
    on_tick_ = on_tick;
    thread_ = std::thread(&ReplayFeed::run, this);
}

void ReplayFeed::stop()
{
    if (!running_)
        return;
    running_ = false;
    if (thread_.joinable())
        thread_.join();
}

void ReplayFeed::run()
{
    std::ifstream in(file_path_);
    if (!in.is_open())
        return;
    std::string line;
    int64_t prev_ts = -1;
    while (running_ && std::getline(in, line))
    {
        // expected NDJSON matching broadcast trade payload fields used to reconstruct MarketTick
        if (line.empty())
            continue;
        // very small ad-hoc parser: look for keys we need
        MarketTick tick{};
        tick.exchange_recv_ts_ms = -1;
        tick.size = 0;
        auto getNum = [&](const std::string &k) -> double
        {
            auto pos = line.find("\"" + k + "\"");
            if (pos == std::string::npos)
                return 0.0;
            pos = line.find(':', pos);
            if (pos == std::string::npos)
                return 0.0;
            size_t end = line.find_first_of(",}\n", pos + 1);
            return std::atof(line.substr(pos + 1, end - pos - 1).c_str());
        };
        auto getStr = [&](const std::string &k) -> std::string
        {
            auto pos = line.find("\"" + k + "\"");
            if (pos == std::string::npos)
                return {};
            pos = line.find(':', pos);
            pos = line.find('"', pos);
            size_t end = line.find('"', pos + 1);
            if (pos == std::string::npos || end == std::string::npos)
                return {};
            return line.substr(pos + 1, end - pos - 1);
        };
        tick.venue = getStr("venue");
        tick.symbol = getStr("symbol");
        tick.price = getNum("price");
        tick.size = getNum("size");
        int64_t ingest_ms = static_cast<int64_t>(getNum("ingest_ts_ms"));
        if (ingest_ms <= 0)
            ingest_ms = static_cast<int64_t>(getNum("server_broadcast_ts_ms"));
        if (ingest_ms <= 0)
        {
            ingest_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                            std::chrono::system_clock::now().time_since_epoch())
                            .count();
        }
        tick.ingest_ts_ms = ingest_ms;
        if (prev_ts > 0)
        {
            int64_t delta = static_cast<int64_t>((ingest_ms - prev_ts) / speed_);
            if (delta > 0)
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(delta));
            }
        }
        prev_ts = ingest_ms;
        if (on_tick_)
            on_tick_(tick);
    }
}
