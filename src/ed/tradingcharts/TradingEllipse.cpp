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
#include <ed/tradingcharts/TradingEllipse.h>
#include <ed/tradingcharts/TradingPlot.h>

#include <QPointF>

namespace ed {

struct ETradingEllipse::Private {
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

ETradingEllipse::ETradingEllipse(ETradingPlot *parent) : QCPItemEllipse(parent), d(new Private) {
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
    setColor(QColor(0x9c, 0x27, 0xb0, 51));
    setPen(QColor(0x9c, 0x27, 0xb0, 255));
    setSelectedPen(QColor(0x9c, 0x27, 0xb0, 255));
    setLayer(d->mUserLayer);

    d->mMoveTimer->setInterval(25);  // 40 FPS
    connect(d->mMoveTimer, SIGNAL(timeout()), this, SLOT(moveToWantedPos()));
}

ETradingEllipse::~ETradingEllipse() {
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

void ETradingEllipse::init() {
    createTopLeftResize();
    createBottomRightResize();
}

void ETradingEllipse::setChoosen(bool on) {
    setSelected(on);
    setResizeActive(on);

    if (!on) {
        setResizeVisible(on);
    }
}

void ETradingEllipse::setVisible(bool on) {
    QCPItemEllipse::setVisible(on);
}

void ETradingEllipse::setActive(bool isActive) {
    setSelected(isActive);
    setResizeVisible(isActive);
}

void ETradingEllipse::startMoving(const QPointF &mousePos, bool shiftIsPressed) {
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

bool ETradingEllipse::isResizeable(const QPointF &mousePos) {
    if (isPointResize(topLeft, mousePos)) {
        return true;
    }

    if (isPointResize(bottomRight, mousePos)) {
        return true;
    }

    return false;
}

void ETradingEllipse::startResizing(const QPointF &mousePos, bool shiftIsPressed) {
    d->mIsDrawing = false;
    if (isPointResize(this->topLeft, mousePos)) {
        setResizeActive(false);
        d->mResizeTopLeft->setActive(true);
        d->mResizeTopLeft->startMoving(EResizeHandle::Mode::mResizing, mousePos, shiftIsPressed);
    } else {
        setResizeActive(false);
        d->mResizeBottomRight->setActive(true);
        d->mResizeBottomRight->startMoving(EResizeHandle::Mode::mResizing, mousePos, shiftIsPressed);
    }
}

void ETradingEllipse::startDrawing(const QPointF &mousePos) {
    d->mIsDrawing = true;
    QPointF pos = d->mParent->pixelsToCoords(mousePos.x(), mousePos.y());

    moveCoord(pos.x(), pos.y(), pos.x(), pos.y());
    d->mResizeBottomRight->startMoving(EResizeHandle::Mode::mDrawing, mousePos, false);
}

const QColor &ETradingEllipse::color() const {
    return brush().color();
}

void ETradingEllipse::setColor(const QColor &color) {
    setBrush(color);
    setSelectedBrush(color);
}

void ETradingEllipse::onCompletedMoving() {
    disconnect(d->mParent, SIGNAL(mouseMove(QMouseEvent *)), this, SLOT(onMouseMove(QMouseEvent *)));
    disconnect(d->mParent, SIGNAL(mouseRelease(QMouseEvent *)), this, SLOT(onCompletedMoving()));

    d->mMoveTimer->stop();
    moveToWantedPos();

    d->mUserLayer->replot();

    d->mParent->releaseKeyboard();
    QApplication::restoreOverrideCursor();

    Q_EMIT completedMoving();
}

void ETradingEllipse::moveCoord(double x1, double y1, double x2, double y2) {
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

void ETradingEllipse::onMouseMove(QMouseEvent *event) {
    QPointF p1 = d->mParent->pixelsToCoords(event->pos().x(), event->pos().y());
    QPointF p2 = d->mParent->pixelsToCoords(d->mDragStart.x(), d->mDragStart.y());
    d->mCurWantedPosPx = p1 - p2;
}

void ETradingEllipse::moveToWantedPos() {
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

void ETradingEllipse::topLeftMoving(const QPointF &pos) {
    this->topLeft->setCoords(pos);
}

void ETradingEllipse::bottomRightMoving(const QPointF &pos) {
    this->bottomRight->setCoords(pos);
}

void ETradingEllipse::createTopLeftResize() {
    if (d->mResizeTopLeft != nullptr) {
        return;
    }

    d->mResizeTopLeft = new EResizeHandle(d->mParent);
    d->mResizeTopLeft->setVisible(false);

    connect(d->mResizeTopLeft, &EResizeHandle::startingMoving, this, [this]() {
        connect(d->mResizeTopLeft, SIGNAL(moved(const QPointF &)), this, SLOT(topLeftMoving(const QPointF &)));
    });
    connect(d->mResizeTopLeft, &EResizeHandle::completedMoving, this, [this]() {
        resizeTopLeftStoppedMoving();
        if (d->mIsDrawing) {
            Q_EMIT drawingCompleted(true);
        }
    });

    connect(d->mResizeTopLeft, &EResizeHandle::cancelledMoving, this, [this]() {
        resizeTopLeftStoppedMoving();
        if (d->mIsDrawing) {
            Q_EMIT drawingCompleted(false);
        }
    });
}

void ETradingEllipse::createBottomRightResize() {
    if (d->mResizeBottomRight != nullptr) {
        return;
    }

    d->mResizeBottomRight = new EResizeHandle(d->mParent);
    d->mResizeBottomRight->setVisible(false);

    connect(d->mResizeBottomRight, &EResizeHandle::startingMoving, this, [this]() {
        connect(d->mResizeBottomRight, SIGNAL(moved(const QPointF &)), this, SLOT(bottomRightMoving(const QPointF &)));
    });
    connect(d->mResizeBottomRight, &EResizeHandle::completedMoving, this, [this]() {
        resizeBottomRightStoppedMoving();
        if (d->mIsDrawing) {
            Q_EMIT drawingCompleted(false);
        }
    });

    connect(d->mResizeBottomRight, &EResizeHandle::cancelledMoving, this, [this]() {
        resizeBottomRightStoppedMoving();
        if (d->mIsDrawing) {
            Q_EMIT drawingCompleted(true);
        }
    });
}

void ETradingEllipse::resizeTopLeftStoppedMoving() {
    setResizeActive(!d->mIsDrawing);
    disconnect(d->mResizeTopLeft, SIGNAL(moved(const QPointF &)), this, SLOT(topLeftMoving(const QPointF &)));
}

void ETradingEllipse::resizeBottomRightStoppedMoving() {
    setResizeActive(!d->mIsDrawing);
    disconnect(d->mResizeBottomRight, SIGNAL(moved(const QPointF &)), this, SLOT(bottomRightMoving(const QPointF &)));
}

bool ETradingEllipse::isPointResize(const QCPItemPosition *pos, const QPointF &mousePos) {
    QPointF point = d->mParent->coordsToPixels(pos->key(), pos->value());
    double distance = ed::interal::distance(point, mousePos);

    return (distance < 10.0);
}

void ETradingEllipse::setResizeActive(bool active) {
    this->d->mResizeTopLeft->setActive(active);
    this->d->mResizeBottomRight->setActive(active);
}

void ETradingEllipse::setResizeVisible(bool visible) {
    this->d->mResizeTopLeft->setVisible(visible);
    this->d->mResizeBottomRight->setVisible(visible);
}

double ETradingEllipse::selectTest(const QPointF &pos, bool onlySelectable, QVariant *details) const {
    if (onlySelectable && !mSelectable) return -1;

    QPointF pTopLeft = d->mParent->coordsToPixels(topLeft->key(), topLeft->value());
    QPointF pBottomRight = d->mParent->coordsToPixels(bottomRight->key(), bottomRight->value());

    double result = QCPItemEllipse::selectTest(pos, onlySelectable, details);
    result = std::min(result, ed::interal::distance(pTopLeft, pos));
    result = std::min(result, ed::interal::distance(pBottomRight, pos));

    return result;
}
}  // namespace ed