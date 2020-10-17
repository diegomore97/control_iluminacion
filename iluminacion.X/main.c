#include <stdio.h>
#include "config.h"
#include "variables.h"
#include "UART.h"
#include "I2C.h"
#include "bh1750.h"

#define TAMANO_CADENA 30

void main(void) {
    
    T_ULONG luzMedida = 0;
    T_BYTE buffer[TAMANO_CADENA];
    
    uart_iniciar(9600);
    i2c_iniciar();
    bh1750_iniciar();
    uart_printf("\r\nSistema Iniciado\n\r");
    
    while(1)
    {
        luzMedida = dameValorLux(cont_H_res_mode1);
        sprintf(buffer, "\rLuz medida: %d luxs\n\r",luzMedida);
        uart_printf(buffer);
        __delay_ms(500);
        
    }
    
    return;
}
