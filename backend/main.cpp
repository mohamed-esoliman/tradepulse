#include <iostream>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <csignal>
#include <atomic>

#include "market_feed.h"
#include "data_source.h"
#include "order_book.h"
#include "strategies/strategy_momentum.h"
#include "strategies/strategy_mean_reversion.h"
#include "strategies/strategy_breakout.h"
#include "strategies/strategy_vwap_reversion.h"
#include "strategies/strategy_macd.h"
#include "strategies/strategy_rsi.h"
#include "latency.h"
#include "websocket_server.h"
#include "config.h"
#include "replay_feed.h"
#include "live_feed_coinbase.h"

// Global flag for graceful shutdown
std::atomic<bool> g_shutdown(false);

void signalHandler(int signal)
{
    std::cout << "\nShutdown signal received (" << signal << ")" << std::endl;
    g_shutdown = true;
}

std::string formatTimestamp(const std::chrono::system_clock::time_point &time_point)
{
    auto time_t = std::chrono::system_clock::to_time_t(time_point);
    std::ostringstream oss;
    oss << std::put_time(std::gmtime(&time_t), "%Y-%m-%dT%H:%M:%SZ");
    return oss.str();
}

int main(int argc, char **argv)
{
    std::cout << "Starting TradePulse High-Frequency Trading Simulator..." << std::endl;

    // Setup signal handlers for graceful shutdown
    signal(SIGINT, signalHandler);
    signal(SIGTERM, signalHandler);

    try
    {
        Config cfg = parseArgs(argc, argv);

        // Initialize components
        OrderBook order_book;
        MarketFeed synth_feed;
        MomentumStrategy momentum(order_book);
        MeanReversionStrategy meanrev(order_book);
        BreakoutStrategy breakout(order_book);
        VwapReversionStrategy vwap(order_book);
        MacdStrategy macd(order_book);
        RsiStrategy rsi(order_book);
        IStrategy *strategy = &momentum;
        if (cfg.strategy == std::string("mean_reversion"))
            strategy = &meanrev;
        else if (cfg.strategy == std::string("breakout"))
            strategy = &breakout;
        else if (cfg.strategy == std::string("vwap_reversion"))
            strategy = &vwap;
        else if (cfg.strategy == std::string("macd"))
            strategy = &macd;
        else if (cfg.strategy == std::string("rsi"))
            strategy = &rsi;
        strategy->setLookback(cfg.strategy_lookback);
        strategy->setOrderQuantity(cfg.strategy_order_qty);
        LatencySimulator latency_simulator;
        WebSocketServer websocket_server(8080);

        // Setup WebSocket server callbacks
        websocket_server.setClientConnectedCallback([](int client_id)
                                                    { std::cout << "Client connected: " << client_id << std::endl; });

        websocket_server.setClientDisconnectedCallback([](int client_id)
                                                       { std::cout << "Client disconnected: " << client_id << std::endl; });

        std::unique_ptr<IDataSource> dynamic_source;
        IDataSource *source_ptr = nullptr;
        bool running = false;

        websocket_server.setHttpHandler([&](const std::string &method, const std::string &path, const std::string &req) -> std::string
                                        {
            if (method == "GET" && path.rfind("/info", 0) == 0) {
                std::ostringstream oss;
                oss << "strategy=" << strategy->name() << "\n";
                oss << "lookback=" << cfg.strategy_lookback << "\n";
                oss << "order_qty=" << cfg.strategy_order_qty << "\n";
                oss << "source=" << (cfg.source == SourceType::SYNTHETIC ? "synthetic" : cfg.source == SourceType::LIVE ? "live" : "replay") << "\n";
                oss << "symbol=" << cfg.symbol << "\n";
                return oss.str();
            }
            if (method == "GET" && path.rfind("/control", 0) == 0) {
                auto qpos = path.find('?');
                std::string qs = (qpos != std::string::npos) ? path.substr(qpos + 1) : std::string();
                auto get = [&](const std::string &k) -> std::string {
                    size_t p = qs.find(k + "=");
                    if (p == std::string::npos) return {};
                    size_t s = p + k.size() + 1;
                    size_t e = qs.find('&', s);
                    return qs.substr(s, e == std::string::npos ? std::string::npos : e - s);
                };

                std::string action = get("action");
                std::string strat = get("strategy");
                std::string look = get("lookback");
                std::string qty = get("order_qty");
                std::string source = get("source");
                std::string symbol = get("symbol");

                if (!strat.empty()) {
                    if (strat == "momentum") strategy = &momentum; else if (strat == "mean_reversion") strategy = &meanrev;
                    cfg.strategy = strat;
                }
                if (!look.empty()) { cfg.strategy_lookback = std::atoi(look.c_str()); strategy->setLookback(cfg.strategy_lookback); }
                if (!qty.empty()) { cfg.strategy_order_qty = std::atoi(qty.c_str()); strategy->setOrderQuantity(cfg.strategy_order_qty); }

                if (!source.empty()) {
                    if (source == "synthetic") {
                        if (!symbol.empty()) cfg.symbol = symbol;
                        synth_feed.setSymbol(cfg.symbol);
                        if (dynamic_source) { dynamic_source->stop(); dynamic_source.reset(); }
                        if (source_ptr == &synth_feed) synth_feed.stop();
                        source_ptr = &synth_feed;
                        synth_feed.start([&](const MarketTick &tick) { strategy->onMarketTick(tick); });
                    } else if (source == "live") {
                        if (source_ptr == &synth_feed) synth_feed.stop();
                        if (dynamic_source) { dynamic_source->stop(); dynamic_source.reset(); }
                        if (!symbol.empty()) cfg.symbol = symbol;
                        dynamic_source = std::make_unique<LiveFeedCoinbase>(cfg.symbol);
                        source_ptr = dynamic_source.get();
                        dynamic_source->start([&](const MarketTick &tick) { strategy->onMarketTick(tick); });
                    }
                }
                if (action == "stop") {
                    if (source_ptr == &synth_feed) synth_feed.stop();
                    if (dynamic_source) dynamic_source->stop();
                    running = false;
                }
                if (action == "start") {
                    if (!source_ptr) {
                        if (cfg.source == SourceType::SYNTHETIC) { source_ptr = &synth_feed; }
                        else if (cfg.source == SourceType::LIVE) { dynamic_source = std::make_unique<LiveFeedCoinbase>(cfg.symbol); source_ptr = dynamic_source.get(); }
                        else if (cfg.source == SourceType::REPLAY) { dynamic_source = std::make_unique<ReplayFeed>(cfg.replay_file, cfg.replay_speed); source_ptr = dynamic_source.get(); }
                    }
                    if (source_ptr == &synth_feed) synth_feed.start([&](const MarketTick &tick) { strategy->onMarketTick(tick); });
                    else if (dynamic_source) dynamic_source->start([&](const MarketTick &tick) { strategy->onMarketTick(tick); });
                    running = true;
                }

                return std::string("OK");
            }
            return ""; });

        synth_feed.setSymbol(cfg.symbol);
        synth_feed.setTickIntervalMs(100);

        // Setup order book callback for trades
        order_book.setTradeCallback([&](const Trade &trade)
                                    {
            // Create WebSocket message
            WebSocketMessage ws_message;
            ws_message.type = "trade";
            ws_message.venue = trade.venue;
            ws_message.symbol = trade.symbol;
            ws_message.price = trade.price;
            ws_message.size = trade.size;
            ws_message.action = (trade.side == OrderSide::BUY) ? "BUY" : "SELL";
            ws_message.modelled_latency_ms = latency_simulator.getVenueLatency(trade.venue);
            ws_message.timestamp = formatTimestamp(trade.timestamp);
            ws_message.pnl = trade.pnl;
            ws_message.order_id = trade.id;
            ws_message.exchange_recv_ts_ms = trade.exchange_recv_ts_ms;
            ws_message.ingest_ts_ms = trade.ingest_ts_ms;
            ws_message.order_created_ts_ms = trade.order_created_ts_ms;
            ws_message.order_executed_ts_ms = trade.order_executed_ts_ms;
            ws_message.server_broadcast_ts_ms = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
            
            // Broadcast to WebSocket clients
            websocket_server.broadcastMessage(ws_message);
            
            std::cout << "Trade executed: " << ws_message.action 
                      << " " << ws_message.venue 
                      << " @ $" << ws_message.price 
                      << " (PnL: $" << ws_message.pnl << ")" << std::endl; });

        // Strategy should not submit directly
        strategy->on_order = [&](const Order &order)
        {
            if (cfg.latency_mode == LatencyMode::MEASURED)
            {
                order_book.submitOrder(order);
            }
            else
            {
                latency_simulator.addOrderDelay(order.id, order.venue, [&order_book, order]()
                                                { order_book.submitOrder(order); });
            }
        };

        // Setup latency simulator callback
        latency_simulator.setLatencyCallback([&](const LatencyEvent &event)
                                             {
            std::cout << "Latency event: " << event.venue 
                      << " - " << event.latency_ms << "ms" << std::endl;
            WebSocketMessage latency_msg;
            latency_msg.type = "latency";
            latency_msg.venue = event.venue;
            latency_msg.symbol = "";
            latency_msg.price = 0.0;
            latency_msg.size = 0.0;
            latency_msg.action = "";
            latency_msg.modelled_latency_ms = event.latency_ms;
            latency_msg.timestamp = "";
            latency_msg.pnl = 0.0;
            latency_msg.order_id = "";
            latency_msg.exchange_recv_ts_ms = -1;
            latency_msg.ingest_ts_ms = -1;
            latency_msg.order_created_ts_ms = -1;
            latency_msg.order_executed_ts_ms = -1;
            latency_msg.server_broadcast_ts_ms = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
            websocket_server.broadcastMessage(latency_msg); });

        // Start all components
        std::cout << "Starting WebSocket server..." << std::endl;
        websocket_server.start();

        std::cout << "Starting latency simulator..." << std::endl;
        latency_simulator.start();
        for (const auto &kv : cfg.modelled_latency_ms)
        {
            latency_simulator.setVenueLatency(kv.first, kv.second);
        }

        // Auto-start market feed based on CLI flags
        if (cfg.source == SourceType::SYNTHETIC)
        {
            source_ptr = &synth_feed;
            synth_feed.start([&](const MarketTick &tick)
                             { strategy->onMarketTick(tick); });
        }
        else if (cfg.source == SourceType::LIVE)
        {
            dynamic_source = std::make_unique<LiveFeedCoinbase>(cfg.symbol);
            source_ptr = dynamic_source.get();
            dynamic_source->start([&](const MarketTick &tick)
                                  { strategy->onMarketTick(tick); });
        }
        else if (cfg.source == SourceType::REPLAY)
        {
            dynamic_source = std::make_unique<ReplayFeed>(cfg.replay_file, cfg.replay_speed);
            source_ptr = dynamic_source.get();
            dynamic_source->start([&](const MarketTick &tick)
                                  { strategy->onMarketTick(tick); });
        }

        std::cout << "TradePulse is running! Connect to ws://localhost:8080 to see live data." << std::endl;
        std::cout << "Press Ctrl+C to stop." << std::endl;

        // Main loop
        while (!g_shutdown)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));

            // Print periodic stats
            static int stats_counter = 0;
            if (++stats_counter % 100 == 0)
            { // Every 10 seconds
                std::cout << "Stats - Connected clients: " << websocket_server.getConnectedClients()
                          << ", Total PnL: $" << order_book.getTotalPnL() << std::endl;
            }
        }

        // Shutdown components
        std::cout << "Shutting down..." << std::endl;
        if (source_ptr == &synth_feed)
        {
            synth_feed.stop();
        }
        else if (dynamic_source)
        {
            dynamic_source->stop();
        }
        latency_simulator.stop();
        websocket_server.stop();

        std::cout << "Final Stats:" << std::endl;
        std::cout << "Total PnL: $" << order_book.getTotalPnL() << std::endl;

        auto recent_trades = order_book.getRecentTrades(5);
        std::cout << "Recent trades:" << std::endl;
        for (const auto &trade : recent_trades)
        {
            std::cout << "  " << trade.id << " - " << trade.venue
                      << " " << ((trade.side == OrderSide::BUY) ? "BUY" : "SELL")
                      << " @ $" << trade.price << " (PnL: $" << trade.pnl << ")" << std::endl;
        }
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    std::cout << "TradePulse stopped successfully." << std::endl;
    return 0;
}