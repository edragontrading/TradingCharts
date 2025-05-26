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

#include <QPointF>

namespace ed {

struct EResizeHandle::Private {
    Private() = default;

    QCPItemTracer *mCenterTracer;
    QPointF mGripDelta;
    QPointF mInitialPos;
    QPointF mLastWantedPos;
    QTimer *mMoveTimer;
    QPointF mCurWantedPosPx;

    bool mIsChangingOnlyOneCoordinate;
    QCPItemStraightLine *mHelperVertical;
    QCPItemStraightLine *mHelperHorizontal;

    Mode mMode;
    QCPLayer *mUserLayer;
    ETradingPlot *mParent;
};

EResizeHandle::EResizeHandle(ETradingPlot *parent, int halfSize) : QCPItemEllipse(parent), d(new Private) {
    d->mParent = parent;
    d->mGripDelta = QPointF();
    d->mInitialPos = QPointF();
    d->mLastWantedPos = QPointF();
    d->mMoveTimer = new QTimer();
    d->mCurWantedPosPx = QPointF();
    d->mIsChangingOnlyOneCoordinate = false;
    d->mCenterTracer = new QCPItemTracer(parent);
    d->mCenterTracer->setStyle(QCPItemTracer::tsNone);
    d->mCenterTracer->setInterpolating(true);
    d->mCenterTracer->setSelectable(false);
    d->mUserLayer = parent->userLayer();
    d->mMode = Mode::mResizing;

    d->mHelperVertical = new QCPItemStraightLine(d->mParent);
    d->mHelperVertical->setAntialiased(false);
    d->mHelperVertical->setLayer(d->mUserLayer);
    d->mHelperVertical->setSelectable(false);

    d->mHelperHorizontal = new QCPItemStraightLine(d->mParent);
    d->mHelperHorizontal->setAntialiased(false);
    d->mHelperHorizontal->setLayer(d->mUserLayer);
    d->mHelperHorizontal->setSelectable(false);

    static const QPen linesPen(Qt::darkGray, 0, Qt::DashLine);
    d->mHelperHorizontal->setPen(linesPen);
    d->mHelperVertical->setPen(linesPen);

    d->mHelperVertical->setVisible(false);
    d->mHelperHorizontal->setVisible(false);

    topLeft->setParentAnchor(d->mCenterTracer->position);
    bottomRight->setParentAnchor(d->mCenterTracer->position);
    topLeft->setType(QCPItemPosition::ptAbsolute);
    bottomRight->setType(QCPItemPosition::ptAbsolute);

    topLeft->setCoords(-halfSize, -halfSize);
    bottomRight->setCoords(halfSize, halfSize);

    setSelectable(false);
    setColor(QColor(0xd1, 0xd4, 0xdc, 255));
    setPen(QPen(Qt::blue, 1));
    setSelectedPen(QPen(Qt::blue, 2));
    setLayer(d->mUserLayer);

    d->mMoveTimer->setInterval(25);  // 40 FPS
    connect(d->mMoveTimer, SIGNAL(timeout()), this, SLOT(moveToWantedPos()));
}

EResizeHandle::~EResizeHandle() {
    d->mMoveTimer->stop();
    delete d->mMoveTimer;

    if (d->mParent->hasItem(d->mHelperVertical)) {
        d->mParent->removeItem(d->mHelperVertical);
    }

    if (d->mParent->hasItem(d->mHelperHorizontal)) {
        d->mParent->removeItem(d->mHelperHorizontal);
    }

    delete d;
}

void EResizeHandle::setActive(bool isActive) {
    setSelected(isActive);
}

void EResizeHandle::startMoving(Mode mode, const QPointF &mousePos, bool shiftIsPressed) {
    d->mMode = mode;
    d->mGripDelta =
        d->mParent->coordsToPixels(d->mCenterTracer->position->key(), d->mCenterTracer->position->value()) - mousePos;

    d->mInitialPos = pos();
    d->mLastWantedPos = d->mInitialPos;
    d->mCurWantedPosPx = QPointF();
    d->mIsChangingOnlyOneCoordinate = shiftIsPressed;

    d->mMoveTimer->start();

    d->mHelperVertical->point1->setCoords(d->mInitialPos.x(), d->mInitialPos.y());
    d->mHelperVertical->point2->setCoords(d->mInitialPos.x(), d->mInitialPos.y() + 1);

    d->mHelperHorizontal->point1->setCoords(d->mInitialPos.x(), d->mInitialPos.y());
    d->mHelperHorizontal->point2->setCoords(d->mInitialPos.x() + 1, d->mInitialPos.y());

    d->mHelperVertical->setVisible(shiftIsPressed);
    d->mHelperHorizontal->setVisible(shiftIsPressed);

    connect(d->mParent, SIGNAL(mouseMove(QMouseEvent *)), this, SLOT(onMouseMove(QMouseEvent *)));
    connect(d->mParent, SIGNAL(mouseRelease(QMouseEvent *)), this, SLOT(mouseRelease(QMouseEvent *)));
    connect(d->mParent, SIGNAL(mousePress(QMouseEvent *)), this, SLOT(mousePress(QMouseEvent *)));
    connect(d->mParent, SIGNAL(escapeKeyCancelled()), this, SLOT(onCancelled()));

    if (d->mMode == mResizing) {
        connect(d->mParent, SIGNAL(shiftStateChanged(bool)), this, SLOT(onShiftStateChanged(bool)));
    }

    d->mParent->grabKeyboard();
    QApplication::setOverrideCursor(Qt::ClosedHandCursor);

    d->mUserLayer->replot();
    Q_EMIT startingMoving();
}

QPointF EResizeHandle::pos() const {
    return d->mCenterTracer->position->coords();
}

const QColor &EResizeHandle::color() const {
    return brush().color();
}

void EResizeHandle::setColor(const QColor &color) {
    setBrush(color);
    setSelectedBrush(color);
}

void EResizeHandle::setVisible(bool on) {
    QCPItemEllipse::setVisible(on);
}

void EResizeHandle::stopMoving() {
    disconnect(d->mParent, SIGNAL(mouseMove(QMouseEvent *)), this, SLOT(onMouseMove(QMouseEvent *)));
    disconnect(d->mParent, SIGNAL(mouseRelease(QMouseEvent *)), this, SLOT(mouseRelease(QMouseEvent *)));
    disconnect(d->mParent, SIGNAL(mousePress(QMouseEvent *)), this, SLOT(mousePress(QMouseEvent *)));
    disconnect(d->mParent, SIGNAL(escapeKeyCancelled()), this, SLOT(onCancelled()));

    if (d->mMode == mResizing) {
        disconnect(d->mParent, SIGNAL(shiftStateChanged(bool)), this, SLOT(onShiftStateChanged(bool)));
    }

    d->mMoveTimer->stop();
    moveToWantedPos();

    d->mHelperVertical->setVisible(false);
    d->mHelperHorizontal->setVisible(false);
    d->mIsChangingOnlyOneCoordinate = false;

    d->mParent->releaseKeyboard();
    QApplication::restoreOverrideCursor();
}

void EResizeHandle::moveCoord(double x, double y) {
    d->mLastWantedPos.setX(x);
    d->mLastWantedPos.setY(y);
    if (d->mIsChangingOnlyOneCoordinate) {
        QPointF p1 = d->mParent->coordsToPixels(x, y);
        QPointF p2 = d->mParent->coordsToPixels(d->mInitialPos.x(), d->mInitialPos.y());
        if (qAbs(p1.x() - p2.x()) < qAbs(p1.y() - p2.y())) {
            x = d->mInitialPos.x();
        } else {
            y = d->mInitialPos.y();
        }
    }

    // X axis is integer
    x = std::round(x);
    d->mCenterTracer->position->setCoords(x, y);

    Q_EMIT moved(QPointF(x, y));
    d->mUserLayer->replot();
}

void EResizeHandle::movePixel(double x, double y) {
    QPointF pos = d->mParent->pixelsToCoords(x, y);
    moveCoord(pos.x(), pos.y());
}

void EResizeHandle::onMouseMove(QMouseEvent *event) {
    d->mCurWantedPosPx = QPointF(event->position().x() + d->mGripDelta.x(), event->position().y() + d->mGripDelta.y());
}

void EResizeHandle::mouseRelease(QMouseEvent *event) {
    if (d->mMode == mResizing) {
        stopMoving();
        Q_EMIT completedMoving();
    }
}

void EResizeHandle::mousePress(QMouseEvent *event) {
    if (d->mMode == mDrawing) {
        stopMoving();
        if (event->button() == Qt::LeftButton) {
            Q_EMIT completedMoving();
        } else if (event->button() == Qt::RightButton) {
            Q_EMIT cancelledMoving();
        }
    }
}

void EResizeHandle::onShiftStateChanged(bool shiftPressed) {
    if (shiftPressed != d->mIsChangingOnlyOneCoordinate) {
        d->mIsChangingOnlyOneCoordinate = shiftPressed;
        d->mHelperVertical->setVisible(shiftPressed);
        d->mHelperHorizontal->setVisible(shiftPressed);
        moveCoord(d->mLastWantedPos.x(), d->mLastWantedPos.y());
    }
}

void EResizeHandle::onCancelled() {
    stopMoving();
    Q_EMIT cancelledMoving();
}

void EResizeHandle::moveToWantedPos() {
    if (!d->mCurWantedPosPx.isNull()) {
        movePixel(d->mCurWantedPosPx.x(), d->mCurWantedPosPx.y());
        d->mCurWantedPosPx = QPointF();
    }
}
}  // namespace ed