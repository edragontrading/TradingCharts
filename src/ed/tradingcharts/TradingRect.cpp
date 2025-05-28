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
    EResizeHandle *mResizeSelect;
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
    d->mResizeSelect = nullptr;

    topLeft->setType(QCPItemPosition::ptPlotCoords);
    bottomRight->setType(QCPItemPosition::ptPlotCoords);
    setAllowFilledRect(false);

    setSelectable(true);
    setColor(QColor(0x9c, 0x27, 0xb0, 51));
    setPen(QColor(0x9c, 0x27, 0xb0, 255));
    setSelectedPen(QColor(0x9c, 0x27, 0xb0, 255));
    setLayer(d->mUserLayer);

    d->mMoveTimer->setInterval(25);  // 40 FPS
    connect(d->mMoveTimer, SIGNAL(timeout()), this, SLOT(moveToWantedPos()));
}

ETradingRect::~ETradingRect() {
    d->mMoveTimer->stop();
    delete d->mMoveTimer;

    if (d->mParent->hasItem(d->mResizeTopLeft)) {
        d->mParent->removeItem(d->mResizeTopLeft);
    }

    if (d->mParent->hasItem(d->mResizeBottomRight)) {
        d->mParent->removeItem(d->mResizeBottomRight);
    }
    delete d;
}

void ETradingRect::init() {
    if (d->mResizeTopLeft == nullptr) {
        d->mResizeTopLeft = createPointResize();
    }

    if (d->mResizeBottomRight == nullptr) {
        d->mResizeBottomRight = createPointResize();
    }
}

void ETradingRect::setChoosen(bool on) {
    setSelected(on);
    setResizeActive(on);

    if (!on) {
        setResizeVisible(on);
    }
}

void ETradingRect::setVisible(bool on) {
    QCPItemRect::setVisible(on);
}

void ETradingRect::setActive(bool isActive) {
    setSelected(isActive);
    setResizeVisible(isActive);
}

void ETradingRect::startMoving(const QPointF &mousePos, bool shiftIsPressed) {
    d->mIsDrawing = false;
    d->mDragStart = mousePos;
    d->mStartTopLeft = this->topLeft->coords();
    d->mStartBottomRight = this->bottomRight->coords();
    d->mMoveTimer->start();

    connect(d->mParent, SIGNAL(mouseMove(QMouseEvent *)), this, SLOT(onMouseMove(QMouseEvent *)));
    connect(d->mParent, SIGNAL(mouseRelease(QMouseEvent *)), this, SLOT(onCompletedMoving()));

    d->mParent->grabKeyboard();
    QApplication::setOverrideCursor(Qt::ClosedHandCursor);
    d->mUserLayer->replot();
}

bool ETradingRect::isResizeable(const QPointF &mousePos) {
    if (ed::internal::near(this->topLeft->pixelPosition(), mousePos)) {
        return true;
    }

    if (ed::internal::near(this->bottomRight->pixelPosition(), mousePos)) {
        return true;
    }

    return false;
}

void ETradingRect::startResizing(const QPointF &mousePos, bool shiftIsPressed) {
    d->mIsDrawing = false;
    if (ed::internal::near(this->topLeft->pixelPosition(), mousePos)) {
        d->mResizeSelect = d->mResizeTopLeft;
    } else {
        d->mResizeSelect = d->mResizeBottomRight;
    }

    setResizeActive(false);
    d->mResizeSelect->setActive(true);
    d->mResizeSelect->startMoving(EResizeHandle::Mode::mResizing, mousePos, shiftIsPressed);
}

void ETradingRect::startDrawing(const QPointF &mousePos) {
    d->mIsDrawing = true;

    QPointF pos = d->mParent->pixelsToCoords(mousePos.x(), mousePos.y());
    moveCoord(pos.x(), pos.y(), pos.x(), pos.y());

    d->mResizeSelect = d->mResizeBottomRight;
    d->mResizeSelect->startMoving(EResizeHandle::Mode::mDrawing, mousePos, false);
}

const QColor &ETradingRect::color() const {
    return brush().color();
}

void ETradingRect::setColor(const QColor &color) {
    setBrush(color);
    setSelectedBrush(color);
}

void ETradingRect::onCompletedMoving() {
    disconnect(d->mParent, SIGNAL(mouseMove(QMouseEvent *)), this, SLOT(onMouseMove(QMouseEvent *)));
    disconnect(d->mParent, SIGNAL(mouseRelease(QMouseEvent *)), this, SLOT(onCompletedMoving()));

    d->mMoveTimer->stop();
    moveToWantedPos();

    d->mUserLayer->replot();

    d->mParent->releaseKeyboard();
    QApplication::restoreOverrideCursor();

    Q_EMIT completedMoving();
}

void ETradingRect::moveCoord(double x1, double y1, double x2, double y2) {
    // X axis is integer
    x1 = std::round(x1);
    x2 = std::round(x2);

    topLeft->setCoords(x1, y1);
    bottomRight->setCoords(x2, y2);

    d->mResizeTopLeft->moveCoord(x1, y1);
    d->mResizeBottomRight->moveCoord(x2, y2);

    Q_EMIT moved(QPointF(x1, y1));
    d->mUserLayer->replot();
}

void ETradingRect::onMouseMove(QMouseEvent *event) {
    QPointF p1 = d->mParent->pixelsToCoords(event->pos().x(), event->pos().y());
    QPointF p2 = d->mParent->pixelsToCoords(d->mDragStart.x(), d->mDragStart.y());
    d->mCurWantedPosPx = p1 - p2;
}

void ETradingRect::moveToWantedPos() {
    if (d->mCurWantedPosPx.isNull()) {
        return;
    }

    double x1 = d->mStartTopLeft.x() + d->mCurWantedPosPx.x();
    double y1 = d->mStartTopLeft.y() + d->mCurWantedPosPx.y();

    double x2 = d->mStartBottomRight.x() + d->mCurWantedPosPx.x();
    double y2 = d->mStartBottomRight.y() + d->mCurWantedPosPx.y();

    moveCoord(x1, y1, x2, y2);
    d->mCurWantedPosPx = QPointF();
}

void ETradingRect::pointMoving(const QPointF &pos) {
    if (d->mResizeSelect == d->mResizeTopLeft) {
        this->topLeft->setCoords(pos);
    } else if (d->mResizeSelect == d->mResizeBottomRight) {
        this->bottomRight->setCoords(pos);
    }
}

EResizeHandle *ETradingRect::createPointResize() {
    EResizeHandle *resizePoint = new EResizeHandle(d->mParent);
    resizePoint->setVisible(false);

    connect(resizePoint, &EResizeHandle::startingMoving, this, [this, resizePoint]() {
        connect(resizePoint, SIGNAL(moved(const QPointF &)), this, SLOT(pointMoving(const QPointF &)));
    });
    connect(resizePoint, &EResizeHandle::completedMoving, this, [this]() { resizePointStoppedMoving(false); });
    connect(resizePoint, &EResizeHandle::cancelledMoving, this, [this]() { resizePointStoppedMoving(true); });

    return resizePoint;
}

void ETradingRect::resizePointStoppedMoving(bool cancelled) {
    setResizeActive(!d->mIsDrawing);
    disconnect(d->mResizeSelect, SIGNAL(moved(const QPointF &)), this, SLOT(pointMoving(const QPointF &)));

    if (d->mIsDrawing) {
        Q_EMIT drawingCompleted(cancelled);
        return;
    }

    d->mUserLayer->replot();
}

void ETradingRect::setResizeActive(bool active) {
    this->d->mResizeTopLeft->setActive(active);
    this->d->mResizeBottomRight->setActive(active);
}

void ETradingRect::setResizeVisible(bool visible) {
    this->d->mResizeTopLeft->setVisible(visible);
    this->d->mResizeBottomRight->setVisible(visible);
}
}  // namespace ed