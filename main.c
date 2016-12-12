#define F_CPU 8000000UL
#include <avr/io.h>
#include <util/delay.h>

//******************************************************************************
// Universallauflicht mit Atiny261/461/861
// Anschlussbelegung:
//                         ____   ____
//       Drehschalter 8  *|PB0 \_/ PA0|* 20 LED
//       Drehschalter 4  *|PB1     PA1|* 19 LED
//       Drehschalter 2  *|PB2     PA2|* 18 LED
//       Drehschalter 1  *|PB3     PA3|* 17 LED
//                +5V 5  *|VCC     GND|* 16 GND
//                GND 6  *|GND    AVCC|* 15 +5V
//         Poti (ADC7)7  *|PB4     PA4|* 14 LED
//                PB5 8  *|PB5     PA5|* 13 LED
//                PB6 9  *|PB6     PA6|* 12 LED
//  (reset) +5V -10k- 10 *|PB7     PA7|* 11 LED
//                        |___________|
//                      Attiny261/461/861
// 
// Funktionsbeschreibung:
// DIP-Drehschalter wählt Lauflichtfunktion aus
// Poti ändert die Geschwindigkeit (ADC4)
// ACHTUNG: Bedingt durch das Layout des DIP-Schalters sind die Positionen
// nicht identisch mit dem Wert, also DIP-Position 1 ist nicht 1 für den 
// Port B !!! Port B muss im Programm invertiert werden.
//
//************************************************************************
// B. Redemann 2011, Freie Software
//************************************************************************


//Variablen für das Pseudorandomlauflicht zur Berechnung des XOR Outputs
uint8_t Wert=1, XORErg = 0, XORWert1, XORWert2;
	  
// Für delay Funktion
//i: für die for-Schleife
//x: Hilfsvariable
uint8_t i, x;





// Vezögerung invertiert zum Verhältnis Potistellung / ADC Wert
// Daher 1024 (1024 - z (=ADC Wert 0...1024))
void delay (uint16_t z)
{
	_delay_ms(1024-z);
}

void initPORTS(void)
{
// Ports einstellen
// Achtung! Wenn der Trimmer und der Kodierschalter bestückt sind, müssen die
// Pins PB0 bis PB4 als Eingang konfiguriert werden. Sonst Kurzschlussgefahr! 
// Drehschalter (PB0-PB3), Potipin (PB4)auf Eingang, Rest Ausgang, Reset Eingang
DDRB = (0<<PB7 | 1<<PB6 | 1<<PB5 | 0<<PB4 | 0<<PB3 | 0<<PB2 | 0<<PB1 | 0<<PB0);
// Port A komplett auf Ausgang
DDRA = 0xff; // Für LEDs

// Ausgangspins Port A auf 0 setzen (alle LEDs aus)
PORTA = 0x00;
}

void initAD(void)
{
// ADC Section **************************************************************
// ADC an Port PB4 (ADC7)
// ADMUX: VREF=AVCC, ADLAR (right), ADC 7
// REFS1=0 REFS0=0 ADLAR=0 MUX4 MUX3 MUX2 MUX1 MUX0
//ADMUX = 0b00100111; oder
ADMUX = (0<<REFS1 | 0<<REFS0 | 0<<ADLAR | 0<<MUX4 | 0<<MUX3 | 1<<MUX2 | 1<<MUX1 | 1<<MUX0);
//
// ADCSRA: Enable, start convert, ohne Trigger, free running, prescalar 0b100 
// ADEN=1 ADSC=1 ADATE=1 ADIF=0 ADIE=0 ADPS2=1 ADPS1=0 ADPS0=0
///ADCSRA = 0b11100111; oder
ADCSRA = (1<<ADEN | 1<<ADSC | 1<<ADATE | 0<<ADIF | 0<<ADIE | 1<<ADPS2 | 0<<ADPS1 | 0<<ADPS0);
//
// ADCSRB wird auf 00 bleiben (default Wert, free running)
ADCSRB = 0x00;
// ADC Section end **********************************************************
}


//#0 Schalterposition
void normal(void)
{
		//Normales Lauflicht
	    PORTA = 1;
		for (i=1; i<8;i++)
		{
		delay(ADC);
		PORTA <<= 1; 		//Linksverschiebung, Aktuelles Bitmuster ausgeben
		}			
	    delay(ADC);
}

//#1 Schalterposition
void blitz(void)
{
		//Blitzlauflicht
	    x = 1; //Hilfsvariable
		for (i=1; i<9;i++)
		{
		delay(ADC);
		PORTA = x;		//Aktuelles Bitmuster ausgeben
		x <<= 1; 		//Linksverschiebung
		delay(ADC);
		PORTA = 0; 		//Portausschalten (typisch für Blitzlauflich)
	    }
		 			// Dann wieder zum Anfang, 7 Stellen nach rechts
	    _delay_ms(100);
}

//#2 Schalterposition
void hinher(void)
{
		//Hin- Herlauflicht
	    PORTA = 1;
		for (i=1; i<8;i++)
		{
		delay(ADC);
		PORTA <<= 1; 		//Linksverschiebung, Aktuelles Bitmuster ausgeben
		}
		for (i=1; i<8;i++)
		{
		delay(ADC);
		PORTA >>= 1; 		//Rechtsverschiebung, Aktuelles Bitmuster ausgeben
		}
		
}	

//#3 Schalterposition
void pseudo(void)
{
      
	  PORTA = Wert;
            
      XORWert1 = (0x40 & Wert) >>6;
      XORWert2 = (0x20 & Wert) >>5;
      XORErg = XORWert1 ^ XORWert2;
      
      if (XORErg == 0)
      {
          Wert = Wert << 1;
      }
      if (XORErg == 1)
      {
          Wert = (Wert << 1) + XORErg;
      }  
      delay(ADC);
	  
} 

//#4 Schalterposition
void wechsel(void)
{
//LED wechsel
	    PORTA = 0x55;
		delay(ADC);
		PORTA = ~PORTA;
		delay(ADC);
}	

//#5 Schalterposition
void ampel1(void)
{
//Sehr einfache Baustellenampel, blaue LEDs werden nicht verwendet
	    PORTA = 0x28; // Rot1 - Grün2
		delay(ADC-1000);
		PORTA = 0x48; // Rot1 - Gelb2
		delay(ADC);
	    PORTA = 0x88; // Rot1 - Rot2
		delay(ADC-1000);
	    PORTA = 0x8C; // ROT_Gelb1 - Rot2
		delay(ADC);
        PORTA = 0x82; // Grün1 - Rot2
		delay(ADC-1000);
		PORTA = 0x84; // Gelb1 - Rot2
		delay(ADC);
		PORTA = 0x88; // Rot1 - Rot2
		delay(ADC-1000);
		PORTA = 0xC8; // Rot1 - Rot_Gelb2
		delay(ADC);
}	

//#6 Schalterposition
void eins_zwei_drei(void)
{
	    PORTA = 1;
		for (i=1; i<9;i++)
		{
		delay(ADC);
		PORTA <<= 1; 		//Aktuelles Bitmuster ausgeben, nach links verschieben
		}
		PORTA = 1;
		delay(ADC);
		PORTA = 3;
		for (i=1; i<9;i++)
		{
		delay(ADC);
		PORTA <<= 1; 		//Aktuelles Bitmuster ausgeben, nach links verschieben
		}
		PORTA = 1;
		delay(ADC);
		PORTA = 3;
		delay(ADC);
		PORTA = 7;
		for (i=1; i<9;i++)
		{
		delay(ADC);
		PORTA <<= 1; 		//Aktuelles Bitmuster ausgeben, nach links verschieben
		}
		PORTA = 0;
		delay(ADC);
		PORTA = 1;
		delay(ADC);
}

//#7 Schalterposition
void werbe1(void)
{
		for (i=1; i<129; i++)
		{
		PORTA = i;	
		delay(ADC);    
		i = i * 2;
		}
		for (i=0; i<4; i++)
		{
		PORTA = 255;
	    delay(ADC);
	    PORTA = 0;
		delay(ADC);
		}
		
}

//#8 Schalterposition
void werbe2(void)
{
		//for (i=1; i<129; i++)
		{
		PORTA = 0x81;	
		delay(ADC);    
		PORTA = 0x42;	
		delay(ADC);
		PORTA = 0x24;	
		delay(ADC);
		PORTA = 0x18;	
		delay(ADC);
		PORTA = 0x24;	
		delay(ADC);
		PORTA = 0x42;	
		delay(ADC);
		PORTA = 0x81;	
		delay(ADC); 
		}
		for (i=0; i<4; i++)
		{
		PORTA = 255;
	    delay(ADC);
	    PORTA = 0;
		delay(ADC);
		}
		
}

//#9 Schalterposition
void werbe3(void)
{
		for (i=0; i<129; i++)
		{
		PORTA = i;	
		delay(ADC);    
		i = i * 2;
		}
		PORTA = 255;
		delay(ADC); 
		for (i=0; i<129; i++)
		{
		PORTA = 255 - i;
		i = i * 2;
		delay(ADC);
		}
		PORTA = 0;
		delay(ADC);
		
} 

//#10 Schalterposition A
void blinkall(void)
{
PORTA =~PORTA;
delay(ADC);
}

int main (void)
{
      
	  
	  // Ports initialisieren
      initPORTS();
	  
initAD();

// Verschiedene Lauflichter, 11 Beispiele
while (1)
{
		PORTB =~PINB & 0x0f; //Schaltereingaben müssen invertiert werden
		switch (PORTB)
			{
			case 0:  normal();   		break;
			case 1:  blitz();			break; 
			case 2:  hinher();   		break; 
			case 3:  pseudo();  		break; 
			case 4:  wechsel();        	break;
			case 5:  ampel1();		   	break; 
			case 6:  eins_zwei_drei(); 	break; 
			case 7:  werbe1();			break; 
			case 8:  werbe2();			break; 
			case 9:  werbe3();			break; 
			case 10: blinkall();	    break;
			case 11: break;
			case 12: break;
			case 13: break;
			case 14: break;
			case 15: break;
			}



}


return 0;
}


