#include <QApplication>
#include <QMessageBox>
#include "MainWindow.h"
#include "DatabaseManager.h"
#include "LoginDialog.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    if (!DatabaseManager::instance().init()) {
        QMessageBox::critical(nullptr, "Database Error",
            "Could not initialize the local trading_app.db database. The app cannot start.");
        return 1;
    }
    
    // Dark Theme Palette
    QPalette darkPalette;
    darkPalette.setColor(QPalette::Window, QColor(53, 53, 53));
    darkPalette.setColor(QPalette::WindowText, Qt::white);
    darkPalette.setColor(QPalette::Base, QColor(25, 25, 25));
    darkPalette.setColor(QPalette::AlternateBase, QColor(53, 53, 53));
    darkPalette.setColor(QPalette::ToolTipBase, Qt::white);
    darkPalette.setColor(QPalette::ToolTipText, Qt::white);
    darkPalette.setColor(QPalette::Text, Qt::white);
    darkPalette.setColor(QPalette::Button, QColor(53, 53, 53));
    darkPalette.setColor(QPalette::ButtonText, Qt::white);
    darkPalette.setColor(QPalette::BrightText, Qt::red);
    darkPalette.setColor(QPalette::Link, QColor(42, 130, 218));
    darkPalette.setColor(QPalette::Highlight, QColor(42, 130, 218));
    darkPalette.setColor(QPalette::HighlightedText, Qt::black);
    a.setPalette(darkPalette);
    a.setStyleSheet("QToolTip { color: #ffffff; background-color: #2a82da; border: 1px solid white; }");

    LoginDialog login;
    if (login.exec() != QDialog::Accepted) {
        return 0; // user closed the login window
    }
    UserSession session = *login.session();

    MainWindow w(session.id);
    w.resize(1024, 768);
    w.setWindowTitle("Qt C++ Real-time Trading App - " + session.username);
    w.show();

    return a.exec();
}
