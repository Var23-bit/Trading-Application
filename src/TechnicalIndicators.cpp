#include "TechnicalIndicators.h"
#include <numeric>
#include <cmath>
#include <algorithm>

std::vector<double> TechnicalIndicators::calculateEMA(const std::vector<double>& prices, int period) {
    std::vector<double> ema;
    if (prices.empty() || period <= 0) return ema;

    ema.resize(prices.size());
    double multiplier = 2.0 / (period + 1);

    // Start with SMA for the first 'period' elements
    if (prices.size() < static_cast<size_t>(period)) {
        // Not enough data, just fill with 0 or behave gracefully
        std::fill(ema.begin(), ema.end(), 0.0);
        return ema;
    }

    double sum = 0.0;
    for (int i = 0; i < period; ++i) {
        sum += prices[i];
    }
    ema[period - 1] = sum / period;

    // Calculate the rest
    for (size_t i = period; i < prices.size(); ++i) {
        ema[i] = (prices[i] - ema[i - 1]) * multiplier + ema[i - 1];
    }

    // Fill initial zeroes
    for (int i = 0; i < period - 1; ++i) {
        ema[i] = 0.0; 
    }

    return ema;
}

std::vector<double> TechnicalIndicators::calculateRSI(const std::vector<double>& prices, int period) {
    std::vector<double> rsi(prices.size(), 0.0);
    if (prices.size() <= static_cast<size_t>(period)) return rsi;

    double avgGain = 0.0;
    double avgLoss = 0.0;

    for (int i = 1; i <= period; ++i) {
        double change = prices[i] - prices[i - 1];
        if (change > 0) avgGain += change;
        else avgLoss += std::abs(change);
    }

    avgGain /= period;
    avgLoss /= period;

    if (avgLoss == 0) rsi[period] = 100;
    else {
        double rs = avgGain / avgLoss;
        rsi[period] = 100.0 - (100.0 / (1.0 + rs));
    }

    for (size_t i = period + 1; i < prices.size(); ++i) {
        double change = prices[i] - prices[i - 1];
        double gain = (change > 0) ? change : 0.0;
        double loss = (change < 0) ? std::abs(change) : 0.0;

        avgGain = (avgGain * (period - 1) + gain) / period;
        avgLoss = (avgLoss * (period - 1) + loss) / period;

        if (avgLoss == 0) rsi[i] = 100;
        else {
            double rs = avgGain / avgLoss;
            rsi[i] = 100.0 - (100.0 / (1.0 + rs));
        }
    }

    return rsi;
}

std::pair<std::vector<double>, std::vector<double>> TechnicalIndicators::calculateMACD(
    const std::vector<double>& prices, int shortPeriod, int longPeriod, int signalPeriod) 
{
    std::vector<double> shortEMA = calculateEMA(prices, shortPeriod);
    std::vector<double> longEMA = calculateEMA(prices, longPeriod);
    
    std::vector<double> macdLine(prices.size(), 0.0);
    
    // MACD Line = Short EMA - Long EMA
    for (size_t i = 0; i < prices.size(); ++i) {
        if (i >= static_cast<size_t>(longPeriod)) {
             macdLine[i] = shortEMA[i] - longEMA[i];
        }
    }

    // Signal Line = EMA of MACD Line
    // We only want to calc signal line on the valid part of MACD line
    // But calculateEMA expects a vector starting from 0. 
    // Ideally we treat the MACD line as the input. 
    // The initial 0s in MACD line might skew initial Signal line values, but it converges.
    std::vector<double> signalLine = calculateEMA(macdLine, signalPeriod);

    return {macdLine, signalLine};
}
