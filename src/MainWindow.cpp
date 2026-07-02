#include "MainWindow.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QDateTime>
#include <QDebug>
#include <QMetaType>
#include <QMessageBox>

MainWindow::MainWindow(int userId, QWidget *parent)
    : QMainWindow(parent), m_isConnected(false), m_userId(userId)
{
    qRegisterMetaType<Tick>("Tick");
    qRegisterMetaType<Candle>("Candle");

    setupUI();
    setupDockWidgets();
    
    m_networkClient = new NetworkClient(this);
    m_portfolioManager = new PortfolioManager(m_userId, this); // loads persisted cash/holdings for this account
    
    // Processor runs on a separate thread
    m_processor = new DataProcessor(); 
    m_processorThread = new QThread();
    m_processor->moveToThread(m_processorThread);

    // Connect Network -> Processor
    connect(m_networkClient, &NetworkClient::tickReceived, m_processor, &DataProcessor::processTick);
    
    // Connect Processor -> UI
    connect(m_processor, &DataProcessor::candleUpdated, this, &MainWindow::onCandleUpdated);
    connect(m_processor, &DataProcessor::indicatorsUpdated, this, &MainWindow::onIndicatorsUpdated);

    // Connect UI -> Network
    connect(m_connectBtn, &QPushButton::clicked, this, &MainWindow::onConnectClicked);
    connect(m_networkClient, &NetworkClient::connectionStatusChanged, this, &MainWindow::onConnectionStatusChanged);
    connect(m_networkClient, &NetworkClient::tickReceived, this, &MainWindow::onTickReceived); 

    // Connect New Widgets
    connect(m_marketWatch, &MarketWatchWidget::symbolSelected, this, &MainWindow::onSymbolSelected);
    connect(m_networkClient, &NetworkClient::tickReceived, m_marketWatch, &MarketWatchWidget::updatePrice);
    
    connect(m_orderPanel, &OrderPanel::buyOrder, this, &MainWindow::onBuyOrder);
    connect(m_orderPanel, &OrderPanel::sellOrder, this, &MainWindow::onSellOrder);
    
    connect(m_portfolioManager, &PortfolioManager::portfolioUpdated, this, &MainWindow::onPortfolioUpdated);
    
    // Initialize Portfolio View
    onPortfolioUpdated();

    m_processorThread->start();
}

MainWindow::~MainWindow() {
    m_processorThread->quit();
    m_processorThread->wait();
    delete m_processor; // Manual delete since no parent
    delete m_processorThread;
}

void MainWindow::setupUI() {
    m_centralWidget = new QWidget(this);
    setCentralWidget(m_centralWidget);
    QVBoxLayout *mainLayout = new QVBoxLayout(m_centralWidget);
    mainLayout->setContentsMargins(0, 0, 0, 0);

    // Top Bar
    QHBoxLayout *topBar = new QHBoxLayout();
    topBar->setContentsMargins(10, 10, 10, 10);
    
    m_apiKeyInput = new QLineEdit(this);
    m_apiKeyInput->setPlaceholderText("Enter Finnhub API Key (Leave empty for Mock)");
    m_apiKeyInput->setStyleSheet("background-color: #333; color: white; border: 1px solid #555; padding: 6px; border-radius: 4px;");

    m_connectBtn = new QPushButton("Connect", this);
    m_statusLabel = new QLabel("Disconnected", this);
    m_priceLabel = new QLabel("0.00", this);
    
    m_connectBtn->setStyleSheet("background-color: #007bff; color: white; border: none; padding: 8px 16px; border-radius: 4px; font-weight: bold;");
    m_statusLabel->setStyleSheet("color: #888; margin-left: 10px;");
    m_priceLabel->setStyleSheet("color: #00ff00; font-size: 18px; font-weight: bold; margin-left: 20px;");
    
    topBar->addWidget(m_apiKeyInput, 1);
    topBar->addWidget(m_connectBtn);
    topBar->addWidget(m_statusLabel);
    topBar->addStretch();
    topBar->addWidget(m_priceLabel);

    mainLayout->addLayout(topBar);

    // Charts
    setupCharts();
    mainLayout->addWidget(m_chartView);
}

void MainWindow::setupDockWidgets() {
    setDockOptions(QMainWindow::AnimatedDocks | QMainWindow::AllowNestedDocks | QMainWindow::AllowTabbedDocks);

    // Market Watch (Left)
    m_dockMarket = new QDockWidget("Market Watch", this);
    m_dockMarket->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    m_marketWatch = new MarketWatchWidget(this);
    m_dockMarket->setWidget(m_marketWatch);
    addDockWidget(Qt::LeftDockWidgetArea, m_dockMarket);

    // Order Panel (Right Top)
    m_dockOrder = new QDockWidget("Order Entry", this);
    m_dockOrder->setAllowedAreas(Qt::RightDockWidgetArea | Qt::BottomDockWidgetArea);
    m_orderPanel = new OrderPanel(this);
    m_dockOrder->setWidget(m_orderPanel);
    addDockWidget(Qt::RightDockWidgetArea, m_dockOrder);

    // Portfolio (Right Bottom)
    m_dockPortfolio = new QDockWidget("Portfolio", this);
    m_dockPortfolio->setAllowedAreas(Qt::RightDockWidgetArea | Qt::BottomDockWidgetArea);
    m_portfolioWidget = new PortfolioWidget(this);
    m_dockPortfolio->setWidget(m_portfolioWidget);
    addDockWidget(Qt::RightDockWidgetArea, m_dockPortfolio);
}

void MainWindow::setupCharts() {
    m_chart = new QChart();
    m_chart->setBackgroundRoundness(0);
    m_chart->setBackgroundBrush(QBrush(QColor(30, 30, 30)));
    m_chart->setTitleBrush(QBrush(Qt::white));
    m_chart->setTitle("Waiting for Data...");

    m_candleSeries = new QCandlestickSeries();
    m_candleSeries->setName("OHLC");
    m_candleSeries->setIncreasingColor(QColor(0, 255, 0)); 
    m_candleSeries->setDecreasingColor(QColor(255, 0, 0));
    m_candleSeries->setBodyOutlineVisible(false);

    m_emaSeries = new QLineSeries();
    m_emaSeries->setName("EMA(20)");
    m_emaSeries->setColor(QColor(255, 165, 0)); 

    m_chart->addSeries(m_candleSeries);
    m_chart->addSeries(m_emaSeries);

    m_axisX = new QDateTimeAxis();
    m_axisX->setFormat("HH:mm:ss");
    m_axisX->setLabelsBrush(QBrush(Qt::lightGray));
    m_axisX->setGridLineColor(QColor(60, 60, 60));
    m_chart->addAxis(m_axisX, Qt::AlignBottom);
    
    m_axisY = new QValueAxis();
    m_axisY->setLabelsBrush(QBrush(Qt::lightGray));
    m_axisY->setGridLineColor(QColor(60, 60, 60));
    m_chart->addAxis(m_axisY, Qt::AlignRight);

    m_candleSeries->attachAxis(m_axisX);
    m_candleSeries->attachAxis(m_axisY);
    m_emaSeries->attachAxis(m_axisX);
    m_emaSeries->attachAxis(m_axisY);

    m_chart->legend()->setVisible(true);
    m_chart->legend()->setLabelColor(Qt::white);
    m_chart->legend()->setAlignment(Qt::AlignBottom);

    m_chartView = new QChartView(m_chart);
    m_chartView->setRenderHint(QPainter::Antialiasing);
}

void MainWindow::onConnectClicked() {
    if (m_isConnected) {
        m_networkClient->disconnectStream();
        m_apiKeyInput->setEnabled(true);
        m_isConnected = false;
        m_connectBtn->setText("Connect");
    } else {
        m_networkClient->setApiKey(m_apiKeyInput->text());
        m_networkClient->connectToSdk(); // Starts Mock or Real based on key
        m_apiKeyInput->setEnabled(false);
        m_isConnected = true;
        m_connectBtn->setText("Disconnect");
    }
}

void MainWindow::onSymbolSelected(const QString &symbol) {
    // Unsubscribe from old, subscribe to new
    // For simplicity, we just change the current symbol logic
    m_chart->setTitle(symbol);
    m_chart->removeAllSeries(); // Clear old data visually
    // Ideally we should reset series data properly, but for demo remove/add is quick dirty way
    // Better way: m_candleSeries->clear(); m_emaSeries->clear();
    m_candleSeries->clear();
    m_emaSeries->clear();
    
    // Re-add to chart if they were removed (removeAllSeries removes them from chart but doesn't delete objects if managed)
    if (m_chart->series().isEmpty()) {
        m_chart->addSeries(m_candleSeries);
        m_chart->addSeries(m_emaSeries);
        m_candleSeries->attachAxis(m_axisX);
        m_candleSeries->attachAxis(m_axisY);
        m_emaSeries->attachAxis(m_axisX);
        m_emaSeries->attachAxis(m_axisY);
    }

    m_networkClient->subscribeToSymbol(symbol);
    m_orderPanel->setCurrentSymbol(symbol, 0.0); // Reset price display
}

void MainWindow::onConnectionStatusChanged(bool connected, QString message) {
    m_statusLabel->setText(message);
    if (!connected && !m_isConnected) {
        // Just disconnected manually
    } else if (!connected) {
       // Lost connection
    }
}

void MainWindow::onTickReceived(const Tick &tick) {
    m_priceLabel->setText(QString::number(tick.price, 'f', 2));
    m_priceLabel->setStyleSheet(QString("color: %1; font-size: 18px; font-weight: bold; margin-left: 20px;")
                                .arg(tick.price >= m_candleSeries->sets().isEmpty() ? "0" : 
                                     (tick.price >= m_candleSeries->sets().last()->close() ? "#00ff00" : "#ff0000")));
    
    m_orderPanel->setCurrentSymbol(tick.symbol, tick.price);
    
    // Update PnL in portfolio (if we had a map of all last prices, it would be better)
    // For now, passing map with just this one price
    QMap<QString, double> prices;
    prices.insert(tick.symbol, tick.price);
    m_portfolioWidget->updatePnL(m_portfolioManager->getUnrealizedPnL(prices));
}

void MainWindow::onCandleUpdated(const Candle &candle) {
    bool exists = false;
    if (m_candleSeries->count() > 0) {
        QCandlestickSet *lastSet = m_candleSeries->sets().last();
        if (std::abs(lastSet->timestamp() - candle.timestamp) < 100) { 
            lastSet->setOpen(candle.open);
            lastSet->setHigh(candle.high);
            lastSet->setLow(candle.low);
            lastSet->setClose(candle.close);
            exists = true;
        }
    }

    if (!exists) {
        QCandlestickSet *newSet = new QCandlestickSet(candle.open, candle.high, candle.low, candle.close, candle.timestamp);
        m_candleSeries->append(newSet);
        
        QDateTime start = QDateTime::fromMSecsSinceEpoch(candle.timestamp).addSecs(-60 * 30);
        QDateTime end = QDateTime::fromMSecsSinceEpoch(candle.timestamp).addSecs(60 * 2);
        m_axisX->setRange(start, end);
    }
    
    if (m_candleSeries->count() > 0) {
        double min = 1e9, max = -1e9;
        int count = m_candleSeries->count();
        int lookback = std::min(count, 50);
        for(int i = count - lookback; i < count; ++i) {
             auto s = m_candleSeries->sets().at(i);
             if (s->low() < min) min = s->low();
             if (s->high() > max) max = s->high();
        }
        double buffer = (max - min) * 0.1;
        if (min < max) {
            m_axisY->setRange(min - buffer, max + buffer);
        }
    }
}

void MainWindow::onIndicatorsUpdated(const std::vector<double> &ema, const std::vector<double> &rsi, const std::pair<std::vector<double>, std::vector<double>> &macd) {
    QList<QPointF> points;
    int candleCount = m_candleSeries->count();
    int emaCount = ema.size();
    int offset = candleCount - emaCount; 
    if (offset < 0) offset = 0;
    
    for (int i = 0; i < emaCount; ++i) {
        if (i + offset >= 0 && i + offset < candleCount) {
             qint64 ts = m_candleSeries->sets().at(i + offset)->timestamp();
             if (ema[i] != 0.0) {
                 points.append(QPointF(ts, ema[i]));
             }
        }
    }
    m_emaSeries->replace(points);
}

void MainWindow::onBuyOrder(const QString &symbol, double price, double quantity) {
    if (m_portfolioManager->buy(symbol, price, quantity)) {
        // Success
    } else {
        QMessageBox::warning(this, "Order Rejected", "Insufficient funds or invalid order.");
    }
}

void MainWindow::onSellOrder(const QString &symbol, double price, double quantity) {
    if (m_portfolioManager->sell(symbol, price, quantity)) {
        // Success
    } else {
        QMessageBox::warning(this, "Order Rejected", "Insufficient position.");
    }
}

void MainWindow::onPortfolioUpdated() {
    m_portfolioWidget->updatePortfolio(m_portfolioManager->getCashBalance(), m_portfolioManager->getPositions());
}
