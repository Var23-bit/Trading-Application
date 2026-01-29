#ifndef PORTFOLIOWIDGET_H
#define PORTFOLIOWIDGET_H

#include <QWidget>
#include <QTableWidget>
#include <QLabel>
#include "PortfolioManager.h"

class PortfolioWidget : public QWidget {
    Q_OBJECT
public:
    explicit PortfolioWidget(QWidget *parent = nullptr);

public slots:
    void updatePortfolio(double cash, const QMap<QString, Position> &positions);
    void updatePnL(double pnl); // Update unrealized PnL from live prices

private:
    void setupUI();
    
    QLabel *m_lblCash;
    QLabel *m_lblPnL;
    QTableWidget *m_table;
};

#endif // PORTFOLIOWIDGET_H
