/*
--------------------------------------------------
    Technika Mikroprocesorowa - semestr 18L
    Laboratorium 4

    Autorzy:
        Konrad Winnicki
        Jakub Sikora
        Marcin Dolicher
---------------------------------------------------
    Oznaczenia portow:

    Port 1 (P1) - obsluga przyciskow - robimy na dwoch kanalach
        |
        |--> Pin 2, start, CCI1A input - capture (kanal 1)
        |--> Pin 3, stop, CCI2A input - capture (kanal 2)


    Port 2 (P2) - obsluga drivera 7-seg LED
        |
        |--> DATA_A - P2OUT.0
        |--> DATA_B - P2OUT.1
        |--> DATA_C - P2OUT.2
        |--> DATA_D - P2OUT.3
        |--> RBI    - P2OUT.4
        |--> BI     - P2OUT.5
        |--> LT     - P2OUT.6
        |--> DP     - P2OUT.7

    Port 3 (P3) - wyborr wyswietlacza 7-segmentowego
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

// Bity sterujace driverem 7-SEG LED
#define LED_DP (1<<7)
#define LED_LT (1<<6)
#define LED_BI (1<<5)
#define LED_RBI (1<<4)

// Ilosc wyswietlanych znakow
#define SEG_LED_NUMBER 7
#define SEG_LED_DOT_POSITION 5
#define SEG_LED_DISPLAY_SYSTEM 10

// Watrość rejestru licznika
#define TACCR0_VALUE 720

volatile char buffer[SEG_LED_NUMBER]; // BUFOR ZLICZANIA CZASU REAKCJI
volatile uint32_t count_time = 0; // wartosc czasu reakcji
volatile bool if_counting = false;
volatile bool end_of_counting=false;


typedef struct captured_cycles
{
    int start_time;
    int num_of_overflows;
    int end_time;
} captured_cycles;

int get_cycles(captured_cycles time)
{
    return (time.num_of_overflows + 1)*TACCR0_VALUE + time.end_time - time.start_time;
}

int get_time(int cycles)
{
    return (int)(cycles/10);
}

void temp_time()
{
    uint16_t n = SEG_LED_DOT_POSITION-3; // zgrubny czas z dokładnością milisekundy
    buffer[n]++; // odśwież pozycję milisekund
    while( n<SEG_LED_NUMBER )
    {
        if( buffer[n]>=SEG_LED_DISPLAY_SYSTEM )
        {
            buffer[n]=0;
            buffer[n+1]++;
            n++;
        }
        else
            break;
    }
}

void copy_buffer(char disp_buffer[])
{
	uint16_t n=0;
    while( n<SEG_LED_NUMBER )
    {
        disp_buffer[n] = buffer[n]; // buffer mamy globalny
        n++;
    }
}

void display(uint16_t disp, char disp_buffer[])
{
	//static uint16_t disp = 0;
	P3OUT = ~(1<<disp); //aktywujemy kolejny wyswietlacz
	P2OUT = ((disp_buffer[disp])&0x0F) | LED_RBI | LED_BI | LED_LT | LED_DP; // wyswietlenie cyfry oraz bity sterujace
	if(disp == SEG_LED_DOT_POSITION) P2OUT &= ~LED_DP; // zapalenie kropki na odpowiedniej pozycji
	//disp++; // wybor kolejnego wyswietlacza
	//if(disp >= SEG_LED_NUMBER) disp = 0;
}

void preapare_to_display_ideal_value(captured_cycles cap_time)
{
    	int locked_time = get_time(get_cycles(cap_time)); //zatrzaskujemy wartosc na czas sekwencji wyswietlania

        uint8_t n=0;
		while(n<SEG_LED_NUMBER)
		{
			buffer[n]=locked_time % 10;
			locked_time /= 10;
			n++;
		}
}

volatile captured_cycles cap_time = {0, 0, 0};



int main(void)
{
	// wylaczenie watchdoga
    WDTCTL = WDTPW + WDTHOLD; 

    // inicjalizacja portu P1
    P1SEL = (BIT2 + BIT3); // ustaw P1.0 i P1.7 jako wejscia timera
    P1DIR &= ~(BIT2 + BIT3); // ustaw jako wejscia
    // piny portu 1 służące do debugowania zależności czasowych przy pomocy oscyloskopu
    P1DIR |= BIT7 | BIT6;
    P1OUT |= BIT7 | BIT6;
	
    // inicjalizacja portu P2
    P2SEL = 0x00; // ustaw caly port 2 jako GPIO
    P2DIR = 0xFF; // ustaw port 2 jako wyjscia
    P2OUT =  (0x08&0x0F) | LED_RBI | LED_BI | LED_LT /*| LED_DP*/; // wyswietlenie ósemki z kropką

    // inicjalizacja portu P3
    P3SEL = 0x00; // ustaw caLy port 3 jako GPIO
    P3DIR = 0xFF; // ustaw port 3 jako wyjscia
    P3OUT = 0x00; // aktywuj wszystkie wyświetlacze

    // Inicjacja kanalu 0, compare
    TACCR0 = TACCR0_VALUE; // 1 kHz
    TACCTL0 = CCIE;         // CCR0 interrupt enabled -OK -zalaczamy przerwania do timerow (jak nie zadziala wlaczac osobna TA i TB)
    // Inicjacja kanalu 1, capture, przycisk start
    TACCTL1 =  CM_2 + SCS + CAP + CCIE + CCIS_0;  // falling edge, synchronus capture, CCI1A
    // Inicjacja kanalu 2, capture, przycisk stop
    TACCTL2 =  CM_0 + SCS + CAP + CCIE + CCIS_0;  // falling edge, synchronus capture, CCI2A
    // Inicjacja Timera A
    TACTL = TASSEL_2 + MC_1 + ID_0 + TAIE;           // SMCLK/8, upmode -OK SMCLK = 1MHz; MC_1 - UP MODE; ID_0 - dzielnik /1 (nie musi byc, ale niech bedzie)

    _bis_SR_register(CPUOFF+GIE); // LPM0, globalne wlaczenie obslugi przerwan

    while(true)
    {

    }

    return 0;
}

#pragma vector=TIMERA0_VECTOR   // TIMERA0_VECTOR - wektor do obslugi compare, kanal 0
__interrupt void timerA0_ISR(void) // timer 1kHz
{
    //P1OUT |= 0x80;

    cap_time.num_of_overflows++;
    static uint16_t disp = 0; //wybór aktywnego wyświetlacza
    static uint16_t led_timer = 0; //timer programowy wyświetlacza dynamicznego
    static char disp_buffer[SEG_LED_NUMBER]; // Bufor wyswietlanych znakow

	if(end_of_counting==true )
	{
		preapare_to_display_ideal_value(cap_time);
		//display(disp, disp_buffer);
		end_of_counting = false;
	}

	if( if_counting==true )
	{
		temp_time();

		if(disp==0)
		{
			copy_buffer(disp_buffer);
		}
	}

	if(led_timer==0) // start sekwencji wyswietlania, 1kHz/2
	{
		led_timer=1;
		display(disp, disp_buffer);
		disp++;
		if(disp>= SEG_LED_NUMBER) disp = 0;
	}else
		led_timer--;

    //P1OUT &= ~0x80;
}

#pragma vector=TIMERA1_VECTOR   // TIMERA1_VECTOR - wektor do obslugi capture, kanal 1, 2
__interrupt void timerA1_ISR(void)
{
    switch(TAIV) //odczyt TAIV
    {
        case 2: //flaga CCIFG
            P1OUT |= 0x40;
        	if_counting = true;
            cap_time.start_time = TACCR1;
            cap_time.num_of_overflows = 0;
            cap_time.end_time = 0;

            TACCTL1 &= ~CM_2;
            TACCTL2 &= ~CM_0;
            TACCTL1 |= CM_0;
            TACCTL2 |= CM_2;

			//end_of_counting=false;

			// zerownie buffer
			uint16_t n=0;
			while(n<SEG_LED_DOT_POSITION)
			{
				buffer[n] = 0;
				n++
			}

            break; //zrodlo TACCR1

        case 4: //flaga CCIFG
            P1OUT |= 0x40;
        	if_counting = false;
            cap_time.end_time = TACCR2;

            TACCTL1 &= ~CM_0;
            TACCTL2 &= ~CM_2;
            TACCTL1 |= CM_2;
            TACCTL2 |= CM_0;


			end_of_counting=true;
            break; //zrodlo TACCR2
    }
    //P1OUT &= ~0x40;
}



