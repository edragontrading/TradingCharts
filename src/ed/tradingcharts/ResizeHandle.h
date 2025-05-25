#ifndef ED_TRADINGCHARTS_RESIZE_HANDLE_H
#define ED_TRADINGCHARTS_RESIZE_HANDLE_H

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

class ED_EXPORT EResizeHandle : public QCPItemEllipse {
    Q_OBJECT

public:
    enum Mode {
        mResizing,
        mDrawing,
    };

public:
    explicit EResizeHandle(ETradingPlot *parent, int halfSize = 5);

    ~EResizeHandle();

    void setActive(bool active);
    void startMoving(Mode mode, const QPointF &mousePos, bool shiftIsPressed);

    QPointF pos() const;
    const QColor &color() const;
    void setColor(const QColor &color);

Q_SIGNALS:
    void activated();     ///< emitted on mouse over
    void disactivated();  ///< emitted when cursor leave us

    void startingMoving();
    void moved(const QPointF &pos);
    void completedMoving();
    void cancelledMoving();

public Q_SLOTS:
    void setVisible(bool on);
    void moveCoord(double x, double y);

private Q_SLOTS:
    void onMouseMove(QMouseEvent *event);
    void mouseRelease(QMouseEvent *event);
    void mousePress(QMouseEvent *event);
    void onShiftStateChanged(bool shiftPressed);
    void stopMoving();
    void onCancelled();
    void moveToWantedPos();

private:
    void movePixel(double x, double y);

private:
    struct Private;
    Private *d;
};
}  // namespace ed

#endif  // ED_TRADINGCHARTS_RESIZE_HANDLE_H