#ifndef AUTHMANAGER_H
#define AUTHMANAGER_H

#include <QString>
#include <optional>

struct UserSession {
    int id;
    QString username;
    double cashBalance;
};

class AuthManager {
public:
    // Returns std::nullopt on success, or an error message on failure.
    static std::optional<QString> registerUser(const QString &username,
                                                 const QString &password);

    // Returns a UserSession on success, nullopt on bad credentials.
    static std::optional<UserSession> login(const QString &username,
                                             const QString &password);

private:
    // PBKDF2-HMAC-SHA256, 100k iterations. Qt has no bcrypt/argon2 built in;
    // this is the standard "good enough with stdlib only" approach.
    static QString hashPassword(const QString &password, const QByteArray &salt);
    static QByteArray generateSalt();
};

#endif // AUTHMANAGER_H
