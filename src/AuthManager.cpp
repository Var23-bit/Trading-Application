#include "AuthManager.h"
#include "DatabaseManager.h"

#include <QCryptographicHash>
#include <QMessageAuthenticationCode>
#include <QRandomGenerator>
#include <QSqlQuery>
#include <QSqlError>

static const int kIterations = 100000;
static const int kSaltBytes = 16;

QByteArray AuthManager::generateSalt() {
    QByteArray salt(kSaltBytes, 0);
    for (int i = 0; i < kSaltBytes; ++i)
        salt[i] = static_cast<char>(QRandomGenerator::global()->bounded(256));
    return salt;
}

QString AuthManager::hashPassword(const QString &password, const QByteArray &salt) {
    QByteArray key = password.toUtf8();
    QByteArray result = QMessageAuthenticationCode::hash(salt, key, QCryptographicHash::Sha256);
    for (int i = 1; i < kIterations; ++i) {
        result = QMessageAuthenticationCode::hash(result, key, QCryptographicHash::Sha256);
    }
    return result.toHex();
}

std::optional<QString> AuthManager::registerUser(const QString &username, const QString &password) {
    if (username.trimmed().length() < 3)
        return "Username must be at least 3 characters";
    if (password.length() < 8)
        return "Password must be at least 8 characters";

    auto &db = DatabaseManager::instance().db();

    QSqlQuery check(db);
    check.prepare("SELECT id FROM users WHERE username = :u");
    check.bindValue(":u", username);
    check.exec();
    if (check.next())
        return "Username already taken";

    QByteArray salt = generateSalt();
    QString hash = hashPassword(password, salt);

    QSqlQuery insert(db);
    insert.prepare("INSERT INTO users (username, password_hash, salt) VALUES (:u, :h, :s)");
    insert.bindValue(":u", username);
    insert.bindValue(":h", hash);
    insert.bindValue(":s", QString(salt.toHex()));

    if (!insert.exec())
        return "Database error: " + insert.lastError().text();

    return std::nullopt;
}

std::optional<UserSession> AuthManager::login(const QString &username, const QString &password) {
    auto &db = DatabaseManager::instance().db();

    QSqlQuery q(db);
    q.prepare("SELECT id, password_hash, salt, cash_balance FROM users WHERE username = :u");
    q.bindValue(":u", username);
    if (!q.exec() || !q.next())
        return std::nullopt;

    QString storedHash = q.value("password_hash").toString();
    QByteArray salt = QByteArray::fromHex(q.value("salt").toString().toUtf8());

    QString attemptHash = hashPassword(password, salt);
    if (attemptHash != storedHash)
        return std::nullopt;

    UserSession session;
    session.id = q.value("id").toInt();
    session.username = username;
    session.cashBalance = q.value("cash_balance").toDouble();
    return session;
}
