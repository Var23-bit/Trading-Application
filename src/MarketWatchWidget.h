#ifndef MARKETWATCHWIDGET_H
#define MARKETWATCHWIDGET_H

#include <QWidget>
#include <QListWidget>
#include "NetworkClient.h"

class MarketWatchWidget : public QWidget {
    Q_OBJECT
public:
    explicit MarketWatchWidget(QWidget *parent = nullptr);

signals:
    void symbolSelected(const QString &symbol);

public slots:
    void updatePrice(const Tick &tick);

private:
    void setupUI();
    QListWidget *m_listWidget;
};

#endif // MARKETWATCHWIDGET_H
