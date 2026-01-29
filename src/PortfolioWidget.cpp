#include "PortfolioWidget.h"
#include <QVBoxLayout>
#include <QHeaderView>

PortfolioWidget::PortfolioWidget(QWidget *parent) : QWidget(parent) {
    setupUI();
}

void PortfolioWidget::setupUI() {
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);

    // Summary Header
    QWidget *summary = new QWidget();
    QHBoxLayout *hlo = new QHBoxLayout(summary);
    
    m_lblCash = new QLabel("Cash: $100,000.00");
    m_lblCash->setStyleSheet("color: white; font-weight: bold;");
    
    m_lblPnL = new QLabel("Unrealized P&L: $0.00");
    m_lblPnL->setStyleSheet("color: white; font-weight: bold; margin-left: 10px;");
    
    hlo->addWidget(m_lblCash);
    hlo->addWidget(m_lblPnL);
    hlo->addStretch();
    
    layout->addWidget(summary);

    // Table
    m_table = new QTableWidget(this);
    m_table->setColumnCount(4);
    m_table->setHorizontalHeaderLabels({"Symbol", "Qty", "Avg Price", "Mkt Price"});
    m_table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    m_table->verticalHeader()->setVisible(false);
    m_table->setStyleSheet("QTableWidget { background-color: #1e1e1e; color: #d0d0d0; border: none; gridline-color: #333; }"
                           "QHeaderView::section { background-color: #2b2b2b; color: white; padding: 4px; border: none; }");
    
    layout->addWidget(m_table);
}

void PortfolioWidget::updatePortfolio(double cash, const QMap<QString, Position> &positions) {
    m_lblCash->setText(QString("Cash: $%1").arg(cash, 0, 'f', 2));

    m_table->setRowCount(0);
    for (auto it = positions.begin(); it != positions.end(); ++it) {
        int row = m_table->rowCount();
        m_table->insertRow(row);
        
        const Position &p = it.value();
        m_table->setItem(row, 0, new QTableWidgetItem(p.symbol));
        m_table->setItem(row, 1, new QTableWidgetItem(QString::number(p.quantity)));
        m_table->setItem(row, 2, new QTableWidgetItem(QString::number(p.avgPrice, 'f', 2)));
        // Market price updated separately or here? For simplicity, placeholder
        m_table->setItem(row, 3, new QTableWidgetItem("---"));
    }
}

void PortfolioWidget::updatePnL(double pnl) {
    m_lblPnL->setText(QString("Unrealized P&L: $%1").arg(pnl, 0, 'f', 2));
    if (pnl >= 0) m_lblPnL->setStyleSheet("color: #00ff00; font-weight: bold; margin-left: 10px;");
    else m_lblPnL->setStyleSheet("color: #ff0000; font-weight: bold; margin-left: 10px;");
}
