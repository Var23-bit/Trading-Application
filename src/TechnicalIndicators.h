#ifndef TECHNICALINDICATORS_H
#define TECHNICALINDICATORS_H

#include <vector>
#include <utility>

class TechnicalIndicators {
public:
    // Exponential Moving Average
    static std::vector<double> calculateEMA(const std::vector<double>& prices, int period);

    // Relative Strength Index
    static std::vector<double> calculateRSI(const std::vector<double>& prices, int period = 14);

    // Moving Average Convergence Divergence
    // Returns pair <MACD line, Signal line>
    static std::pair<std::vector<double>, std::vector<double>> calculateMACD(
        const std::vector<double>& prices, 
        int shortPeriod = 12, 
        int longPeriod = 26, 
        int signalPeriod = 9
    );
};

#endif // TECHNICALINDICATORS_H
