#ifndef DATAPROCESSOR_H
#define DATAPROCESSOR_H

#include <QObject>
#include <QThread>
#include <QMutex>
#include "MarketData.h"
#include "TechnicalIndicators.h"

class DataProcessor : public QObject {
    Q_OBJECT
public:
    explicit DataProcessor(QObject *parent = nullptr);

public slots:
    void processTick(const Tick &tick);
    void setHistory(const QList<Candle> &history);

signals:
    void candleUpdated(const Candle &candle);
    void indicatorsUpdated(const std::vector<double> &ema, 
                           const std::vector<double> &rsi,
                           const std::pair<std::vector<double>, std::vector<double>> &macd);

private:
    void updateIndicators();

    QList<Candle> m_candles;
    Candle m_currentCandle;
    QMutex m_mutex;
    
    // Config
    int m_emaPeriod = 20;
    int m_rsiPeriod = 14;
    
    // We aggregate ticks into 1-minute candles for this example
    const int CANDLE_INTERVAL_MS = 60 * 1000; 
};

#endif // DATAPROCESSOR_H
