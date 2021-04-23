
/* 
 * File:   lab08_main.c
 * Author: brand
 *
 * Created on 19 de abril de 2021, 03:28 PM
 */

/*
 *Entrega Prelab: Botones puertos RB0 y RB1, leds contador botones puerto C
 * leds contador timer0 puerto D
 */


// PIC16F887 Configuration Bit Settings

// 'C' source line config statements

// CONFIG1
#pragma config FOSC = INTRC_NOCLKOUT//Oscillator Selection bits(INTOSC oscillator: 
                                  //CLKOUT function on RA6/OSC2/CLKOUT pin, 
                                  //I/O function on RA7/OSC1/CLKIN)
#pragma config WDTE = OFF // Watchdog Timer Enable bit (WDT disabled and 
                          //can be enabled by SWDTEN bit of the WDTCON register)
#pragma config PWRTE = ON       // Power-up Timer Enable bit (PWRT enabled)
#pragma config MCLRE = OFF      // RE3/MCLR pin function select bit (RE3/MCLR  
                                //pin function is digital input, MCLR internally 
                                //tied to VDD)
#pragma config CP = OFF         // Code Protection bit (Program memory code 
                                //protection is disabled)
#pragma config CPD = OFF        // Data Code Protection bit (Data memory code 
                                //protection is disabled)
#pragma config BOREN = OFF      // Brown Out Reset Selection bits (BOR disabled)
#pragma config IESO = OFF       // Internal External Switchover bit 
                                //Internal/External Switchover mode is disabled
#pragma config FCMEN = OFF      // Fail-Safe Clock Monitor Enabled bit 
                                //(Fail-Safe Clock Monitor is disabled)
#pragma config LVP = ON         //Low Voltage Programming Enable bit(RB3/PGM pin 
                                //has PGM function, low voltage programming 
                                //enabled)

// CONFIG2
#pragma config BOR4V = BOR40V   // Brown-out Reset Selection bit (Brown-out 
                                //Reset set to 4.0V)
#pragma config WRT = OFF        // Flash Program Memory Self Write Enable bits 
                                //(Write protection off)


// #pragma config statements should precede project file includes.
// Use project enums instead of #define for ON and OFF.

#include <xc.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#define _XTAL_FREQ 4000000
/*
 * Declaraciones y variables
 */

//tabla para el display 7 segmentos
char tabla[10]={0x3F,0x06,0x5B,0x4F,0x66,0x6D,0x7D,0x07,0x7F,0x67};
//uint8_t
//variables temporales en multiplexado
uint8_t var_temp;
uint8_t decenas_temp = 0;

//variables de conversion Hex to Dec
uint8_t unidades = 0;
uint8_t decenas = 0;
uint8_t centenas = 0;

//variables de valores decimales convertidos con la tabla
uint8_t unidades_display = 0;
uint8_t decenas_display = 0;
uint8_t centenas_display = 0;


// Prototipos
void setup();
void config_reloj();
void config_io();
void config_int_enable();
void config_timer0();
void TMR0_interrupt();
void contador();

/*
 *  main
 */

void main(void) {
    
    setup(); // llamar función de configuraciones
    ADCON0bits.GO = 1;
    
    while(1){  //loop principal
        
        
        //habilitar la conversión
        if(ADCON0bits.GO == 0){
            if(ADCON0bits.CHS==0){
                ADCON0bits.CHS = 1;
            }else{
                ADCON0bits.CHS = 0;
            }
            __delay_us(50);
            ADCON0bits.GO = 1;
            contador();
        }
        
    }
    
    return;
}

void setup(){
    
    config_reloj();// configuraciones principales del programa
    config_io();
    config_int_enable();
    config_timer0();
    
    return;
};


// funcion contador
void contador(void){
        
    centenas = var_temp / 100; // se divide dentro de la base 
    decenas_temp = var_temp % 100; // y luego se obtiene el modulo para ver 
    decenas = decenas_temp / 10; // el residuo y guardar el valor en la variable
    unidades = var_temp % 10;
    
    unidades_display = tabla[unidades];//se obtiene valores para los displays
    decenas_display = tabla[decenas];
    centenas_display = tabla[centenas];
}



// interrupciones

void __interrupt() isr(void){

    if (INTCONbits.T0IF){//se verifica que bandera de interrupcion se levanto
        TMR0_interrupt();
    }
    if (PIR1bits.ADIF){//se verifica que bandera de interrupcion se levanto
        
        if(ADCON0bits.CHS==0){
            PORTC = ADRESH;  
        }else{
           var_temp = ADRESH;
        }
        PIR1bits.ADIF = 0;
    }
         
}


void TMR0_interrupt(){
    
    PORTE = 0x00; // se limpia el puerto de multiplexado
    if(PORTB == 0){
        PORTD = unidades_display;
        PORTEbits.RE0 = 1;
        PORTB++; // se utiliza port D como la bandera de displays a multiplexar
    }else if(PORTB == 1){
        PORTD = decenas_display;
        PORTEbits.RE1 = 1;
        PORTB++;        
    }else {
        PORTD = centenas_display;
        PORTEbits.RE2 = 1;
        PORTB = PORTB + 2;
    }
    
    TMR0 = 235; // valor de n para t=5ms
    INTCONbits.T0IF = 0;
    
    return;
}

// configuraciones

void config_reloj(){

    OSCCONbits.IRCF2 =1 ; // IRCF = 110 (4Mz) 
    OSCCONbits.IRCF1 =1 ;
    OSCCONbits.IRCF0 =0 ;
    OSCCONbits.SCS = 1; // Habilitar reloj interno
    
    return;
}

void config_io(){
   
    ANSEL = 0x03;
    ANSELH = 0x00; 
    
    TRISA = 0x03;
    TRISC = 0x00; // puertos c y d como salidas
    TRISD = 0x00;
    TRISE = 0x00;
    TRISB = 0xf8;

    PORTB = 0x00;
    PORTA = 0x00;
    PORTC = 0x00;
    PORTD = 0x00;
    PORTE = 0x01;
    
   // configuracion del adc
    ADCON1bits.ADFM = 0; // justificacion a la izquierda
    ADCON1bits.VCFG0 = 0; //configuracion vdd y ground en vss
    ADCON1bits.VCFG1 = 0; 
    
    ADCON0bits.ADCS = 0b10; // conversion del clock a fosc/32
    ADCON0bits.CHS = 0;
    __delay_us(50);
    ADCON0bits.ADON = 1;
      
    
    return;
}

void config_int_enable(){
    
    INTCONbits.GIE = 1; // Se habilitan las interrupciones globales
    INTCONbits.PEIE = 1;
    
    PIE1bits.ADIE = 1;
    PIR1bits.ADIF = 0;
    
    INTCONbits.T0IE = 1; // Se habilitan la interrupción del TMR0
    INTCONbits.T0IF = 0; // Se limpia la bandera
    
    return;
}
    
void config_timer0(){

    OPTION_REGbits.T0CS = 0;
    OPTION_REGbits.PSA = 0;
    OPTION_REGbits.PS2 = 1; // PS 111 = 256
    OPTION_REGbits.PS1 = 1;
    OPTION_REGbits.PS0 = 1;
    
    TMR0 = 235; // valor de n para t=5ms
    INTCONbits.T0IF = 0;
            
    return;
}




