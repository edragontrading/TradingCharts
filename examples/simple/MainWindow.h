#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

/**
 * This example shows, how to place a dock widget container and a static
 * sidebar into a QMainWindow
 */
class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private:
    /// The menu manager for the dock widgets.
    QWidget *mCentralWidget;
};

#endif  // MAINWINDOW_H
