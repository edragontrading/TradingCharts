#ifndef ED_TRADINGCHARTS_PLOTABLE_H
#define ED_TRADINGCHARTS_PLOTABLE_H

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

#include <ed/tradingcharts/ed_globals.h>

namespace ed {

class ED_EXPORT ETradingPlotable {
public:
    virtual ~ETradingPlotable() = default;

    virtual void setActive(bool active) = 0;
    virtual void startMoving(const QPointF& mousePos, bool shiftIsPressed) = 0;
    virtual bool isResizeable(const QPointF& mousePos) = 0;
    virtual void startResizing(const QPointF& mousePos, bool shiftIsPressed) = 0;
    virtual void startDrawing(const QPointF& mousePos) = 0;
};
}  // namespace ed

#define ETradingPlotable_iid "ed.tradingcharts.ETradingPlotable"
Q_DECLARE_INTERFACE(ed::ETradingPlotable, ETradingPlotable_iid);

#endif  // ED_TRADINGCHARTS_PLOTABLE_H