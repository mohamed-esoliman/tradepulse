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

interface LatencyChartProps {
  trades: TradeData[];
  maxDataPoints?: number;
}

export const LatencyChart: React.FC<LatencyChartProps> = ({ trades, maxDataPoints = 50 }) => {
  const chartData = useMemo(() => {
    const recent = trades.slice(-maxDataPoints);
    const labels = recent.map(t => new Date(t.server_broadcast_ts_ms || Date.now()).toLocaleTimeString());
    const venues = Array.from(new Set(recent.map(t => t.venue)));
    const colors = [
      'rgba(59, 130, 246, 0.8)',
      'rgba(16, 185, 129, 0.8)',
      'rgba(239, 68, 68, 0.8)',
      'rgba(245, 158, 11, 0.8)'
    ];

    const datasets: any[] = [];
    venues.forEach((venue, idx) => {
      const venueTrades = recent.filter(t => t.venue === venue);
      const modelled = venueTrades.map(t => t.modelled_latency_ms || 0);
      const execLatency = venueTrades.map(t => {
        if (t.order_executed_ts_ms && t.order_created_ts_ms) {
          return Math.max(0, t.order_executed_ts_ms - t.order_created_ts_ms);
        }
        return 0;
      });
      datasets.push({
        label: `${venue} • Modelled`,
        data: modelled,
        borderColor: colors[idx % colors.length],
        backgroundColor: colors[idx % colors.length].replace('0.8', '0.2'),
        borderWidth: 2,
        fill: false,
        tension: 0.1,
      });
      datasets.push({
        label: `${venue} • Measured Exec`,
        data: execLatency,
        borderColor: colors[(idx + 1) % colors.length],
        backgroundColor: colors[(idx + 1) % colors.length].replace('0.8', '0.2'),
        borderDash: [6, 4],
        borderWidth: 2,
        fill: false,
        tension: 0.1,
      });
    });

    return { labels, datasets };
  }, [trades, maxDataPoints]);

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
        text: 'Latency by Venue Over Time',
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
        },
        grid: {
          color: 'rgba(255, 255, 255, 0.1)',
        },
        title: {
          display: true,
          text: 'Latency (ms)',
          color: 'white',
        },
      },
    },
  };

  return (
    <div className="chart-container">
      <h3 className="text-xl font-semibold mb-4">Latency Chart</h3>
      
      {trades.length === 0 ? (
        <div className="text-gray-400 text-center py-8">
          Waiting for latency data...
        </div>
      ) : (
        <div className="h-64">
          <Line data={chartData} options={options} />
        </div>
      )}
    </div>
  );
}; 