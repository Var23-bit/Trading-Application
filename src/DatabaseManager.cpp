#include "DatabaseManager.h"

#include <QDir>
#include <QSqlError>
#include <QSqlQuery>
#include <QStandardPaths>
#include <QDebug>

DatabaseManager& DatabaseManager::instance() {
    static DatabaseManager mgr;
    return mgr;
}

bool DatabaseManager::init(const QString &dbPath) {
    if (m_initialized) return true;

    QString path = dbPath;
    if (path.isEmpty()) {
        QString dir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
        QDir().mkpath(dir);
        path = dir + "/trading_app.db";
    }

    m_db = QSqlDatabase::addDatabase("QSQLITE", "main_connection");
    m_db.setDatabaseName(path);

    if (!m_db.open()) {
        qCritical() << "Failed to open database:" << m_db.lastError().text();
        return false;
    }

    QSqlQuery pragma(m_db);
    pragma.exec("PRAGMA foreign_keys = ON;");

    if (!initSchema()) {
        qCritical() << "Failed to initialize schema";
        return false;
    }

    m_initialized = true;
    qInfo() << "Database ready at" << path;
    return true;
}

bool DatabaseManager::initSchema() {
    static const char* kSchema[] = {
        R"(CREATE TABLE IF NOT EXISTS users (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            username TEXT UNIQUE NOT NULL,
            password_hash TEXT NOT NULL,
            salt TEXT NOT NULL,
            cash_balance REAL NOT NULL DEFAULT 100000.0,
            created_at TEXT NOT NULL DEFAULT (datetime('now'))
        ))",
        R"(CREATE TABLE IF NOT EXISTS holdings (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            user_id INTEGER NOT NULL REFERENCES users(id),
            symbol TEXT NOT NULL,
            quantity REAL NOT NULL DEFAULT 0,
            avg_price REAL NOT NULL DEFAULT 0.0,
            UNIQUE(user_id, symbol)
        ))",
        R"(CREATE TABLE IF NOT EXISTS orders (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            user_id INTEGER NOT NULL REFERENCES users(id),
            symbol TEXT NOT NULL,
            side TEXT NOT NULL CHECK(side IN ('BUY','SELL')),
            quantity REAL NOT NULL,
            price REAL NOT NULL,
            status TEXT NOT NULL DEFAULT 'FILLED',
            created_at TEXT NOT NULL DEFAULT (datetime('now'))
        ))",
        R"(CREATE TABLE IF NOT EXISTS transactions (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            order_id INTEGER NOT NULL REFERENCES orders(id),
            user_id INTEGER NOT NULL REFERENCES users(id),
            symbol TEXT NOT NULL,
            side TEXT NOT NULL,
            quantity REAL NOT NULL,
            price REAL NOT NULL,
            executed_at TEXT NOT NULL DEFAULT (datetime('now'))
        ))",
        "CREATE INDEX IF NOT EXISTS idx_holdings_user ON holdings(user_id)",
        "CREATE INDEX IF NOT EXISTS idx_orders_user ON orders(user_id)",
        "CREATE INDEX IF NOT EXISTS idx_transactions_user ON transactions(user_id)"
    };

    QSqlQuery q(m_db);
    for (const char* stmt : kSchema) {
        if (!q.exec(stmt)) {
            qCritical() << "Schema statement failed:" << q.lastError().text();
            return false;
        }
    }
    return true;
}

QSqlDatabase& DatabaseManager::db() {
    return m_db;
}
