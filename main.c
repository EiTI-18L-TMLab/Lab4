/*
--------------------------------------------------
    Technika Mikroprocesorowa - semestr 18L
    Laboratorium 4

    Autorzy:
        Konrad Winnicki
        Jakub Sikora
        Marcin Dolicher
---------------------------------------------------
    Oznaczenia portów:

    Port 1 (P1) - obsługa przyciskow
        |
        |--> Przycisk 7 - rozpoczyna zliczanie
        |--> Przycisk 0 - konczy zliczanie

    Port 2 (P2) - obsługa drivera 7-seg LED
        |
        |--> DATA_A - P2OUT.0
        |--> DATA_B - P2OUT.1
        |--> DATA_C - P2OUT.2
        |--> DATA_D - P2OUT.3
        |--> RBI 	- P2OUT.4
        |--> BI 	- P2OUT.5
        |--> LT 	- P2OUT.6
        |--> DP 	- P2OUT.7

    Port 3 (P3) - wybór wyświetlacza 7-segmentowego
        |
        |--> SEG_0 - P2OUT.0
        |--> SEG_1 - P2OUT.1
        |--> SEG_2 - P2OUT.2
        |--> SEG_3 - P2OUT.3
        |--> SEG_4 - P2OUT.4
        |--> SEG_5 - P2OUT.5
        |--> SEG_6 - P2OUT.6
        |--> SEG_7 - P2OUT.7

*/

#include <msp430.h>
#include <stdio.h>
#include <stdint.h>

#define bool  uint8_t
#define true 1
#define false 0

// Bity sterujące driverem 7-SEG LED
#define LED_DP (1<<7)
#define LED_LT (1<<6)
#define LED_BI (1<<5)
#define LED_RBI (1<<4)

// Ilość wyświetlanych znaków
#define 7SEG_LED_NUMBER 6
#define 7SEG_LED_DOT_POSITION 3
#define 7SEG_LED_DISPLAY_SYSTEM 10

volatile char buffer[7SEG_LED_NUMBER]; // BUFOR ZLICZANIA CZASU REAKCJI
volatile uint32_t count_time = 0; // wartośc czasu reakcji
volatile bool if_counting = false;

int main(void)
{
    WDTCTL = WDTPW + WDTHOLD; // wylaczenie watchdoga

	CCTL0 = CCIE;			// CCR0 interrupt enabled -OK -zalaczamy przerwania do timerow (jak nie zadziala wlaczac osobna TA i TB)
	// Timer A
	TACTL = TASSEL_2 + MC_1 + ID_0;           // SMCLK/8, upmode -OK SMCLK = 1MHz; MC_1 - UP MODE; ID_0 - dzielnik /1 (nie musi byc, ale niech bedzie)                 
	TACCR0 = 1000; // 1 kHz
	
    // inicjalizacja portu P1
    //P1SEL |= (BIT0 + BIT7); // ustaw P1.0 i P1.7 jako GPIO
    P1SEL &= ~(BIT0 + BIT7); // ustaw P1.0 i P1.7 jako GPIO
    P1DIR &= ~(BIT0 + BIT7); // ustaw jako wejscia
    P1OUT |= (BIT0 + BIT7); // pullup
    P1IES |= (BIT0 + BIT7); // zbocze opadajace
    P1IE = BIT7; //wlaczenie przerwan

    // inicjalizacja portu P2
    P2SEL = 0x00; // ustaw cały port 2 jako GPIO
    P2DIR = 0xFF; // ustaw port 2 jako wyjścia
    P2OUT = 0x00; // domyslnie zero

    // inicjalizacja portu P3
    P3SEL = 0x00; // ustaw cały port 3 jako GPIO
    P3DIR = 0xFF; // ustaw port 3 jako wyjścia
    P3OUT = 0xFF; // wyswietl wszedzie zera

    _bis_SR_register(CPUOFF+GIE); // LPM0, globalne włączenie obsługi przerwań
	
	while(true)
	{

	}
	
	return 0;
}

#pragma vector=PORT1_VECTOR
__interrupt void Port_1(void)
{
    if(P1IFG & BIT0) // konczymy zliczac
    {
    	P1IE &= ~BIT0;
    	P1IE |= BIT7;
        P1IFG &= ~(BIT0 + BIT7);
        if_counting = false;
        // tu zastopuj timer
    }

    if(P1IFG & BIT7) // zaczynamy zliczac
    {
    	P1IE |= BIT0;
    	P1IE &= ~BIT7;
        P1IFG &= ~(BIT0 + BIT7);
        count_time = 0; // wyczyszczenie starej zliczonej wartości
        uint16_t n=0;
    	while( n<7SEG_LED_NUMBER ) 
    	{
    		buffer[n] = 0;
    		n++;
    	}
        if_counting = true;
    }
}


#pragma vector=TIMERA0_VECTOR
__interrupt void display(void) // timer 1kHz
{
    static uint16_t disp = 0; //ktory segment ma byc podswietlony
    static uint16_t led_timer = 0; //odswiezamy co drugie cykniec
    static char disp_buffer[7SEG_LED_NUMBER]; // Bufor wyświetlanych znakow

    if(if_counting)
    {
    	count_time++;
		buffer[0]++;
    	uint16_t n = 0;
    	while( n<7SEG_LED_NUMBER )
    	{
    		if( buffer[n]>=7SEG_LED_DISPLAY_SYSTEM )
    		{
    			buffer[n]=0;
    			buffer[n+1]++;
				n++;
    		}
			else 
				break;
    	}
    }

    if( disp==0 ) // Przekopiowanie buforów
    {
    	uint16_t n=0;
    	while( n<7SEG_LED_NUMBER )
    	{
    		disp_buffer[n] = buffer[n];
    		n++;
    	}
    }
	
    if(led_timer==0) // start sekwencji wyswietlania, 1kHz/2
    {
        led_timer = 1; // co drugie wejscie do obsługi przerwania od timera

        P3OUT = ~(1<<disp); //aktywujemy kolejny wyswietlacz
        P2OUT = ((disp_buffer[disp])&0x0F) | LED_RBI | LED_BI | LED_LT | LED_DP; // wyświetlenie cyfry oraz bity sterujące
        if(disp == 7SEG_LED_DOT_POSITION) P2OUT &= ~LED_DP; // zapalenie kropki na odpowiedniej pozycji
        disp++; // wybór kolejnego wyświetlacza
        if(disp >= 7SEG_LED_NUMBER) disp = 0;
    }
    else led_timer--;
}