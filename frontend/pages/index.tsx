import React, { useEffect } from 'react';
import { useRouter } from 'next/router';
import Head from 'next/head';

export default function Home() {
  const router = useRouter();

  useEffect(() => {
    // Redirect to dashboard
    router.push('/dashboard');
  }, [router]);

  return (
    <>
      <Head>
        <title>TradePulse - HFT Trading Simulator</title>
        <meta name="description" content="Real-time high-frequency trading simulator" />
        <meta name="viewport" content="width=device-width, initial-scale=1" />
        <link rel="icon" href="/favicon.ico" />
      </Head>

      <div className="min-h-screen bg-bg-dark text-white flex items-center justify-center">
        <div className="text-center">
          <h1 className="text-4xl font-bold text-trade-blue mb-4">TradePulse</h1>
          <p className="text-gray-400 mb-8">High-Frequency Trading Simulator</p>
          <div className="animate-spin rounded-full h-8 w-8 border-b-2 border-trade-blue mx-auto"></div>
          <p className="text-sm text-gray-400 mt-4">Redirecting to dashboard...</p>
        </div>
      </div>
    </>
  );
} 