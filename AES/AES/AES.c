/*
 * AES.c
 *
 * Created: 16.01.2012 21:00:14
 *  Author: Heiko Metzger
 */ 

#define F_CPU 8000000

#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdbool.h>

// Fuer die Speicherkarte:
#include "sd/mmc_config.h"
#include "sd/file.h"
#include "sd/fat.h"
#include "sd/mmc.h"

volatile uint8_t 	TimingDelay;

// Fuer die Pins:

#define	LED_ROT		PC5
#define	LED_GELB	PC3
#define	LED_GRUEN	PC4
#define TST			PC2

// Fuer die Messung:

bool first = true, messen = true;						// Benoetigte Variablen definieren
char datensatz[4];
uint8_t low, high;

int main(void)
{
	// Anschalten
	// Eingangs-/Ausgangspins, Pullup-Widerstaende
	DDRC	= (1<<PC3) | (1<<PC4) | (1<<PC5);			// Ausgaenge fuer LEDs
	PORTC	= (1<<LED_GRUEN) | (1<<LED_ROT) | (1<<TST);	// LEDs "beschaeftigt" und rot an, Pullup fuer Taster an PC2
	
	while(PINC & (1<<TST)) {}							// Warten bis Taster an PC2 gedrueckt
	
	// Initialisierung:
	PORTC	|=  (1<<LED_GELB);							// LED "beschaeftigt" an
	PORTC	&= ~(1<<LED_GRUEN);							// LED "bereit" aus
	
	// TIMER_COUNTER_2: 8-Bit Counter f�rs Beenden der Aufzeichnung
	TIMSK2	|= (1<<TOIE2);								// Interrupt bei Owerflow aktivieren
	
	// AD_Wandler an PC0:								// Werte fuer Pin-Wahl und Vergleichswert sind initialwerte
	DIDR0	|= (1<<ADC0D);								// Digitalen Eingang an PC0 deaktivieren
	ADCSRA	|= (1<<ADPS2) | (1<<ADPS0);					// Prescaler: clk/32 = 250kHz
	ADCSRA	|= (1<<ADEN);								// A/D-Wandler aktivieren
	ADCSRA	|= (1<<ADSC);								// Erste Wandlung durchfuehren (Benoetigt 25 statt 13 Zyklen)
	while(ADCSRA & (1<<ADSC)) {}						// Auf Abschluss der Wandlung warten
	ADCSRA	|= (1<<ADATE) | (1<<ADIE);					// Free Running Mode
	
	
	// TIMER_COUNTER_0: 8-Bit Timer f�r die SD-Karte
	// Initialisierung, auf jeden Fall vor mmc_init()
	TimingDelay = 0;
	TCCR0A	= (1<<WGM01); 								// Clear Timer on Compare Match (CTC)
	TIMSK0	= (1<<OCIE0A);								// Interrupt bei Compare Match aktivieren
	TCNT0	= 0xB2;										// Startwert		| Berechnet mit http://www.avrcalc.com/download.html,
	OCR0A	= 0x4D;										// Vergleichswert	| aus diesen Werten (und dem Prescaler) ergibt sich die Tickzeit, hier 10ms.
	TCCR0B	|= (1<<CS02) | (1<<CS00);					// TIMER_COUNTER_0 mit Prescaler clk/1024 starten
	sei();												// Auf Interrupts reagieren
	
	while(mmc_init() == FALSE) {}						// SD-Karte Initialisieren, bei Fehler erneut bis es klappt
	
	if(fat_loadFatData() == FALSE) {					// Dateisystem laden
		PORTC	|= (1<<LED_GRUEN);							// Wenn das Dateisystem nicht geoeffnet werden kann, alle Lichter an
		while(1) {}											// Und nichts mehr tun
	}
	
	// Datei anlegen
	uint8_t file_bin [] = "messung.bin";
	if( MMC_FILE_OPENED == ffopen(file_bin,'r') ){		// Falls schon vorhanden, einfach loeschen		
		ffrm(file_bin);
	}
	
	if(MMC_FILE_ERROR == ffopen(file_bin,'c') ){		// Datei zum Schreiben oeffnen.
		PORTC	|= (1<<LED_GRUEN);							// Wenn die Datei nicht geoeffnet werden kann, alle Lichter an
		while(1) {}											// Und nichts mehr tun
	}
	
	// Bereit zur Messung
	PORTC	|=  (1<<LED_GRUEN);							// LED "bereit" an
	PORTC	&= ~(1<<LED_GELB);							// LED "beschaeftigt" aus
	
	while(PINC & (1<<TST)) {}							// Warten bis Taster an PC2 gedrueckt
	
	// Messung beginnt:
	
	PORTC	&= ~(1<<LED_GRUEN);							// LED "bereit" aus
	PORTC	|= (1<<LED_GELB);							// LED "beschaeftigt" an
	
	TCCR2B	|= (1<<CS22) | (1<<CS21) | (1<<CS20);		// TIMER_COUNTER_2: mit Prescaler clk/1024 starten
	
	ADCSRA	|= (1<<ADSC);								// Analog/Digital Wandler starten
	
	while(messen) {}									// Messung abwarten, den Rest regeln Interrupts
	
	ADCSRA	&= ~(1<<ADEN);								// ADC Ausschalten
	ffclose();											// Datei schlie�en
	
	// Messung ist abgeschlossen, nun muss das Ergebnis fuer Menschen lesbar gemacht werden:
	// Datei anlegen
	uint8_t file_hr [] = "messung.csv";
	if( MMC_FILE_OPENED == ffopen(file_hr,'r') ){		// Falls schon vorhanden, einfach loeschen		
		ffrm(file_hr);
	}
	
	if(MMC_FILE_ERROR == ffopen(file_hr,'c') ){			// Datei zum Schreiben oeffnen.
		PORTC	|= (1<<LED_GRUEN);							// Wenn die Datei nicht geoeffnet werden kann, alle Lichter an
		while(1) {}											// Und nichts mehr tun
	}
	
	ffclose();
	ffopen(file_bin, 'r');								// Messergebnis zum Lesen oeffnen
	uint32_t seek = file.length;						// Dateigroesse zwischenspeichern
	ffclose();
	while(seek > 1) {
		ffopen(file_bin, 'r');							// Messergebnis zum Lesen oeffnen
		ffseek(file.length - seek);						// Zu aktueller Position springen
		low		= ffread();								// 2 Bytes lesen
		high	= ffread();
		ffclose();
		ffopen(file_hr, 'w');							// Zieldatei zum Schreiben oeffnen
		ffseek(file.length);							// Ans Dateiende springen
		sprintf(datensatz, "%04i", low + (high<<8));	// Datensatz formatieren
		for(int i = 0; i < 4; i++) {					// Den Formatierten Datensatz in die Datei schreiben
			ffwrite((uint8_t)datensatz[i]);
		}
		ffwrite(0x0A);									// Neue Zeile
		ffclose();
		seek -= 2;										// Position um 2 Byte weiter
	}
	
	PORTC	&= ~(1<<LED_GELB);							// LED "beschaeftigt" aus
	PORTC	|= (1<<LED_GRUEN);							// LED "bereit" an
	PORTC	&= ~(1<<LED_ROT);							// rote LED aus
	
	MCUCR	|= (1<<SE) | (1<<SM1);						// Sleepmode einstellen
	asm volatile("sleep");								// Ausschalten
}

ISR (TIMER2_OVF_vect)
{
	if(!first)											// Beim ersten Durchlauf nichts �ndern
	{
		messen = false;									// While-Schleife verlassen
	}else{
		first = false;									// Ersten Durchlauf markieren
	}	
}

ISR (TIMER0_COMPA_vect)
{
	TimingDelay = (TimingDelay==0) ? 0 : TimingDelay-1;
}

ISR (ADC_vect)
{
	ffwrite(ADCL);										// Schreibe auf SD-Karte
	ffwrite(ADCH);
}