/****************************************************************************/
/*		Aufzeichnung elektrischer Schwingungen - Analyse					*/
/*																			*/
/*		Autor:		Heiko Metzger											*/
/*		Erstellt:	01.02.2012												*/
/****************************************************************************/

#include <iostream>
#include <string>

using namespace std;

int main() {
	FILE * file;
	int peaks, i, value[650];
	int buffer;
	i = 0;
	peaks = 0;

	if(file = fopen("MESSUNG.BIN", "r")) {
		while(!feof(file)) {
			buffer = fgetc(file) + (fgetc(file)<<8);
			i ++;
			cout << i << ": " << i * 0.1265 << " - " << value[i] << endl;
		}
		if(true) {
			peaks ++;
		}
		
		
		fclose(file);
	}else{
		cout << "Datei konnte nicht gefunden werden. " << endl;
	}

	cout << "'Return' zum Beenden";
	cin.get();

	return (0);
}