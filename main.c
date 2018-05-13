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

    Port 3 (P3) - wyb�r wy�wietlacza 7-segmentowego
        |
        |--> SEG_0 - P2OUT.0
        |--> SEG_1 - P2OUT.1
        |--> SEG_2 - P2OUT.2
        |--> SEG_3 - P2OUT.3
        |--> SEG_4 - P2OUT.4
        |--> SEG_5 - P2OUT.5
        |--> SEG_6 - P2OUT.6
        |--> SEG_7 - P2OUT.7

        ##############################################
        ##############################################

        ZAJRZYJ KOLEGO DROGI DO KOMENTARZA NA DOLE, PRZED FUNKCJAMI PRZERWAN

        ###################################################
        ###################################################

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
#define SEG_LED_NUMBER 6
#define SEG_LED_DOT_POSITION 3
#define SEG_LED_DISPLAY_SYSTEM 10

volatile char buffer[SEG_LED_NUMBER]; // BUFOR ZLICZANIA CZASU REAKCJI
volatile uint32_t count_time = 0; // wartosc czasu reakcji
volatile bool if_counting = false;

struct captured_cycles
{
    int start_time;
    int num_of_overflows;
    int end_time;
}

int get_cycles(captured_cycles time)
{
    return num_of_overflows*1000 + end_time - start_time;
}

int main(void)
{
    WDTCTL = WDTPW + WDTHOLD; // wylaczenie watchdoga

    // Startujemy z wszystkimi bitamni na 0
    // Inicjacja kanalu 0, compare
    TACCR0 = 1000; // 1 kHz
    TACCTL0 = CCIE;         // CCR0 interrupt enabled -OK -zalaczamy przerwania do timerow (jak nie zadziala wlaczac osobna TA i TB)

    // Inicjacja kanalu 1, capture, przycisk start
    TACCTL1 =  CM_2 + SCS + CAP + CCIE + CCIS_0;  // falling edge, synchronus capture, CCI1A

    // Inicjacja kanalu 2, capture, przycisk stop
    TACCTL2 =  CM_2 + SCS + CAP + CCIE + CCIS_0;  // falling edge, synchronus capture, CCI2A

    // Inicjacja Timera A
    TACTL = TASSEL_2 + MC_1 + ID_0 + TAIE;           // SMCLK/8, upmode -OK SMCLK = 1MHz; MC_1 - UP MODE; ID_0 - dzielnik /1 (nie musi byc, ale niech bedzie)

    // inicjalizacja portu P1
    P1SEL |= (BIT2 + BIT3); // ustaw P1.0 i P1.7 jako wejscia timera
    P1DIR &= ~(BIT2 + BIT3); // ustaw jako wejscia
    P1OUT |= (BIT2 + BIT3); // pullup
    //P1IES |= (BIT2 + BIT3); // zbocze opadajace
    //P1IE = BIT7; //wlaczenie przerwan

    // inicjalizacja portu P2
    P2SEL = 0x00; // ustaw ca�y port 2 jako GPIO
    P2DIR = 0xFF; // ustaw port 2 jako wyj�cia
    P2OUT = 0x00; // domyslnie zero

    // inicjalizacja portu P3
    P3SEL = 0x00; // ustaw ca�y port 3 jako GPIO
    P3DIR = 0xFF; // ustaw port 3 jako wyj�cia
    P3OUT = 0xFF; // wyswietl wszedzie zera
    // pomysl ficzera: test wyswietlaczy (zapalanie wszystkiego)

    _bis_SR_register(CPUOFF+GIE); // LPM0, globalne wlaczenie obslugi przerwan

    while(true)
    {

    }

    //return 0;
}

/*
 * Hello motherfucker ;)
 * wykorzystujemy jeden timer, a w nim trzy kanaly,
 *  0 - compare, podstawa czasu 1ms
 *  1 - capture, przycisk START, aktywny zboczem opadajacym
 *  2 - capture, przycisk STOP, aktywny zboczem opadajacym
 *  zrobione tak aby moc doprowadzic sygnaly zewnetrzne do timera,
 *  nie dalo sie doprowadzic dwoch sygnalow do jednego kanalu z opcja przelaczania w locie
 *
 * mamy dwa wektory przerwan od timera A - A0 i A1 (strona 368 w mps430 user manual-u)
 * propozycja wykonania - mysl moja:
 * A0 - CCIFG_0_HANDLER
 *  przepelnienie od compare co 1ms
 * A1 - TAIFG_HANDLER(podobne do poprzedniego, nie jestem pewny jak to sie zachowa),
 *      CCIFG_2_HND - przerwanie zgloszone przez kanal 2 - capture STOP
 *      CCIFG_1_HND - przerwanie zgloszone przez kanal 1 - capture START
 *
 */


#pragma vector=TIMERA0_VECTOR   // TIMERA0_VECTOR - wektor do obslugi compare, kanal 0
__interrupt void timerA0_ISR(void) // timer 1kHz
{
    static uint16_t disp = 0; //ktory segment ma byc podswietlony
    static uint16_t led_timer = 0; //odswiezamy co drugie cykniecie
    static char disp_buffer[SEG_LED_NUMBER]; // Bufor wyswietlanych znakow

    if(if_counting)
    {
        count_time++;
        buffer[0]++;
        uint16_t n = 0;
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

    if( disp==0 ) // Przekopiowanie buforow
    {
        uint16_t n=0;
        while( n<SEG_LED_NUMBER )
        {
            disp_buffer[n] = buffer[n];
            n++;
        }
    }

    if(led_timer==0) // start sekwencji wyswietlania, 1kHz/2
    {
        led_timer = 1; // co drugie wejscie do obslugi przerwania od timera

        P3OUT = ~(1<<disp); //aktywujemy kolejny wyswietlacz
        P2OUT = ((disp_buffer[disp])&0x0F) | LED_RBI | LED_BI | LED_LT | LED_DP; // wyswietlenie cyfry oraz bity steruj�ce
        if(disp == SEG_LED_DOT_POSITION) P2OUT &= ~LED_DP; // zapalenie kropki na odpowiedniej pozycji
        disp++; // wybor kolejnego wyswietlacza
        if(disp >= SEG_LED_NUMBER) disp = 0;
    }
    else led_timer--;
}

#pragma vector=TIMERA1_VECTOR   // TIMERA1_VECTOR - wektor do obslugi capture, kanal 1, 2
__interrupt void timerA1_ISR(void)
{
    switch(TAIV) //odczyt TAIV
    {
        case 2: //flaga CCIFG
            break; //zrodlo TACCR1
        case 4: //flaga CCIFG
            break; //zrodlo TACCR2
        case 10: //flaga TAIFG
            break; //zrodlo TAR overflow
    }       









}



