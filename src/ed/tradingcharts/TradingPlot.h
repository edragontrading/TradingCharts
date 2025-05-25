#ifndef ED_TRADINGCHARTS_PLOT_H
#define ED_TRADINGCHARTS_PLOT_H

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
#include <qcp/qcp.h>

namespace ed {

class ED_EXPORT ETradingPlot : public QCustomPlot {
    Q_OBJECT

public:
    enum Mode {
        pmNone,
        pmDrawingRect,
    };

public:
    explicit ETradingPlot(QWidget *parent = nullptr);
    ~ETradingPlot() override;

    QCPLayer *userLayer() const;
    void setMode(Mode mode);

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void keyReleaseEvent(QKeyEvent *event) override;

private Q_SLOTS:
    void handleMousePress(QMouseEvent *event);
    void handleMouseWheel(QWheelEvent *event);
    void onDrawingCompleted(bool cancelled);

Q_SIGNALS:
    void shiftStateChanged(bool);
    void escapeKeyCancelled();

private:
    struct Private;
    Private *d;
};
}  // namespace ed

#endif  // ED_TRADINGCHARTS_PLOT_H