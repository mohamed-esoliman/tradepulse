import React, { useState, useEffect, useCallback } from 'react';
import Head from 'next/head';
import { WebSocketClient, TradeData, HeartbeatData } from '../utils/websocket';
import { TradeStream } from '../components/TradeStream';
import { LatencyChart } from '../components/LatencyChart';
import { PnLChart } from '../components/PnLChart';

export default function Dashboard() {
  const [trades, setTrades] = useState<TradeData[]>([]);
  const [connectionStatus, setConnectionStatus] = useState<'disconnected' | 'connecting' | 'connected'>('disconnected');
  const [wsClient, setWsClient] = useState<WebSocketClient | null>(null);
  const [stats, setStats] = useState({
    totalTrades: 0,
    totalPnL: 0,
    avgModelledLatency: 0,
    rttMs: 0,
    lastMessageAgeMs: 0,
    activeVenues: new Set<string>(),
    strategy: 'unknown',
    lookback: 0,
    orderQty: 0,
    source: 'synthetic',
    symbol: 'BTC-USD',
    running: 0,
  });

  // Initialize WebSocket connection
  useEffect(() => {
    const client = new WebSocketClient('ws://127.0.0.1:8080');
    setWsClient(client);

    client.onConnect(() => {
      setConnectionStatus('connected');
      console.log('Connected to TradePulse backend');
    });

    client.onDisconnect(() => {
      setConnectionStatus('disconnected');
      console.log('Disconnected from TradePulse backend');
    });

    client.onError((error) => {
      console.error('WebSocket error:', error);
      setConnectionStatus('disconnected');
    });

    client.onTrade((trade) => {
      setTrades(prevTrades => [...prevTrades, trade]);
    });

    client.onHeartbeat((hb: HeartbeatData) => {
      const now = Date.now();
      const rtt = Math.max(0, now - hb.server_ts_ms);
      setStats(prev => ({ ...prev, rttMs: rtt }));
    });

    // Attempt to connect
    setConnectionStatus('connecting');
    client.connect().catch(console.error);

    return () => {
      client.disconnect();
    };
  }, []);

  // Fetch server info for strategy display
  useEffect(() => {
    async function fetchInfo() {
      try {
        const res = await fetch('http://127.0.0.1:8080/info');
        const text = await res.text();
        const map = Object.fromEntries(text.split('\n').filter(Boolean).map(line => {
          const idx = line.indexOf('=');
          return [line.slice(0, idx), line.slice(idx + 1)];
        }));
        setStats(prev => ({
          ...prev,
          strategy: map['strategy'] || 'unknown',
          lookback: Number(map['lookback'] || 0),
          orderQty: Number(map['order_qty'] || 0),
          source: map['source'] || 'synthetic',
          symbol: map['symbol'] || 'BTC-USD',
          running: Number(map['running'] || 0),
        }));
      } catch {}
    }
    fetchInfo();
  }, []);

  // Update stats when trades change
  useEffect(() => {
    const totalPnL = trades.reduce((sum, t) => sum + t.pnl, 0);
    const avgModelledLatency = trades.length > 0 ? trades.reduce((s, t) => s + (t.modelled_latency_ms || 0), 0) / trades.length : 0;
    const activeVenues = new Set(trades.map(t => t.venue));
    const now = Date.now();
    const lastMessageAgeMs = wsClient ? Math.max(0, now - (wsClient.lastMessageAtMs || now)) : 0;
    setStats(prev => ({
      ...prev,
      totalTrades: trades.length,
      totalPnL,
      avgModelledLatency,
      lastMessageAgeMs,
      activeVenues,
    }));
  }, [trades]);

  const formatPnL = (pnl: number) => {
    const formatted = Math.abs(pnl).toFixed(2);
    return pnl >= 0 ? `+$${formatted}` : `-$${formatted}`;
  };

  const getConnectionStatusColor = () => {
    switch (connectionStatus) {
      case 'connected':
        return 'text-trade-green';
      case 'connecting':
        return 'text-yellow-500';
      case 'disconnected':
        return 'text-trade-red';
      default:
        return 'text-gray-400';
    }
  };

  const getConnectionStatusText = () => {
    switch (connectionStatus) {
      case 'connected':
        return 'Connected';
      case 'connecting':
        return 'Connecting...';
      case 'disconnected':
        return 'Disconnected';
      default:
        return 'Unknown';
    }
  };

  const clearData = useCallback(() => {
    setTrades([]);
  }, []);

  return (
    <>
      <Head>
        <title>TradePulse - HFT Trading Dashboard</title>
        <meta name="description" content="Real-time high-frequency trading simulator dashboard" />
        <meta name="viewport" content="width=device-width, initial-scale=1" />
        <link rel="icon" href="/favicon.ico" />
      </Head>

      <div className="min-h-screen bg-bg-dark text-white">
        {/* Header */}
        <header className="bg-bg-card shadow-lg">
          <div className="max-w-7xl mx-auto px-4 sm:px-6 lg:px-8">
            <div className="flex justify-between items-center h-16">
              <div className="flex items-center">
                <h1 className="text-2xl font-bold text-trade-blue">TradePulse</h1>
                <span className="ml-2 text-sm text-gray-400">HFT Simulator</span>
              </div>
              <div className="hidden md:flex items-center space-x-4">
                <div className="text-xs text-gray-300">Strategy: <span className="font-semibold text-white">{stats.strategy}</span></div>
                <div className="text-xs text-gray-300">Source: <span className="font-semibold text-white">{stats.source}</span></div>
                <div className="text-xs text-gray-300">Lookback: <span className="font-semibold text-white">{stats.lookback}</span></div>
                <div className="text-xs text-gray-300">Order Qty: <span className="font-semibold text-white">{stats.orderQty}</span></div>
              </div>
              
              <div className="flex items-center space-x-4">
                <div className="flex items-center space-x-2">
                  <div className={`w-3 h-3 rounded-full ${
                    connectionStatus === 'connected' ? 'bg-trade-green' : 
                    connectionStatus === 'connecting' ? 'bg-yellow-500' : 'bg-trade-red'
                  }`}></div>
                  <span className={`text-sm ${getConnectionStatusColor()}`}>
                    {getConnectionStatusText()}
                  </span>
                </div>
                <div className="text-xs text-gray-400">RTT: {stats.rttMs}ms</div>
                <div className="text-xs text-gray-400">Last msg: {stats.lastMessageAgeMs}ms ago</div>
                
                <button
                  onClick={clearData}
                  className="px-3 py-1 bg-gray-600 hover:bg-gray-500 rounded text-sm transition-colors"
                >
                  Clear Data
                </button>
              </div>
            </div>
          </div>
        </header>

        {/* Main Content */}
        <main className="max-w-7xl mx-auto px-4 sm:px-6 lg:px-8 py-8">
          {/* Stats Cards */}
          <div className="grid grid-cols-1 md:grid-cols-4 gap-6 mb-8">
            <div className="metric-card">
              <div className="text-sm text-gray-400">Total Trades</div>
              <div className="text-2xl font-bold text-white">{stats.totalTrades}</div>
            </div>
            
            <div className="metric-card">
              <div className="text-sm text-gray-400">Total PnL</div>
              <div className={`text-2xl font-bold ${
                stats.totalPnL >= 0 ? 'text-trade-green' : 'text-trade-red'
              }`}>
                {formatPnL(stats.totalPnL)}
              </div>
            </div>
            
            <div className="metric-card">
              <div className="text-sm text-gray-400">Avg Modelled Latency</div>
              <div className="text-2xl font-bold text-white">
                {stats.avgModelledLatency.toFixed(1)}ms
              </div>
            </div>
            
            <div className="metric-card">
              <div className="text-sm text-gray-400">Active Venues</div>
              <div className="text-2xl font-bold text-white">{stats.activeVenues.size}</div>
            </div>
          </div>

          {/* Controls removed; backend controlled via CLI flags */}

          {/* Charts Grid */}
          <div className="grid grid-cols-1 xl:grid-cols-2 gap-8 mb-8">
            <PnLChart trades={trades} />
            <LatencyChart trades={trades} />
          </div>

          {/* Trade Stream */}
          <div className="mb-8">
            <TradeStream trades={trades} />
          </div>

          {/* Connection Status Info */}
          {connectionStatus === 'disconnected' && (
            <div className="bg-trade-red bg-opacity-10 border border-trade-red rounded-lg p-4 text-center">
              <div className="text-trade-red font-medium">
                Not connected to TradePulse backend
              </div>
              <div className="text-sm text-gray-400 mt-2">
                Make sure the C++ backend is running on port 8080
              </div>
            </div>
          )}
        </main>
      </div>
    </>
  );
} 