#ifndef PORTFOLIOMANAGER_H
#define PORTFOLIOMANAGER_H

#include <QObject>
#include <QMap>
#include <QDateTime>

struct Position {
    QString symbol;
    double quantity;
    double avgPrice;
};

class PortfolioManager : public QObject {
    Q_OBJECT
public:
    explicit PortfolioManager(QObject *parent = nullptr);
    
    void setInitialCapital(double amount);
    double getCashBalance() const;
    double getUnrealizedPnL(const QMap<QString, double> &currentPrices) const;
    QMap<QString, Position> getPositions() const;

    // Trading Actions
    bool buy(const QString &symbol, double price, double quantity);
    bool sell(const QString &symbol, double price, double quantity);

signals:
    void portfolioUpdated(); // Balance or positions changed
    void tradeExecuted(QString type, QString symbol, double price, double qty);
    void orderRejected(QString reason);

private:
    double m_cashBalance;
    QMap<QString, Position> m_positions; // Symbol -> Position
};

#endif // PORTFOLIOMANAGER_H
