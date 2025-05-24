/*******************************************************************************
** Qt Trading Charts System
** Copyright (C) 2025 ED Trading
**
** This library is free software; you can redistribute it and/or
** modify it under the terms of the GNU Lesser General Public
** License as published by the Free Software Foundation; either
** version 3.0 of the License, or (at your option) any later version.
**
** This library is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
** Lesser General Public License for more details.
**
** You should have received a copy of the GNU Lesser General Public
** License along with this library; If not, see <http://www.gnu.org/licenses/>.
******************************************************************************/

//============================================================================
/// \author Phuoc Truong
/// \date   21.05.2025
//============================================================================

#include <ed/tradingcharts/ResizeHandle.h>
#include <ed/tradingcharts/TradingPlot.h>
#include <ed/tradingcharts/TradingRect.h>

#include <QPointF>

namespace ed {

struct ETradingRect::Private {
    Private() = default;

    QPointF mGripDeltaTopLeft;
    QPointF mGripDeltaBottomRight;
    QPointF mCurWantedPosPxTopLeft;
    QPointF mCurWantedPosPxBottomRight;
    QTimer *mMoveTimer;

    QCPLayer *mUserLayer;
    bool mIsMoving;

    EResizeHandle *mResizeTopLeft;
    EResizeHandle *mResizeBottomRight;
};

ETradingRect::ETradingRect(ETradingPlot *parent) : QCPItemRect(parent), d(new Private) {
    d->mMoveTimer = new QTimer(this);
    d->mIsMoving = false;
    d->mGripDeltaTopLeft = QPointF();
    d->mGripDeltaBottomRight = QPointF();
    d->mCurWantedPosPxTopLeft = QPointF();
    d->mCurWantedPosPxBottomRight = QPointF();
    d->mUserLayer = parent->userLayer();

    topLeft->setType(QCPItemPosition::ptPlotCoords);
    bottomRight->setType(QCPItemPosition::ptPlotCoords);
    setAllowFilledRect(false);

    setSelectable(true);
    setColor(QColor(35, 125, 100, 255));
    setPen(QPen(Qt::blue));
    setSelectedPen(QPen(Qt::red, 3));
    setLayer(d->mUserLayer);

    d->mMoveTimer->setInterval(25);  // 40 FPS
    connect(d->mMoveTimer, SIGNAL(timeout()), this, SLOT(moveToWantedPos()));
}

ETradingRect::~ETradingRect() {
    delete d;
}

void ETradingRect::setActive(bool isActive) {
    setSelected(isActive);

    createTopLeftResize();
    createBottomRightResize();

    d->mResizeTopLeft->setVisible(isActive);
    d->mResizeBottomRight->setVisible(isActive);

    Q_EMIT(isActive ? activated() : disactivated());
}

void ETradingRect::startMoving(const QPointF &mousePos, bool shiftIsPressed) {
    d->mIsMoving = true;
    d->mGripDeltaTopLeft.setX(parentPlot()->xAxis->coordToPixel(topLeft->key()) - mousePos.x());
    d->mGripDeltaTopLeft.setY(parentPlot()->yAxis->coordToPixel(topLeft->value()) - mousePos.y());

    d->mGripDeltaBottomRight.setX(parentPlot()->xAxis->coordToPixel(bottomRight->key()) - mousePos.x());
    d->mGripDeltaBottomRight.setX(parentPlot()->xAxis->coordToPixel(bottomRight->value()) - mousePos.y());

    d->mCurWantedPosPxTopLeft = QPointF();
    d->mCurWantedPosPxBottomRight = QPointF();
    d->mMoveTimer->start();

    connect(parentPlot(), SIGNAL(mouseMove(QMouseEvent *)), this, SLOT(onMouseMove(QMouseEvent *)));
    connect(parentPlot(), SIGNAL(mouseRelease(QMouseEvent *)), this, SLOT(stopMoving()));

    parentPlot()->grabKeyboard();
    QApplication::setOverrideCursor(Qt::ClosedHandCursor);
}

bool ETradingRect::isMoving() {
    return d->mIsMoving;
}

const QColor &ETradingRect::color() const {
    return brush().color();
}

void ETradingRect::setColor(const QColor &color) {
    setBrush(color);
    setSelectedBrush(color);
}

void ETradingRect::setVisible(bool on) {
    QCPItemRect::setVisible(on);
}

void ETradingRect::stopMoving() {
    d->mIsMoving = false;

    disconnect(parentPlot(), SIGNAL(mouseMove(QMouseEvent *)), this, SLOT(onMouseMove(QMouseEvent *)));
    disconnect(parentPlot(), SIGNAL(mouseRelease(QMouseEvent *)), this, SLOT(stopMoving()));

    d->mMoveTimer->stop();
    moveToWantedPos();

    d->mUserLayer->replot();

    parentPlot()->releaseKeyboard();
    QApplication::restoreOverrideCursor();

    Q_EMIT stoppedMoving();
}

void ETradingRect::moveCoord(double x1, double y1, double x2, double y2) {
    // X axis is integer
    x1 = std::round(x1);
    x2 = std::round(x2);

    topLeft->setCoords(x1, y1);
    bottomRight->setCoords(x2, y2);

    createTopLeftResize();
    createBottomRightResize();

    d->mResizeTopLeft->moveCoord(x1, y1);
    d->mResizeBottomRight->moveCoord(x2, y2);

    Q_EMIT moved(QPointF(x1, y1));
    d->mUserLayer->replot();
}

void ETradingRect::movePixel(double x1, double y1, double x2, double y2) {
    moveCoord(parentPlot()->xAxis->pixelToCoord(x1), parentPlot()->yAxis->pixelToCoord(y1),
              parentPlot()->xAxis->pixelToCoord(x2), parentPlot()->yAxis->pixelToCoord(y2));
}

void ETradingRect::onMouseMove(QMouseEvent *event) {
    d->mCurWantedPosPxTopLeft =
        QPointF(event->position().x() + d->mGripDeltaTopLeft.x(), event->position().y() + d->mGripDeltaTopLeft.y());
    d->mCurWantedPosPxBottomRight = QPointF(event->position().x() + d->mGripDeltaBottomRight.x(),
                                            event->position().y() + d->mGripDeltaBottomRight.y());
}

void ETradingRect::moveToWantedPos() {
    if (!d->mCurWantedPosPxTopLeft.isNull() && !d->mCurWantedPosPxBottomRight.isNull()) {
        movePixel(d->mCurWantedPosPxTopLeft.x(), d->mCurWantedPosPxTopLeft.y(), d->mCurWantedPosPxBottomRight.x(),
                  d->mCurWantedPosPxBottomRight.y());
        d->mCurWantedPosPxTopLeft = QPointF();
        d->mCurWantedPosPxBottomRight = QPointF();
    }
}

void ETradingRect::topLeftMoving(const QPointF &pos) {
    this->topLeft->setCoords(pos);
}

void ETradingRect::bottomRightMoving(const QPointF &pos) {
    this->bottomRight->setCoords(pos);
}

void ETradingRect::createTopLeftResize() {
    if (d->mResizeTopLeft != nullptr) {
        return;
    }
    d->mResizeTopLeft = new EResizeHandle(qobject_cast<ETradingPlot *>(parentPlot()));

    connect(d->mResizeTopLeft, &EResizeHandle::startingMoving, this, [this]() {
        connect(d->mResizeTopLeft, SIGNAL(moved(const QPointF &)), this, SLOT(topLeftMoving(const QPointF &)));
    });
    connect(d->mResizeTopLeft, &EResizeHandle::stoppedMoving, this, [this]() {
        disconnect(d->mResizeTopLeft, SIGNAL(moved(const QPointF &)), this, SLOT(topLeftMoving(const QPointF &)));
    });

    connect(d->mResizeTopLeft, &EResizeHandle::activated, this, [this]() { this->setActive(true); });
    connect(d->mResizeTopLeft, &EResizeHandle::disactivated, this, [this]() { this->setActive(false); });
}

void ETradingRect::createBottomRightResize() {
    if (d->mResizeBottomRight != nullptr) {
        return;
    }

    d->mResizeBottomRight = new EResizeHandle(qobject_cast<ETradingPlot *>(parentPlot()));

    connect(d->mResizeBottomRight, &EResizeHandle::startingMoving, this, [this]() {
        connect(d->mResizeBottomRight, SIGNAL(moved(const QPointF &)), this, SLOT(bottomRightMoving(const QPointF &)));
    });
    connect(d->mResizeBottomRight, &EResizeHandle::stoppedMoving, this, [this]() {
        disconnect(d->mResizeBottomRight, SIGNAL(moved(const QPointF &)), this,
                   SLOT(bottomRightMoving(const QPointF &)));
    });

    connect(d->mResizeBottomRight, &EResizeHandle::activated, this, [this]() { this->setActive(true); });
    connect(d->mResizeBottomRight, &EResizeHandle::disactivated, this, [this]() { this->setActive(false); });
}

}  // namespace ed