#include "websocket_server.h"
#include <iostream>
#include <sstream>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <openssl/sha.h>
#include <openssl/evp.h>
#include <openssl/bio.h>
#include <openssl/buffer.h>
#include <regex>
#include <iomanip>
#include <chrono>

WebSocketServer::WebSocketServer(int port) : port_(port), server_socket_(-1), running_(false)
{
}

WebSocketServer::~WebSocketServer()
{
    stop();
}

void WebSocketServer::start()
{
    if (running_)
        return;

    running_ = true;
    server_thread_ = std::thread(&WebSocketServer::serverLoop, this);

    heartbeat_thread_ = std::thread([this]()
                                    {
        while (running_)
        {
            auto now_ms = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
            std::string hb = heartbeatToJson(now_ms);
            std::lock_guard<std::mutex> lock(clients_mutex_);
            for (auto it = connected_clients_.begin(); it != connected_clients_.end();)
            {
                int client = *it;
                try
                {
                    sendWebSocketFrame(client, hb);
                    ++it;
                }
                catch (...)
                {
                    close(client);
                    it = connected_clients_.erase(it);
                    if (client_disconnected_callback_)
                    {
                        client_disconnected_callback_(client);
                    }
                }
            }
            std::this_thread::sleep_for(std::chrono::seconds(5));
        } });
}

void WebSocketServer::stop()
{
    if (!running_)
        return;

    running_ = false;

    if (server_socket_ != -1)
    {
        close(server_socket_);
        server_socket_ = -1;
    }

    if (server_thread_.joinable())
    {
        server_thread_.join();
    }

    if (heartbeat_thread_.joinable())
    {
        heartbeat_thread_.join();
    }

    // Close all client connections
    std::lock_guard<std::mutex> lock(clients_mutex_);
    for (int client : connected_clients_)
    {
        close(client);
    }
    connected_clients_.clear();
}

void WebSocketServer::broadcastMessage(const WebSocketMessage &message)
{
    std::string json_message = messageToJson(message);

    std::lock_guard<std::mutex> lock(clients_mutex_);
    for (auto it = connected_clients_.begin(); it != connected_clients_.end();)
    {
        int client = *it;
        try
        {
            sendWebSocketFrame(client, json_message);
            ++it;
        }
        catch (...)
        {
            // Client disconnected, remove from set
            close(client);
            it = connected_clients_.erase(it);
            if (client_disconnected_callback_)
            {
                client_disconnected_callback_(client);
            }
        }
    }
}

void WebSocketServer::setClientConnectedCallback(std::function<void(int)> callback)
{
    client_connected_callback_ = callback;
}

void WebSocketServer::setClientDisconnectedCallback(std::function<void(int)> callback)
{
    client_disconnected_callback_ = callback;
}

void WebSocketServer::setHttpHandler(std::function<std::string(const std::string &, const std::string &, const std::string &)> handler)
{
    http_handler_ = handler;
}

int WebSocketServer::getConnectedClients() const
{
    std::lock_guard<std::mutex> lock(clients_mutex_);
    return connected_clients_.size();
}

void WebSocketServer::serverLoop()
{
    // Create socket
    server_socket_ = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket_ == -1)
    {
        std::cerr << "Failed to create socket" << std::endl;
        return;
    }

    // Set socket options
    int opt = 1;
    setsockopt(server_socket_, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    // Bind socket
    struct sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port_);

    if (bind(server_socket_, (struct sockaddr *)&address, sizeof(address)) < 0)
    {
        std::cerr << "Failed to bind socket to port " << port_ << std::endl;
        close(server_socket_);
        return;
    }

    // Listen for connections
    if (listen(server_socket_, 10) < 0)
    {
        std::cerr << "Failed to listen on socket" << std::endl;
        close(server_socket_);
        return;
    }

    std::cout << "WebSocket server listening on port " << port_ << std::endl;

    // Accept connections
    while (running_)
    {
        struct sockaddr_in client_address;
        socklen_t client_len = sizeof(client_address);

        int client_socket = accept(server_socket_, (struct sockaddr *)&client_address, &client_len);
        if (client_socket >= 0)
        {
            std::thread(&WebSocketServer::handleConnection, this, client_socket).detach();
        }
    }
}

void WebSocketServer::handleConnection(int client_socket)
{
    char buffer[4096];
    int bytes_read = recv(client_socket, buffer, sizeof(buffer) - 1, 0);

    if (bytes_read <= 0)
    {
        close(client_socket);
        return;
    }

    buffer[bytes_read] = '\0';
    std::string request(buffer);

    // Check if it's a WebSocket handshake
    if (request.find("Upgrade: websocket") != std::string::npos)
    {
        std::string response = performWebSocketHandshake(request);
        send(client_socket, response.c_str(), response.length(), 0);

        // Add client to connected clients
        {
            std::lock_guard<std::mutex> lock(clients_mutex_);
            connected_clients_.insert(client_socket);
        }

        if (client_connected_callback_)
        {
            client_connected_callback_(client_socket);
        }

        std::cout << "WebSocket client connected: " << client_socket << std::endl;

        // Keep connection alive and handle messages
        while (running_)
        {
            bytes_read = recv(client_socket, buffer, sizeof(buffer), 0);
            if (bytes_read <= 0)
            {
                break;
            }
            // For now, we only send messages, not receive them
        }
    }
    else
    {
        std::string method = request.substr(0, request.find(' '));
        std::string path;
        {
            size_t start = request.find(' ') + 1;
            size_t end = request.find(' ', start);
            if (start != std::string::npos && end != std::string::npos)
                path = request.substr(start, end - start);
        }
        // CORS preflight
        if (method == "OPTIONS")
        {
            std::string headers =
                "HTTP/1.1 204 No Content\r\n"
                "Access-Control-Allow-Origin: *\r\n"
                "Access-Control-Allow-Methods: GET, OPTIONS\r\n"
                "Access-Control-Allow-Headers: *\r\n"
                "Content-Length: 0\r\n\r\n";
            send(client_socket, headers.c_str(), headers.size(), 0);
        }
        else
        {
            std::string body;
            if (http_handler_)
            {
                body = http_handler_(method, path, request);
            }
            if (body.empty())
                body = "TradePulse WebSocket Server";
            std::ostringstream resp;
            resp << "HTTP/1.1 200 OK\r\n"
                 << "Content-Type: text/plain\r\n"
                 << "Access-Control-Allow-Origin: *\r\n"
                 << "Access-Control-Allow-Methods: GET, OPTIONS\r\n"
                 << "Access-Control-Allow-Headers: *\r\n"
                 << "Content-Length: " << body.size() << "\r\n\r\n"
                 << body;
            auto s = resp.str();
            send(client_socket, s.c_str(), s.size(), 0);
        }
    }

    // Clean up
    {
        std::lock_guard<std::mutex> lock(clients_mutex_);
        connected_clients_.erase(client_socket);
    }

    if (client_disconnected_callback_)
    {
        client_disconnected_callback_(client_socket);
    }

    close(client_socket);
}

std::string WebSocketServer::performWebSocketHandshake(const std::string &request)
{
    // Extract WebSocket-Key from request
    std::regex key_regex("Sec-WebSocket-Key: ([A-Za-z0-9+/=]+)");
    std::smatch match;

    if (!std::regex_search(request, match, key_regex))
    {
        return "HTTP/1.1 400 Bad Request\r\n\r\n";
    }

    std::string client_key = match[1].str();
    std::string magic_string = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";
    std::string accept_string = client_key + magic_string;

    // Calculate SHA-1 hash
    unsigned char hash[SHA_DIGEST_LENGTH];
    SHA1(reinterpret_cast<const unsigned char *>(accept_string.c_str()), accept_string.length(), hash);

    // Base64 encode
    BIO *bio = BIO_new(BIO_s_mem());
    BIO *b64 = BIO_new(BIO_f_base64());
    BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL);
    bio = BIO_push(b64, bio);

    BIO_write(bio, hash, SHA_DIGEST_LENGTH);
    BIO_flush(bio);

    BUF_MEM *buffer_ptr;
    BIO_get_mem_ptr(bio, &buffer_ptr);

    std::string accept_key(buffer_ptr->data, buffer_ptr->length);
    BIO_free_all(bio);

    // Create response
    std::ostringstream response;
    response << "HTTP/1.1 101 Switching Protocols\r\n"
             << "Upgrade: websocket\r\n"
             << "Connection: Upgrade\r\n"
             << "Sec-WebSocket-Accept: " << accept_key << "\r\n"
             << "\r\n";

    return response.str();
}

void WebSocketServer::sendWebSocketFrame(int client_socket, const std::string &message)
{
    std::string frame = createWebSocketFrame(message);
    int bytes_sent = send(client_socket, frame.c_str(), frame.length(), 0);
    if (bytes_sent <= 0)
    {
        throw std::runtime_error("Failed to send WebSocket frame");
    }
}

std::string WebSocketServer::createWebSocketFrame(const std::string &message)
{
    std::string frame;

    // First byte: FIN=1, opcode=0x1 (text frame)
    frame.push_back(0x81);

    // Payload length
    size_t payload_len = message.length();
    if (payload_len < 126)
    {
        frame.push_back(static_cast<char>(payload_len));
    }
    else if (payload_len < 65536)
    {
        frame.push_back(126);
        frame.push_back(static_cast<char>((payload_len >> 8) & 0xFF));
        frame.push_back(static_cast<char>(payload_len & 0xFF));
    }
    else
    {
        frame.push_back(127);
        for (int i = 7; i >= 0; i--)
        {
            frame.push_back(static_cast<char>((payload_len >> (i * 8)) & 0xFF));
        }
    }

    // Payload
    frame.append(message);

    return frame;
}

std::string WebSocketServer::messageToJson(const WebSocketMessage &message)
{
    std::ostringstream json;
    json << std::fixed << std::setprecision(6);
    json << "{"
         << "\"type\":\"" << message.type << "\","
         << "\"venue\":\"" << message.venue << "\","
         << "\"symbol\":\"" << message.symbol << "\","
         << "\"side\":\"" << message.action << "\","
         << "\"price\":" << message.price << ","
         << "\"size\":" << message.size << ","
         << "\"pnl\":" << message.pnl << ","
         << "\"orderId\":\"" << message.order_id << "\","
         << "\"modelled_latency_ms\":" << message.modelled_latency_ms << ","
         << "\"exchange_recv_ts_ms\":" << message.exchange_recv_ts_ms << ","
         << "\"ingest_ts_ms\":" << message.ingest_ts_ms << ","
         << "\"order_created_ts_ms\":" << message.order_created_ts_ms << ","
         << "\"order_executed_ts_ms\":" << message.order_executed_ts_ms << ","
         << "\"server_broadcast_ts_ms\":" << message.server_broadcast_ts_ms
         << "}";
    return json.str();
}

std::string WebSocketServer::heartbeatToJson(int64_t server_ts_ms)
{
    std::ostringstream json;
    json << "{\"type\":\"hb\",\"server_ts_ms\":" << server_ts_ms << "}";
    return json.str();
}