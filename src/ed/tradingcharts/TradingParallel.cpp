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
#include <ed/tradingcharts/TradingParallel.h>
#include <ed/tradingcharts/TradingPlot.h>

#include <QPointF>

namespace ed {

struct ETradingParallel::Private {
    Private() = default;

    bool mIsDrawing;
    bool mPoint2Completed;

    QPointF point4;
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
    EResizeHandle *mResizeSelect;
    ETradingPlot *mParent;
};

ETradingParallel::ETradingParallel(ETradingPlot *parent) : QCPItemParallel(parent), d(new Private) {
    d->mParent = parent;
    d->mIsDrawing = false;
    d->mPoint2Completed = false;
    d->mMoveTimer = new QTimer();
    d->point4 = QPointF();
    d->mDragStart = QPointF();
    d->mStartPoint1 = QPointF();
    d->mStartPoint2 = QPointF();
    d->mStartPoint3 = QPointF();
    d->mUserLayer = parent->userLayer();
    d->mCurWantedPosPx = QPointF();
    d->mResizePoint1 = nullptr;
    d->mResizePoint2 = nullptr;
    d->mResizePoint3 = nullptr;
    d->mResizeSelect = nullptr;

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

ETradingParallel::~ETradingParallel() {
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

void ETradingParallel::init() {
    if (d->mResizePoint1 == nullptr) {
        d->mResizePoint1 = createPointResize();
    }

    if (d->mResizePoint2 == nullptr) {
        d->mResizePoint2 = createPointResize();
        connect(d->mResizePoint2, &EResizeHandle::startingMoving, this,
                [this]() { d->point4 = this->point1->coords() + (this->point3->coords() - this->point2->coords()); });
    }

    if (d->mResizePoint3 == nullptr) {
        d->mResizePoint3 = createPointResize();
    }
}

void ETradingParallel::setChoosen(bool on) {
    setSelected(on);
    setResizeActive(on);

    if (!on) {
        setResizeVisible(on);
    }
}

void ETradingParallel::setVisible(bool on) {
    QCPItemParallel::setVisible(on);
}

void ETradingParallel::setActive(bool isActive) {
    setSelected(isActive);
    setResizeVisible(isActive);
}

void ETradingParallel::startMoving(const QPointF &mousePos, bool shiftIsPressed) {
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

bool ETradingParallel::isResizeable(const QPointF &mousePos) {
    if (ed::internal::near(this->point1->pixelPosition(), mousePos)) {
        return true;
    }

    if (ed::internal::near(this->point2->pixelPosition(), mousePos)) {
        return true;
    }

    if (ed::internal::near(this->point3->pixelPosition(), mousePos)) {
        return true;
    }

    return false;
}

void ETradingParallel::startResizing(const QPointF &mousePos, bool shiftIsPressed) {
    d->mIsDrawing = false;
    if (ed::internal::near(this->point1->pixelPosition(), mousePos)) {
        d->mResizeSelect = d->mResizePoint1;
    } else if (ed::internal::near(this->point2->pixelPosition(), mousePos)) {
        d->mResizeSelect = d->mResizePoint2;
    } else {
        d->mResizeSelect = d->mResizePoint3;
    }

    setResizeActive(false);
    d->mResizeSelect->setActive(true);
    d->mResizeSelect->startMoving(EResizeHandle::Mode::mResizing, mousePos, shiftIsPressed);
}

void ETradingParallel::startDrawing(const QPointF &mousePos) {
    d->mIsDrawing = true;
    d->mPoint2Completed = false;

    QPointF pos = d->mParent->pixelsToCoords(mousePos.x(), mousePos.y());
    moveCoord(pos.x(), pos.y(), pos.x(), pos.y(), pos.x(), pos.y());

    d->mResizeSelect = d->mResizePoint3;
    d->mResizeSelect->startMoving(EResizeHandle::Mode::mDrawing, mousePos, false);
}

const QColor &ETradingParallel::color() const {
    return brush().color();
}

void ETradingParallel::setColor(const QColor &color) {
    setBrush(color);
    setSelectedBrush(color);
}

void ETradingParallel::onCompletedMoving() {
    disconnect(d->mParent, SIGNAL(mouseMove(QMouseEvent *)), this, SLOT(onMouseMove(QMouseEvent *)));
    disconnect(d->mParent, SIGNAL(mouseRelease(QMouseEvent *)), this, SLOT(onCompletedMoving()));

    d->mMoveTimer->stop();
    moveToWantedPos();

    d->mUserLayer->replot();

    d->mParent->releaseKeyboard();
    QApplication::restoreOverrideCursor();

    Q_EMIT completedMoving();
}

void ETradingParallel::moveCoord(double x1, double y1, double x2, double y2, double x3, double y3) {
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

void ETradingParallel::onMouseMove(QMouseEvent *event) {
    QPointF p1 = d->mParent->pixelsToCoords(event->pos().x(), event->pos().y());
    QPointF p2 = d->mParent->pixelsToCoords(d->mDragStart.x(), d->mDragStart.y());
    d->mCurWantedPosPx = p1 - p2;
}

void ETradingParallel::moveToWantedPos() {
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

void ETradingParallel::pointMoving(const QPointF &pos) {
    if (d->mResizeSelect == d->mResizePoint1) {
        this->point1->setCoords(pos);
    } else if (d->mResizeSelect == d->mResizePoint2) {
        QPointF p3 = d->point4 + (pos - this->point1->coords());
        this->point2->setCoords(pos);
        this->point3->setCoords(p3);
        d->mResizePoint3->moveCoord(p3.x(), p3.y());
    } else if (d->mResizeSelect == d->mResizePoint3) {
        this->point3->setCoords(pos);
        if (d->mIsDrawing && d->mPoint2Completed == false) {
            this->point2->setCoords(pos);
            d->mResizePoint2->moveCoord(pos.x(), pos.y());
        }
    }
}

EResizeHandle *ETradingParallel::createPointResize() {
    EResizeHandle *resizePoint = new EResizeHandle(d->mParent);
    resizePoint->setVisible(false);

    connect(resizePoint, &EResizeHandle::startingMoving, this, [this, resizePoint]() {
        connect(resizePoint, SIGNAL(moved(const QPointF &)), this, SLOT(pointMoving(const QPointF &)));
    });
    connect(resizePoint, &EResizeHandle::completedMoving, this, [this]() { resizePointStoppedMoving(false); });
    connect(resizePoint, &EResizeHandle::cancelledMoving, this, [this]() { resizePointStoppedMoving(true); });

    return resizePoint;
}

void ETradingParallel::resizePointStoppedMoving(bool cancelled) {
    setResizeActive(!d->mIsDrawing);
    disconnect(d->mResizeSelect, SIGNAL(moved(const QPointF &)), this, SLOT(pointMoving(const QPointF &)));

    if (cancelled && d->mIsDrawing) {
        Q_EMIT drawingCompleted(true);
        return;
    }

    if (d->mIsDrawing && d->mResizeSelect == d->mResizePoint3) {
        if (d->mPoint2Completed) {
            Q_EMIT drawingCompleted(false);
        } else {
            d->mPoint2Completed = true;
            QPointF mousePos = d->mParent->coordsToPixels(this->point2->key(), this->point2->value());
            d->mResizePoint3->startMoving(EResizeHandle::Mode::mDrawing, mousePos, false);
        }
    }
}

void ETradingParallel::setResizeActive(bool active) {
    this->d->mResizePoint1->setActive(active);
    this->d->mResizePoint2->setActive(active);
    this->d->mResizePoint3->setActive(active);
}

void ETradingParallel::setResizeVisible(bool visible) {
    this->d->mResizePoint1->setVisible(visible);
    this->d->mResizePoint2->setVisible(visible);
    this->d->mResizePoint3->setVisible(visible);
}
}  // namespace ed