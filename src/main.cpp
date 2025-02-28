#include <Arduino.h>

#include "DisplayTemp.hpp"
#include "Boton.hpp"

#define PIN_ADC A1

#define T_MUESTREO 500 // ms
#define T_MODO_CALIB 1000 //ms

#define V_INS_MIN_CUENTAS 52   // [cuentas] = 40  mV
#define V_INS_MAX_CUENTAS 3962 // [cuentas] = 3 V


float getTemperatura(int cuentas);
int getCuentasRollingAvg();

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

    // Debug
    Serial.begin();


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
    analogSetAttenuation(ADC_11db); // Vref = 3100 mV

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

    // Muestreo
    int cuentas = getCuentasRollingAvg();

    switch (modo) {

    case MEDICION:

        if(botonUnidad.fuePresionado()){
            Display.toggleUnidad();
        }       
        // Actualizar temp;
        if(millis() - ult_conversion_ms > T_MUESTREO) {
            
            ult_conversion_ms = millis();
            
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
        Serial.println(cuentas);
        
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

            R = 3920 Ohms

            Vexc = 5V
            Vref = 3.1 V

            Valores para V salida de amplificador operacional /
            entrada del ADC

            max = 3 V
            min = 40  mV

            Para trabajar en la zona lineal de los AO

            Valores calculados con MATLAB
            Deben cambiar si cambia alguno de los parámetros del circuito

            Como la relacion entre cuentas y temp es aprox lineal, se uso una ecuacion lineal
        */

        double temp = ((double)(cuentas - V_INS_MIN_CUENTAS)/(double)V_INS_MAX_CUENTAS) * 100.0; // [grados C]        
    
        // limitamos los valores a resoluciones de 0.05
        int16_t temp_en_2000_cuentas = 20 * temp; // Redondea para abajo
        
        return (float) (temp_en_2000_cuentas / 20.0);
}

int getCuentasRollingAvg(){

    // Hace un promedio de las ultimas 10 mediciones

    constexpr int cant_medi = 30;

    static int ultimas_mediciones[cant_medi]; // Promedio de 10 ultimas cuentas
    static int ind_act;

    int nueva_medicion = analogRead(PIN_ADC);

    ultimas_mediciones[ind_act] = nueva_medicion;
    ind_act++;
    if (ind_act >= cant_medi) ind_act = 0; // Buffer circular

    int32_t suma = 0;
    for (int i = 0; i < cant_medi; i++) {
        suma += ultimas_mediciones[i];
    }

    return suma / cant_medi;

}