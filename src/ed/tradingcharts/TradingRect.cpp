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

    bool mIsDrawing;

    QPointF mDragStart;
    QPointF mStartTopLeft;
    QPointF mCurWantedPosPx;
    QPointF mStartBottomRight;
    QTimer *mMoveTimer;

    QCPLayer *mUserLayer;
    EResizeHandle *mResizeTopLeft;
    EResizeHandle *mResizeBottomRight;
    ETradingPlot *mParent;
};

ETradingRect::ETradingRect(ETradingPlot *parent) : QCPItemRect(parent), d(new Private) {
    d->mParent = parent;
    d->mIsDrawing = false;
    d->mMoveTimer = new QTimer();
    d->mDragStart = QPointF();
    d->mStartTopLeft = QPointF();
    d->mStartBottomRight = QPointF();
    d->mUserLayer = parent->userLayer();
    d->mCurWantedPosPx = QPointF();
    d->mResizeTopLeft = nullptr;
    d->mResizeBottomRight = nullptr;

    topLeft->setType(QCPItemPosition::ptPlotCoords);
    bottomRight->setType(QCPItemPosition::ptPlotCoords);
    setAllowFilledRect(false);

    setSelectable(true);
    setColor(QColor(35, 125, 100, 255));
    setPen(QPen(Qt::blue));
    setSelectedPen(QPen(Qt::red, 1));
    setLayer(d->mUserLayer);

    d->mMoveTimer->setInterval(25);  // 40 FPS
    connect(d->mMoveTimer, SIGNAL(timeout()), this, SLOT(moveToWantedPos()));
}

ETradingRect::~ETradingRect() {
    d->mMoveTimer->stop();
    delete d->mMoveTimer;

    if (parentPlot()->hasItem(d->mResizeTopLeft)) {
        parentPlot()->removeItem(d->mResizeTopLeft);
    }

    if (parentPlot()->hasItem(d->mResizeBottomRight)) {
        parentPlot()->removeItem(d->mResizeBottomRight);
    }
    delete d;
}

void ETradingRect::setActive(bool isActive) {
    setSelected(isActive);

    d->mResizeTopLeft->setVisible(isActive);
    d->mResizeBottomRight->setVisible(isActive);

    if (!isActive) {
        d->mResizeTopLeft->setActive(isActive);
        d->mResizeBottomRight->setActive(isActive);
    }

    Q_EMIT(isActive ? activated() : disactivated());
}

void ETradingRect::startMoving(const QPointF &mousePos, bool shiftIsPressed) {
    d->mIsDrawing = false;
    d->mDragStart = mousePos;
    d->mStartTopLeft = this->topLeft->coords();
    d->mStartBottomRight = this->bottomRight->coords();
    d->mMoveTimer->start();

    connect(parentPlot(), SIGNAL(mouseMove(QMouseEvent *)), this, SLOT(onMouseMove(QMouseEvent *)));
    connect(parentPlot(), SIGNAL(mouseRelease(QMouseEvent *)), this, SLOT(onCompletedMoving()));

    parentPlot()->grabKeyboard();
    QApplication::setOverrideCursor(Qt::ClosedHandCursor);
}

bool ETradingRect::isResizeable(const QPointF &mousePos) {
    if (isTopLeftResize(mousePos)) {
        return true;
    }

    if (isBottomRightResize(mousePos)) {
        return true;
    }

    return false;
}

void ETradingRect::startResizing(const QPointF &mousePos, bool shiftIsPressed) {
    d->mIsDrawing = false;
    if (isTopLeftResize(mousePos)) {
        d->mResizeTopLeft->setActive(true);
        d->mResizeBottomRight->setActive(false);
        d->mResizeTopLeft->startMoving(EResizeHandle::Mode::mResizing, mousePos, shiftIsPressed);
    } else {
        d->mResizeTopLeft->setActive(false);
        d->mResizeBottomRight->setActive(true);
        d->mResizeBottomRight->startMoving(EResizeHandle::Mode::mResizing, mousePos, shiftIsPressed);
    }
}

void ETradingRect::startDrawing(const QPointF &mousePos) {
    d->mIsDrawing = true;
    double x = parentPlot()->xAxis->pixelToCoord(mousePos.x());
    double y = parentPlot()->yAxis->pixelToCoord(mousePos.y());

    moveCoord(x, y, x, y);

    d->mResizeBottomRight->startMoving(EResizeHandle::Mode::mDrawing, mousePos, false);

    d->mUserLayer->replot();
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

void ETradingRect::onCompletedMoving() {
    disconnect(parentPlot(), SIGNAL(mouseMove(QMouseEvent *)), this, SLOT(onMouseMove(QMouseEvent *)));
    disconnect(parentPlot(), SIGNAL(mouseRelease(QMouseEvent *)), this, SLOT(onCompletedMoving()));

    d->mMoveTimer->stop();
    moveToWantedPos();

    d->mUserLayer->replot();

    parentPlot()->releaseKeyboard();
    QApplication::restoreOverrideCursor();

    Q_EMIT completedMoving();
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

void ETradingRect::onMouseMove(QMouseEvent *event) {
    d->mCurWantedPosPx = QPointF(
        parentPlot()->xAxis->pixelToCoord(event->pos().x()) - parentPlot()->xAxis->pixelToCoord(d->mDragStart.x()),
        parentPlot()->yAxis->pixelToCoord(event->pos().y()) - parentPlot()->yAxis->pixelToCoord(d->mDragStart.y()));
}

void ETradingRect::moveToWantedPos() {
    if (!d->mCurWantedPosPx.isNull()) {
        moveCoord(d->mStartTopLeft.x() + d->mCurWantedPosPx.x(), d->mStartTopLeft.y() + d->mCurWantedPosPx.y(),
                  d->mStartBottomRight.x() + d->mCurWantedPosPx.x(), d->mStartBottomRight.y() + d->mCurWantedPosPx.y());
        d->mCurWantedPosPx = QPointF();
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

    d->mResizeTopLeft = new EResizeHandle(d->mParent);
    d->mResizeTopLeft->setVisible(false);

    connect(d->mResizeTopLeft, &EResizeHandle::startingMoving, this, [this]() {
        connect(d->mResizeTopLeft, SIGNAL(moved(const QPointF &)), this, SLOT(topLeftMoving(const QPointF &)));
    });
    connect(d->mResizeTopLeft, &EResizeHandle::completedMoving, this, [this]() {
        this->d->mResizeTopLeft->setActive(false);
        disconnect(d->mResizeTopLeft, SIGNAL(moved(const QPointF &)), this, SLOT(topLeftMoving(const QPointF &)));

        if (d->mIsDrawing) {
            Q_EMIT drawingCompleted(true);
        }
    });

    connect(d->mResizeTopLeft, &EResizeHandle::cancelledMoving, this, [this]() {
        this->d->mResizeTopLeft->setActive(false);
        disconnect(d->mResizeTopLeft, SIGNAL(moved(const QPointF &)), this, SLOT(topLeftMoving(const QPointF &)));

        if (d->mIsDrawing) {
            Q_EMIT drawingCompleted(false);
        }
    });
}

void ETradingRect::createBottomRightResize() {
    if (d->mResizeBottomRight != nullptr) {
        return;
    }

    d->mResizeBottomRight = new EResizeHandle(d->mParent);
    d->mResizeBottomRight->setVisible(false);

    connect(d->mResizeBottomRight, &EResizeHandle::startingMoving, this, [this]() {
        connect(d->mResizeBottomRight, SIGNAL(moved(const QPointF &)), this, SLOT(bottomRightMoving(const QPointF &)));
    });
    connect(d->mResizeBottomRight, &EResizeHandle::completedMoving, this, [this]() {
        this->d->mResizeBottomRight->setActive(false);
        disconnect(d->mResizeBottomRight, SIGNAL(moved(const QPointF &)), this,
                   SLOT(bottomRightMoving(const QPointF &)));
        if (d->mIsDrawing) {
            Q_EMIT drawingCompleted(false);
        }
    });

    connect(d->mResizeBottomRight, &EResizeHandle::cancelledMoving, this, [this]() {
        this->d->mResizeBottomRight->setActive(false);
        disconnect(d->mResizeBottomRight, SIGNAL(moved(const QPointF &)), this,
                   SLOT(bottomRightMoving(const QPointF &)));

        if (d->mIsDrawing) {
            Q_EMIT drawingCompleted(true);
        }
    });
}

bool ETradingRect::isTopLeftResize(const QPointF &mousePos) {
    QPointF pTopLeft =
        QPointF(parentPlot()->xAxis->coordToPixel(topLeft->key()), parentPlot()->yAxis->coordToPixel(topLeft->value()));
    double distance = ed::interal::distance(pTopLeft, mousePos);

    return (distance < 10.0);
}

bool ETradingRect::isBottomRightResize(const QPointF &mousePos) {
    QPointF pBottomRight = QPointF(parentPlot()->xAxis->coordToPixel(bottomRight->key()),
                                   parentPlot()->yAxis->coordToPixel(bottomRight->value()));
    double distance = ed::interal::distance(pBottomRight, mousePos);

    return (distance < 10.0);
}
}  // namespace ed