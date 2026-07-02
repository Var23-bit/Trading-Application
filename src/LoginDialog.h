#ifndef LOGINDIALOG_H
#define LOGINDIALOG_H

#include <QDialog>
#include <optional>
#include "AuthManager.h"

class QLineEdit;
class QLabel;

// Modal login/register dialog shown before MainWindow. If exec() returns
// QDialog::Accepted, call session() to get the logged-in user.
class LoginDialog : public QDialog {
    Q_OBJECT
public:
    explicit LoginDialog(QWidget *parent = nullptr);
    std::optional<UserSession> session() const { return m_session; }

private slots:
    void onLoginClicked();
    void onRegisterClicked();

private:
    QLineEdit *m_usernameEdit;
    QLineEdit *m_passwordEdit;
    QLabel *m_statusLabel;
    std::optional<UserSession> m_session;
};

#endif // LOGINDIALOG_H
