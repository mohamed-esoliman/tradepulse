#pragma once

#include <string>
#include <map>

enum class SourceType
{
    SYNTHETIC,
    LIVE,
    REPLAY
};
enum class ExchangeType
{
    COINBASE,
    BINANCE
};
enum class LatencyMode
{
    MEASURED,
    MODELLED,
    BOTH
};

struct Config
{
    SourceType source{SourceType::SYNTHETIC};
    ExchangeType exchange{ExchangeType::COINBASE};
    std::string symbol{"BTC-USD"};
    std::string replay_file{"./ticks.ndjson"};
    double replay_speed{1.0};
    LatencyMode latency_mode{LatencyMode::BOTH};
    std::map<std::string, double> modelled_latency_ms{{"SYNTH", 20.0}, {"COINBASE", 30.0}, {"LSE", 70.0}};
    std::string strategy{"momentum"}; // momentum|mean_reversion|breakout|vwap_reversion
    int strategy_lookback{3};
    int strategy_order_qty{100};
};

Config parseArgs(int argc, char **argv);
