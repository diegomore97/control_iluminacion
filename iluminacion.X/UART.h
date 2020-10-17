#ifndef UART_H
#define	UART_H

#define RX TRISC7
#define TX TRISC6

void uart_iniciar(T_LONG BAUD);
T_UBYTE uart_leer(void);
void uart_escribir(T_BYTE dato);
void uart_printf(T_BYTE* cadena);


void uart_iniciar(T_LONG BAUD) {
    
    T_LONG frecuenciaCristal = _XTAL_FREQ;
    TX = 0; //TX OUTPUT
    RX = 1; //RX INPUT

    //Baudios
    SPBRG = (frecuenciaCristal / 16 / BAUD) - 1;

    //Configuraci�n 
    TXSTAbits.BRGH = 1; //High Speed
    TXSTAbits.SYNC = 0; //Asincrono
    RCSTAbits.SPEN = 1; //Habilitar Tx y Rx

    //Transmisi�n
    TXSTAbits.TX9 = 0; //8 bits
    TXSTAbits.TXEN = 1; //Activamos transmisi�n

    //Recepci�n
    RCSTAbits.RC9 = 0; //8 bits
    RCSTAbits.CREN = 1; //Activamos recepci�n
}

T_UBYTE uart_leer(void) {
    while (!RCIF);
    RCIF = 0;
    return RCREG;
}

void uart_escribir(T_BYTE dato) {
    TXREG = dato;
    while (!TXSTAbits.TRMT);
}

void uart_printf(T_BYTE* cadena) {
    while (*cadena) {
        uart_escribir(*cadena++);
    }
}

#endif	