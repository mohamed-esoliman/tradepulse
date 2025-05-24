export interface TradeData {
  type: 'trade';
  venue: string;
  symbol: string;
  side: 'BUY' | 'SELL';
  price: number;
  size: number;
  pnl: number;
  orderId: string;
  modelled_latency_ms: number;
  exchange_recv_ts_ms: number;
  ingest_ts_ms: number;
  order_created_ts_ms: number;
  order_executed_ts_ms: number;
  server_broadcast_ts_ms: number;
}

export interface LatencyData {
  type: 'latency';
  venue: string;
  modelled_latency_ms: number;
  ts: number;
}

export interface HeartbeatData {
  type: 'hb';
  server_ts_ms: number;
}

export class WebSocketClient {
  private ws: WebSocket | null = null;
  private url: string;
  private reconnectAttempts = 0;
  private maxReconnectAttempts = 5;
  private reconnectInterval = 1000;
  private onTradeCallback?: (trade: TradeData) => void;
  private onLatencyCallback?: (latency: LatencyData) => void;
  private onConnectCallback?: () => void;
  private onDisconnectCallback?: () => void;
  private onErrorCallback?: (error: Error) => void;
  private onHeartbeatCallback?: (hb: HeartbeatData) => void;
  public lastServerTsMs: number = 0;
  public lastMessageAtMs: number = 0;
  public drops: number = 0;

  constructor(url: string) {
    this.url = url;
  }

  connect(): Promise<void> {
    return new Promise((resolve, reject) => {
      try {
        this.ws = new WebSocket(this.url);

        this.ws.onopen = () => {
          console.log('WebSocket connected');
          this.reconnectAttempts = 0;
          this.onConnectCallback?.();
          resolve();
        };

        this.ws.onmessage = (event) => {
          try {
            const data = JSON.parse(event.data);
            this.handleMessage(data);
          } catch (error) {
            console.error('Error parsing WebSocket message:', error);
            this.drops++;
          }
        };

        this.ws.onclose = () => {
          console.log('WebSocket disconnected');
          this.onDisconnectCallback?.();
          this.attemptReconnect();
        };

        this.ws.onerror = (error) => {
          console.error('WebSocket error:', error);
          this.onErrorCallback?.(new Error('WebSocket connection error'));
          reject(error);
        };
      } catch (error) {
        reject(error);
      }
    });
  }

  private handleMessage(data: any) {
    const now = Date.now();
    this.lastMessageAtMs = now;
    if (data.type === 'hb') {
      const hb: HeartbeatData = { type: 'hb', server_ts_ms: data.server_ts_ms };
      this.lastServerTsMs = data.server_ts_ms || 0;
      this.onHeartbeatCallback?.(hb);
      return;
    }
    if (data.type === 'latency') {
      const latency: LatencyData = {
        type: 'latency',
        venue: data.venue,
        modelled_latency_ms: data.modelled_latency_ms,
        ts: data.server_broadcast_ts_ms || now,
      };
      this.onLatencyCallback?.(latency);
      return;
    }
    if (data.type === 'trade') {
      const trade: TradeData = {
        type: 'trade',
        venue: data.venue,
        symbol: data.symbol,
        side: data.side,
        price: data.price,
        size: data.size,
        pnl: data.pnl,
        orderId: data.orderId,
        modelled_latency_ms: data.modelled_latency_ms,
        exchange_recv_ts_ms: data.exchange_recv_ts_ms ?? -1,
        ingest_ts_ms: data.ingest_ts_ms ?? -1,
        order_created_ts_ms: data.order_created_ts_ms ?? -1,
        order_executed_ts_ms: data.order_executed_ts_ms ?? -1,
        server_broadcast_ts_ms: data.server_broadcast_ts_ms ?? now,
      };
      this.onTradeCallback?.(trade);
      return;
    }
  }

  private attemptReconnect() {
    if (this.reconnectAttempts < this.maxReconnectAttempts) {
      this.reconnectAttempts++;
      console.log(`Attempting to reconnect (${this.reconnectAttempts}/${this.maxReconnectAttempts})...`);
      
      setTimeout(() => {
        this.connect().catch(console.error);
      }, this.reconnectInterval * this.reconnectAttempts);
    } else {
      console.error('Max reconnection attempts reached');
    }
  }

  disconnect() {
    if (this.ws) {
      this.ws.close();
      this.ws = null;
    }
  }

  isConnected(): boolean {
    return this.ws?.readyState === WebSocket.OPEN;
  }

  onTrade(callback: (trade: TradeData) => void) {
    this.onTradeCallback = callback;
  }

  onLatency(callback: (latency: LatencyData) => void) {
    this.onLatencyCallback = callback;
  }

  onConnect(callback: () => void) {
    this.onConnectCallback = callback;
  }

  onDisconnect(callback: () => void) {
    this.onDisconnectCallback = callback;
  }

  onError(callback: (error: Error) => void) {
    this.onErrorCallback = callback;
  }

  onHeartbeat(callback: (hb: HeartbeatData) => void) {
    this.onHeartbeatCallback = callback;
  }
} 