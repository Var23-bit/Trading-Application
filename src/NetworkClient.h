#ifndef NETWORKCLIENT_H
#define NETWORKCLIENT_H

#include <QObject>
#include <QWebSocket>
#include <QTimer>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include "MarketData.h"

class NetworkClient : public QObject {
    Q_OBJECT
public:
    explicit NetworkClient(QObject *parent = nullptr);
    ~NetworkClient();

    void setApiKey(const QString &key);
    void connectToSdk(); 
    void disconnectStream();
    
    void subscribeToSymbol(const QString &symbol);
    void unsubscribeFromSymbol(const QString &symbol);

    void fetchHistoricalData(const QString &symbol);

signals:
    void tickReceived(const Tick &tick);
    void historicalDataReceived(const QList<Candle> &candles);
    void connectionStatusChanged(bool connected, QString message);

private slots:
    void onConnected();
    void onDisconnected();
    void onTextMessageReceived(const QString &message);
    void onError(QAbstractSocket::SocketError error);
    void onMockTimerTriggered();

private:
    QWebSocket m_webSocket;
    QTimer m_reconnectTimer;
    QTimer m_mockTimer;
    QNetworkAccessManager m_netManager;
    QUrl m_finnhubUrl;
    QString m_apiKey;
    bool m_isMockMode;
    
    // Track active subscription to resubscribe on reconnect
    QString m_currentSymbol;

    // For mock data generation
    double m_lastPrice;
};

#endif // NETWORKCLIENT_H
