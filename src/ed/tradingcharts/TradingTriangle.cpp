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
#include <ed/tradingcharts/TradingTriangle.h>

#include <QPointF>

namespace ed {

struct ETradingTriangle::Private {
    Private() = default;

    bool mIsDrawing;

    QPointF mDragStart;
    QPointF mStartPoint1;
    QPointF mStartPoint2;
    QPointF mStartPoint3;
    QPointF mCurWantedPosPx;
    QTimer *mMoveTimer;

    QCPLayer *mUserLayer;
    EResizeHandle *mResizePoint1;
    EResizeHandle *mResizePoint2;
    EResizeHandle *mResizePoint3;
    ETradingPlot *mParent;
};

ETradingTriangle::ETradingTriangle(ETradingPlot *parent) : QCPItemTriangle(parent), d(new Private) {
    d->mParent = parent;
    d->mIsDrawing = false;
    d->mMoveTimer = new QTimer();
    d->mDragStart = QPointF();
    d->mStartPoint1 = QPointF();
    d->mStartPoint2 = QPointF();
    d->mStartPoint3 = QPointF();
    d->mUserLayer = parent->userLayer();
    d->mCurWantedPosPx = QPointF();
    d->mResizePoint1 = nullptr;
    d->mResizePoint2 = nullptr;
    d->mResizePoint3 = nullptr;

    this->point1->setType(QCPItemPosition::ptPlotCoords);
    this->point2->setType(QCPItemPosition::ptPlotCoords);
    this->point3->setType(QCPItemPosition::ptPlotCoords);
    setAllowFilledRect(false);

    setSelectable(true);
    setColor(QColor(0x9c, 0x27, 0xb0, 51));
    setPen(QColor(0x9c, 0x27, 0xb0, 255));
    setSelectedPen(QColor(0x9c, 0x27, 0xb0, 255));
    setLayer(d->mUserLayer);

    d->mMoveTimer->setInterval(25);  // 40 FPS
    connect(d->mMoveTimer, SIGNAL(timeout()), this, SLOT(moveToWantedPos()));
}

ETradingTriangle::~ETradingTriangle() {
    d->mMoveTimer->stop();
    delete d->mMoveTimer;

    if (d->mParent->hasItem(d->mResizePoint1)) {
        d->mParent->removeItem(d->mResizePoint1);
    }

    if (d->mParent->hasItem(d->mResizePoint2)) {
        d->mParent->removeItem(d->mResizePoint2);
    }

    if (d->mParent->hasItem(d->mResizePoint3)) {
        d->mParent->removeItem(d->mResizePoint3);
    }
    delete d;
}

void ETradingTriangle::init() {
    createPoint1Resize();
    createPoint2Resize();
    createPoint3Resize();
}

void ETradingTriangle::setChoosen(bool on) {
    setSelected(on);
    setResizeActive(on);

    if (!on) {
        setResizeVisible(on);
    }
}

void ETradingTriangle::setVisible(bool on) {
    QCPItemTriangle::setVisible(on);
}

void ETradingTriangle::setActive(bool isActive) {
    setSelected(isActive);
    setResizeVisible(isActive);
}

void ETradingTriangle::startMoving(const QPointF &mousePos, bool shiftIsPressed) {
    d->mIsDrawing = false;
    d->mDragStart = mousePos;
    d->mStartPoint1 = this->point1->coords();
    d->mStartPoint2 = this->point2->coords();
    d->mStartPoint3 = this->point3->coords();
    d->mMoveTimer->start();

    connect(d->mParent, SIGNAL(mouseMove(QMouseEvent *)), this, SLOT(onMouseMove(QMouseEvent *)));
    connect(d->mParent, SIGNAL(mouseRelease(QMouseEvent *)), this, SLOT(onCompletedMoving()));

    d->mParent->grabKeyboard();
    QApplication::setOverrideCursor(Qt::ClosedHandCursor);
    d->mUserLayer->replot();
}

bool ETradingTriangle::isResizeable(const QPointF &mousePos) {
    if (isPointResize(this->point1, mousePos)) {
        return true;
    }

    if (isPointResize(this->point2, mousePos)) {
        return true;
    }

    if (isPointResize(this->point3, mousePos)) {
        return true;
    }

    return false;
}

void ETradingTriangle::startResizing(const QPointF &mousePos, bool shiftIsPressed) {
    d->mIsDrawing = false;
    if (isPointResize(this->point1, mousePos)) {
        setResizeActive(false);
        d->mResizePoint1->setActive(true);
        d->mResizePoint1->startMoving(EResizeHandle::Mode::mResizing, mousePos, shiftIsPressed);
    } else if (isPointResize(this->point2, mousePos)) {
        setResizeActive(false);
        d->mResizePoint2->setActive(true);
        d->mResizePoint2->startMoving(EResizeHandle::Mode::mResizing, mousePos, shiftIsPressed);
    } else {
        setResizeActive(false);
        d->mResizePoint3->setActive(true);
        d->mResizePoint3->startMoving(EResizeHandle::Mode::mResizing, mousePos, shiftIsPressed);
    }
}

void ETradingTriangle::startDrawing(const QPointF &mousePos) {
    d->mIsDrawing = true;
    QPointF pos = d->mParent->pixelsToCoords(mousePos.x(), mousePos.y());

    moveCoord(pos.x(), pos.y(), pos.x(), pos.y(), pos.x(), pos.y());
    d->mResizePoint2->startMoving(EResizeHandle::Mode::mDrawing, mousePos, false);
}

const QColor &ETradingTriangle::color() const {
    return brush().color();
}

void ETradingTriangle::setColor(const QColor &color) {
    setBrush(color);
    setSelectedBrush(color);
}

void ETradingTriangle::onCompletedMoving() {
    disconnect(d->mParent, SIGNAL(mouseMove(QMouseEvent *)), this, SLOT(onMouseMove(QMouseEvent *)));
    disconnect(d->mParent, SIGNAL(mouseRelease(QMouseEvent *)), this, SLOT(onCompletedMoving()));

    d->mMoveTimer->stop();
    moveToWantedPos();

    d->mUserLayer->replot();

    d->mParent->releaseKeyboard();
    QApplication::restoreOverrideCursor();

    Q_EMIT completedMoving();
}

void ETradingTriangle::moveCoord(double x1, double y1, double x2, double y2, double x3, double y3) {
    // X axis is integer
    x1 = std::round(x1);
    x2 = std::round(x2);
    x3 = std::round(x3);

    point1->setCoords(x1, y1);
    point2->setCoords(x2, y2);
    point3->setCoords(x3, y3);

    d->mResizePoint1->moveCoord(x1, y1);
    d->mResizePoint2->moveCoord(x2, y2);
    d->mResizePoint3->moveCoord(x3, y3);

    Q_EMIT moved(QPointF(x1, y1));
    d->mUserLayer->replot();
}

void ETradingTriangle::onMouseMove(QMouseEvent *event) {
    QPointF p1 = d->mParent->pixelsToCoords(event->pos().x(), event->pos().y());
    QPointF p2 = d->mParent->pixelsToCoords(d->mDragStart.x(), d->mDragStart.y());
    d->mCurWantedPosPx = p1 - p2;
}

void ETradingTriangle::moveToWantedPos() {
    if (d->mCurWantedPosPx.isNull()) {
        return;
    }

    double x1 = d->mStartPoint1.x() + d->mCurWantedPosPx.x();
    double y1 = d->mStartPoint1.y() + d->mCurWantedPosPx.y();

    double x2 = d->mStartPoint2.x() + d->mCurWantedPosPx.x();
    double y2 = d->mStartPoint2.y() + d->mCurWantedPosPx.y();

    double x3 = d->mStartPoint3.x() + d->mCurWantedPosPx.x();
    double y3 = d->mStartPoint3.y() + d->mCurWantedPosPx.y();

    moveCoord(x1, y1, x2, y2, x3, y3);
    d->mCurWantedPosPx = QPointF();
}

void ETradingTriangle::point1Moving(const QPointF &pos) {
    this->point1->setCoords(pos);
}

void ETradingTriangle::point2Moving(const QPointF &pos) {
    this->point2->setCoords(pos);
    if (d->mIsDrawing) {
        this->point3->setCoords(pos);
        this->d->mResizePoint3->moveCoord(pos.x(), pos.y());
    }
}

void ETradingTriangle::point3Moving(const QPointF &pos) {
    this->point3->setCoords(pos);
}

void ETradingTriangle::createPoint1Resize() {
    if (d->mResizePoint1 != nullptr) {
        return;
    }

    d->mResizePoint1 = new EResizeHandle(d->mParent);
    d->mResizePoint1->setVisible(false);

    connect(d->mResizePoint1, &EResizeHandle::startingMoving, this, [this]() {
        connect(d->mResizePoint1, SIGNAL(moved(const QPointF &)), this, SLOT(point1Moving(const QPointF &)));
    });
    connect(d->mResizePoint1, &EResizeHandle::completedMoving, this, [this]() {
        resizePoint1StoppedMoving();
        if (d->mIsDrawing) {
            Q_EMIT drawingCompleted(true);
        }
    });

    connect(d->mResizePoint1, &EResizeHandle::cancelledMoving, this, [this]() {
        resizePoint1StoppedMoving();
        if (d->mIsDrawing) {
            Q_EMIT drawingCompleted(false);
        }
    });
}

void ETradingTriangle::createPoint2Resize() {
    if (d->mResizePoint2 != nullptr) {
        return;
    }

    d->mResizePoint2 = new EResizeHandle(d->mParent);
    d->mResizePoint2->setVisible(false);

    connect(d->mResizePoint2, &EResizeHandle::startingMoving, this, [this]() {
        connect(d->mResizePoint2, SIGNAL(moved(const QPointF &)), this, SLOT(point2Moving(const QPointF &)));
    });
    connect(d->mResizePoint2, &EResizeHandle::completedMoving, this, [this]() {
        resizePoint2StoppedMoving();
        if (d->mIsDrawing) {
            QPointF mousePos = d->mParent->coordsToPixels(this->point2->key(), this->point2->value());
            d->mResizePoint3->startMoving(EResizeHandle::Mode::mDrawing, mousePos, false);
        }
    });

    connect(d->mResizePoint2, &EResizeHandle::cancelledMoving, this, [this]() {
        resizePoint2StoppedMoving();
        if (d->mIsDrawing) {
            Q_EMIT drawingCompleted(true);
        }
    });
}

void ETradingTriangle::createPoint3Resize() {
    if (d->mResizePoint3 != nullptr) {
        return;
    }

    d->mResizePoint3 = new EResizeHandle(d->mParent);
    d->mResizePoint3->setVisible(false);

    connect(d->mResizePoint3, &EResizeHandle::startingMoving, this, [this]() {
        connect(d->mResizePoint3, SIGNAL(moved(const QPointF &)), this, SLOT(point3Moving(const QPointF &)));
    });
    connect(d->mResizePoint3, &EResizeHandle::completedMoving, this, [this]() {
        resizePoint3StoppedMoving();
        if (d->mIsDrawing) {
            Q_EMIT drawingCompleted(false);
        }
    });

    connect(d->mResizePoint3, &EResizeHandle::cancelledMoving, this, [this]() {
        resizePoint3StoppedMoving();
        if (d->mIsDrawing) {
            Q_EMIT drawingCompleted(true);
        }
    });
}

void ETradingTriangle::resizePoint1StoppedMoving() {
    setResizeActive(!d->mIsDrawing);
    disconnect(d->mResizePoint1, SIGNAL(moved(const QPointF &)), this, SLOT(point1Moving(const QPointF &)));
}

void ETradingTriangle::resizePoint2StoppedMoving() {
    setResizeActive(!d->mIsDrawing);
    disconnect(d->mResizePoint2, SIGNAL(moved(const QPointF &)), this, SLOT(point2Moving(const QPointF &)));
}

void ETradingTriangle::resizePoint3StoppedMoving() {
    setResizeActive(!d->mIsDrawing);
    disconnect(d->mResizePoint3, SIGNAL(moved(const QPointF &)), this, SLOT(point3Moving(const QPointF &)));
}

bool ETradingTriangle::isPointResize(const QCPItemPosition *pos, const QPointF &mousePos) {
    QPointF point = d->mParent->coordsToPixels(pos->key(), pos->value());
    double distance = ed::interal::distance(point, mousePos);

    return (distance < 10.0);
}

void ETradingTriangle::setResizeActive(bool active) {
    this->d->mResizePoint1->setActive(active);
    this->d->mResizePoint2->setActive(active);
    this->d->mResizePoint3->setActive(active);
}

void ETradingTriangle::setResizeVisible(bool visible) {
    this->d->mResizePoint1->setVisible(visible);
    this->d->mResizePoint2->setVisible(visible);
    this->d->mResizePoint3->setVisible(visible);
}
}  // namespace ed