#ifndef ED_TRADINGCHARTS_ELLIPSE_H
#define ED_TRADINGCHARTS_ELLIPSE_H

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

#include <ed/tradingcharts/TradingPlotable.h>
#include <ed/tradingcharts/ed_globals.h>
#include <qcp/qcp.h>

namespace ed {

class ETradingPlot;

class ED_EXPORT ETradingEllipse : public QCPItemEllipse, public ETradingPlotable {
    Q_OBJECT
    Q_INTERFACES(ed::ETradingPlotable)

public:
    explicit ETradingEllipse(ETradingPlot *parent);

    ~ETradingEllipse() override;

    void init() override;
    void setVisible(bool visible) override;
    void setChoosen(bool on) override;
    void setActive(bool active) override;
    void startMoving(const QPointF &mousePos, bool shiftIsPressed) override;
    bool isResizeable(const QPointF &mousePos) override;
    void startResizing(const QPointF &mousePos, bool shiftIsPressed) override;
    void startDrawing(const QPointF &mousePos) override;

    const QColor &color() const;
    void setColor(const QColor &color);

Q_SIGNALS:
    void moved(const QPointF &pos);
    void completedMoving();

public Q_SLOTS:
    void moveCoord(double x1, double y1, double x2, double y2);

private Q_SLOTS:
    void onMouseMove(QMouseEvent *event);
    void onCompletedMoving();
    void moveToWantedPos();
    void topLeftMoving(const QPointF &pos);
    void bottomRightMoving(const QPointF &pos);

private:
    void createTopLeftResize();
    void createBottomRightResize();
    void resizeTopLeftStoppedMoving();
    void resizeBottomRightStoppedMoving();
    bool isPointResize(const QCPItemPosition *pos, const QPointF &mousePos);
    void setResizeActive(bool active);
    void setResizeVisible(bool visible);
    double selectTest(const QPointF &pos, bool onlySelectable, QVariant *details = nullptr) const override;

private:
    struct Private;
    Private *d;
};
}  // namespace ed

#endif  // ED_TRADINGCHARTS_ELLIPSE_H