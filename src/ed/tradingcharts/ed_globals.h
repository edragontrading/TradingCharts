#ifndef ED_TRADINGCHARTS_GLOBALS_H
#define ED_TRADINGCHARTS_GLOBALS_H

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
#include <QDebug>
#include <QObject>

#if defined(Q_OS_UNIX) && !defined(Q_OS_MACOS)
#include <xcb/xcb.h>
#endif

#ifndef ED_TRADINGCHARTS_STATIC
#ifdef ED_TRADINGCHARTS_SHARED_EXPORT
#define ED_EXPORT Q_DECL_EXPORT
#else
#define ED_EXPORT Q_DECL_IMPORT
#endif
#else
#define ED_EXPORT
#endif

#define ED_DEBUG_PRINT
#ifdef ED_DEBUG_PRINT
#define ED_PRINT(s) qDebug() << s
#else
#define ED_PRINT(s)
#endif

#include <math.h>

#include <QPoint>
#include <QPointF>

namespace ed {
Q_NAMESPACE

namespace interal {

template <typename TPoint>
inline double distance(const TPoint& p1, const TPoint& p2) {
    double dx = p2.x() - p1.x();
    double dy = p2.y() - p1.y();
    return std::sqrt(dx * dx + dy * dy);
}

}  // namespace interal

}  // namespace ed

#endif
