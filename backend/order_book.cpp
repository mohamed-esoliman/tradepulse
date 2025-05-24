#include "order_book.h"
#include <iostream>
#include <sstream>
#include <algorithm>

OrderBook::OrderBook() : total_pnl_(0.0), trade_counter_(0)
{
}

OrderBook::~OrderBook()
{
}

void OrderBook::submitOrder(const Order &order)
{
    processOrder(order);
}

void OrderBook::setTradeCallback(std::function<void(const Trade &)> callback)
{
    trade_callback_ = callback;
}

double OrderBook::getTotalPnL() const
{
    return total_pnl_;
}

std::vector<Trade> OrderBook::getRecentTrades(int count) const
{
    std::vector<Trade> recent_trades;
    int start_idx = std::max(0, static_cast<int>(trades_.size()) - count);

    for (int i = start_idx; i < static_cast<int>(trades_.size()); ++i)
    {
        recent_trades.push_back(trades_[i]);
    }

    return recent_trades;
}

void OrderBook::processOrder(const Order &order)
{
    // Simple market order execution at current price
    Trade trade;
    trade.id = generateTradeId();
    trade.venue = order.venue;
    trade.side = order.side;
    trade.price = order.price;
    trade.quantity = order.quantity;
    trade.timestamp = std::chrono::system_clock::now();
    trade.size = static_cast<double>(order.quantity);
    trade.symbol = order.symbol;
    trade.order_created_ts_ms = std::chrono::duration_cast<std::chrono::milliseconds>(order.timestamp.time_since_epoch()).count();
    trade.order_executed_ts_ms = std::chrono::duration_cast<std::chrono::milliseconds>(trade.timestamp.time_since_epoch()).count();
    trade.server_broadcast_ts_ms = -1;
    trade.exchange_recv_ts_ms = order.exchange_recv_ts_ms;
    trade.ingest_ts_ms = order.ingest_ts_ms;
    trade.modelled_latency_ms = 0.0;

    // Calculate PnL based on position changes
    double pnl = 0.0;
    int &position = positions_[order.venue];
    double &avg_price = avg_prices_[order.venue];

    if (order.side == OrderSide::BUY)
    {
        if (position < 0)
        {
            // Closing short position
            int close_qty = std::min(order.quantity, -position);
            pnl += close_qty * (avg_price - order.price);
            position += close_qty;

            // Opening long position with remainder
            int open_qty = order.quantity - close_qty;
            if (open_qty > 0)
            {
                avg_price = (avg_price * position + order.price * open_qty) / (position + open_qty);
                position += open_qty;
            }
        }
        else
        {
            // Adding to long position
            avg_price = (avg_price * position + order.price * order.quantity) / (position + order.quantity);
            position += order.quantity;
        }
    }
    else
    { // SELL
        if (position > 0)
        {
            // Closing long position
            int close_qty = std::min(order.quantity, position);
            pnl += close_qty * (order.price - avg_price);
            position -= close_qty;

            // Opening short position with remainder
            int open_qty = order.quantity - close_qty;
            if (open_qty > 0)
            {
                avg_price = (avg_price * position + order.price * open_qty) / (position - open_qty);
                position -= open_qty;
            }
        }
        else
        {
            // Adding to short position
            avg_price = (avg_price * (-position) + order.price * order.quantity) / (-position + order.quantity);
            position -= order.quantity;
        }
    }

    trade.pnl = pnl;
    total_pnl_ += pnl;

    trades_.push_back(trade);
    last_prices_[order.venue] = order.price;

    // Call the callback if set
    if (trade_callback_)
    {
        trade_callback_(trade);
    }
}

std::string OrderBook::generateTradeId()
{
    std::ostringstream oss;
    oss << "T" << (++trade_counter_);
    return oss.str();
}