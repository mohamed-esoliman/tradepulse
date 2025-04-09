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
    std::string venue;
    double price;
    std::string action;
    double latency_ms;
    std::string timestamp;
    double pnl;
    std::string order_id;
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

    int getConnectedClients() const;

private:
    void serverLoop();
    void handleConnection(int client_socket);
    std::string performWebSocketHandshake(const std::string &request);
    void sendWebSocketFrame(int client_socket, const std::string &message);
    std::string createWebSocketFrame(const std::string &message);
    std::string messageToJson(const WebSocketMessage &message);

    int port_;
    int server_socket_;
    std::atomic<bool> running_;
    std::thread server_thread_;

    std::set<int> connected_clients_;
    std::mutex clients_mutex_;

    std::function<void(int)> client_connected_callback_;
    std::function<void(int)> client_disconnected_callback_;

    std::queue<WebSocketMessage> message_queue_;
    std::mutex message_queue_mutex_;
};