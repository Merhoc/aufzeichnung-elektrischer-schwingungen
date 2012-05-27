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
 * analyzer.h
 *
 * Created: 27.05.2012
 *  Author: Heiko Metzger
 *
 */

#ifndef ANALYZER_H
#define ANALYZER_H

#include <QMainWindow>
#include "ui_analyzer.h"
#include "plot.h"

namespace Ui {
class analyzer;
class plot;
}

class analyzer : public QMainWindow, Ui::analyzer
{
    Q_OBJECT
    
public:
    explicit analyzer(QMainWindow *parent = 0);
    ~analyzer();
    
private:
    analyzer *ui;
    Ui::plot *plot;

private slots:
    void openFile();
    void analyze();
};

#endif // ANALYZER_H
