#include "MarketWatchWidget.h"
#include <QVBoxLayout>
#include <QLabel>
#include <QListWidgetItem>

MarketWatchWidget::MarketWatchWidget(QWidget *parent) : QWidget(parent) {
    setupUI();
}

void MarketWatchWidget::setupUI() {
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    
    QLabel *header = new QLabel("Market Watch", this);
    header->setStyleSheet("font-weight: bold; padding: 5px; background-color: #2b2b2b; color: white;");
    layout->addWidget(header);

    m_listWidget = new QListWidget(this);
    m_listWidget->setStyleSheet("QListWidget { background-color: #1e1e1e; color: #d0d0d0; border: none; }"
                                "QListWidget::item { padding: 8px; border-bottom: 1px solid #333; }"
                                "QListWidget::item:selected { background-color: #3a3a3a; }");
    
    // Default Symbols
    QStringList symbols = {
        "AAPL", "MSFT", "GOOGL", "AMZN", "TSLA", "NVDA", "BTCUSDT", "ETHUSDT", "SPY", "QQQ"
    };

    for (const QString &sym : symbols) {
        QListWidgetItem *item = new QListWidgetItem(sym);
        // Store symbol in data
        item->setData(Qt::UserRole, sym);
        m_listWidget->addItem(item);
    }

    connect(m_listWidget, &QListWidget::itemClicked, this, [this](QListWidgetItem *item) {
        emit symbolSelected(item->data(Qt::UserRole).toString());
    });

    layout->addWidget(m_listWidget);
}

void MarketWatchWidget::updatePrice(const Tick &tick) {
    // Brute force search for simplicity (optimize with map if list is long)
    for(int i = 0; i < m_listWidget->count(); ++i) {
        QListWidgetItem *item = m_listWidget->item(i);
        if (item->data(Qt::UserRole).toString() == tick.symbol) {
            item->setText(QString("%1   %2").arg(tick.symbol).arg(tick.price, 0, 'f', 2));
            
            // Flash color?
            // item->setBackground( ... )
            return;
        }
    }
}
