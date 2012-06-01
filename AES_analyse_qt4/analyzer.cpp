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
 * analyzer.cpp
 *
 * Created: 27.05.2012
 *  Author: Heiko Metzger
 *
 */

#include "analyzer.h"
#include "ui_analyzer.h"

#include<fstream>

#include<QtGui>

analyzer::analyzer(QMainWindow *parent) :
    QMainWindow(parent),
    ui(new Ui::analyzer)
{
    ui->setupUi(this);

    zeichnen = false;

    connect(ui->inputFileButton, SIGNAL(clicked()), this, SLOT(openFile()));
    connect(ui->startButton, SIGNAL(clicked()), this, SLOT(analyze()));
}

analyzer::~analyzer()
{
}

void analyzer::openFile() {
    // Datei waehlen... -Dialog
    ui->inputFileText->setText(QFileDialog::getOpenFileName(this, tr("Datei waehlen..."), "", tr("BIN Dateien (*.bin)")));
}

void analyzer::analyze() {
    // Datei einlesen
    std::ifstream file;
    unsigned int records;
    unsigned short bufferl, bufferh, buffertl, bufferth;
    file.open(ui->inputFileText->text().toAscii().constData(), std::ios_base::binary);
    if(!file.good()) {
        ui->statusBar->showMessage(tr("\"") + ui->inputFileText->text() + tr("\" kann nicht geoeffnet werden oder ist leer!"));
        return;
    }

    // Anzahl der Datensaetze ermitteln:
    file.seekg(0, std::ios_base::end);
    records = (unsigned int)file.tellg() / 4;
    file.seekg(0, std::ios_base::beg);
    ui->progressBar->setValue(10);
    time = 0;
    daten = records;
    bottom = 1024;
    top = 0;

    // Datensaetze einlesen:
    for(unsigned int i = 0; records > 0; records--)
    {
        buffertl = file.get();
        bufferth = file.get();
        bufferl = file.get();
        bufferh = file.get();
        werte[i] = bufferl + (bufferh<<8);
        time += buffertl + (bufferth<<8);
        zeiten[i] = buffertl + (bufferth<<8);
        i ++;
        if(werte[i] < bottom)
            bottom = werte[i];
        if(werte[i] > top)
            top = werte[i];
    }
    file.close();
    ui->statusBar->showMessage(tr("Auslesen beendet."));
    sprintf(str, "%d Daten mit %d Mikrosekunden gelesen.", daten, time);
    ui->statusBar->showMessage(str);
    ui->progressBar->setValue(60);
    // Zeichnen:
    zeichnen = true;
    this->repaint();                    // Loest Paint-Event aus
    ui->progressBar->setValue(100);
}

void analyzer::paintEvent(QPaintEvent *)
{
    if(zeichnen) {
        QPainter painter(this);
        painter.setPen(QPen(QColor(0, 0xFF, 0), 1, Qt::SolidLine));
        QRect rect;
        qreal width = ui->plot->geometry().width();
        QPointF shifth(width, 0);
        qreal height = ui->plot->geometry().height();
        QPointF shiftv(0, height);

        QPointF point[2];

        point[0] = ui->centralWidget->geometry().topLeft();
        point[0]+= ui->verticalLayout->geometry().topLeft();
        point[0]+= ui->plot->geometry().topLeft();
        point[0].setX(point[0].x() + 9);
        point[0].setY(point[0].y() + 9);
        rect = ui->plot->rect();
        rect.moveTo(point[0].x(), point[0].y());

        // Hintergrund Faerben
        painter.fillRect(rect, QColor(0, 0x33, 0));

        point[0]+= shiftv/2;
        werte[0] = 512;

        // Graph zeichnen
        for(int i=1; i < daten; i++) {
            point[1] = point[0] + ((zeiten[i]*shifth)/time) + (((werte[i] - werte[i-1]) * shiftv)/top);
            painter.drawLine(point[0], point[1]);
            point[0] = point[1];
        }
        sprintf(str, "Top: %d, Bottom: %d", top, bottom);
        ui->statusBar->showMessage(str);
    }
}
