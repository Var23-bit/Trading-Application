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

// Same public interface as before (buy/sell/getCashBalance/getPositions),
// but now backed by SQLite via DatabaseManager instead of pure in-memory
// state. Pass the logged-in user's id so trades and balance persist across
// restarts, per account.
class PortfolioManager : public QObject {
    Q_OBJECT
public:
    explicit PortfolioManager(int userId, QObject *parent = nullptr);

    void setInitialCapital(double amount); // only used for accounts with no DB row yet
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
    void loadFromDb();
    void persistOrder(const QString &side, const QString &symbol, double price, double quantity);

    int m_userId;
    double m_cashBalance;
    QMap<QString, Position> m_positions; // Symbol -> Position, mirrors `holdings` table
};

#endif // PORTFOLIOMANAGER_H
