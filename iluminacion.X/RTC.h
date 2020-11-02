
void escribe_rtc(T_UBYTE direccion, T_UBYTE dato) {
    i2c_inicia_com();
    i2c_envia_dato(0xD0); // Codigo del RTC
    i2c_envia_dato(direccion);
    i2c_envia_dato(dato);
    i2c_detener();
}

/*
 * Se encarga de leer una posicion del RTC
 */
T_UBYTE leer_rtc(T_UBYTE direccion) {
    T_UBYTE dato;
    i2c_inicia_com();
    i2c_envia_dato(0xD0); // Codigo del RTC
    i2c_envia_dato(direccion);
    i2c_reinicia_com();
    i2c_envia_dato(0xD1);
    dato = i2c_recibe_dato();
    i2c_detener();
    return dato;
}

/*
 * Toma un dato del RTC y lo devuleve en decimal
 */
T_UBYTE convertirDato(T_UBYTE dato) {
    T_UBYTE datoR = 0;
    datoR = (dato & 0xF0) >> 4;
    datoR = (datoR * 10) + (dato & 0x0F);
    return datoR;
}
