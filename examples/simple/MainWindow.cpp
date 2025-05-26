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

    mCentralWidget = new QWidget(parent);
    QVBoxLayout *layout_central = new QVBoxLayout(mCentralWidget);
    layout_central->setSpacing(0);
    layout_central->setContentsMargins(0, 0, 0, 0);

    ed::ETradingPlot *tradingPlot = new ed::ETradingPlot(mCentralWidget);

    QWidget *buttonWidget = new QWidget(parent);
    QHBoxLayout *layout_button = new QHBoxLayout(buttonWidget);
    QPushButton *button1 = new QPushButton("Normal", mCentralWidget);
    QPushButton *button2 = new QPushButton("Rect", mCentralWidget);
    QPushButton *button3 = new QPushButton("Ellipse", mCentralWidget);
    QPushButton *button4 = new QPushButton("Triangle", mCentralWidget);

    connect(button1, &QPushButton::clicked, this,
            [tradingPlot](bool) { tradingPlot->setMode(ed::ETradingPlot::Mode::pmNone); });

    connect(button2, &QPushButton::clicked, this,
            [tradingPlot](bool) { tradingPlot->setMode(ed::ETradingPlot::Mode::pmDrawingRect); });

    connect(button3, &QPushButton::clicked, this,
            [tradingPlot](bool) { tradingPlot->setMode(ed::ETradingPlot::Mode::pmDrawingEllipse); });

    connect(button4, &QPushButton::clicked, this,
            [tradingPlot](bool) { tradingPlot->setMode(ed::ETradingPlot::Mode::pmDrawingTriangle); });

    layout_button->addWidget(button1);
    layout_button->addWidget(button2);
    layout_button->addWidget(button3);
    layout_button->addWidget(button4);

    layout_central->addWidget(tradingPlot, 1);
    layout_central->addWidget(buttonWidget);
    setCentralWidget(mCentralWidget);
}

MainWindow::~MainWindow() {
    delete mCentralWidget;
}