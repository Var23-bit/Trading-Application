# Real-Time Trading Desktop Application (C++ / Qt)

A real-time stock market desktop application built using C++ and the Qt framework.
The application streams live market prices via WebSocket and renders real-time
candlestick charts with technical indicators while keeping the UI responsive
under high-frequency updates.

## Features
- Live market data streaming using WebSocket
- Real-time tick-by-tick price updates
- Candlestick charts with EMA, RSI, and MACD indicators
- Clean and modern UI built with Qt Widgets
- Responsive performance using multithreading

## System Architecture
- UI Layer: Qt Widgets for interface, QtCharts for real-time visualization
- Data Layer: WebSocket for live price feed, FinnHub API for initial/symbol data
- Processing Layer: Background worker threads (QThread) for data handling,
  with signal-slot based communication for safe UI updates

## Tech Stack
- C++: core application logic and data processing
- Qt: event-driven architecture and UI framework
- QtCharts: real-time financial chart rendering
- WebSocket: continuous streaming of live market prices
- FinnHub API: fetching reference and initial market data
- Multithreading (QThread): ensuring non-blocking UI under heavy data flow

## Performance & Responsiveness
- Market data processing runs in background threads
- UI remains responsive during high-frequency price updates
- Thread-safe updates using Qt signals and slots

## What I Learned
- Designing real-time, event-driven desktop applications
- Qt event loop and signal-slot communication
- Multithreading in C++ using QThread
- Integrating WebSocket and FinnHub APIs in Qt
- Building responsive UIs for high-frequency data systems
