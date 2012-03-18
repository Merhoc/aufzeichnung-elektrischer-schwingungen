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

/****************************************************************************/
/*		Aufzeichnung elektrischer Schwingungen - Analyse					*/
/*																			*/
/*		Autor:		Heiko Metzger											*/
/*		Erstellt:	01.02.2012												*/
/****************************************************************************/

#include <iostream>
#include <string>
#include <fstream>

using namespace std;

int main() {
	string path;
	unsigned int peak_h[200][2], peak_l[200][2], time, i, ph, pl, records;
	unsigned int wert[900];
	unsigned short bufferl, bufferh, buffert;
	bool error, suchel;
	ifstream file;
	i = 1;
	time = 0;
	ph = 0;
	pl = 0;
	suchel = true;

	cout << "Pfad zur MESSUNG.BIN: ";
	cin >> path;

	// Datei oeffnen:
	file.open(path + "/MESSUNG.BIN", ios_base::binary);
	if(!file.good()) {
		cout << path << "/MESSUNG.BIN kann nicht geoeffnet werden oder ist leer!" << endl;
		cout << "'Return' zum Beenden";
		cin.get();
		return(0);
	}

	// Anzahl der Datensaetze ermitteln:
	file.seekg(0, ios_base::end);
	records = file.tellg() / 3;
	file.seekg(0, ios_base::beg);
	cout << "Lese Datei ein..." << endl;
	while(records > 0)
	{
		buffert = file.get();
		bufferl = file.get();
		bufferh = file.get();
		wert[i] = bufferl + (bufferh<<8);
		// Ueberpruefe auf Spitzenwerte:
		if( !suchel & (wert[i-1] > 512) & ((wert[i-1] > wert[i-2]) & (wert[i-1] >= wert[i])) ) {
			peak_h[ph][0] = time;
			peak_h[ph][1] = wert[i-1];
			ph ++;
			suchel = true;
		}
		if( suchel & (wert[i-1] < 512) & ((wert[i-1] <= wert[i-2]) & (wert[i-1] < wert[i])) ) {
			peak_l[pl][0] = time;
			peak_l[pl][1] = wert[i-1];
			pl ++;
			suchel = false;
		}
		// Falsche Spitzenwerte korrigieren (messfehler):
		if( suchel & (wert[i-1] > peak_h[ph-1][1]) ) {
			peak_h[ph-1][0] = time;
			peak_h[ph-1][1] = wert[i-1];
		}
		if( !suchel & (wert[i-1] < peak_l[pl-1][1]) ) {
			peak_l[pl-1][0] = time;
			peak_l[pl-1][1] = wert[i-1];
		}
		time += buffert;
		i ++;
		records --;
	}
	file.close();
	printf("%d Datensaetze gelesen, je %d Spitzen gefunden.\n", i, ph);
	
	// Halbe Generatorperiode suchen:
	int gen_beg, gen_end, time_beg, time_end;
	bool beg_l = false, end_l = false;
	float fG, fH, fL, f;
	gen_beg = 0;
	gen_end = 0;
	for(i = 1; i < ph; i ++) {
		if( peak_h[i][1] > peak_h[i-1][1] ) {
			if(!gen_beg)
				gen_beg = i;
			else
				gen_end = i;
		}
		if(gen_end)
			break;
	}
	if(peak_l[gen_beg][1] < peak_l[gen_beg-1][1]) {
		beg_l = true;
	}
	if(peak_l[gen_end][1] < peak_l[gen_end-1][1]) {
		end_l = true;
	}
	time_beg = (beg_l)?(peak_l[gen_beg][0]):(peak_h[gen_beg][0]);
	time_end = (end_l)?(peak_l[gen_end][0]):(peak_h[gen_end][0]);
	fG = 500000 / ((float)time_end - (float)time_beg);
	printf("Halbe Generatorperiode von %d bis %d, Generatorfrequenz = %fHz\n", time_beg, time_end, fG);

	if(gen_end-gen_beg < 3) {
		cout << "Zu wenige Schwingungen fuer zuverlaessige Berechnung!" << endl << "Generatorfrequenz verringern!" << endl;
		cout << "'Return' zum Beenden";
		cin.get();
		return(0);
	}

	fH = ( ((gen_end-gen_beg)-3) * 1000000) / (peak_h[gen_end-1][0] - peak_h[gen_beg + 1][0]);
	fL = ( ((gen_end-gen_beg)-3) * 1000000) / (peak_l[gen_end-1][0] - peak_l[gen_beg + 1][0]);
	f  = (fH + fL) / 2;
	printf("Eigenfrequenz des Schwingkreises = %fHz\n", f);

	cout << "'Return' zum Beenden";
	cin.get();
	cin.get();

	return (0);
}