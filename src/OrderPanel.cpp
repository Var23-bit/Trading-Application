#include "OrderPanel.h"
#include <QGridLayout>
#include <QGroupBox>

OrderPanel::OrderPanel(QWidget *parent) : QWidget(parent), m_lastPrice(0) {
    setupUI();
}

void OrderPanel::setCurrentSymbol(const QString &symbol, double price) {
    m_symbol = symbol;
    m_lastPrice = price;
    m_lblSymbol->setText(symbol);
    m_lblPrice->setText(QString::number(price, 'f', 2));
    
    // Auto set limit price to current price if zero
    if (m_limitPriceBox->value() == 0) {
        m_limitPriceBox->setValue(price);
    }
}

void OrderPanel::setupUI() {
    QGroupBox *group = new QGroupBox("Order Entry", this);
    group->setStyleSheet("QGroupBox { color: white; border: 1px solid #555; margin-top: 10px; }"
                         "QGroupBox::title { subcontrol-origin: margin; left: 10px; padding: 0 3px; }");
    
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0,0,0,0);
    mainLayout->addWidget(group);

    QGridLayout *layout = new QGridLayout(group);
    
    layout->addWidget(new QLabel("Symbol:"), 0, 0);
    m_lblSymbol = new QLabel("---");
    m_lblSymbol->setStyleSheet("font-weight: bold; color: orange;");
    layout->addWidget(m_lblSymbol, 0, 1);

    layout->addWidget(new QLabel("Price:"), 1, 0);
    m_lblPrice = new QLabel("0.00");
    layout->addWidget(m_lblPrice, 1, 1);
    
    layout->addWidget(new QLabel("Limit Price:"), 2, 0);
    m_limitPriceBox = new QDoubleSpinBox();
    m_limitPriceBox->setMaximum(1000000);
    m_limitPriceBox->setDecimals(2);
    layout->addWidget(m_limitPriceBox, 2, 1);

    layout->addWidget(new QLabel("Quantity:"), 3, 0);
    m_qtyBox = new QSpinBox();
    m_qtyBox->setRange(1, 100000);
    layout->addWidget(m_qtyBox, 3, 1);

    m_btnBuy = new QPushButton("BUY");
    m_btnBuy->setStyleSheet("background-color: #28a745; color: white; border-radius: 4px; padding: 6px; font-weight: bold;");
    layout->addWidget(m_btnBuy, 4, 0);

    m_btnSell = new QPushButton("SELL");
    m_btnSell->setStyleSheet("background-color: #dc3545; color: white; border-radius: 4px; padding: 6px; font-weight: bold;");
    layout->addWidget(m_btnSell, 4, 1);
    
    connect(m_btnBuy, &QPushButton::clicked, this, &OrderPanel::onBuyClicked);
    connect(m_btnSell, &QPushButton::clicked, this, &OrderPanel::onSellClicked);
}

void OrderPanel::onBuyClicked() {
    if (!m_symbol.isEmpty()) {
        emit buyOrder(m_symbol, m_limitPriceBox->value() > 0 ? m_limitPriceBox->value() : m_lastPrice, m_qtyBox->value());
    }
}

void OrderPanel::onSellClicked() {
    if (!m_symbol.isEmpty()) {
        emit sellOrder(m_symbol, m_limitPriceBox->value() > 0 ? m_limitPriceBox->value() : m_lastPrice, m_qtyBox->value());
    }
}
