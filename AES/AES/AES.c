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

volatile uint8_t 	TimingDelay;	// fuer mmc.c

// Fuer die Pins:

#define	LED_ROT		PC5
#define	LED_GELB	PC3
#define	LED_GRUEN	PC4
#define TST			PC2

// Fuer die Messung:

bool messen = true, first = true;						// Benötigte Variablen definieren
uint16_t wert;
char datensatz[4];

int main(void)
{
	// Initialisierung:
	// Eingangs-/Ausgangspins, Pullup-Widerstände
	DDRC	= (1<<PC3) | (1<<PC4) | (1<<PC5);			// Ausgaenge fuer LEDs
	PORTC	= (1<<LED_GELB) | (1<<LED_ROT) | (1<<TST);	// LEDs "beschäftigt" und rot an, Pullup fuer Taster an PC2
	
	// TIMER_COUNTER_1: 16-Bit Counter für Zeitmessung
	//TIMSK1	|= (1<<TOIE1);								// Interrupt bei Owerflow aktivieren
	// TIMER_COUNTER_2: 8-Bit Counter fürs Beenden der Aufzeichnung
	TIMSK2	|= (1<<TOIE2);								// Interrupt bei Owerflow aktivieren
	
	// AD_Wandler an PC0:								// Werte fuer Pin-Wahl und Vergleichswert sind initialwerte
	//DIDR0	|= (1<<ADC0D);								// Digitalen Eingang an PC0 deaktivieren
	ADCSRA	|= (1<<ADPS2) | (1<<ADPS1);					// Prescaler: clk/64
	ADCSRA	|= (1<<ADEN);								// A/D-Wandler aktivieren
	ADCSRA	|= (1<<ADSC);								// Erste Wandlung durchfuehren (Benoetigt 25 statt 13 Zyklen)
	while(ADCSRA & (1<<ADSC)) {}						// Auf Abschluss der Wandlung warten
	
	// TIMER_COUNTER_0: 8-Bit Timer für die SD-Karte
	// Initialisierung, auf jeden Fall vor mmc_init()
	TimingDelay = 0;
	TCCR0A	= (1<<WGM01); 								// Clear Timer on Compare Match (CTC)
	TIMSK0	= (1<<OCIE0A);								// Interrupt bei Compare Match aktivieren
	TCNT0	= 0xB2;										// Startwert		| Berechnet mit http://www.avrcalc.com/download.html,
	OCR0A	= 0x4D;										// Vergleichswert	| aus diesen Werten (und dem Prescaler) ergibt sich die Tickzeit, hier 10ms.
	TCCR0B	|= (1<<CS02) | (1<<CS00);					// TIMER_COUNTER_0 mit Prescaler clk/1024 starten
	sei();												// Auf Interrupts reagieren
	
	// SD-Karte Initialisieren, bei Fehler erneut bis es klappt
	while(mmc_init() == FALSE) {
		PORTC	&= ~(1<<LED_GELB);							// LED "beschäftigt" aus
		PORTC	|=  (1<<LED_GELB);							// LED "beschäftigt" an
	}
	
	// Dateisystem Laden
	if(fat_loadFatData() == FALSE) {
		PORTC	|= (1<<LED_GRUEN);							// Wenn das Dateisystem nicht geöffnet werden kann, alle Lichter an
		while(1) {}											// Und nichts mehr tun
	}
	
	// Datei anlegen
	uint8_t file_name [] = "messung.csv";
	if( MMC_FILE_OPENED == ffopen(file_name,'r') ){		// Falls schon vorhanden, einfach löschen		
		ffrm(file_name);
	}
	
	if(MMC_FILE_ERROR == ffopen(file_name,'c') ){		// Datei zum Schreiben öffnen.
		PORTC	|= (1<<LED_GRUEN);							// Wenn die Datei nicht geöffnet werden kann, alle Lichter an
		while(1) {}											// Und nichts mehr tun
	}
	
	// Bereit zur Messung
	PORTC	|=  (1<<LED_GRUEN);							// LED "bereit" an
	PORTC	&= ~(1<<LED_GELB);							// LED "beschäftigt" aus
	
	while(PINC & (1<<TST)) {}							// Warten bis Taster an PC2 gedrückt
	
	// MESSUNG BEGINNT:
	
	PORTC	&= ~(1<<LED_GRUEN);							// LED "bereit" aus
	PORTC &= ~(1<<LED_ROT);
	
	TCCR2B	|= (1<<CS22) | (1<<CS21) | (1<<CS20);		// TIMER_COUNTER_2: mit Prescaler clk/1024 starten
	
	ADCSRA	|= (1<<ADSC);								// Analog/Digital Wandler starten
	PORTC	|= (1<<LED_GRUEN);							// LED "beschäftigt" an
	//TCCR1B	|= (1<<CS10);								// TIMER_COUNTER_1: clk/1 - Ohne Prescaling starten
	while(messen) {
		while(ADCSRA & (1<<ADSC)) {}
		ADCSRA |= (1<<ADSC);
		
		sprintf(datensatz, "%04i", ADC);			// Datensatz formatieren und als Zeichenkette speichern
		for(int i = 0; i < 4; i++) {						// Den Formatierten Datensatz in die Datei schreiben
			ffwrite((uint8_t)datensatz[i]);
		}
		ffwrite(0x0A);										// Neue Zeile für Windows
	}
	
	ffclose();											// Datei schließen
	
	MCUCR	|= (1<<SE) | (1<<SM1);						// Sleepmode einstellen
	asm volatile("sleep");								// Power down
}

/*ISR (ADC_vect)
{
	PORTC	&= ~(1<<LED_GELB);							// LED "beschäftigt" aus
	time = TCNT1;										// Zeit-(X-)Wert zwischenspeichern
	TCNT1 = 14;											// TIMER_COUNTER_1 von vorn zählen lassen - Die Vergangenen Takte werden als Startzeit hinzugerechnet.
	
	sprintf(datensatz, "%05i,%04i", time, ADC);			// Datensatz formatieren und als Zeichenkette speichern
	
	for(uint8_t i = 0; i < 10; i++) {					// Den Formatierten Datensatz in die Datei schreiben
		ffwrite((uint8_t)datensatz[i]);
	}		
	ffwrite(0x0D);										// Neue Zeile
	ffwrite(0x0A);										// Neue Zeile für Windows
	PORTC	|= (1<<LED_GELB);							// LED "beschäftigt" an
}*/

ISR (TIMER2_OVF_vect)
{
	if(!first)											// Beim ersten Durchlauf nichts ändern
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
