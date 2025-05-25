#ifndef ED_TRADINGCHARTS_RECT_H
#define ED_TRADINGCHARTS_RECT_H

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

class ED_EXPORT ETradingRect : public QCPItemRect, public ETradingPlotable {
    Q_OBJECT
    Q_INTERFACES(ed::ETradingPlotable)

public:
    explicit ETradingRect(ETradingPlot *parent);

    ~ETradingRect();

    void setActive(bool active) override;
    void startMoving(const QPointF &mousePos, bool shiftIsPressed) override;
    bool isResizeable(const QPointF &mousePos) override;
    void startResizing(const QPointF& mousePos, bool shiftIsPressed) override;
    void startDrawing(const QPointF& mousePos) override;

    QPointF pos() const;
    const QColor &color() const;
    void setColor(const QColor &color);

Q_SIGNALS:
    void activated();     ///< emitted on mouse over
    void disactivated();  ///< emitted when cursor leave us

    void moved(const QPointF &pos);
    void completedMoving();

public Q_SLOTS:
    void setVisible(bool on);
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
    bool isTopLeftResize(const QPointF& mousePos);
    bool isBottomRightResize(const QPointF& mousePos);

private:
    struct Private;
    Private *d;
};
}  // namespace ed

#endif  // ED_TRADINGCHARTS_RECT_H