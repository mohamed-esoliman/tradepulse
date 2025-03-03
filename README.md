# TradePulse 📈

A fun trading simulator that shows how high-frequency trading works in real time! Watch as algorithms make trades across different stock exchanges while you see the profits and losses update live.

## What does it do?

TradePulse simulates a simple trading bot that:

- Watches fake stock prices from multiple exchanges (NASDAQ, NYSE, etc.)
- Makes buy/sell decisions based on price momentum
- Shows you everything happening in real time on a web dashboard
- Tracks how much money it's making or losing

It's built with C++ for the trading engine (because speed matters!) and a Next.js web interface so you can watch it all happen.

## Quick Start

### You'll need:

- A Mac or Linux computer
- Node.js installed
- Basic development tools (cmake, gcc)

### Get it running:

**1. Get the code:**

```bash
git clone <your-repo-url>
cd tradepulse
```

**2. Build the trading engine:**

```bash
cd backend
mkdir build && cd build
cmake .. && make
```

**3. Set up the web interface:**

```bash
cd ../../frontend
npm install
```

**4. Start everything:**

First, start the trading engine:

```bash
cd backend/build
./tradepulse
```

Then start the web interface:

```bash
cd frontend
npm run dev
```

Open http://localhost:3000 and watch the magic happen!

## How it works

**The Trading Bot:**

- Generates fake stock prices that move like real ones
- Looks for momentum (when prices keep going up or down)
- Buys when it sees upward momentum, sells when it sees downward momentum
- Each exchange has different delays (latency) to make it realistic

**What you see:**

- Live trades happening in real time
- Charts showing profit/loss over time
- Latency measurements for each exchange
- A stream of all trades being executed

## Want to tinker with it?

The cool thing about this simulator is you can easily modify how it works:

- **Change the trading strategy**: Edit `backend/strategy.cpp` to try different approaches
- **Add new exchanges**: Modify `backend/market_feed.cpp` to include more venues
- **Adjust latency**: Play with delays in `backend/latency.cpp`
- **Customize the dashboard**: The frontend code is all in `frontend/components/`

## License

MIT License - feel free to use this however you want!

---