# TradePulse

A high-frequency trading simulation platform that demonstrates algorithmic trading strategies and real-time market data processing. TradePulse provides a comprehensive environment for understanding the mechanics of automated trading systems across multiple exchange venues.

### Key Features

- **Real-time Market Simulation**: Generates realistic price movements across multiple exchange venues
- **Momentum-based Trading Algorithm**: Implements algorithmic trading strategies based on price momentum analysis
- **Latency Modeling**: Simulates realistic network latency between exchanges to model HFT conditions
- **Live Dashboard**: Web-based interface providing real-time visualization of trading activity and performance metrics
- **Performance Analytics**: Comprehensive profit and loss tracking with historical data analysis

## Prerequisites

- **Operating System**: macOS or Linux
- **Node.js**: Version 18.0 or higher
- **Development Tools**: CMake, GCC compiler suite
- **Package Manager**: npm or yarn

## Installation & Setup

### 1. Clone the Repository

```bash
git clone https://github.com/mohamed-esoliman/tradepulse.git
cd tradepulse
```

### 2. Build the Trading Engine

```bash
cd backend
mkdir build && cd build
cmake ..
make
```

### 3. Install Frontend Dependencies

```bash
cd ../../frontend
npm install
```

## Running the Application

### Start the Trading Engine

```bash
cd backend/build
./tradepulse
```

The trading engine will start on port 8080 and begin generating market data.

### Launch the Dashboard

```bash
cd frontend
npm run dev
```

Access the dashboard at `http://localhost:3000`

## Dashboard Features

The web dashboard provides real-time monitoring of:

- **Trade Execution Stream**: Live feed of all buy/sell orders
- **Profit & Loss Charts**: Historical performance visualization
- **Latency Metrics**: Exchange-specific latency measurements
- **Market Data**: Real-time price movements across venues

## Configuration & Customization

### Trading Strategy Modification

Modify the trading algorithm by editing `backend/strategy.cpp`. The current implementation uses momentum-based signals but can be extended with additional technical indicators.

### Exchange Configuration

Add or modify exchange venues in `backend/market_feed.cpp`. Each exchange can be configured with specific latency characteristics and price behavior.

### Latency Parameters

Adjust network latency simulation parameters in `backend/latency.cpp` to model different network conditions and geographic distributions.

### Frontend Customization

Dashboard components are located in `frontend/components/` and can be modified to display additional metrics or customize the user interface.

## License

This project is licensed under the MIT License. See the LICENSE file for details.

## Contributing

Contributions are welcome. Please ensure all code follows the established patterns and includes appropriate documentation.