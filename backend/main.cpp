#include <iostream>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <csignal>
#include <atomic>

#include "market_feed.h"
#include "order_book.h"
#include "strategy.h"
#include "latency.h"
#include "websocket_server.h"

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

int main()
{
    std::cout << "Starting TradePulse High-Frequency Trading Simulator..." << std::endl;

    // Setup signal handlers for graceful shutdown
    signal(SIGINT, signalHandler);
    signal(SIGTERM, signalHandler);

    try
    {
        // Initialize components
        OrderBook order_book;
        MarketFeed market_feed;
        MomentumStrategy strategy(order_book);
        LatencySimulator latency_simulator;
        WebSocketServer websocket_server(8080);

        // Setup WebSocket server callbacks
        websocket_server.setClientConnectedCallback([](int client_id)
                                                    { std::cout << "Client connected: " << client_id << std::endl; });

        websocket_server.setClientDisconnectedCallback([](int client_id)
                                                       { std::cout << "Client disconnected: " << client_id << std::endl; });

        // Setup market feed callback
        market_feed.setTickCallback([&](const MarketTick &tick)
                                    {
            // Send tick to strategy
            strategy.onMarketTick(tick); });

        // Setup order book callback for trades
        order_book.setTradeCallback([&](const Trade &trade)
                                    {
            // Create WebSocket message
            WebSocketMessage ws_message;
            ws_message.venue = trade.venue;
            ws_message.price = trade.price;
            ws_message.action = (trade.side == OrderSide::BUY) ? "BUY" : "SELL";
            ws_message.latency_ms = latency_simulator.getVenueLatency(trade.venue);
            ws_message.timestamp = formatTimestamp(trade.timestamp);
            ws_message.pnl = trade.pnl;
            ws_message.order_id = trade.id;
            
            // Broadcast to WebSocket clients
            websocket_server.broadcastMessage(ws_message);
            
            std::cout << "Trade executed: " << ws_message.action 
                      << " " << ws_message.venue 
                      << " @ $" << ws_message.price 
                      << " (PnL: $" << ws_message.pnl << ")" << std::endl; });

        // Setup strategy callback for orders (with latency simulation)
        strategy.setOrderCallback([&](const Order &order)
                                  {
            // Add latency delay before order execution
            latency_simulator.addOrderDelay(order.id, order.venue, [&order_book, order]() {
                // This callback will be executed after the latency delay
                order_book.submitOrder(order);
            }); });

        // Setup latency simulator callback
        latency_simulator.setLatencyCallback([&](const LatencyEvent &event)
                                             {
            // Optionally broadcast latency events
            // For now, just log them
            std::cout << "Latency event: " << event.venue 
                      << " - " << event.latency_ms << "ms" << std::endl; });

        // Start all components
        std::cout << "Starting WebSocket server..." << std::endl;
        websocket_server.start();

        std::cout << "Starting latency simulator..." << std::endl;
        latency_simulator.start();

        std::cout << "Starting market feed..." << std::endl;
        market_feed.start();

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
        market_feed.stop();
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