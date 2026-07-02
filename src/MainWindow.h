#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtCharts/QChartView>
#include <QtCharts/QCandlestickSeries>
#include <QtCharts/QCandlestickSet>
#include <QtCharts/QLineSeries>
#include <QtCharts/QValueAxis>
#include <QtCharts/QDateTimeAxis>
#include <QPushButton>
#include <QLabel>
#include <QThread>
#include <QLineEdit>
#include <QDockWidget>

#include "NetworkClient.h"
#include "DataProcessor.h"
#include "PortfolioManager.h"
#include "MarketWatchWidget.h"
#include "OrderPanel.h"
#include "PortfolioWidget.h"

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(int userId, QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onConnectClicked();
    void onTickReceived(const Tick &tick);
    void onCandleUpdated(const Candle &candle);
    void onIndicatorsUpdated(const std::vector<double> &ema, 
                             const std::vector<double> &rsi,
                             const std::pair<std::vector<double>, std::vector<double>> &macd);
    void onConnectionStatusChanged(bool connected, QString message);
    
    // New Slots
    void onSymbolSelected(const QString &symbol);
    void onBuyOrder(const QString &symbol, double price, double quantity);
    void onSellOrder(const QString &symbol, double price, double quantity);
    void onPortfolioUpdated();

private:
    void setupUI();
    void setupCharts();
    void setupDockWidgets();

    QWidget *m_centralWidget;
    QChart *m_chart;
    QChartView *m_chartView;
    
    // Series
    QCandlestickSeries *m_candleSeries;
    QLineSeries *m_emaSeries;
    
    // Axes
    QDateTimeAxis *m_axisX; 
    QValueAxis *m_axisY;

    // Controls
    QPushButton *m_connectBtn;
    QLineEdit *m_apiKeyInput; 
    QLabel *m_statusLabel;
    QLabel *m_priceLabel;

    // Backend
    NetworkClient *m_networkClient;
    DataProcessor *m_processor;
    QThread *m_processorThread;
    PortfolioManager *m_portfolioManager;

    // New Widgets
    MarketWatchWidget *m_marketWatch;
    OrderPanel *m_orderPanel;
    PortfolioWidget *m_portfolioWidget;
    
    // Docks
    QDockWidget *m_dockMarket;
    QDockWidget *m_dockOrder;
    QDockWidget *m_dockPortfolio;

    bool m_isConnected;
    int m_userId;
};

#endif // MAINWINDOW_H
