#include "NetworkClient.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QRandomGenerator>
#include <QDebug>

NetworkClient::NetworkClient(QObject *parent)
    : QObject(parent), m_isMockMode(true), m_lastPrice(150.0), m_currentSymbol("BINANCE:BTCUSDT") // Default mock
{
    // Finnhub URL (needs token appended)
    m_finnhubUrl = QUrl("wss://ws.finnhub.io"); 

    connect(&m_webSocket, &QWebSocket::connected, this, &NetworkClient::onConnected);
    connect(&m_webSocket, &QWebSocket::disconnected, this, &NetworkClient::onDisconnected);
    typedef void (QWebSocket::*ErrorSignal)(QAbstractSocket::SocketError);
    connect(&m_webSocket, static_cast<ErrorSignal>(&QWebSocket::errorOccurred), this, &NetworkClient::onError);
    connect(&m_webSocket, &QWebSocket::textMessageReceived, this, &NetworkClient::onTextMessageReceived);

    // Reconnect logic
    m_reconnectTimer.setInterval(5000);
    connect(&m_reconnectTimer, &QTimer::timeout, this, [this]() {
        if (!m_isMockMode && m_webSocket.state() == QAbstractSocket::UnconnectedState) {
            connectToSdk();
        }
    });

    // Mock timer
    m_mockTimer.setInterval(100); 
    connect(&m_mockTimer, &QTimer::timeout, this, &NetworkClient::onMockTimerTriggered);
}

NetworkClient::~NetworkClient() {
    m_webSocket.close();
}

void NetworkClient::setApiKey(const QString &key) {
    m_apiKey = key;
    m_isMockMode = m_apiKey.isEmpty();
}

void NetworkClient::connectToSdk() {
    if (m_isMockMode) {
        m_mockTimer.start();
        emit connectionStatusChanged(true, "Connected (Mock)");
        return;
    }

    QUrl url = m_finnhubUrl;
    url.setQuery("token=" + m_apiKey);
    
    m_mockTimer.stop();
    m_webSocket.open(url);
    emit connectionStatusChanged(false, "Connecting to Finnhub...");
}

void NetworkClient::disconnectStream() {
    m_mockTimer.stop();
    m_webSocket.close();
    emit connectionStatusChanged(false, "Disconnected");
}

void NetworkClient::subscribeToSymbol(const QString &symbol) {
    m_currentSymbol = symbol;
    if (m_isMockMode) {
         // Reset mock price for fun
         m_lastPrice = 100.0 + (QRandomGenerator::global()->generate() % 100);
         return;
    }

    if (m_webSocket.state() == QAbstractSocket::ConnectedState) {
        // Finnhub subscription message
        QString msg = QString("{\"type\":\"subscribe\",\"symbol\":\"%1\"}").arg(symbol);
        m_webSocket.sendTextMessage(msg);
    }
}

void NetworkClient::unsubscribeFromSymbol(const QString &symbol) {
     if (!m_isMockMode && m_webSocket.state() == QAbstractSocket::ConnectedState) {
        QString msg = QString("{\"type\":\"unsubscribe\",\"symbol\":\"%1\"}").arg(symbol);
        m_webSocket.sendTextMessage(msg);
    }
}

void NetworkClient::fetchHistoricalData(const QString &symbol) {
    if (m_isMockMode) {
        // ... (Same mock logic as before) ...
        QList<Candle> history;
        QDateTime now = QDateTime::currentDateTime();
        double price = m_lastPrice;
        for (int i = 0; i < 100; ++i) {
            Candle c;
            c.timestamp = now.addSecs(-i * 60).toMSecsSinceEpoch();
            c.open = price;
            double change = QRandomGenerator::global()->generateDouble() * 2.0 - 1.0;
            c.close = price + change;
            c.high = std::max(c.open, c.close) + 0.5;
            c.low = std::min(c.open, c.close) - 0.5;
            history.prepend(c);
            price = c.close;
        }
        emit historicalDataReceived(history);
        return;
    }

    // Real REST API call to Finnhub for candles
    // https://finnhub.io/api/v1/stock/candle?symbol=AAPL&resolution=1&from=...&to=...&token=...
    
    qint64 to = QDateTime::currentSecsSinceEpoch();
    qint64 from = to - (3600 * 24); // Last 24 hours
    
    QString urlStr = QString("https://finnhub.io/api/v1/stock/candle?symbol=%1&resolution=1&from=%2&to=%3&token=%4")
                     .arg(symbol).arg(from).arg(to).arg(m_apiKey);
                     
    QNetworkRequest req((QUrl(urlStr)));
    QNetworkReply *reply = m_netManager.get(req);
    
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        if (reply->error() == QNetworkReply::NoError) {
            QByteArray data = reply->readAll();
            QJsonDocument doc = QJsonDocument::fromJson(data);
            QJsonObject obj = doc.object();
            
            if (obj["s"].toString() == "ok") {
                QList<Candle> candles;
                QJsonArray c = obj["c"].toArray(); // close
                QJsonArray h = obj["h"].toArray(); // high
                QJsonArray l = obj["l"].toArray(); // low
                QJsonArray o = obj["o"].toArray(); // open
                QJsonArray t = obj["t"].toArray(); // timestamp
                
                for(int i=0; i < t.size(); ++i) {
                    Candle candle;
                    candle.timestamp = t[i].toVariant().toLongLong() * 1000;
                    candle.open = o[i].toDouble();
                    candle.high = h[i].toDouble();
                    candle.low = l[i].toDouble();
                    candle.close = c[i].toDouble();
                    candles.append(candle);
                }
                emit historicalDataReceived(candles);
            }
        } else {
            qDebug() << "REST Error:" << reply->errorString();
        }
        reply->deleteLater();
    });
}

void NetworkClient::onConnected() {
    m_reconnectTimer.stop();
    emit connectionStatusChanged(true, "Connected to Finnhub");
    
    // Resubscribe
    if (!m_currentSymbol.isEmpty()) {
        subscribeToSymbol(m_currentSymbol);
    }
}

void NetworkClient::onDisconnected() {
    emit connectionStatusChanged(false, "Disconnected (Retrying...)");
    if (!m_isMockMode) {
        m_reconnectTimer.start();
    }
}

void NetworkClient::onError(QAbstractSocket::SocketError error) {
    Q_UNUSED(error);
}

void NetworkClient::onTextMessageReceived(const QString &message) {
    // Finnhub format: {"data": [{"p": 150.2, "s": "AAPL", "t": 123456, "v": 10}], "type": "trade"}
    QJsonDocument doc = QJsonDocument::fromJson(message.toUtf8());
    if (doc.isObject()) {
        QJsonObject root = doc.object();
        if (root["type"].toString() == "trade") {
            QJsonArray data = root["data"].toArray();
            for (const auto &val : data) {
                QJsonObject trade = val.toObject();
                Tick t;
                t.symbol = trade["s"].toString();
                t.price = trade["p"].toDouble();
                t.timestamp = trade["t"].toVariant().toLongLong();
                emit tickReceived(t);
            }
        }
    }
}

void NetworkClient::onMockTimerTriggered() {
    double change = (QRandomGenerator::global()->generateDouble() - 0.5) * 0.5;
    m_lastPrice += change;
    Tick t;
    t.symbol = m_currentSymbol.isEmpty() ? "MOCK" : m_currentSymbol;
    t.price = m_lastPrice;
    t.timestamp = QDateTime::currentMSecsSinceEpoch();
    emit tickReceived(t);
}
