#include <stdio.h>
#include <stdint.h>
#include "config.h"
#include "variables.h"
#include "UART.h"
#include "I2C.h"
#include "bh1750.h"
#include "pwm.h"
#include "control.h"
#include "RTC.h"
#include "AUTOMATIZADOR.h"

void __interrupt() desbordamiento(void) {

    if (INTCONbits.TMR0IF) {

        if (esperandoDatos) {

            tiempoInactividadTrans++;

            if (tiempoInactividadTrans == MAX_TIEMPO_INACTIVIDAD)
                esperaDatoConcluida = 1; //Ya pasaron 10 segundos de espera


        }

        INTCONbits.TMR0IF = 0; //Regresando Bandera a 0 (Interrupcion por Timer 0)
        TMR0 = VALOR_TIMER0; //Overflow cada 10 Segundos
        overflowTimer = 1;

    }

    if (PIR1bits.RCIF) { // interrupción USART PIC por recepción

        byteUart = RCREG;
        datoRecibido = 1;

    }

}

void main(void) {

    UART_init(9600);
    i2c_iniciar();
    bh1750_iniciar();
    configPwm(CANAL_1);
    inicializarObjetos();
    //configBluetoothHC_06(); //Configurar el modulo Bluetooth | comentar una vez configurado

    INTCONbits.GIE = 1; //GLOBALS INTERRUPTIONS ENABLED
    INTCONbits.PEIE = 1; // PERIPHERAL INTERRUPTIONS ENABLED
    INTCONbits.TMR0IE = 1; //INTERRUPTION BY TIMER 0 ON

    PIE1bits.RCIE = 1; //habilita interrupción por recepción USART PIC.

    T0CON = 0b00000111; //Timer 0 apagado, 16 bits , Temporizador, Prescaler 1:256

    TMR0 = VALOR_TIMER0; //Overflow cada 10 Segundos

    INTCONbits.TMR0IF = 1; //Inicializando Bandera del TIMER0


    T0CONbits.TMR0ON = 1; //Iniciar Timer 0

    //mostrarMenu(); //comentar

    UART_printf("\r\nSistema Iniciado\n\r");

    while (1) {

        if (datoRecibido) {
            
            datoRecibido = 0; //Bajando bandera
            byteUart -= 48; //Convirtiendo ASCII A ENTERO
            sistemaPrincipal(byteUart);

        }

        if (overflowTimer) {

            esperandoDatos = 0;
            overflowTimer = 0; //Bajando bandera
            sistemaIluminado();
        }

    }
    return;
}
