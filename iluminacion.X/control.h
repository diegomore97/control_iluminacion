#ifndef CONTROL_H
#define	CONTROL_H

#define constrain(amt,low,high) ((amt)<(low)?(low):((amt)>(high)?(high):(amt)))

#define CANAL_1 1
#define PWM_INICIAL 100

//Ajustar estas variables de control para evitar chocar con las paredes laterales
#define KP 0.8 //Entre mas se aumente esta variable mas brusco sea el cambio para centrar
#define KI 1.2 //Valor para eliminar el error en estado estacionario
#define KD 1.5 //Entre mas aumente esta variabe mas oscilara tratando de centrarse

#define MAX_ERROR 200 //Maximo valor en el acomulador Integral
#define MAX_PWM 100 //Maximo valor que puede tener el PWM
#define MIN_PWM 5 //Minimo valor que puede tener el PWM
#define MAX_TOLERANCIA_ERROR 4 //Unidades em LUX de error permitido

T_BOOL reiniciarPID;
T_ULONG luzMedida, setPoint = 15, setPointAnterior;
T_BOOL cambioSetPoint; //Bandera que valida si se cambio el SetPoint


void PID(void);

void PID(void) {

    T_ULONG dif = 0;
    T_INT pwmSal = 0;
    static T_ULONG P, I = 0, D;
    static T_ULONG difAnt = 0;

    if (reiniciarPID || I >= MAX_ERROR) {
        I = 0;
        difAnt = 0;
        reiniciarPID = 0;
    }

    //calcula la diferencia
    dif = abs(setPoint - luzMedida);

    // calculo del error (se redondea)
    P = dif * KP;
    I = (I + dif) * KI;
    D = (dif - difAnt) * KD;

    difAnt = dif;

    pwmSal = P + I + D;
    // para mantener en memoria la dif anterior

    if (pwmSal > MAX_PWM)
        pwmSal = MAX_PWM;
    else if (pwmSal < MIN_PWM)
        pwmSal = MIN_PWM;

    pwmDuty(pwmSal, CANAL_1);

}



#endif