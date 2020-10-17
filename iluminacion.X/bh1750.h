#ifndef BH1750_H
#define	BH1750_H

//Direeciones de I2C
#define  BH1750_address_w     0x46                               
#define  BH1750_address_r     0x47

//Comandos del sensor
#define  power_down           0x00
#define  power_up             0x01
#define  reset                0x07
#define  cont_H_res_mode1     0x10
#define  cont_H_res_mode2     0x11 
#define  cont_L_res_mode      0x13                                         
#define  one_time_H_res_mode1 0x20
#define  one_time_H_res_mode2 0x21
#define  one_time_L_res_mode  0x23   
#define  RETARDO_SENSOR       180
#define  RETARDO_INICIO       100
                     
void bh1750_iniciar(void);
void bh1750_escribir(T_UBYTE comando);
T_ULONG bh1750_leer();
T_ULONG dameValorLux(T_UBYTE modo);


void bh1750_iniciar(void) {
    __delay_ms(RETARDO_INICIO);
    bh1750_escribir(power_down);
}

void bh1750_escribir(T_UBYTE comando) {
    i2c_inicia_com();
    i2c_envia_dato(BH1750_address_w);
    i2c_envia_dato(comando);
    i2c_detener();
}

T_ULONG bh1750_leer() {
    register T_ULONG valor = 0;
    T_UBYTE lb = 0;
    T_UBYTE hb = 0;

    i2c_inicia_com();
    i2c_envia_dato(BH1750_address_r);
    hb = i2c_recibe_datoACK(1);
    lb = i2c_recibe_datoACK(0);
    i2c_detener();

    valor = (hb <<8 ) + lb;
    valor /= 1.2;

    return valor;
}

T_ULONG dameValorLux(T_UBYTE modo) {
    register T_ULONG valorLux = 0;
    bh1750_escribir(power_up);
    bh1750_escribir(modo);
    valorLux = bh1750_leer();
    __delay_ms(RETARDO_SENSOR);
    bh1750_escribir(power_down);

    return valorLux;
}



#endif	

