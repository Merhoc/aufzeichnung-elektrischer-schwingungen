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
    QMainWindow(parent)
{
    setupUi(this);

    plot = new plot;

    connect(inputFileButton, SIGNAL(clicked()), this, SLOT(openFile()));
    connect(startButton, SIGNAL(clicked()), this, SLOT(analyze()));
}

analyzer::~analyzer()
{
}

void analyzer::openFile() {
    inputFileText->insert(QFileDialog::getOpenFileName(this, tr("Datei waehlen..."), "", tr("BIN Dateien (*.bin)")));
}

void analyzer::analyze() {
    std::ifstream file;
    unsigned int wert[1300], time = 0, i = 2, records;
    unsigned short bufferl, bufferh, buffertl, bufferth;
    file.open(ui->inputFileText->text().toAscii().constData(), std::ios_base::binary);
    if(!file.good()) {
        statusBar->showMessage(tr("\"") + inputFileText->text() + tr("\" kann nicht geoeffnet werden oder ist leer!"));
        return;
    }

    // Anzahl der Datensaetze ermitteln:
    file.seekg(0, std::ios_base::end);
    records = (unsigned int)file.tellg() / 4;
    file.seekg(0, std::ios_base::beg);
    progressBar->setValue(10);

    while(records > 0)
    {
        buffertl = file.get();
        bufferth = file.get();
        bufferl = file.get();
        bufferh = file.get();
        wert[i] = bufferl + (bufferh<<8);
        time += buffertl + (bufferth<<8);
        i ++;
        records --;
    }
    file.close();
    statusBar->showMessage(tr("Auslesen beendet."));
    progressBar->setValue(100);
}
