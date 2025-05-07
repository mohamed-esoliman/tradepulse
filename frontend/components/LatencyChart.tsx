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
    // Group trades by venue
    const venueData: { [venue: string]: { latency: number; timestamp: string }[] } = {};
    
    trades.slice(-maxDataPoints).forEach(trade => {
      if (!venueData[trade.venue]) {
        venueData[trade.venue] = [];
      }
      venueData[trade.venue].push({
        latency: trade.latency_ms,
        timestamp: trade.timestamp,
      });
    });

    // Create labels from timestamps
    const labels = trades
      .slice(-maxDataPoints)
      .map(trade => {
        const date = new Date(trade.timestamp);
        return date.toLocaleTimeString();
      });

    // Create datasets for each venue
    const datasets = Object.entries(venueData).map(([venue, data], index) => {
      const colors = [
        'rgba(59, 130, 246, 0.8)',  // blue
        'rgba(16, 185, 129, 0.8)',  // green
        'rgba(239, 68, 68, 0.8)',   // red
        'rgba(245, 158, 11, 0.8)',  // yellow
      ];
      
      return {
        label: venue,
        data: data.map(d => d.latency),
        borderColor: colors[index % colors.length],
        backgroundColor: colors[index % colors.length].replace('0.8', '0.2'),
        borderWidth: 2,
        fill: false,
        tension: 0.1,
      };
    });

    return {
      labels,
      datasets,
    };
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