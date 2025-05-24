#include <QApplication>
#include <QByteArray>
#include <QFile>
#include <QPalette>
#include <QProxyStyle>
#include <QStyleFactory>
#include <QTextEdit>
#include <QtGlobal>

#include "MainWindow.h"

const QString style = "Fusion";
const QString appstylePath = ":/recources/style/appstyles.qss";

int main(int argc, char *argv[]) {
#if defined(Q_OS_UNIX) && !defined(Q_OS_DARWIN)
    // the qads library has issues on wayland so we force qt to use x11 instead
    // see https://invent.kde.org/education/labplot/-/issues/1067
    QByteArray xcbQtQpaEnvVar("xcb");
    qputenv("QT_QPA_PLATFORM", xcbQtQpaEnvVar);
#endif

    QApplication app(argc, argv);

    MainWindow window;
    window.showMaximized();

    return app.exec();
}
