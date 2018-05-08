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

    Port 2 (P2) - obsługa dynamicznego wyswietlacza
        |
        |--> SEG_A - P2OUT.0
        |--> SEG_B - P2OUT.1
        |--> SEG_C - P2OUT.2
        |--> SEG_D - P2OUT.3
        |--> SEG_E - P2OUT.4
        |--> SEG_F - P2OUT.5
        |--> SEG_G - P2OUT.6
        |--> SEG_DP - P2OUT.7

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

//#define uint8_t unsigned int
#include <stdint.h>

#define bool  uint8_t
#define true 1
#define false 0

// makra segmentów wyświetlacza
#define SEG_A BIT0
#define SEG_B BIT1
#define SEG_C BIT2
#define SEG_D BIT3
#define SEG_E BIT4
#define SEG_F BIT5
#define SEG_G BIT6
#define SEG_DP BIT7

// makra do wypisywania liczb
#define W0 (SEG_A + SEG_B + SEG_C + SEG_D + SEG_E + SEG_F)
#define W1 (SEG_B + SEG_C)
#define W2 (SEG_A + SEG_B + SEG_G + SEG_E + SEG_D)
#define W3 (SEG_A + SEG_B + SEG_G + SEG_C + SEG_D)
#define W4 (SEG_F + SEG_G + SEG_B + SEG_C)
#define W5 (SEG_A + SEG_F + SEG_G + SEG_C + SEG_D)
#define W6 (SEG_A + SEG_F + SEG_G + SEG_C + SEG_D + SEG_E)
#define W7 (SEG_A + SEG_B + SEG_C)
#define W8 (SEG_A + SEG_B + SEG_C + SEG_D + SEG_E + SEG_F + SEG_G)
#define W9 (SEG_A + SEG_B + SEG_C + SEG_D + SEG_F + SEG_G)
#define WA (SEG_A + SEG_B + SEG_C + SEG_E + SEG_F + SEG_G)
#define WB (SEG_F + SEG_G + SEG_C + SEG_D + SEG_E)
#define WC (SEG_A + SEG_F + SEG_E + SEG_D)
#define WD (SEG_B + SEG_C + SEG_D + SEG_E + SEG_G)
#define WE (SEG_A + SEG_D + SEG_E + SEG_F + SEG_G)
#define WF (SEG_A + SEG_E + SEG_F + SEG_G)



// gaszenie wszystkich diod
#define OFF ~(SEG_A + SEG_B + SEG_C + SEG_D + SEG_E + SEG_F + SEG_G + SEG_DP)

uint8_t decode(char ch);
volatile char buffer[10] = {0, 0, 0, 0, 0, 0}; // BUFOR ZLICZANIA CZASU REAKCJI

volatile uint32_t count_time;
volatile bool if_counting;
//volatile bool

int main(void)
{
    WDTCTL = WDTPW + WDTHOLD; // wylaczenie watchdoga

	CCTL0 = CCIE;			// CCR0 interrupt enabled -OK -zalaczamy przerwania do timerow (jak nie zadziala wlaczac osobna TA i TB)
	// Timer A
	TACTL = TASSEL_2 + MC_1 + ID_0;           // SMCLK/8, upmode -OK SMCLK = 1MHz; MC_1 - UP MODE; ID_0 - dzielnik /1 (nie musi byc, ale niech bedzie)
	//TACCR0 =  738;                     // 1 kHz
	TACCR0 = 1000;
    // inicjalizacja portu P1
    //P1SEL |= (BIT0 + BIT7); // ustaw P1.0 i P1.7 jako GPIO
    P1SEL &= ~(BIT0 + BIT7); // ustaw P1.0 i P1.7 jako GPIO
    P1DIR &= ~(BIT0 + BIT7); // ustaw jako wejscia
    P1OUT |= (BIT0 + BIT7); // pullup
    P1IES |= (BIT0 + BIT7); // zbocze opadajace
    P1IE = BIT7; //wlaczenie przerwan

    // POMIAR FREWUENCY TIMER INTERRUPT
    P1DIR |= 0x04;
    P1OUT |= 0x04;

    // inicjalizacja portu P2
    P2SEL = 0x00; // ustaw cały port 2 jako GPIO
    P2DIR = 0xFF; // ustaw port 2 jako wyjścia
    P2OUT = 0x00; // domyslnie zero

    // inicjalizacja portu P3
    P3SEL = 0x00; // ustaw cały port 3 jako GPIO
    P3DIR = 0xFF; // ustaw port 3 jako wyjścia
    P3OUT = 0xFF; // wyswietl wszedzie zera


    count_time = 0; // wyzerowanie policzonego czasu
    if_counting = false;
/*
    //P3OUT = 0xFF;
    //_delay_cycles(10000);
    //P2OUT = 0xFF;
    //_delay_cycles(10000);
    //P2OUT = 0x00;
    //P2OUT = 0xFF;
    P3OUT = 0x30;
    P2OUT = 0xF1-0x80;
    P2OUT = 0xF2-0x80;
    P2OUT = 0xF3;
    P2OUT = 0xF4;
    //P2OUT = 0xFF;*/

    //__enable_interrupt();
    _bis_SR_register(CPUOFF+GIE);
	
	while(true)
	{

	}
	//return 0;
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
        count_time = 0;
        uint16_t n=0;
    	while( n<6 )
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
    P1OUT ^= 0x04;
	//TACTL &= ~TAIFG;

    static uint8_t disp = 0; //ktory segment ma byc podswietlony
    static bool parity_clk = true; //odswiezamy co drugie cykniec
    static uint32_t locked_time = 123456;
    static char disp_buffer[10] = {0, 0, 0, 0, 0, 0};

    if(if_counting)
    {
    	count_time++;

    	uint16_t n = 0;
    	while( n<6 )
    	{
    		if( buffer[n]>=9 )
    		{
    			buffer[n]=0;
    			buffer[n+1]++;
    		}
    		else if( n==0 )
    			buffer[n]++;

    			n++;
    	}

    }
//if(0)
    if( disp==0 )
    {
    	uint16_t n=0;
    	while( n<6 )
    	{
    		disp_buffer[n] = buffer[n];
    		n++;
    	}

    }

if(false)
    if(disp == 0)
    {
    	locked_time = count_time; //zatrzaskujemy wartosc na czas sekwencji wyswietlania

        uint8_t n=0;
		while(n<6)
		{
			buffer[n]=locked_time % 10;
			locked_time /= 10;
			n++;
		}
    }

//if(false)
    if(parity_clk) // start sekwencji wyswietlania co drugie wejscie do obsługi przerwania od timera
    {
        parity_clk = false;
        //P3OUT &= 0x00; // gasimy poprzedni wyswietlacz
        //P2OUT &= 0x00; // gasimy wartosc

        //sprintf(buffer, "%.6d", (int)locked_time);

        //sprintf(buffer, "%d", locked_time);

        P2OUT = (disp_buffer[disp]) + 0x70 + 0x80; //nowa wartosc

        if(disp == 3) P2OUT &= ~0x80;

        P3OUT = ~(1<<disp); //aktywujemy kolejny wyswietlacz

        disp++;
        if(disp >= 6) disp = 0;
    }
    else parity_clk = true;

}

uint8_t decode(char ch)
{
    switch(ch)
    {
        case '0': return W0;
        case '1': return W1;
        case '2': return W2;
        case '3': return W3;
        case '4': return W4;
        case '5': return W5;
        case '6': return W6;
        case '7': return W7;
        case '8': return W8;
        case '9': return W9;
        default: return 0;
    }
}


