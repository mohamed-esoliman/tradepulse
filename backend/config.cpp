#include "config.h"
#include <cstring>
#include <cstdlib>

static bool starts_with(const char *s, const char *p) { return std::strncmp(s, p, std::strlen(p)) == 0; }

Config parseArgs(int argc, char **argv)
{
    Config cfg;
    for (int i = 1; i < argc; ++i)
    {
        const char *a = argv[i];
        if (starts_with(a, "--source="))
        {
            const char *v = a + 9;
            if (std::strcmp(v, "synthetic") == 0)
                cfg.source = SourceType::SYNTHETIC;
            else if (std::strcmp(v, "live") == 0)
                cfg.source = SourceType::LIVE;
            else if (std::strcmp(v, "replay") == 0)
                cfg.source = SourceType::REPLAY;
        }
        else if (starts_with(a, "--exchange="))
        {
            const char *v = a + 11;
            if (std::strcmp(v, "coinbase") == 0)
                cfg.exchange = ExchangeType::COINBASE;
            else if (std::strcmp(v, "binance") == 0)
                cfg.exchange = ExchangeType::BINANCE;
        }
        else if (starts_with(a, "--symbol="))
        {
            cfg.symbol = std::string(a + 9);
        }
        else if (starts_with(a, "--replay_file="))
        {
            cfg.replay_file = std::string(a + 14);
        }
        else if (starts_with(a, "--replay_speed="))
        {
            cfg.replay_speed = std::atof(a + 15);
        }
        else if (starts_with(a, "--latency_mode="))
        {
            const char *v = a + 15;
            if (std::strcmp(v, "measured") == 0)
                cfg.latency_mode = LatencyMode::MEASURED;
            else if (std::strcmp(v, "modelled") == 0)
                cfg.latency_mode = LatencyMode::MODELLED;
            else if (std::strcmp(v, "both") == 0)
                cfg.latency_mode = LatencyMode::BOTH;
        }
        else if (starts_with(a, "--modelled_latency_ms="))
        {
            std::string s = std::string(a + 22);
            // format VENUE:ms,VENUE:ms
            size_t pos = 0;
            while (pos < s.size())
            {
                size_t comma = s.find(',', pos);
                std::string token = s.substr(pos, comma == std::string::npos ? std::string::npos : comma - pos);
                size_t colon = token.find(':');
                if (colon != std::string::npos)
                {
                    std::string venue = token.substr(0, colon);
                    double ms = std::atof(token.substr(colon + 1).c_str());
                    cfg.modelled_latency_ms[venue] = ms;
                }
                if (comma == std::string::npos)
                    break;
                pos = comma + 1;
            }
        }
        else if (starts_with(a, "--strategy="))
        {
            cfg.strategy = std::string(a + 11);
        }
        else if (starts_with(a, "--lookback="))
        {
            cfg.strategy_lookback = std::atoi(a + 11);
        }
        else if (starts_with(a, "--order_qty="))
        {
            cfg.strategy_order_qty = std::atoi(a + 12);
        }
    }
    return cfg;
}
