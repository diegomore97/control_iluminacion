#ifndef XC_AUTOMATIZADOR_H
#define	XC_AUTOMATIZADOR_H

#define HORAS_DIA 24
#define DIAS_SEMANA 7
#define MINUTOS_HORA 60

#define REPETICIONES 30  //REPETICIONES PARA QUE TRANSCURRA 1 MINUTO
#define MAX_TIEMPO_INACTIVIDAD 1 //Decenas de segundo de espera para que el 
//usuario setie datos en el sistema a traves del protocolo UART
#define TAMANO_CADENA 50 
#define TAMANO_CADENA_HORARIO 30
#define CARACTER_VALIDADOR 'T'

//INSTRUCCIONES DE CONTROL
#define SETEO_DENEGADO '@' //Variable que se mandara por UART a otro Micro
#define SETEO_EXITOSO 'E' //Variable que se mandara por UART a otro Micro
#define SOLICITUD_DATOS_SENSORES 'R' //Variable que se mandara por UART a otro Micro
#define ENVIANDO_DATOS_SENSORES 'O' //El otro micro ya nos confirmo que mandara los datos
#define INTERRUMPIR_COMANDOS '&' //Se utiliza esta constante para la opcion de leer sensores

#define DEMO 1 // 1 = Modo Demo | 0 Modo Real (MINUTOS * 60) = Sistema a base de horas

typedef struct {
    T_UBYTE hora; //0 - 23
    T_BYTE dias[DIAS_SEMANA + 1]; //Dias para regar
    T_UBYTE iluminar; // Boolean
    T_UBYTE iluminado; //Boolean para saber si ya se rego en ese horario
    T_UBYTE tiempoIluminar; //Horas que se regaran en esa hora
} Horario;

Horario horarios[HORAS_DIA]; //24 HORAS DEL DIA
T_UBYTE hora = 0, minutos = 0, segundos = 0;
T_UBYTE diaActual = 0;
T_UBYTE datoRecibido = 0;
T_UBYTE overflowTimer = 0;
T_UBYTE tempHora = 0;
T_UBYTE flagIluminado;
T_BYTE buffer[TAMANO_CADENA];
T_BYTE buffer2[TAMANO_CADENA];

T_LONG VALOR_TIMER0 = 57723;
T_INT contInterrupciones = 0;
T_UWORD minutosIluminar = 0;
T_UWORD minutosTranscurridos = 0;
T_UBYTE iluminando = 0;
T_UBYTE setPointIntro;
T_UBYTE peticionLecturaSensores = 0; //Flag para saber si se recibio la lectura

void inicializarObjetos(void);
void asignarHorarios(void); //MODULO ESP8266
T_INT horaIluminar(void);
void dameHoraActual(void); //MODULO RTC 3231
void dameDiaActual(void);
T_UBYTE setRtc(T_UBYTE direccion);
T_UBYTE leer_eeprom(uint16_t direccion);
void escribe_eeprom(uint16_t direccion, T_UBYTE dato);
void escribeHorariosMemoria(void);
void leeHorariosMemoria(void);
void setRtcDefault(void);
void fijaHoraRtc(void);
void fijaDiaRtc(void);
void setTiempoIluminar(void);
void mostrarMenu(void);
void sistemaPrincipal(T_UBYTE opcion);
void sistemaIluminado(void);
void dameDatosSistema(void);
void mostrarDatosSensores(void);
T_ULONG getValue(T_WORD numCharacters); //ASCII TO NUMBER FROM UART
void configBluetoothHC_06(void);
void limpiarBuffer(void);
void asignarSetPoint(void);
T_BOOL rangoPermitidoSetPoint(void);

T_INT horaIluminar() {

    return (horarios[hora].iluminar) && (!horarios[hora].iluminado) &&
            (horarios[hora].dias[diaActual - 1] == '1');
}

void inicializarObjetos() {

    for (T_INT i = 0; i < HORAS_DIA; i++) {
        horarios[i].hora = i;
        horarios[i].iluminar = 0;
        horarios[i].iluminado = 0;
        horarios[i].tiempoIluminar = 1;
    }

    for (T_INT i = 0; i < HORAS_DIA; i++) {

        for (T_INT j = 0; j < DIAS_SEMANA; j++)
            horarios[i].dias[j] = 0;

        horarios[i].dias[DIAS_SEMANA - 1] = '\0';
    }

    pwmDuty(0, CANAL_1);
    setPointAnterior = setPoint;

}

void dameHoraActual() { //RTC DS3231

    segundos = convertirDato(leer_rtc(0x00));
    minutos = convertirDato(leer_rtc(0x01));
    hora = convertirDato(leer_rtc(0x02));
}

void dameDiaActual(void) {

    diaActual = convertirDato(leer_rtc(0x03));
}

void fijaDiaRtc(void) {

    if (!setPointIntro) {

        //UART_printf("\r\n Envie el dia de la semana Ej: 01 = DOMINGO ... 07 = SABADO \r\n"); //comentar

        if (setRtc(0x03)) {
            //UART_printf("\r\n DIA ESTABLECIDO CORRECTAMENTE \r\n"); //comentar
            UART_write(SETEO_EXITOSO);
        }

    } else
        setPointIntro = 0;


}

T_UBYTE setRtc(T_UBYTE direccion) {

    T_UBYTE dato = 0;
    T_UBYTE seteado = 0;
    T_UBYTE datoRtc = 0;

    dato = getValue(2);

    if (dato != SETEO_DENEGADO) {

        datoRtc = ((dato / 10) & 0x0F) << 4;
        datoRtc |= (dato % 10) & 0x0F;
        escribe_rtc(direccion, datoRtc);
        seteado = 1;
    }

    return seteado;
}

void escribe_eeprom(uint16_t direccion, T_UBYTE dato) {
    i2c_inicia_com(); //inicia la comunicación serial i2C PIC
    i2c_envia_dato(0xAE); //envía la dirección del esclavo con el que el maestro se
    i2c_envia_dato(direccion >> 8); //envia parte alta de la direccion del registro
    //de la EEPROM donde se guardará el dato
    i2c_envia_dato(direccion); //envia parte baja de la direccion del registro
    //de la EEPROM donde se guardará el dato
    i2c_envia_dato(dato);
    i2c_detener();
    __delay_ms(10);
}

T_UBYTE leer_eeprom(uint16_t direccion) {

    T_UBYTE dato;

    i2c_inicia_com();
    i2c_envia_dato(0xAE);
    i2c_envia_dato(direccion >> 8);
    i2c_envia_dato(direccion);
    i2c_reinicia_com();
    i2c_envia_dato(0xAF);
    dato = i2c_recibe_dato();
    i2c_detener();
    __delay_ms(10);

    return dato;
}

void escribeHorariosMemoria() {

    T_INT contMemoria = 0; //Variable que cuenta cuantos Bytes se ha escrito en la EEPROM

    escribe_eeprom(contMemoria++, CARACTER_VALIDADOR); //digito verificador para leer la memoria
    //y evitar datos corruptos

    for (T_INT i = 0; i < HORAS_DIA; i++) {
        escribe_eeprom(contMemoria++, horarios[i].hora);

        for (T_INT j = 0; j < DIAS_SEMANA; j++) {
            escribe_eeprom(contMemoria++, horarios[i].dias[j]);
        }

        escribe_eeprom(contMemoria++, horarios[i].iluminar);
        escribe_eeprom(contMemoria++, horarios[i].tiempoIluminar);
    }
}

void leeHorariosMemoria() {

    T_INT contMemoria = 0; //Variable que cuenta cuantos Bytes se han leido en la EEPROM
    T_UBYTE caracterLeido;

    caracterLeido = leer_eeprom(contMemoria++);

    if (caracterLeido == CARACTER_VALIDADOR) {

        for (T_INT i = 0; i < HORAS_DIA; i++) {
            horarios[i].hora = leer_eeprom(contMemoria++);

            for (T_INT j = 0; j < DIAS_SEMANA; j++) {
                horarios[i].dias[j] = leer_eeprom(contMemoria++);
            }

            horarios[i].iluminar = leer_eeprom(contMemoria++);
            horarios[i].tiempoIluminar = leer_eeprom(contMemoria++);
        }

        //UART_printf("\r\n HORARIOS CARGADOS CON EXITO!\r\n"); //comentar
        UART_write(SETEO_EXITOSO);

    } else {
        //UART_printf("\r\n NO HAY DATOS EN LA MEMORIA \r\n"); //comentar
        UART_write(SETEO_DENEGADO); //Notificar al otro micro que no se recibira el dato

    }

}

void setRtcDefault(void) {
    T_UBYTE horaRtc;

    horaRtc = ((1) & 0x0F) << 4;

    horaRtc |= (2) & 0x0F;

    escribe_rtc(0x02, horaRtc); //HORA: 12

    escribe_rtc(0x01, 0); //MINUTOS: 0 MINUTOS
    escribe_rtc(0x00, 0); //SEGUNDOS: 0 SEGUNDOS
    UART_printf("\r\nRTC DEFAULT\n\r");
}

void fijaHoraRtc(void) {

    //UART_printf("\r\n FIJA HORA \r\n"); //comentar

    //// Seccion Horas ///
    //UART_printf("\r\n Envie las Horas en formato 24 Ej: 15 \r\n"); //comentar
    ///////Seccion minutos/////

    if (setRtc(0x02)) {
        //UART_printf("\r\n Envie los Minutos Ej: 25 \r\n"); //comentar
        if (setRtc(0x01)) {
            //UART_printf("\r\n HORA ESTABLECIDA CORRECTAMENTE \r\n"); //comentar 
            escribe_rtc(0x00, 0); //SEGUNDOS: 0 SEGUNDOS
            UART_write(SETEO_EXITOSO);

        }
    }

}

void asignarHorarios() //ESP8266
{
    T_UBYTE hora;
    T_UBYTE Rx;
    T_UBYTE diaIluminar;

    //UART_printf("\r\n OPCIONES DE REGADO \r\n"); //comentar

    //UART_printf("\r\n Ingrese una hora en formato de 24 hrs ej: 15 \r\n"); //comentar
    hora = getValue(2);

    if (hora != SETEO_DENEGADO) {

        //UART_printf("\r\n Ingrese 1 para activar || ingrese 0 para desactivar: \r\n"); //comentar

        Rx = getValue(1);


        if (Rx == 1) {

            //UART_printf("\r\n Ingrese 1 para activar || ingrese 0 para desactivar: \r\n"); //comentar
            //UART_printf("\r\n DOMINGO = [1] ... SABADO = [7] \r\n"); //comentar

            for (T_INT i = 0; i < DIAS_SEMANA; i++) {

                //sprintf(buffer, "\r\n[%d]: ", i + 1); //comentar
                //UART_printf(buffer); //comentar
                diaIluminar = getValue(1);

                if (diaIluminar != SETEO_DENEGADO) {

                    switch (diaIluminar) {
                        case 0:
                            diaIluminar = '0';
                            break;

                        case 1:
                            diaIluminar = '1';
                            break;
                    }


                    horarios[hora].dias[i] = diaIluminar;
                }

            }

            horarios[hora].iluminar = Rx;

            escribeHorariosMemoria(); //HACER ESTO INDEPENDIENTEMENTE DE SI MANDA DATO
            //POR UART

            //UART_printf("\r\n Horario Modificado! \r\n"); //comentar
            UART_write(SETEO_EXITOSO);

        } else if (Rx == 0) {
            horarios[hora].iluminar = Rx;
            //UART_printf("\r\n Horario Modificado! \r\n"); //comentar
            UART_write(SETEO_EXITOSO);
        }

    }

}

void setTiempoIluminar() {

    T_UBYTE hora;
    T_UBYTE tiempoIluminar;

    //UART_printf("\r\n TIEMPO DE REGADO \r\n"); //comentar

    //UART_printf("\r\n Ingrese una hora en formato de 24 hrs ej: 15 \r\n"); //comentar
    hora = getValue(2);

    if (hora != SETEO_DENEGADO) {

        //UART_printf("\r\n Ingrese las horas que desee que se riegue en ese horario ej: 15 \r\n"); //comentar
        tiempoIluminar = getValue(2);

        if (tiempoIluminar != SETEO_DENEGADO) {



            //UART_printf("\r\n Minutos de riego establecidos! \r\n"); //comentar

            horarios[hora].tiempoIluminar = tiempoIluminar;
            minutosIluminar = horarios[hora].tiempoIluminar;

            escribeHorariosMemoria();

            UART_write(SETEO_EXITOSO);

        }
    }

}

void mostrarMenu(void) {

    UART_printf("\r\n Ingresa una opcion a Realizar: \r\n");
    UART_printf("\r\n 1. Fijar Hora Actual \r\n");
    UART_printf("\r\n 2. Asignar Horarios para Iluminar \r\n");
    UART_printf("\r\n 3. Programar tiempo de Iluminacion en un horario \r\n");
    UART_printf("\r\n 4. Dame datos del sistema \r\n");
    UART_printf("\r\n 5. Mostrar valores sensores \r\n");
    UART_printf("\r\n 6. Asignar Setpoint\r\n");
    UART_printf("\r\n 7. Fijar Dia Actual \r\n");
    UART_printf("\r\n 8. Cargar datos de la memoria \r\n");
    UART_printf("\r\n Opcion:  \r");
    UART_printf("\r\n");
}

void sistemaPrincipal(T_UBYTE opcion) {

    PIE1bits.RCIE = 0; //deshabilita interrupción por recepción USART PIC.

    switch (opcion) {

        case 1:
            fijaHoraRtc();
            break;

        case 2:
            asignarHorarios();
            break;

        case 3:
            setTiempoIluminar();
            break;

        case 4:
            dameDatosSistema();
            break;

        case 5:
            mostrarDatosSensores();
            break;

        case 6:
            asignarSetPoint();

        case 7:
            fijaDiaRtc();
            break;

        case 8:
            leeHorariosMemoria();
            break;

        case 9:
            setRtcDefault(); //Comentar despues de programar el chip por primera vez y volver a programar
            break;


        default:
            //UART_printf("\r\n Solo se permiten numeros del 1 al 8 \r\n"); //comentar
            break;
    }

    //mostrarMenu(); //comentar
    PIE1bits.RCIE = 1; //habilita interrupción por recepción USART PIC.

}

void sistemaIluminado(void) {

    //UART_printf(".\n"); //Para verificar si se desborda el timer cada 10 seg

    if (iluminando) {

        luzMedida = dameValorLux(cont_H_res_mode1);

        if (rangoPermitidoSetPoint() && !cambioSetPoint) {

        } else {

            if (cambioSetPoint) {
                cambioSetPoint = 0;
                reiniciarPID = 1;
                pwmDuty(MIN_PWM, CANAL_1);
            }

            PID();
        }

        contInterrupciones++;

        if (contInterrupciones == REPETICIONES) {
            contInterrupciones = 0;
            minutosTranscurridos++;
            //UART_printf("MINUTO TRANSCURRIDO!\n");

            if (minutosTranscurridos >= minutosIluminar) {
                //UART_printf("\n\nRiego Finalizado!\n");
                pwmDuty(0, CANAL_1);
                minutosTranscurridos = 0;
                iluminando = 0;
                horarios[hora].iluminado = 1;
                tempHora = hora;
                flagIluminado = 0;
            }
        }

    } else {

        dameHoraActual();
        dameDiaActual();

        if (hora != tempHora && !flagIluminado) {
            horarios[tempHora].iluminado = 0;
            flagIluminado = 1;
        }

        if (horaIluminar()) {

            //UART_printf("\n\nRiego Iniciado!\n");    
            if (DEMO)
                minutosIluminar = horarios[hora].tiempoIluminar;
            else
                minutosIluminar = (horarios[hora].tiempoIluminar) * MINUTOS_HORA;

            iluminando = 1;
            horarios[hora].iluminado = 0;

        }
    }

}

void dameDatosSistema(void) {

    T_BYTE bufferHorario[TAMANO_CADENA_HORARIO];

    //UART_write(INTERRUMPIR_COMANDOS); //Esta Indica que inicio la transmicion de cadenas

    sprintf(buffer, "\r\nSetpoint = %d | ", setPoint);
    UART_printf(buffer);

    switch (diaActual) {
        case 1:
            sprintf(buffer2, "HORA = %0.2d:%0.2d | DIA = DOMINGO\r\n", hora, minutos);
            break;

        case 2:
            sprintf(buffer2, "HORA = %0.2d:%0.2d | DIA = LUNES\r\n", hora, minutos);
            break;

        case 3:
            sprintf(buffer2, "HORA = %0.2d:%0.2d | DIA = MARTES\r\n", hora, minutos);
            break;

        case 4:
            sprintf(buffer2, "HORA = %0.2d:%0.2d | DIA = MIERCOLES\r\n", hora, minutos);
            break;

        case 5:
            sprintf(buffer2, "HORA = %0.2d:%0.2d | DIA = JUEVES\r\n", hora, minutos);
            break;

        case 6:
            sprintf(buffer2, "HORA = %0.2d:%0.2d | DIA = VIERNES\r\n", hora, minutos);
            break;

        case 7:
            sprintf(buffer2, "HORA = %0.2d:%0.2d | DIA = SABADO\r\n", hora, minutos);
            break;

        default:
            break;
    }


    UART_printf(buffer2);

    UART_printf("\r\nH = HORA\r\n");
    UART_printf("\r\nR = ILUMINAR( 1 SI | 0 NO)\r\n");
    UART_printf("\r\nT = MINUTOS QUE DURARA LA ILUMINACION\r\n");
    UART_printf("\r\nD = DIAS QUE EN LOS QUE SE ILUMINARA\r\n");

    UART_printf("                           DLMIJVS\r\n");


    for (T_INT i = 0; i < HORAS_DIA; i++) {

        if (horarios[i].iluminar) {

            sprintf(bufferHorario, "H:%2d|R:%d|T:%2d|D:%s\r\n",
                    horarios[i].hora, horarios[i].iluminar, horarios[i].tiempoIluminar,
                    horarios[i].dias);

            UART_printf(bufferHorario);

        }

    }

    limpiarBuffer();

    UART_write(INTERRUMPIR_COMANDOS); //Esta Indica que acabo la transmicion de cadenas

}

void mostrarDatosSensores(void) {

    luzMedida = dameValorLux(cont_H_res_mode1);
    sprintf(buffer, "\rLuz medida: %d luxs\n\r", luzMedida);
    UART_printf(buffer);

    UART_write(INTERRUMPIR_COMANDOS); //Esta Indica que acabo la transmicion de cadenas

}

void configBluetoothHC_06(void) {
    __delay_ms(1000);
    UART_printf("AT+NAMESMARTHOME");
    __delay_ms(1000);
    UART_printf("AT+BAUD4"); //9600 Baudios
    __delay_ms(1000);
    UART_printf("AT+PIN2501");
    __delay_ms(1000);
}

T_ULONG getValue(T_WORD numCharacters) { //ASCII TO NUMBER FROM UART

    T_UBYTE Rx = 0;
    T_UBYTE datoIncorrecto = 0;
    T_ULONG dato = 0;

    switch (numCharacters) {

        case 1:
            Rx = UART_read();

            if (Rx >= 48 && Rx <= 57) {
                Rx -= 48;
                dato = Rx;
            } else {
                UART_write(SETEO_DENEGADO); //Notificar al otro micro que no se recibira el dato
                datoIncorrecto = 1;
            }

            break;

        case 2:
            for (T_INT i = 0; i < 2; i++) {//en las direccienes que van desde 0 a 50

                Rx = UART_read();

                if (Rx >= 48 && Rx <= 57) {

                    Rx -= 48;

                    switch (i) {
                        case 0:
                            dato = Rx;
                            dato *= 10;
                            break;

                        case 1:
                            dato += Rx;
                            break;

                        default:
                            break;
                    }

                } else {
                    //UART_printf("\r\n DATO NO RECIBIDO \r\n"); //comentar
                    datoIncorrecto = 1;
                    UART_write(SETEO_DENEGADO); //Notificar al otro micro que no se recibira el dato
                    break;
                }
            }
            break;

        case 3:
            for (T_INT i = 0; i < 3; i++) {//en las direccienes que van desde 0 a 50

                Rx = UART_read();

                if (Rx >= 48 && Rx <= 57) {

                    Rx -= 48;

                    switch (i) {
                        case 0:
                            dato = Rx;
                            dato *= 100;
                            break;

                        case 1:
                            dato += (Rx * 10);
                            break;

                        case 2:
                            dato += Rx;
                            break;

                        default:
                            break;
                    }

                } else {
                    //UART_printf("\r\n DATO NO RECIBIDO \r\n"); //comentar
                    datoIncorrecto = 1;
                    UART_write(SETEO_DENEGADO); //Notificar al otro micro que no se recibira el dato
                    break;
                }
            }
            break;

        case 4:
            for (T_INT i = 0; i < 4; i++) {//en las direccienes que van desde 0 a 50

                Rx = UART_read();

                if (Rx >= 48 && Rx <= 57) {

                    Rx -= 48;

                    switch (i) {
                        case 0:
                            dato = Rx;
                            dato *= 1000;
                            break;

                        case 1:
                            dato += (Rx * 100);
                            break;

                        case 2:
                            dato += (Rx * 10);
                            break;

                        case 3:
                            dato += Rx;
                            break;

                        default:
                            break;
                    }

                } else {
                    //UART_printf("\r\n DATO NO RECIBIDO \r\n"); //comentar
                    datoIncorrecto = 1;
                    UART_write(SETEO_DENEGADO); //Notificar al otro micro que no se recibira el dato
                    break;
                }
            }
            break;

        case 5:
            for (T_INT i = 0; i < 5; i++) {//en las direccienes que van desde 0 a 50

                Rx = UART_read();

                if (Rx >= 48 && Rx <= 57) {

                    Rx -= 48;

                    switch (i) {
                        case 0:
                            dato = Rx;
                            dato *= 10000;
                            break;

                        case 1:
                            dato += (Rx * 1000);
                            break;

                        case 2:
                            dato += (Rx * 100);
                            break;

                        case 3:
                            dato += (Rx * 10);

                        case 4:
                            dato += Rx;
                            break;

                        default:
                            break;
                    }

                } else {
                    //UART_printf("\r\n DATO NO RECIBIDO \r\n"); //comentar
                    datoIncorrecto = 1;
                    UART_write(SETEO_DENEGADO); //Notificar al otro micro que no se recibira el dato
                    break;
                }
            }
            break;

        default:
            break;
    }



    if (esperandoDatos || datoIncorrecto) {

        //UART_printf("\nFALLO EL SETEO\r\n"); //comentar
        return SETEO_DENEGADO;
    } else
        return dato;

}

void limpiarBuffer(void) {
    for (T_INT i = 0; i < TAMANO_CADENA; i++) {
        buffer[i] = 0;
    }

    buffer[TAMANO_CADENA - 1] = '\0';
}

T_BOOL rangoPermitidoSetPoint(void) {
    if ((luzMedida <= (setPoint + MAX_TOLERANCIA_ERROR)) && (luzMedida >= (setPoint - MAX_TOLERANCIA_ERROR)))
        return 1;
    else
        return 0;

}

void asignarSetPoint(void) {

    T_ULONG setPointTemp = 0;

    setPointIntro = 0;

    setPointTemp = getValue(1) * 1000;
    setPointTemp = getValue(1) * 100;
    setPointTemp += getValue(2);

    if (setPointTemp != SETEO_DENEGADO) {

        setPoint = setPointTemp;

        if (setPoint != setPointAnterior)
            cambioSetPoint = 1;
        else
            cambioSetPoint = 0;

        setPointAnterior = setPoint;


        UART_write(SETEO_EXITOSO);

    }

    setPointIntro = 1;
}


#endif	

