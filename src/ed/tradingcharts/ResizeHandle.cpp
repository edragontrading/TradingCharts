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

    bool mIsMoving;
    bool mIsChangingOnlyOneCoordinate;
    QCPItemStraightLine *mHelperVertical;
    QCPItemStraightLine *mHelperHorizontal;

    QCPLayer *mUserLayer;
};

EResizeHandle::EResizeHandle(ETradingPlot *parent, int halfSize) : QCPItemEllipse(parent), d(new Private) {
    d->mGripDelta = QPointF();
    d->mInitialPos = QPointF();
    d->mLastWantedPos = QPointF();
    d->mMoveTimer = new QTimer();
    d->mCurWantedPosPx = QPointF();
    d->mIsChangingOnlyOneCoordinate = false;
    d->mIsMoving = false;
    d->mCenterTracer = new QCPItemTracer(parent);
    d->mCenterTracer->setStyle(QCPItemTracer::tsNone);
    d->mCenterTracer->setInterpolating(true);
    d->mUserLayer = parent->userLayer();

    d->mHelperVertical = new QCPItemStraightLine(parentPlot());
    d->mHelperVertical->setAntialiased(false);
    d->mHelperVertical->setLayer(d->mUserLayer);

    d->mHelperHorizontal = new QCPItemStraightLine(parentPlot());
    d->mHelperHorizontal->setAntialiased(false);
    d->mHelperHorizontal->setLayer(d->mUserLayer);

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
    setColor(QColor(35, 125, 100, 255));
    setPen(QPen(Qt::blue, 3));
    setSelectedPen(QPen(Qt::red, 3));
    setLayer(d->mUserLayer);

    d->mMoveTimer->setInterval(25);  // 40 FPS
    connect(d->mMoveTimer, SIGNAL(timeout()), this, SLOT(moveToWantedPos()));
}

EResizeHandle::~EResizeHandle() {
    d->mMoveTimer->stop();
    delete d->mMoveTimer;

    if (parentPlot()->hasItem(d->mHelperVertical)) {
        parentPlot()->removeItem(d->mHelperVertical);
    }
    
    if (parentPlot()->hasItem(d->mHelperHorizontal)) {
        parentPlot()->removeItem(d->mHelperHorizontal);
    }

    delete d;
}

void EResizeHandle::setActive(bool isActive) {
    setSelected(isActive);
    Q_EMIT(isActive ? activated() : disactivated());
}

void EResizeHandle::startMoving(const QPointF &mousePos, bool shiftIsPressed) {
    d->mIsMoving = true;
    d->mGripDelta.setX(parentPlot()->xAxis->coordToPixel(d->mCenterTracer->position->key()) - mousePos.x());
    d->mGripDelta.setY(parentPlot()->yAxis->coordToPixel(d->mCenterTracer->position->value()) - mousePos.y());

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

    connect(parentPlot(), SIGNAL(mouseMove(QMouseEvent *)), this, SLOT(onMouseMove(QMouseEvent *)));
    connect(parentPlot(), SIGNAL(mouseRelease(QMouseEvent *)), this, SLOT(stopMoving()));
    connect(parentPlot(), SIGNAL(shiftStateChanged(bool)), this, SLOT(onShiftStateChanged(bool)));

    parentPlot()->grabKeyboard();
    QApplication::setOverrideCursor(Qt::ClosedHandCursor);

    Q_EMIT startingMoving();
}

bool EResizeHandle::isMoving() {
    return d->mIsMoving;
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
    d->mIsMoving = false;

    disconnect(parentPlot(), SIGNAL(mouseMove(QMouseEvent *)), this, SLOT(onMouseMove(QMouseEvent *)));
    disconnect(parentPlot(), SIGNAL(mouseRelease(QMouseEvent *)), this, SLOT(stopMoving()));
    disconnect(parentPlot(), SIGNAL(shiftStateChanged(bool)), this, SLOT(onShiftStateChanged(bool)));

    d->mMoveTimer->stop();
    moveToWantedPos();

    d->mHelperVertical->setVisible(false);
    d->mHelperHorizontal->setVisible(false);

    d->mUserLayer->replot();

    parentPlot()->releaseKeyboard();
    QApplication::restoreOverrideCursor();

    Q_EMIT stoppedMoving();
}

void EResizeHandle::moveCoord(double x, double y) {
    d->mLastWantedPos.setX(x);
    d->mLastWantedPos.setY(y);
    if (d->mIsChangingOnlyOneCoordinate) {
        double x1 = parentPlot()->xAxis->coordToPixel(x);
        double x2 = parentPlot()->xAxis->coordToPixel(d->mInitialPos.x());
        double y1 = parentPlot()->yAxis->coordToPixel(y);
        double y2 = parentPlot()->yAxis->coordToPixel(d->mInitialPos.y());
        if (qAbs(x1 - x2) < qAbs(y1 - y2)) {
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
    moveCoord(parentPlot()->xAxis->pixelToCoord(x), parentPlot()->yAxis->pixelToCoord(y));
}

void EResizeHandle::onMouseMove(QMouseEvent *event) {
    d->mCurWantedPosPx = QPointF(event->position().x() + d->mGripDelta.x(), event->position().y() + d->mGripDelta.y());
}

void EResizeHandle::moveToWantedPos() {
    if (!d->mCurWantedPosPx.isNull()) {
        movePixel(d->mCurWantedPosPx.x(), d->mCurWantedPosPx.y());
        d->mCurWantedPosPx = QPointF();
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
}  // namespace ed