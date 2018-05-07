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

*/

#include <msp430.h>

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

// gaszenie wszystkich diod]
#define OFF ~(SEG_A + SEG_B + SEG_C + SEG_D + SEG_E + SEG_F + SEG_G + SEG_DP)

int main(void)
{
    WDTCTCL = WDTPW + WDTHOLD; // wylaczenie watchdoga

    // inicjalizacja portu P1
    P1SEL |= (BIT0 + BIT7); // ustaw P1.0 i P1.7 jako GPIO
    P1DIR &= ~(BIT0 + BIT7); // ustaw jako wejscia
    P1REN |= (BIT0 + BIT7); // umozliwia pullup 
    P1OUT |= (BIT0 + BIT7); // pullup

    P1IES |= (BIT0 + BIT7); // zbocze opadajace
    P1IE = BIT0; //wlaczenie przerwan


    // inicjalizacja portu P2
    P2SEL |= 0x00; // ustaw cały port 2 jako GPIO
    P2DIR |= 0XFF; // ustaw port 2 jako wyjścia

    __enable_interrupt();

    while(1)
    {

    }

}

#pragma vector=PORT1_VECTOR
__interrupt void Port_1(void)
{
    
}


