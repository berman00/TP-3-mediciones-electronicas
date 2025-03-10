#include <Arduino.h>

#include "DisplayTemp.hpp"
#include "Boton.hpp"

#define PIN_ADC A1

#define T_MUESTREO 1500 // ms
#define T_MODO_CALIB 1000 //ms

#define V_INS_MIN_CUENTAS 52   // [cuentas] = 40  mV
#define V_INS_MAX_CUENTAS 4095 // [cuentas] = 3 V


// CLI
#include <CmdParser.hpp>
#include <CmdBuffer.hpp>
#include <CmdCallback.hpp>

CmdParser cmdParser;
CmdBuffer<32> myBuffer;
CmdCallback<1> cmdCallback;

void setCuentasAdc(CmdParser *parser);
uint16_t cuentas_adc_manual;


float getTemperatura(int cuentas);
float redondearTemp(float temp_raw);
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

    // CLI
    Serial.begin(115200);
    delay(1000);
    Serial.println("Serial listo");

    cmdCallback.addCmd(PSTR("CUENTAS"), &setCuentasAdc);
    myBuffer.setEcho(true);


    // Para usar con bateria
    pinMode(15, OUTPUT);
    digitalWrite(15, HIGH);
    
    Display.init();
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

    // CLI
    cmdCallback.updateCmdProcessing(&cmdParser, &myBuffer, &Serial);

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
    * Valores calculados por interpolación lineal
    */

    constexpr int tam = 11;
    
    static constexpr struct {
        const int x[tam] = {
            0,
            300,  // 3.8 /
            482,  // 6.4 /
            857,  // 17.9 /
            1068, // 23.2 /
            1763, // 42.9 /
            2074, // 50.3 /
            2472, // 61.2 
            3021, // 75.1
            3581, // 88.8
            V_INS_MAX_CUENTAS,
        };
        const double y[tam] {
            0.0,
            3.8,
            6.4,
            17.9,
            23.2,
            42.9,
            50.3,
            61.2,
            75.1,
            88.8,
            100.0,
        };
    } puntos;

    // Edge case cuentas menores q primer punto
    if (cuentas < puntos.x[0]) {

        return redondearTemp(puntos.y[0]);
    }

    // Edge case cuentas mayores que ultimo punto
    if (cuentas >= puntos.x[tam-1]) {

        return redondearTemp(puntos.y[tam-1]);
    }

    for (int i = 0; i < tam - 1; i++) {

        if (cuentas >= puntos.x[i] && cuentas < puntos.x[i+1]) {

            double a = (puntos.y[i+1] - puntos.y[i]) / (double)(puntos.x[i+1] - puntos.x[i]);
            double b = puntos.y[i] - a * puntos.x[i];

            return redondearTemp(a*(double)cuentas+b);
        }
    }

    return -1; // Algo fallo. Puede ser que los puntos no estan bien ordenados
}

float redondearTemp(float temp_raw) {

    // limitamos los valores a resoluciones de 0.05
    int16_t temp_en_2000_cuentas = 20 * temp_raw; // Redondea para abajo

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

void setCuentasAdc(CmdParser *parser) {

    if (parser->getParamCount() != 2) {
        Serial.println("Especificar el valor de cuentas ADC");
        return;
    }

    String param_string = String(parser->getCmdParam(1));
    cuentas_adc_manual = param_string.toInt();
}