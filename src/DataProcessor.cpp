#include "DataProcessor.h"
#include <QDebug>

DataProcessor::DataProcessor(QObject *parent) : QObject(parent) {
    m_currentCandle.timestamp = 0;
}

void DataProcessor::setHistory(const QList<Candle> &history) {
    QMutexLocker locker(&m_mutex);
    m_candles = history;
    updateIndicators();
}

void DataProcessor::processTick(const Tick &tick) {
    QMutexLocker locker(&m_mutex);
    
    // Determine which candle this tick belongs to (1-minute buckets)
    qint64 candleStart = (tick.timestamp / CANDLE_INTERVAL_MS) * CANDLE_INTERVAL_MS;

    if (m_currentCandle.timestamp != candleStart) {
        // Close previous candle if exists and it's not the very first run (ts=0)
        if (m_currentCandle.timestamp != 0) {
            m_candles.append(m_currentCandle);
            emit candleUpdated(m_currentCandle); // Final update for closed candle
            
            // Recalculate indicators on candle close
            updateIndicators();
        }

        // Start new candle
        m_currentCandle.timestamp = candleStart;
        m_currentCandle.open = tick.price;
        m_currentCandle.high = tick.price;
        m_currentCandle.low = tick.price;
        m_currentCandle.close = tick.price;
        m_currentCandle.volume = 0; // Tick volume?
    } else {
        // Update current candle
        m_currentCandle.high = std::max(m_currentCandle.high, tick.price);
        m_currentCandle.low = std::min(m_currentCandle.low, tick.price);
        m_currentCandle.close = tick.price;
    }
    
    // Emitting updates for the forming candle allows real-time chart animation
    emit candleUpdated(m_currentCandle);
}

void DataProcessor::updateIndicators() {
    if (m_candles.isEmpty()) return;

    // Extract closing prices
    std::vector<double> closes;
    closes.reserve(m_candles.size());
    for(const auto& c : m_candles) {
        closes.push_back(c.close);
    }
    
    // Calc EMA
    std::vector<double> ema = TechnicalIndicators::calculateEMA(closes, m_emaPeriod);
    
    // Calc RSI
    std::vector<double> rsi = TechnicalIndicators::calculateRSI(closes, m_rsiPeriod);

    // Calc MACD
    auto macd = TechnicalIndicators::calculateMACD(closes);

    emit indicatorsUpdated(ema, rsi, macd);
}
