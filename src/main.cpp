#include <Arduino.h>

#include "DisplayTemp.hpp"
#include "Boton.hpp"

#define PIN_ADC A0

#define T_MUESTREO 500 // ms
#define T_MODO_CALIB 1000 //ms

#define V_INS_MIN_CUENTAS 37   // [cuentas] = 30  mV
#define V_INS_MAX_CUENTAS 3970 // [cuentas] = 3.2 V


float getTemperatura(int cuentas);

uint32_t ult_conversion_ms;
bool bandera_salida_calib;

Boton botonUnidad(BUTTON_1, true);
Boton botonCalibracion(BUTTON_2, true);

enum {
    MEDICION,
    CALIBRACION
} modo;

enum {
    CALIB_POTE,
    CALIB_GANACIA
} calib_submodo;

void setup() {


    // Para usar con bateria
    pinMode(15, OUTPUT);
    digitalWrite(15, HIGH);
    
    Display.init();
    Serial.begin(115200);

    Display.setTemp(0.0f);
    Display.updateDisplay();

    // Canal analógico
    pinMode(PIN_ADC, INPUT);
    analogReadResolution(12);

    // Botones
    botonUnidad.init();
    botonCalibracion.init();

    // Tiempo
    ult_conversion_ms = millis();

    // Modo
    modo = MEDICION;
    bandera_salida_calib = true;
}

void loop() {

    int cuentas;

    switch (modo) {

    case MEDICION:

        if(botonUnidad.fuePresionado()){
            Display.toggleUnidad();
        }       
        // Muestreo
        if(millis() - ult_conversion_ms > T_MUESTREO) {
            
            ult_conversion_ms = millis();
            
            cuentas = analogRead(PIN_ADC);
            
            Display.setTemp(getTemperatura(cuentas));
        }
        
        // Entrada modo de calibración
        if (botonCalibracion.fueSoltado()) bandera_salida_calib = true; // Para q no aparezca el mensaje de calibracion cuando salis de ese modo
        if (botonCalibracion.estaPresionado() && bandera_salida_calib){
            float porcentaje_barra = (float)botonCalibracion.getTiempoPresionadoMs()/(float)T_MODO_CALIB;
            Display.mostrarBarraPresionado(porcentaje_barra);
            if (porcentaje_barra > 1.0) {
                modo = CALIBRACION;
                calib_submodo = CALIB_POTE;
                Display.setModo(Disp_CALIBRACION);
                botonCalibracion.fuePresionado(); // Borro la bandera para no salir del modo inmediatamente
            }
        }
        else {
            Display.quitarBarraPresionado();
        }
        break;

    case CALIBRACION:

        float pos_aguja;
        cuentas = analogRead(PIN_ADC);
        
        switch (calib_submodo) {
        case CALIB_POTE:
            pos_aguja = (cuentas - V_INS_MIN_CUENTAS) / 20.0;  // rango [V_INS_MIN_CUENTAS - 20; V_INS_MIN_CUENTAS + 20]
            Display.mostrarCalibracion(Disp_CALIB_POTE, pos_aguja);
            // Entrada prox submodo
            if (botonCalibracion.fuePresionado()) {
                calib_submodo = CALIB_GANACIA;
            }
            break;
        case CALIB_GANACIA:
            pos_aguja = (cuentas - V_INS_MAX_CUENTAS) / 100.0; // rango [V_INS_MAX_CUENTAS - 100; V_INS_MAX_CUENTAS + 100]
            Display.mostrarCalibracion(Disp_CALIB_GANANCIA, pos_aguja);
            // Entrada modo Medicion
            if (botonCalibracion.fuePresionado()){
                modo = MEDICION;
                Display.setModo(Disp_MEDICION);
                bandera_salida_calib = false;
                Display.quitarBarraPresionado(); // Elimina un residuo q dura 1 frame
            }
            break;
            
        default: // Algo salio mal, volver a modo de medicion
            modo = MEDICION;
            Display.setModo(Disp_MEDICION);
            break;
        }
        break;

    default:
        modo = MEDICION;
        Display.setModo(Disp_MEDICION);
        break;
    }

    
    // Display
    Display.updateDisplay();

}


float getTemperatura(int cuentas) {

        /*
            Se usaron los sig valores

            Vexc = 5V

            Valores para V salida de amplificador operacional /
            entrada del ADC

            max = 3.2 V
            min = 30  mV

            Para trabajar en la zona lineal de los AO

            Valores calculados con MATLAB
            Deben cambiar si cambia alguno de los parámetros del circuito
        */

        double Ka = 2.051838252064429e-05;
        double Kc = 0.499236156577981;
        double alplha = 0.385; // [Ohms/C°]

    double temp_float = ( -100.0 / alplha ) * ( ( 1 - 2*Ka*cuentas - 2*Kc ) / ( 1 - Ka*cuentas - Kc) ); 

    // limitamos los valores a resoluciones de 0.05
    int16_t temp_en_2000_cuentas = 20 * temp_float; // Redondea para abajo
    
    return (float) (temp_en_2000_cuentas / 20.0);
}