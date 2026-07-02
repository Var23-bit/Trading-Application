#ifndef DATABASEMANAGER_H
#define DATABASEMANAGER_H

#include <QSqlDatabase>
#include <QString>

// Singleton wrapper around the app's local SQLite connection.
// Not thread-safe on its own -- if a QThread needs DB access, open a
// *separate* QSqlDatabase connection on that thread. SQLite connections
// cannot be shared across threads.
class DatabaseManager {
public:
    static DatabaseManager& instance();

    // Opens (creating if needed) trading_app.db in the app's writable
    // data location and creates tables if they don't exist yet.
    bool init(const QString &dbPath = QString());

    QSqlDatabase& db();

private:
    DatabaseManager() = default;
    bool initSchema();

    QSqlDatabase m_db;
    bool m_initialized = false;
};

#endif // DATABASEMANAGER_H
