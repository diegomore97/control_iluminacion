#ifndef BH1750_H
#define	BH1750_H

#define  BH1750_address_w     0x46                               
#define  BH1750_address_r     0x47
         
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
                       

void BH1750_init(void);
void BH1750_write(T_UBYTE cmd);                                           
T_ULONG BH1750_read_word();
T_ULONG get_lux_value(T_UBYTE mode); 

void BH1750_init(void)
{
   __delay_ms(100); 
   BH1750_write(power_down);
}                 
             

void BH1750_write(T_UBYTE cmd)
{
   i2c_inicia_com();
   i2c_envia_dato(BH1750_address_w);       
   i2c_envia_dato(cmd);               
   i2c_detener();
}


T_ULONG BH1750_read_word()
{                     
   T_UBYTE lb = 0;
   T_UBYTE hb = 0;
   
   i2c_inicia_com();
   i2c_envia_dato(BH1750_address_r);   
   hb = i2c_recibe_datoACK(1); 
   lb = i2c_recibe_datoACK(0);
   i2c_detener();                           
   
   return (hb << 8) + lb;
}


T_ULONG get_lux_value(T_UBYTE mode)
{
   register T_ULONG lux_value = 0; 
   BH1750_write(power_up);
   BH1750_write(mode);
   lux_value = BH1750_read_word();
   __delay_ms(RETARDO_SENSOR);
   BH1750_write(power_down);
   lux_value /= 1.2;
   return lux_value;                                 
}                  



#endif	

