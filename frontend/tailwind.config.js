/** @type {import('tailwindcss').Config} */
module.exports = {
  content: [
    './pages/**/*.{js,ts,jsx,tsx,mdx}',
    './components/**/*.{js,ts,jsx,tsx,mdx}',
    './app/**/*.{js,ts,jsx,tsx,mdx}',
  ],
  theme: {
    extend: {
      colors: {
        'trade-green': '#10b981',
        'trade-red': '#ef4444',
        'trade-blue': '#3b82f6',
        'bg-dark': '#1f2937',
        'bg-card': '#374151',
      },
    },
  },
  plugins: [],
} 