#include "PortfolioManager.h"
#include "DatabaseManager.h"

#include <QSqlQuery>
#include <QSqlError>
#include <QVariant>

PortfolioManager::PortfolioManager(int userId, QObject *parent)
    : QObject(parent), m_userId(userId), m_cashBalance(100000.0)
{
    loadFromDb();
}

void PortfolioManager::loadFromDb() {
    auto &db = DatabaseManager::instance().db();

    QSqlQuery userQ(db);
    userQ.prepare("SELECT cash_balance FROM users WHERE id = :id");
    userQ.bindValue(":id", m_userId);
    if (userQ.exec() && userQ.next()) {
        m_cashBalance = userQ.value(0).toDouble();
    }

    m_positions.clear();
    QSqlQuery holdQ(db);
    holdQ.prepare("SELECT symbol, quantity, avg_price FROM holdings WHERE user_id = :id AND quantity > 0");
    holdQ.bindValue(":id", m_userId);
    holdQ.exec();
    while (holdQ.next()) {
        Position pos;
        pos.symbol = holdQ.value(0).toString();
        pos.quantity = holdQ.value(1).toDouble();
        pos.avgPrice = holdQ.value(2).toDouble();
        m_positions.insert(pos.symbol, pos);
    }
}

void PortfolioManager::setInitialCapital(double amount) {
    m_cashBalance = amount;
    auto &db = DatabaseManager::instance().db();
    QSqlQuery q(db);
    q.prepare("UPDATE users SET cash_balance = :cash WHERE id = :id");
    q.bindValue(":cash", amount);
    q.bindValue(":id", m_userId);
    q.exec();
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

void PortfolioManager::persistOrder(const QString &side, const QString &symbol, double price, double quantity) {
    auto &db = DatabaseManager::instance().db();

    QSqlQuery orderIns(db);
    orderIns.prepare(
        "INSERT INTO orders (user_id, symbol, side, quantity, price) VALUES (:uid, :sym, :side, :qty, :price)");
    orderIns.bindValue(":uid", m_userId);
    orderIns.bindValue(":sym", symbol);
    orderIns.bindValue(":side", side);
    orderIns.bindValue(":qty", quantity);
    orderIns.bindValue(":price", price);
    orderIns.exec();
    int orderId = orderIns.lastInsertId().toInt();

    QSqlQuery txIns(db);
    txIns.prepare(
        "INSERT INTO transactions (order_id, user_id, symbol, side, quantity, price) "
        "VALUES (:oid, :uid, :sym, :side, :qty, :price)");
    txIns.bindValue(":oid", orderId);
    txIns.bindValue(":uid", m_userId);
    txIns.bindValue(":sym", symbol);
    txIns.bindValue(":side", side);
    txIns.bindValue(":qty", quantity);
    txIns.bindValue(":price", price);
    txIns.exec();
}

bool PortfolioManager::buy(const QString &symbol, double price, double quantity) {
    double cost = price * quantity;
    if (m_cashBalance < cost) {
        emit orderRejected("Insufficient Funds");
        return false;
    }

    auto &db = DatabaseManager::instance().db();
    db.transaction();

    m_cashBalance -= cost;

    double newQty, newAvg;
    if (m_positions.contains(symbol)) {
        Position &pos = m_positions[symbol];
        double totalCost = (pos.quantity * pos.avgPrice) + cost;
        pos.quantity += quantity;
        pos.avgPrice = totalCost / pos.quantity;
        newQty = pos.quantity;
        newAvg = pos.avgPrice;
    } else {
        Position pos;
        pos.symbol = symbol;
        pos.quantity = quantity;
        pos.avgPrice = price;
        m_positions.insert(symbol, pos);
        newQty = quantity;
        newAvg = price;
    }

    QSqlQuery cashQ(db);
    cashQ.prepare("UPDATE users SET cash_balance = :cash WHERE id = :id");
    cashQ.bindValue(":cash", m_cashBalance);
    cashQ.bindValue(":id", m_userId);
    cashQ.exec();

    QSqlQuery holdQ(db);
    holdQ.prepare(
        "INSERT INTO holdings (user_id, symbol, quantity, avg_price) VALUES (:uid, :sym, :qty, :avg) "
        "ON CONFLICT(user_id, symbol) DO UPDATE SET quantity = :qty, avg_price = :avg");
    holdQ.bindValue(":uid", m_userId);
    holdQ.bindValue(":sym", symbol);
    holdQ.bindValue(":qty", newQty);
    holdQ.bindValue(":avg", newAvg);
    holdQ.exec();

    persistOrder("BUY", symbol, price, quantity);

    db.commit();

    emit tradeExecuted("BUY", symbol, price, quantity);
    emit portfolioUpdated();
    return true;
}

bool PortfolioManager::sell(const QString &symbol, double price, double quantity) {
    if (!m_positions.contains(symbol) || m_positions[symbol].quantity < quantity) {
        emit orderRejected("Insufficient Position");
        return false;
    }

    auto &db = DatabaseManager::instance().db();
    db.transaction();

    double revenue = price * quantity;
    m_cashBalance += revenue;

    Position &pos = m_positions[symbol];
    pos.quantity -= quantity;
    double remainingQty = pos.quantity;
    double avg = pos.avgPrice;
    if (pos.quantity <= 0.0001) {
        m_positions.remove(symbol);
        remainingQty = 0;
    }

    QSqlQuery cashQ(db);
    cashQ.prepare("UPDATE users SET cash_balance = :cash WHERE id = :id");
    cashQ.bindValue(":cash", m_cashBalance);
    cashQ.bindValue(":id", m_userId);
    cashQ.exec();

    QSqlQuery holdQ(db);
    holdQ.prepare("UPDATE holdings SET quantity = :qty, avg_price = :avg WHERE user_id = :uid AND symbol = :sym");
    holdQ.bindValue(":qty", remainingQty);
    holdQ.bindValue(":avg", avg);
    holdQ.bindValue(":uid", m_userId);
    holdQ.bindValue(":sym", symbol);
    holdQ.exec();

    persistOrder("SELL", symbol, price, quantity);

    db.commit();

    emit tradeExecuted("SELL", symbol, price, quantity);
    emit portfolioUpdated();
    return true;
}
