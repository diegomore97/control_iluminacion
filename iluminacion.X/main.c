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
    
    UART_init(9600);
    UART_printf("\r\nSistema Iniciado\n\r");
    i2c_iniciar();
    BH1750_init();
    
    while(1)
    {
        luzMedida = get_lux_value(cont_H_res_mode1);
        sprintf(buffer, "\rLuz medida: %d luxs\n\r",luzMedida);
        UART_printf(buffer);
        __delay_ms(500);
        
    }
    
    return;
}
