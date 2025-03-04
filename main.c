/*
 * Name: Nathan Culler
 * Date: 8/4/2023
 * Assignment: Lab 5
 * YouTube: https://youtu.be/4unpweh5rqg
 *
 * This Program: program a game controller using buttons and a potentiometer
 *
 */

#include <msp430.h>
#include <stdint.h>
#include <stdbool.h>

void init();
void initClocks();
void initUART();
void initADC();
volatile uint8_t readADC();
bool ButtonOnePressed();
bool ButtonTwoPressed();
void TXUART(uint8_t test);

int main(void)
{
    volatile uint8_t numADC;
    init();
    initClocks();
    initUART();
    initADC();

    __enable_interrupt();
    while(1){
        numADC = readADC();
        TXUART(numADC);
    }

    return 0;
}

void init(){
    WDTCTL = WDTPW | WDTHOLD;                // Stop watchdog timer
    PM5CTL0 &= ~LOCKLPM5;

    P2DIR &= ~BIT3;
    P2REN |= BIT3;
    P2OUT |= BIT3;
    P4DIR &= ~BIT1;
    P4REN |= BIT1;
    P4OUT |= BIT1;

}

void initClocks(){
    // Set XT1CLK as ACLK source
    CSCTL4 |= SELA__XT1CLK;
    // Use external clock in low-frequency mode
    CSCTL6 |= XT1BYPASS_1 | XTS_0;
}

void initUART(){ //9600 baud, 8 data bits, no parity, 1 stop bit, eUSCI_A1
    // Configure UART pins
    P4SEL0 |= BIT3 | BIT2;                    // set 2-UART pin as second function
    //P4SEL1 &= ~(BIT2 & BIT3);
    P4SEL1 &= ~(BIT2 | BIT3);

    // Configure UART
    UCA1CTLW0 = UCSWRST;                     // Hold UART in reset state

    UCA1CTLW0 |= UCSSEL__ACLK;               // CLK = ACLK
    // Baud Rate calculation
    // 32768/(9600) = 3.4133
    // Fractional portion = 0.4133
    // User's Guide Table 17-4: 0x92
    UCA1BR0 = 3;                             // 32768/9600
    UCA1BR1 = 0;
    UCA1MCTLW |= 0x9200;    //0x9200 is UCBRSx = 0x92

    UCA1CTLW0 &= ~UCSWRST;                    // Release reset state for operation
    UCA1IE |= UCRXIE;                         // Enable USCI_A0 RX interrupt
}

void initADC(){
    P1SEL0 |= BIT1;
    P1SEL1 |= BIT1;

    ADCMCTL0 |= ADCINCH_1 | ADCSREF_0; //3.3 +voltage, 0 -voltage
    ADCCTL0 = ADCSHT_2 | ADCON;
    ADCCTL1 |= ADCSHP | ADCSSEL_1; //use ACLK for ADC
    ADCCTL2 = ADCRES_0; //8 bit resolution

}

volatile uint8_t readADC(){
    volatile uint8_t ADC_Result;

    ADCCTL0 |= ADCENC | ADCSC;

    while(ADCCTL1 & ADCBUSY);
    ADC_Result = ADCMEM0;

    if(ADC_Result == 255){
        ADC_Result = 254;
    }
    if(ButtonOnePressed() || ButtonTwoPressed()){
        ADC_Result = 255;
    }


    return ADC_Result;

}

bool ButtonOnePressed(){
    if((P4IN & BIT1) == 0X00){
        return true;
    }
    else {
        return false;
    }
}

bool ButtonTwoPressed(){
    if((P2IN & BIT3) == 0X00){
        return true;
    }
    else {
        return false;
    }
}

void TXUART(uint8_t test){

    while(!(UCA1IFG & UCTXIFG));
    UCA1TXBUF = test;
}

#pragma vector=USCI_A1_VECTOR
__interrupt void USCI_A1_ISR(void)
{
  __no_operation();

  // Clear flag
  UCA1IFG = 0;
}
