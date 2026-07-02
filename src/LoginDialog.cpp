#include "LoginDialog.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QLabel>
#include <QPushButton>

LoginDialog::LoginDialog(QWidget *parent) : QDialog(parent) {
    setWindowTitle("Sign in - Qt C++ Real-time Trading App");
    setMinimumWidth(340);

    auto *layout = new QVBoxLayout(this);

    layout->addWidget(new QLabel("Username"));
    m_usernameEdit = new QLineEdit(this);
    layout->addWidget(m_usernameEdit);

    layout->addWidget(new QLabel("Password"));
    m_passwordEdit = new QLineEdit(this);
    m_passwordEdit->setEchoMode(QLineEdit::Password);
    layout->addWidget(m_passwordEdit);

    m_statusLabel = new QLabel(this);
    m_statusLabel->setStyleSheet("color: #e74c3c;");
    m_statusLabel->setWordWrap(true);
    layout->addWidget(m_statusLabel);

    auto *btnRow = new QHBoxLayout();
    auto *loginBtn = new QPushButton("Login", this);
    auto *registerBtn = new QPushButton("Register", this);
    btnRow->addWidget(loginBtn);
    btnRow->addWidget(registerBtn);
    layout->addLayout(btnRow);

    connect(loginBtn, &QPushButton::clicked, this, &LoginDialog::onLoginClicked);
    connect(registerBtn, &QPushButton::clicked, this, &LoginDialog::onRegisterClicked);
    connect(m_passwordEdit, &QLineEdit::returnPressed, this, &LoginDialog::onLoginClicked);
}

void LoginDialog::onLoginClicked() {
    auto result = AuthManager::login(m_usernameEdit->text().trimmed(), m_passwordEdit->text());
    if (!result) {
        m_statusLabel->setStyleSheet("color: #e74c3c;");
        m_statusLabel->setText("Invalid username or password");
        return;
    }
    m_session = result;
    accept();
}

void LoginDialog::onRegisterClicked() {
    auto err = AuthManager::registerUser(m_usernameEdit->text().trimmed(), m_passwordEdit->text());
    if (err) {
        m_statusLabel->setStyleSheet("color: #e74c3c;");
        m_statusLabel->setText(*err);
        return;
    }
    m_statusLabel->setStyleSheet("color: #2ecc71;");
    m_statusLabel->setText("Registered with $100,000 paper cash. Click Login to continue.");
}
