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
 * Aufzeichnung elektrischer Schwingungen - Analyse
 *
 * main.cpp
 * 
 * Created: 01.02.2012
 *  Author: Heiko Metzger
 * 
 */

#include <iostream>
#include <string>
#include <fstream>

using namespace std;

int main() {
	string path;
	unsigned int peak_h[200][2], peak_l[200][2], wert[1300], time = 0, i = 2, records, time_beg, time_end;
	unsigned short bufferl, bufferh, buffertl, bufferth, ph = 0, pl = 0, gen_beg = 0, gen_end = 0;
	bool error = false, suchel = true, beg_l = false, end_l = false;
	float fG1, fH1, fL1, f1;
	float fG2, fH2, fL2, f2;
	ifstream file;

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
	records = (unsigned int)file.tellg() / 4;
	file.seekg(0, ios_base::beg);
	cout << "Lese Datei ein..." << endl;
	while(records > 0)
	{
		buffertl = file.get();
		bufferth = file.get();
		bufferl = file.get();
		bufferh = file.get();
		wert[i] = bufferl + (bufferh<<8);
		// Ueberpruefe auf Spitzenwerte:
		if( !suchel & (wert[i-1] > 520) & ((wert[i-1] > wert[i-2]) & (wert[i-1] >= wert[i])) ) {
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
		time += buffertl + (bufferth<<8);
		i ++;
		records --;
	}
	file.close();
	printf("\n%d Datensaetze ueber %d * 10^(-6)s gelesen, je %d Spitzen gefunden.\n", i, time, ph +1);
	
	cin >> i;
	// Halbe Generatorperiode suchen:
	for(i = i; i < ph; i ++) {
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
	fG1 = 500000 / ((float)time_end - (float)time_beg);
	printf("\nHalbe Generatorperiode von %d bis %d, Generatorfrequenz = %fHz\n", time_beg, time_end, fG1);

	if(gen_end-gen_beg < 3) {
		cout << "Zu wenige Schwingungen fuer zuverlaessige Berechnung!" << endl << "Generatorfrequenz verringern!" << endl;
		cout << "'Return' zum Beenden";
		cin.get();
		return(0);
	}

	fH1 = ( (((float)gen_end-(float)gen_beg)-3) * 1000000) / ((float)peak_h[gen_end-1][0] - (float)peak_h[gen_beg + 1][0]);
	fL1 = ( (((float)gen_end-(float)gen_beg)-3) * 1000000) / ((float)peak_l[gen_end-1][0] - (float)peak_l[gen_beg + 1][0]);
	f1  = (fH1 + fL1) / 2;
	printf("Eigenfrequenz des Schwingkreises = %fHz\n", f1);

	// Halbe Generatorperiode suchen:
	gen_beg = i;
	i++;
	gen_end = 0;
	while(i < ph) {
		if( peak_h[i][1] > peak_h[i-1][1] ) {
			gen_end = i;
			break;
		}
		i ++;
	}
	if(peak_l[gen_beg][1] < peak_l[gen_beg-1][1]) {
		beg_l = true;
	}
	if(peak_l[gen_end][1] < peak_l[gen_end-1][1]) {
		end_l = true;
	}
	time_beg = (beg_l)?(peak_l[gen_beg][0]):(peak_h[gen_beg][0]);
	time_end = (end_l)?(peak_l[gen_end][0]):(peak_h[gen_end][0]);
	fG2 = 500000 / ((float)time_end - (float)time_beg);
	printf("Halbe Generatorperiode von %d bis %d, Generatorfrequenz = %fHz\n", time_beg, time_end, fG2);

	if(gen_end-gen_beg < 3) {
		cout << "Zu wenige Schwingungen fuer zuverlaessige Berechnung!" << endl << "Generatorfrequenz verringern!" << endl;
		cout << "'Return' zum Beenden";
		cin.get();
		return(0);
	}

	fH2 = ( (((float)gen_end-(float)gen_beg)-3) * 1000000) / ((float)peak_h[gen_end-1][0] - (float)peak_h[gen_beg + 1][0]);
	fL2 = ( (((float)gen_end-(float)gen_beg)-3) * 1000000) / ((float)peak_l[gen_end-1][0] - (float)peak_l[gen_beg + 1][0]);
	f2  = (fH2 + fL2) / 2;
	printf("Eigenfrequenz des Schwingkreises = %fHz\n", f2);

	printf("\nMittelwerte: fG = %fHz, fS= %fHz\n\n", ( (fG1 + fG2) / 2 ), ( (f1 + f2) / 2 ));

	cout << "'Return' zum Beenden";
	cin.get();
	cin.get();

	return (0);
}