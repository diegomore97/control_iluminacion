#ifndef CONTROL_H
#define	CONTROL_H

#define constrain(amt,low,high) ((amt)<(low)?(low):((amt)>(high)?(high):(amt)))

#define CANAL_1 1
#define PWM_INICIAL 100

//Ajustar estas variables de control para evitar chocar con las paredes laterales
#define KP 0.8 //Entre mas se aumente esta variable mas brusco sera el cambio para centrar
#define KI 0.1 //Valor para eliminar el error en estado estacionario
#define KD 1.9 //Entre mas aumente esta variabe mas oscilara tratando de centrarse

T_BOOL reiniciarPID;
T_ULONG luzMedida, setPoint = 100;

void PID(void);

void PID(void) {

    T_ULONG dif = 0;
    T_INT pwmSal = 0;
    static T_ULONG P, I = 0, D;
    static T_ULONG difAnt = 0;

    if (reiniciarPID) {
        I = 0;
        difAnt = 0;
        reiniciarPID = 0;
    }

    //calcula la diferencia
    dif = setPoint - luzMedida;

    // calculo del error (se redondea)
    P = dif * KP;
    I = (I + dif) * KI;
    D = (dif - difAnt) * KD;

    difAnt = dif;

    pwmSal = P + I + D;
    // para mantener en memoria la dif anterior
    
    if(pwmSal > 100)
        pwmSal = 100;
    else if(pwmSal < 0)
        pwmSal = 0;

    pwmDuty(pwmSal, CANAL_1);

}


#endif