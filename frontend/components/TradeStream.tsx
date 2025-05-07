import React from 'react';
import { TradeData } from '../utils/websocket';

interface TradeStreamProps {
  trades: TradeData[];
  maxTrades?: number;
}

export const TradeStream: React.FC<TradeStreamProps> = ({ trades, maxTrades = 50 }) => {
  // Display only the most recent trades
  const recentTrades = trades.slice(-maxTrades).reverse();

  const formatTime = (timestamp: string) => {
    const date = new Date(timestamp);
    return date.toLocaleTimeString();
  };

  const formatPrice = (price: number) => {
    return price.toFixed(2);
  };

  const formatPnL = (pnl: number) => {
    const formatted = pnl.toFixed(2);
    return pnl >= 0 ? `+$${formatted}` : `-$${Math.abs(pnl).toFixed(2)}`;
  };

  return (
    <div className="chart-container">
      <h3 className="text-xl font-semibold mb-4">Live Trade Stream</h3>
      
      {trades.length === 0 ? (
        <div className="text-gray-400 text-center py-8">
          Waiting for trades...
        </div>
      ) : (
        <div className="overflow-y-auto max-h-96">
          <div className="space-y-2">
            {recentTrades.map((trade, index) => (
              <div
                key={`${trade.order_id}-${index}`}
                className="flex items-center justify-between p-3 bg-gray-800 rounded-lg hover:bg-gray-700 transition-colors"
              >
                <div className="flex items-center space-x-4">
                  <div className="flex flex-col">
                    <span className="text-sm font-medium">{trade.venue}</span>
                    <span className="text-xs text-gray-400">{formatTime(trade.timestamp)}</span>
                  </div>
                  
                  <div className="flex items-center space-x-2">
                    <span
                      className={`px-2 py-1 rounded text-xs font-medium ${
                        trade.action === 'BUY'
                          ? 'bg-trade-green bg-opacity-20 text-trade-green'
                          : 'bg-trade-red bg-opacity-20 text-trade-red'
                      }`}
                    >
                      {trade.action}
                    </span>
                    <span className="text-white font-medium">${formatPrice(trade.price)}</span>
                  </div>
                </div>
                
                <div className="flex items-center space-x-4">
                  <div className="text-right">
                    <div className="text-sm text-gray-400">PnL</div>
                    <div
                      className={`text-sm font-medium ${
                        trade.pnl >= 0 ? 'text-trade-green' : 'text-trade-red'
                      }`}
                    >
                      {formatPnL(trade.pnl)}
                    </div>
                  </div>
                  
                  <div className="text-right">
                    <div className="text-sm text-gray-400">Latency</div>
                    <div className="text-sm text-white">
                      {trade.latency_ms.toFixed(1)}ms
                    </div>
                  </div>
                </div>
              </div>
            ))}
          </div>
        </div>
      )}
    </div>
  );
}; 