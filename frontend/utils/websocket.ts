export interface TradeData {
  venue: string;
  price: number;
  action: 'BUY' | 'SELL';
  latency_ms: number;
  timestamp: string;
  pnl: number;
  order_id: string;
}

export interface LatencyData {
  venue: string;
  latency_ms: number;
  timestamp: string;
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
    // Check if it's a trade message
    if (data.venue && data.price && data.action) {
      const trade: TradeData = {
        venue: data.venue,
        price: data.price,
        action: data.action,
        latency_ms: data.latency_ms,
        timestamp: data.timestamp,
        pnl: data.pnl,
        order_id: data.order_id,
      };
      this.onTradeCallback?.(trade);
    }

    // Check if it's a latency message
    if (data.venue && data.latency_ms && !data.price) {
      const latency: LatencyData = {
        venue: data.venue,
        latency_ms: data.latency_ms,
        timestamp: data.timestamp,
      };
      this.onLatencyCallback?.(latency);
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
} 