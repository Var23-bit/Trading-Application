#ifndef ORDERPANEL_H
#define ORDERPANEL_H

#include <QWidget>
#include <QPushButton>
#include <QDoubleSpinBox>
#include <QSpinBox>
#include <QLabel>

class OrderPanel : public QWidget {
    Q_OBJECT
public:
    explicit OrderPanel(QWidget *parent = nullptr);
    void setCurrentSymbol(const QString &symbol, double price);

signals:
    void buyOrder(const QString &symbol, double price, double quantity);
    void sellOrder(const QString &symbol, double price, double quantity);

private slots:
    void onBuyClicked();
    void onSellClicked();

private:
    void setupUI();
    
    QLabel *m_lblSymbol;
    QLabel *m_lblPrice; // Last price for reference
    QDoubleSpinBox *m_limitPriceBox;
    QSpinBox *m_qtyBox;
    QPushButton *m_btnBuy;
    QPushButton *m_btnSell;
    
    QString m_symbol;
    double m_lastPrice;
};

#endif // ORDERPANEL_H
