#pragma once

#include <string>
#include <vector>
#include <thread>
#include <atomic>
#include <functional>
#include <set>
#include <mutex>
#include <queue>

struct WebSocketMessage
{
    std::string type;
    std::string venue;
    std::string symbol;
    double price;
    double size;
    std::string action;
    double modelled_latency_ms;
    std::string timestamp;
    double pnl;
    std::string order_id;
    int64_t exchange_recv_ts_ms;
    int64_t ingest_ts_ms;
    int64_t order_created_ts_ms;
    int64_t order_executed_ts_ms;
    int64_t server_broadcast_ts_ms;
};

class WebSocketServer
{
public:
    WebSocketServer(int port = 8080);
    ~WebSocketServer();

    void start();
    void stop();

    void broadcastMessage(const WebSocketMessage &message);
    void setClientConnectedCallback(std::function<void(int)> callback);
    void setClientDisconnectedCallback(std::function<void(int)> callback);
    void setHttpHandler(std::function<std::string(const std::string &, const std::string &, const std::string &)> handler);

    int getConnectedClients() const;

private:
    void serverLoop();
    void handleConnection(int client_socket);
    std::string performWebSocketHandshake(const std::string &request);
    void sendWebSocketFrame(int client_socket, const std::string &message);
    std::string createWebSocketFrame(const std::string &message);
    std::string messageToJson(const WebSocketMessage &message);
    std::string heartbeatToJson(int64_t server_ts_ms);

    int port_;
    int server_socket_;
    std::atomic<bool> running_;
    std::thread server_thread_;

    std::set<int> connected_clients_;
    mutable std::mutex clients_mutex_;

    std::function<void(int)> client_connected_callback_;
    std::function<void(int)> client_disconnected_callback_;

    std::queue<WebSocketMessage> message_queue_;
    std::mutex message_queue_mutex_;

    std::thread heartbeat_thread_;
    std::function<std::string(const std::string &, const std::string &, const std::string &)> http_handler_;
};