#include "MainWindow.h"

#include <ed/tradingcharts/ResizeHandle.h>
#include <ed/tradingcharts/TradingPlot.h>
#include <ed/tradingcharts/TradingRect.h>

#include <QAction>
#include <QDebug>
#include <QLabel>
#include <QMenu>
#include <QPlainTextEdit>
#include <QPoint>
#include <QPushButton>
#include <QScreen>
#include <QToolBar>
#include <QVBoxLayout>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
    setWindowTitle("MainWindow");
    setObjectName("MainWindow");

    QWidget *centralWidget = new QWidget;
    QVBoxLayout *layout_central = new QVBoxLayout(centralWidget);
    layout_central->setSpacing(0);
    layout_central->setContentsMargins(0, 0, 0, 0);

    ed::ETradingPlot *tradingPlot = new ed::ETradingPlot(centralWidget);

    ed::ETradingRect *handle = new ed::ETradingRect(tradingPlot);
    handle->moveCoord(30, 40, 40, 20);

    layout_central->addWidget(tradingPlot);
    setCentralWidget(centralWidget);
}

MainWindow::~MainWindow() {
}