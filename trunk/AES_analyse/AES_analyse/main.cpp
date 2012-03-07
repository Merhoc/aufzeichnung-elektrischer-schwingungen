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
	FILE * file;
	int peaks, i;
	unsigned short value[650];
	char bufferl, bufferh;
	i = 0;
	peaks = 0;
	ifstream ifs("MESSUNG.BIN", ifstream::in);
	if(ifs.good()) {
		while(ifs.good()) {
			ifs.read(&bufferl, sizeof(bufferl));
			ifs.read(&bufferh, sizeof(bufferh));
			value[i] = (unsigned char)bufferl + ((unsigned char)bufferh<<8);
			i ++;
			cout << i << ": " << i * 0.1265 << " - " << (unsigned int)bufferl << endl;
		}
		if(true) {
			peaks ++;
		}
	}else{
		cout << "Datei konnte nicht gefunden werden. " << endl;
	}

	cout << "'Return' zum Beenden";
	cin.get();

	return (0);
}