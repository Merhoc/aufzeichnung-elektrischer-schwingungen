/*
 * Copyright (C) 2012 Heiko Metzger <www.heikometzger.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

/*
 * Aufzeichnung elektrischer Schwingungen - Analyse Qt4
 *
 * plot.cpp
 *
 * Created: 27.05.2012
 *  Author: Heiko Metzger
 *
 */

#include<QtGui>
#include<QPainter>

#include "plot.h"

plot::plot(QWidget *parent)
    : QWidget(parent)
{
    setBackgroundRole(QPalette::Base);
    setAutoFillBackground(true);
}

void plot::paintEvent(QPaintEvent * /* event */)
{
    QPainter painter(this);
    painter.setPen(QPen(Qt::black, 1, Qt::SolidLine));

    painter.drawLine(10, 5, 20, 100);

    painter.restore();
}
