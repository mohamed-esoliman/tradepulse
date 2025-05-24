import React, { useMemo } from 'react';
import {
  Chart as ChartJS,
  CategoryScale,
  LinearScale,
  PointElement,
  LineElement,
  Title,
  Tooltip,
  Legend,
  ChartOptions,
} from 'chart.js';
import { Line } from 'react-chartjs-2';
import { TradeData } from '../utils/websocket';

ChartJS.register(
  CategoryScale,
  LinearScale,
  PointElement,
  LineElement,
  Title,
  Tooltip,
  Legend
);

interface PnLChartProps {
  trades: TradeData[];
  maxDataPoints?: number;
}

export const PnLChart: React.FC<PnLChartProps> = ({ trades, maxDataPoints = 100 }) => {
  const chartData = useMemo(() => {
    // Calculate cumulative PnL
    let cumulativePnL = 0;
    const pnlData = trades.slice(-maxDataPoints).map(trade => {
      cumulativePnL += trade.pnl;
      return {
        ts_ms: trade.server_broadcast_ts_ms || trade.ingest_ts_ms || Date.now(),
        cumulativePnL,
        tradePnL: trade.pnl,
      };
    });

    const labels = pnlData.map(data => {
      const date = new Date(data.ts_ms);
      return date.toLocaleTimeString();
    });

    const datasets = [
      {
        label: 'Cumulative PnL',
        data: pnlData.map(data => data.cumulativePnL),
        borderColor: 'rgba(59, 130, 246, 1)',
        backgroundColor: 'rgba(59, 130, 246, 0.1)',
        borderWidth: 2,
        fill: true,
        tension: 0.1,
      },
    ];

    return {
      labels,
      datasets,
    };
  }, [trades, maxDataPoints]);

  const totalPnL = useMemo(() => {
    return trades.reduce((sum, trade) => sum + trade.pnl, 0);
  }, [trades]);

  const options: ChartOptions<'line'> = {
    responsive: true,
    maintainAspectRatio: false,
    plugins: {
      legend: {
        position: 'top' as const,
        labels: {
          color: 'white',
        },
      },
      title: {
        display: true,
        text: 'Cumulative Profit & Loss',
        color: 'white',
      },
    },
    scales: {
      x: {
        ticks: {
          color: 'white',
        },
        grid: {
          color: 'rgba(255, 255, 255, 0.1)',
        },
      },
      y: {
        ticks: {
          color: 'white',
          callback: function(value) {
            return '$' + Number(value).toFixed(2);
          },
        },
        grid: {
          color: 'rgba(255, 255, 255, 0.1)',
        },
        title: {
          display: true,
          text: 'PnL ($)',
          color: 'white',
        },
      },
    },
  };

  const formatPnL = (pnl: number) => {
    const formatted = Math.abs(pnl).toFixed(2);
    return pnl >= 0 ? `+$${formatted}` : `-$${formatted}`;
  };

  return (
    <div className="chart-container">
      <div className="flex justify-between items-center mb-4">
        <h3 className="text-xl font-semibold">Profit & Loss</h3>
        <div className="metric-card">
          <div className="text-sm text-gray-400">Total PnL</div>
          <div
            className={`text-lg font-bold ${
              totalPnL >= 0 ? 'text-trade-green' : 'text-trade-red'
            }`}
          >
            {formatPnL(totalPnL)}
          </div>
        </div>
      </div>
      
      {trades.length === 0 ? (
        <div className="text-gray-400 text-center py-8">
          Waiting for PnL data...
        </div>
      ) : (
        <div className="h-64">
          <Line data={chartData} options={options} />
        </div>
      )}
    </div>
  );
}; 