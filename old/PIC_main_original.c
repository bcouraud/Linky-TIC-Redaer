/*
 * File:   main.c
 * Author: Benoit Couraud
 *
 * Created on February 7, 2023, 3:28 PM
 */


#include <xc.h>


// CONFIG1
#pragma config FOSC = INTOSC    // Oscillator Selection (INTOSC oscillator: I/O function on CLKIN pin)
#pragma config WDTE = ON       // Watchdog Timer Enable (WDT disabled)
#pragma config PWRTE = OFF      // Power-up Timer Enable (PWRT disabled)
#pragma config MCLRE = ON       // MCLR Pin Function Select (MCLR/VPP pin function is MCLR)
#pragma config CP = OFF         // Flash Program Memory Code Protection (Program memory code protection is disabled)
#pragma config CPD = OFF        // Data Memory Code Protection (Data memory code protection is disabled)
#pragma config BOREN = ON       // Brown-out Reset Enable (Brown-out Reset enabled)
#pragma config CLKOUTEN = OFF   // Clock Out Enable (CLKOUT function is disabled. I/O or oscillator function on the CLKOUT pin)
#pragma config IESO = ON        // Internal/External Switchover (Internal/External Switchover mode is enabled)
#pragma config FCMEN = ON       // Fail-Safe Clock Monitor 

// CONFIG2
#pragma config WRT = OFF        // Flash Memory Self-Write Protection (Write protection off)
#pragma config PLLEN = ON       // PLL Enable (4x PLL enabled)
#pragma config STVREN = ON      // Stack Overflow/Underflow Reset Enable (Stack Overflow or Underflow will cause a Reset)
#pragma config BORV = LO        // Brown-out Reset Voltage Selection (Brown-out Reset Voltage (Vbor), low trip point selected.)
#pragma config LVP = OFF        // Low-Voltage Programming Enable (High-voltage on MCLR/VPP must be used for programming)

#define _XTAL_FREQ 1000000     // Clock a 1MHz

#include <xc.h>
#include <math.h>

unsigned char threshold1 = 180;
unsigned char threshold2 = 187;
char thresholdCounter = 20;//5; //20;
char thresholdCounterAcquisition = 220;//10; //220;
char counterAcquisition = 1;

unsigned char adc_char;

unsigned char ADC_Lecture()
{
  ADCON0bits.ADON = 1;// &= 0b00000001; //Clearing the Channel Selection Bits
  //ADCON0 |= channel<<2; //Setting the required Bits
  __delay_ms(6);
  ADCON0bits.GO = 1; //Initializes A/D Conversion
  while(ADCON0bits.GO_nDONE); //Wait for A/D Conversion to complete
  //unsigned int output = (ADRESH<<8)+ADRESL;
  return (ADRESH); //((ADRESH<<8)+ADRESL); //Returns Result
}

void variable_delay(int attente)
{
        int i;
        for (i = 0; i < attente; i++)
        {
            __delay_ms(10);
        } 
}

char counter;
char uart_data;
char acquisition = 0;
void main(void) {
    counter = 1;
         //Configure Watchdog to approx 1 sec
    CLRWDT();
    OPTION_REGbits.PSA = 1; //prescaler is for wdt   
    OPTION_REGbits.PS = 0b111; //prescaler 1:256
    INTCON = 0; //disable every interrupt
    PIE1=0;
    PIR1=0;
    INTCONbits.GIE = 0;
    
    
    
 // COnfiguration de l'oscillateur
        OSCCON = 0b01011010;  //configuration clock 1MHz
    OSCCONbits.SCS1 = 1;        // Selection de l'oscillateur interne (HF)
   // OSCCONbits.IRCF = 0b1011;   // Oscillateur interne à 1MHz
    // Configuration des ports entrée-sortie
    TRISA = 0b00000001;          // PIN RA0 configuré en input, les autres en output
    LATA = 0x00;                // Tous les PIN en état bas
    ANSELAbits.ANSA0=1;
    ADCON1bits.ADCS=000;
    ADCON1bits.ADPREF =00;
    ADCON0bits.CHS =0000;
    ADCON1bits.ADFM=0;
 //#########################    mainloop   #####################################
  
    
    
    
    
    
    
    
    
    
// INIT UART
    APFCONbits.RXDTSEL = 1; //Select RX on RA5 (instead of RA0)
    APFCONbits.TXCKSEL = 1; //Select TX on RA5 (instead of RA1)
    APFCONbits.P1BSEL = 0 ;
    TRISAbits.TRISA4 = 0;   // RA4 = TX= output
    TRISAbits.TRISA5 = 1;   // RA5 = RX = INPUT
    //Configuration Data Rate
    BAUDCONbits.BRG16 = 1;//0;
    SPBRG = 25;   // = Fosc/(16*DataRate)-1;  Fosc = 1MHz et DataRate = 9600
    //SPBRGH=25>>8;
    //SPBRGL = 25&0xFF;
    TXSTA = 0b00110110; // Asynchron TX config, sur 8bits,
    TXSTAbits.BRGH = 1;
    TXSTAbits.SYNC = 0;
    RCSTA = 0b10010000; // Asynchron RX config, sur 8 bits
    ANSELA = 0x00;      // pas d'analog

    __delay_ms(100);

    
      do{
        LATAbits.LATA1 =0;
        SLEEP();
        NOP();
        
        if (acquisition==1)
        {
           // TXREG = 111;
            //while(!TXSTAbits.TRMT);
            counterAcquisition = counterAcquisition+1;
           // TXREG = counterAcquisition ;
           // while(!TXSTAbits.TRMT);
        CLRWDT();
             
            if (counterAcquisition>thresholdCounterAcquisition)
             {
                 counterAcquisition = 1;
                 LATAbits.LATA1 =1;
                 adc_char = ADC_Lecture();
                 LATAbits.LATA1 =0;
                 if (adc_char>threshold2){
                     acquisition=0;
                 }
             }
        }else // start of the microcontroller
        {
        CLRWDT();
            // we start a lecture
                LATAbits.LATA1 =1; 
               adc_char = ADC_Lecture();
        //       TXREG = 222;
        //          while(!TXSTAbits.TRMT);
                  
        //       TXREG = adc_char;
        //          while(!TXSTAbits.TRMT); // we send the result through UART
                       LATAbits.LATA1 =0;
               if (adc_char>threshold1) // if the voltage measured at the diode is above threshold 1: we are 
                   // at a stage where the voltage of the battery is below 3.5V, so we do not start to think to switch on
               {
                   counter = counter - 1;
                   if (counter<1)
                   {
                       counter  = 1;
                   }
               }else   // if the voltage is below the threshold, this means the voltage at the diode is
                   // kept at 3.2V, whereas the supply voltage is above (4.5V)
               {
                   counter = counter +1;
                   if (counter>thresholdCounter) // we count the number of time the supply voltage is large enough
                   {
                       counter  = thresholdCounter;
                   }
                 // SLEEP();
               }

        CLRWDT();
                       
               if (counter == thresholdCounter) // when there has been a long enough time that the supply voltage is above
                   // 4.5V:
               {
                   acquisition = 1; // we say that the "acquisition" starts: acquisition = we count a given
                   // number of cycles
               //  TXREG = 11;
               //   while(!TXSTAbits.TRMT); 
                //   TXREG = adc_char;
                 // while(!TXSTAbits.TRMT);
                  LATAbits.LATA2=1; 
               }else
               {
                // TXREG = counter;
                //  while(!TXSTAbits.TRMT);
                  LATAbits.LATA2=0;
               }
        }                       
}while(1);
    
   
   /*   while(1) {
        while(!PIR1bits.RCIF) ;
        if(RCSTAbits.FERR || RCSTAbits.OERR)
        {
            RCSTAbits.SPEN=0;
            __delay_us(5);
            RCSTAbits.SPEN = 1;
          }
        else
        {
            uart_data = RCREG;
            TXREG = uart_data;
            while(!TXSTAbits.TRMT);
        }
        uart_data = ADC_Lecture();
            TXREG = uart_data;
            while(!TXSTAbits.TRMT);
            
__delay_ms(1000);
    }

    return; */
    
  /*     do{
           SLEEP();
        NOP();
        
       adc_char = ADC_Lecture();
       if (adc_char>threshold)
       {
           LATAbits.LATA5=1;
       }else
       {
         // SLEEP();
            TXREG = adc_char;
            while(!TXSTAbits.TRMT);
          LATAbits.LATA5=0;
       }
}while(1);*/

}
               
               
            /*   
               
        //Changer l'état de la PIN RA5
        LATAbits.LATA5 = ~LATAbits.LATA5;
        uart_data = 252;
            TXREG = uart_data;
            while(!TXSTAbits.TRMT);
        __delay_ms(1000);
        CLRWDT();              // Clear WDT
        // Delai de 500ms
                  // LATAbits.LATA5=1;
       } while(1);
    
       
       
       

    while(1) {
        while(!PIR1bits.RCIF) ;
        if(RCSTAbits.FERR || RCSTAbits.OERR)
        {
            RCSTAbits.SPEN=0;
            __delay_us(5);
            RCSTAbits.SPEN = 1;
          }
        else
        {
            uart_data = RCREG;
            TXREG = uart_data;
            while(!TXSTAbits.TRMT);
        }
        uart_data = 252;
            TXREG = uart_data;
            while(!TXSTAbits.TRMT);
__delay_ms(1000);
    }

    return;*/





























/*


void main(void) {
    
       //Configure Watchdog to approx 1 sec
    CLRWDT();
    OPTION_REGbits.PSA = 1; //prescaler is for wdt   
    OPTION_REGbits.PS = 0b111; //prescaler 1:256
    INTCON = 0; //disable every interrupt
    PIE1=0;
    PIR1=0;
    INTCONbits.GIE = 0;
    
    
    
 // COnfiguration de l'oscillateur
    OSCCONbits.SCS1 = 1;        // Selection de l'oscillateur interne (HF)
    OSCCONbits.IRCF = 0b1011;   // Oscillateur interne à 1MHz
    // Configuration des ports entrée-sortie
    TRISA = 0b00000011;          // PIN RA2 configuré en output, les autres en entrée
    LATA = 0x00;                // Tous les PIN en état bas
    ANSELAbits.ANSA0=1;
    ADCON1bits.ADCS=000;
    ADCON1bits.ADPREF =00;
    ADCON0bits.CHS =0000;
    ADCON1bits.ADFM=0;
 //#########################    mainloop   #####################################
  
  
    do{
        SLEEP();
        NOP();
        //Changer l'état de la PIN RA5
        LATAbits.LATA5 = ~LATAbits.LATA5;

        CLRWDT();              // Clear WDT
        // Delai de 500ms
                  // LATAbits.LATA5=1;
       } while(1);
    
/*    do{

        //Changer l'état de la PIN RA2
       // LATAbits.LATA2 = ~LATAbits.LATA2;
        // Delai de 500ms
                  // LATAbits.LATA5=1;

      //  variable_delay(valeur_adc);
       adc_char = ADC_Lecture();
       if (adc_char>threshold)
       {
           LATAbits.LATA5=1;
       }else
       {
         // SLEEP();
          LATAbits.LATA5=0;
       }

       } while(1);
}


*/

/*
// #pragma config statements should precede project file includes.
// Use project enums instead of #define for ON and OFF.

// CONFIG1
#pragma config FOSC = INTOSC    // Oscillator Selection (INTOSC oscillator: I/O function on CLKIN pin)
#pragma config WDTE = OFF       // Watchdog Timer Enable (WDT disabled)
#pragma config PWRTE = OFF      // Power-up Timer Enable (PWRT disabled)
#pragma config MCLRE = OFF      // MCLR Pin Function Select (MCLR/VPP pin function is digital input)
#pragma config CP = OFF         // Flash Program Memory Code Protection (Program memory code protection is disabled)
#pragma config CPD = OFF        // Data Memory Code Protection (Data memory code protection is disabled)
#pragma config BOREN = ON       // Brown-out Reset Enable (Brown-out Reset enabled)
#pragma config CLKOUTEN = OFF   // Clock Out Enable (CLKOUT function is disabled. I/O or oscillator function on the CLKOUT pin)
#pragma config IESO = ON        // Internal/External Switchover (Internal/External Switchover mode is enabled)
#pragma config FCMEN = ON       // Fail-Safe Clock Monitor Enable (Fail-Safe Clock Monitor is enabled)

// CONFIG2
#pragma config WRT = OFF        // Flash Memory Self-Write Protection (Write protection off)
#pragma config PLLEN = ON       // PLL Enable (4x PLL enabled)
#pragma config STVREN = ON      // Stack Overflow/Underflow Reset Enable (Stack Overflow or Underflow will cause a Reset)
#pragma config BORV = LO        // Brown-out Reset Voltage Selection (Brown-out Reset Voltage (Vbor), low trip point selected.)
#pragma config LVP = ON         // Low-Voltage Programming Enable (Low-voltage programming enabled)

#define _XTAL_FREQ 4000000   //frequence à 4MHz

char uart_data;

unsigned char ADC_Lecture()
{
  ADCON0bits.ADON = 1;// &= 0b00000001; //Clearing the Channel Selection Bits
  //ADCON0 |= channel<<2; //Setting the required Bits
  __delay_ms(6);
  ADCON0bits.GO = 1; //Initializes A/D Conversion
  while(ADCON0bits.GO_nDONE); //Wait for A/D Conversion to complete
  //unsigned int output = (ADRESH<<8)+ADRESL;
  return (ADRESH); //((ADRESH<<8)+ADRESL); //Returns Result
}


void main(void) {
uart_data = 0x57;
    OSCCON = 0b01101010;  //configuration clock 4MHz
// INIT UART
    APFCONbits.RXDTSEL = 1; //Select RX on RA5 (instead of RA0)
    APFCONbits.TXCKSEL = 1; //Select TX on RA5 (instead of RA1)
    APFCONbits.P1BSEL = 0 ;
    TRISAbits.TRISA4 = 0;   // RA4 = TX= output
    TRISAbits.TRISA5 = 1;   // RA5 = RX = INPUT
    //Configuration Data Rate
    BAUDCONbits.BRG16 = 0;
    SPBRG = 25;   // = Fosc/(16*DataRate)-1;  Fosc = 4MHz et DataRate = 9600
    //SPBRGH=25>>8;
    //SPBRGL = 25&0xFF;
    TXSTA = 0b00100110; // Asynchron TX config, sur 8bits,
    RCSTA = 0b10010000; // Asynchron RX config, sur 8 bits
    ANSELA = 0x00;      // pas d'analog
    TRISAbits.TRISA0=1;
    LATA = 0x00;                // Tous les PIN en état bas
    ANSELAbits.ANSA0=1;
    ADCON1bits.ADCS=000;
    ADCON1bits.ADPREF =00;
    ADCON0bits.CHS =0000;
    ADCON1bits.ADFM=0;
   // valeur_adc = 100;

    __delay_ms(100);


    while(1) {
       // while(!PIR1bits.RCIF) ;
        
        __delay_ms(1000);
               uart_data = ADC_Lecture();

        if(RCSTAbits.FERR || RCSTAbits.OERR)
        {
            RCSTAbits.SPEN=0;
            __delay_us(5);
            RCSTAbits.SPEN = 1;
          }
        else
        {
            //uart_data = RCREG;
            TXREG = 0x57; //19;//uart_data;
            while(!TXSTAbits.TRMT);
        }

    }

    return;

}




/* // CONFIG1
#pragma config FOSC = INTOSC    // Oscillator Selection (INTOSC oscillator: I/O function on CLKIN pin)
#pragma config WDTE = OFF       // Watchdog Timer Enable (WDT disabled)
#pragma config PWRTE = OFF      // Power-up Timer Enable (PWRT disabled)
#pragma config MCLRE = ON       // MCLR Pin Function Select (MCLR/VPP pin function is MCLR)
#pragma config CP = OFF         // Flash Program Memory Code Protection (Program memory code protection is disabled)
#pragma config CPD = OFF        // Data Memory Code Protection (Data memory code protection is disabled)
#pragma config BOREN = ON       // Brown-out Reset Enable (Brown-out Reset enabled)
#pragma config CLKOUTEN = OFF   // Clock Out Enable (CLKOUT function is disabled. I/O or oscillator function on the CLKOUT pin)
#pragma config IESO = ON        // Internal/External Switchover (Internal/External Switchover mode is enabled)
#pragma config FCMEN = ON       // Fail-Safe Clock Monitor 

// CONFIG2
#pragma config WRT = OFF        // Flash Memory Self-Write Protection (Write protection off)
#pragma config PLLEN = ON       // PLL Enable (4x PLL enabled)
#pragma config STVREN = ON      // Stack Overflow/Underflow Reset Enable (Stack Overflow or Underflow will cause a Reset)
#pragma config BORV = LO        // Brown-out Reset Voltage Selection (Brown-out Reset Voltage (Vbor), low trip point selected.)
#pragma config LVP = OFF        // Low-Voltage Programming Enable (High-voltage on MCLR/VPP must be used for programming)

#define _XTAL_FREQ 1000000     // Clock a 1MHz

#include <xc.h>
char uart_rd ;
char uart_data;


unsigned char LastValue = 0;
unsigned int valeur_adc;
unsigned char adc_char;
char uart_data;

unsigned char ADC_Lecture()
{
  ADCON0bits.ADON = 1;// &= 0b00000001; //Clearing the Channel Selection Bits
  //ADCON0 |= channel<<2; //Setting the required Bits
  __delay_ms(6);
  ADCON0bits.GO = 1; //Initializes A/D Conversion
  while(ADCON0bits.GO_nDONE); //Wait for A/D Conversion to complete
  //unsigned int output = (ADRESH<<8)+ADRESL;
  return (ADRESH); //((ADRESH<<8)+ADRESL); //Returns Result
}

void variable_delay(int attente)
{
        int i;
        for (i = 0; i < attente; i++)
        {
            __delay_ms(10);
        } 
}


void main(void) {
 // COnfiguration de l'oscillateur
    OSCCONbits.SCS1 = 1;        // Selection de l'oscillateur interne (HF)
    OSCCONbits.IRCF = 0b1011;   // Oscillateur interne à 1MHz
    // Configuration des ports entrée-sortie
    TRISA = 0b00111011;          // PIN RA2 configuré en output, les autres en entrée
    LATA = 0x00;                // Tous les PIN en état bas
    
    // INIT UART
    APFCONbits.RXDTSEL = 1; //Select RX on RA5 (instead of RA0)
    APFCONbits.TXCKSEL = 1; //Select TX on RA5 (instead of RA1)
    APFCONbits.P1BSEL = 0 ;
    TRISAbits.TRISA4 = 0;   // RA4 = TX= output
    TRISAbits.TRISA5 = 1;   // RA5 = RX = INPUT
    //Configuration Data Rate
    BAUDCONbits.BRG16 = 0;
    SPBRG = 5;   // = Fosc/(8*DataRate)-1;  Fosc = 1MHz et DataRate = 9600
    //SPBRGH=25>>8;
    //SPBRGL = 25&0xFF;
    TXSTA = 0b00100110; // Asynchron TX config, sur 8bits,
    RCSTA = 0b10010000; // Asynchron RX config, sur 8 bits
    ANSELA = 0x00;      // pas d'analog

    __delay_ms(100);

    
    
    
    
    ANSELAbits.ANSA0=1;
    ADCON1bits.ADCS=000;
    ADCON1bits.ADPREF =00;
    ADCON0bits.CHS =0000;
    ADCON1bits.ADFM=0;
    valeur_adc = 100;
 //#########################    mainloop   #####################################
    
    do{
        //Changer l'état de la PIN RA2
      //  LATAbits.LATA2 = ~LATAbits.LATA2;
        // Delai de 500ms
        //variable_delay(valeur_adc);
       adc_char = ADC_Lecture();
       
      // while(!PIR1bits.RCIF) ;
  /*      if(RCSTAbits.FERR || RCSTAbits.OERR)
        {
            RCSTAbits.SPEN=0;
            __delay_us(5);
            RCSTAbits.SPEN = 1;
          }
        else
        {
            uart_data = RCREG;
       RCSTAbits.SPEN=1;
       TXSTAbits.SYNC = 0;
       TXSTAbits.TXEN = 1;
            TXREG = 65;
            while(!TXSTAbits.TRMT);
            __delay_ms(1000);
       // }

       
       
       } while(1);
}

 * 
 * */















