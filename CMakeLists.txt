cmake_minimum_required(VERSION 3.16)

project(TradingApp VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 COMPONENTS Widgets Charts Network WebSockets Sql REQUIRED)

set(PROJECT_SOURCES
    src/main.cpp
    src/MainWindow.cpp
    src/MainWindow.h
    src/NetworkClient.cpp
    src/NetworkClient.h
    src/MarketData.h
    src/DataProcessor.cpp
    src/DataProcessor.h
    src/TechnicalIndicators.cpp
    src/TechnicalIndicators.h
    src/PortfolioManager.cpp
    src/PortfolioManager.h
    src/MarketWatchWidget.cpp
    src/MarketWatchWidget.h
    src/OrderPanel.cpp
    src/OrderPanel.h
    src/PortfolioWidget.cpp
    src/PortfolioWidget.h
    src/DatabaseManager.cpp
    src/DatabaseManager.h
    src/AuthManager.cpp
    src/AuthManager.h
    src/LoginDialog.cpp
    src/LoginDialog.h
)

add_executable(TradingApp ${PROJECT_SOURCES})

target_link_libraries(TradingApp PRIVATE
    Qt6::Widgets
    Qt6::Charts
    Qt6::Network
    Qt6::WebSockets
    Qt6::Sql
)

if(WIN32)
    set_property(TARGET TradingApp PROPERTY WIN32_EXECUTABLE ON)
endif()
