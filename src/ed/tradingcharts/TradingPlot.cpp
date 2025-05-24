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

#include <ed/tradingcharts/TradingPlot.h>
#include <ed/tradingcharts/TradingPlotable.h>

namespace ed {

struct ETradingPlot::Private {
    Private() = default;

    QCPLayer *mUserLayer;
    ETradingPlotable *mPointUnderCursor;
    ETradingPlotable *mPointSelected;
};

ETradingPlot::ETradingPlot(QWidget *parent) : QCustomPlot(parent), d(new Private) {
    d->mPointUnderCursor = nullptr;
    d->mPointSelected = nullptr;

    setInteractions(QCP::iRangeDrag | QCP::iRangeZoom | QCP::iSelectAxes | QCP::iSelectPlottables);
    setAutoAddPlottableToLegend(false);
    plotLayout()->setRowSpacing(0);
    plotLayout()->setColumnSpacing(0);
    plotLayout()->setMargins(QMargins(0, 0, 0, 0));

    this->addLayer("user", this->layer("main"), QCustomPlot::limAbove);
    d->mUserLayer = this->layer("user");
    d->mUserLayer->setMode(QCPLayer::lmBuffered);

    this->axisRect()->setMinimumMargins(QMargins(1, 1, 1, 1));
    this->axisRect()->setMargins(QMargins(1, 1, 12, 12));
    this->axisRect()->setupFullAxesBox(true);
    this->axisRect()->setRangeZoomFactor(0.96, 0.98);

    this->legend->setVisible(false);

    this->yAxis2->setVisible(true);
    this->yAxis2->setTickLabels(true);
    this->yAxis2->setScaleType(QCPAxis::ScaleType::stLinear);
    this->yAxis2->ticker()->setTickCount(5);
    this->yAxis2->grid()->setVisible(true);
    this->yAxis2->setSubTicks(true);
    this->yAxis2->ticker()->setTickStepStrategy(QCPAxisTicker::TickStepStrategy::tssReadability);

    this->yAxis->setVisible(false);
    this->yAxis->setPadding(0);
    this->yAxis->ticker()->setTickCount(5);
    this->yAxis->setScaleType(QCPAxis::ScaleType::stLinear);
    this->yAxis->ticker()->setTickStepStrategy(QCPAxisTicker::TickStepStrategy::tssReadability);

    this->xAxis2->setVisible(false);
    this->xAxis2->setPadding(0);
    this->xAxis2->ticker()->setTickStepStrategy(QCPAxisTicker::TickStepStrategy::tssIntegerTick);

    this->xAxis->setVisible(true);
    this->xAxis->setPadding(0);
    this->xAxis->setSubTicks(true);
    this->xAxis->ticker()->setTickStepStrategy(QCPAxisTicker::TickStepStrategy::tssIntegerTick);
    this->xAxis->grid()->setZeroLinePen(Qt::NoPen);

    // this->yAxis->setScaleType(QCPAxis::ScaleType::stLogarithmic);
    // this->yAxis2->setScaleType(QCPAxis::ScaleType::stLogarithmic);
    this->yAxis->setRange(10, 100);
    this->xAxis->setRange(10, 100);

    this->rescaleAxes(true);

    connect(this, SIGNAL(mousePress(QMouseEvent *)), this, SLOT(handleMousePress(QMouseEvent *)));
    connect(this, SIGNAL(mouseWheel(QWheelEvent *)), this, SLOT(handleMouseWheel(QWheelEvent *)));
}

ETradingPlot::~ETradingPlot() {
}

QCPLayer *ETradingPlot::userLayer() const {
    return d->mUserLayer;
}

void ETradingPlot::mousePressEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
        ETradingPlotable *plotPoint = qobject_cast<ETradingPlotable *>(itemAt(event->position(), true));
        if (nullptr != plotPoint) {
            if (d->mPointUnderCursor != nullptr && d->mPointUnderCursor != plotPoint) {
                d->mPointUnderCursor->setActive(false);
            }

            plotPoint->setActive(true);
            d->mPointSelected = plotPoint;
            d->mPointUnderCursor = nullptr;
            d->mPointSelected->setActive(true);
            d->mUserLayer->replot();
            return;
        } else if (d->mPointSelected != nullptr) {
            d->mPointSelected->setActive(false);
            d->mPointSelected = nullptr;
            d->mUserLayer->replot();
        }
    }

    QCustomPlot::mousePressEvent(event);
}

void ETradingPlot::mouseMoveEvent(QMouseEvent *event) {
    QCustomPlot::mouseMoveEvent(event);

    if (event->buttons() == Qt::NoButton) {
        ETradingPlotable *plotPoint = qobject_cast<ETradingPlotable *>(itemAt(event->position(), true));

        if (plotPoint != d->mPointUnderCursor) {
            if (d->mPointUnderCursor == nullptr) {
                // cursor moved from empty space to item
                plotPoint->setActive(true);
                setCursor(Qt::OpenHandCursor);
            } else if (plotPoint == nullptr) {
                // cursor move from item to empty space
                if (d->mPointUnderCursor != d->mPointSelected) {
                    d->mPointUnderCursor->setActive(false);
                }
                unsetCursor();
            } else {
                // cursor moved from item to item
                d->mPointUnderCursor->setActive(false);
                plotPoint->setActive(true);
            }
            d->mPointUnderCursor = plotPoint;
            d->mUserLayer->replot();
        }
    } else if (event->buttons() == Qt::LeftButton) {
        if (d->mPointSelected != nullptr && !d->mPointSelected->isMoving()) {
            d->mPointSelected->startMoving(event->position(), event->modifiers().testFlag(Qt::ShiftModifier));
        }
    }
}

void ETradingPlot::keyPressEvent(QKeyEvent *event) {
    if (event->key() == Qt::Key_Shift) {
        Q_EMIT shiftStateChanged(true);
    }
    QCustomPlot::keyPressEvent(event);
}

void ETradingPlot::keyReleaseEvent(QKeyEvent *event) {
    if (event->key() == Qt::Key_Shift) {
        Q_EMIT shiftStateChanged(false);
    }
    QCustomPlot::keyReleaseEvent(event);
}

void ETradingPlot::handleMousePress(QMouseEvent *event) {
    // if an axis is selected, only allow the direction of that axis to be dragged
    // if no axis is selected, both directions may be dragged

    if (this->xAxis->selectedParts().testFlag(QCPAxis::spAxis)) {
        this->axisRect()->setRangeDrag(this->xAxis->orientation());
    } else if (this->yAxis2->selectedParts().testFlag(QCPAxis::spAxis)) {
        this->axisRect()->setRangeDrag(this->yAxis2->orientation());
    } else {
        this->axisRect()->setRangeDrag(Qt::Horizontal | Qt::Vertical);
    }
}

void ETradingPlot::handleMouseWheel(QWheelEvent *event) {
    // if an axis is selected, only allow the direction of that axis to be zoomed
    // if no axis is selected, both directions may be zoomed

    if (this->xAxis->selectedParts().testFlag(QCPAxis::spAxis)) {
        this->axisRect()->setRangeZoom(this->xAxis->orientation());
    } else if (this->yAxis2->selectedParts().testFlag(QCPAxis::spAxis)) {
        this->axisRect()->setRangeZoom(this->yAxis2->orientation());
    } else {
        this->axisRect()->setRangeZoom(Qt::Horizontal | Qt::Vertical);
    }
}

}  // namespace ed