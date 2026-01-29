#include "PortfolioManager.h"

PortfolioManager::PortfolioManager(QObject *parent) 
    : QObject(parent), m_cashBalance(100000.0) // $100k Default Paper Money
{
}

void PortfolioManager::setInitialCapital(double amount) {
    m_cashBalance = amount;
    emit portfolioUpdated();
}

double PortfolioManager::getCashBalance() const {
    return m_cashBalance;
}

QMap<QString, Position> PortfolioManager::getPositions() const {
    return m_positions;
}

double PortfolioManager::getUnrealizedPnL(const QMap<QString, double> &currentPrices) const {
    double pnl = 0.0;
    for (auto it = m_positions.begin(); it != m_positions.end(); ++it) {
        if (currentPrices.contains(it.key())) {
            double currentVal = currentPrices[it.key()] * it.value().quantity;
            double costBasis = it.value().avgPrice * it.value().quantity;
            pnl += (currentVal - costBasis);
        }
    }
    return pnl;
}

bool PortfolioManager::buy(const QString &symbol, double price, double quantity) {
    double cost = price * quantity;
    if (m_cashBalance >= cost) {
        m_cashBalance -= cost;
        
        if (m_positions.contains(symbol)) {
            Position &pos = m_positions[symbol];
            double totalCost = (pos.quantity * pos.avgPrice) + cost;
            pos.quantity += quantity;
            pos.avgPrice = totalCost / pos.quantity;
        } else {
            Position pos;
            pos.symbol = symbol;
            pos.quantity = quantity;
            pos.avgPrice = price;
            m_positions.insert(symbol, pos);
        }
        
        emit tradeExecuted("BUY", symbol, price, quantity);
        emit portfolioUpdated();
        return true;
    } else {
        emit orderRejected("Insufficient Funds");
        return false;
    }
}

bool PortfolioManager::sell(const QString &symbol, double price, double quantity) {
    if (m_positions.contains(symbol)) {
        Position &pos = m_positions[symbol];
        if (pos.quantity >= quantity) {
            double revenue = price * quantity;
            m_cashBalance += revenue;
            
            pos.quantity -= quantity;
            if (pos.quantity <= 0.0001) {
                m_positions.remove(symbol);
            }
            
            emit tradeExecuted("SELL", symbol, price, quantity);
            emit portfolioUpdated();
            return true;
        }
    }
    emit orderRejected("Insufficient Position");
    return false;
}
