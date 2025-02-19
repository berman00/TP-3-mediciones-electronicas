#include <Arduino.h>

#include "DisplayTemp.hpp"
#include "Boton.hpp"

#define PIN_ADC A0

#define T_MUESTREO 1000 // ms
#define T_MODO_CALIB 1000 //ms

#include <CmdParser.hpp>
#include <CmdBuffer.hpp>
#include <CmdCallback.hpp>

CmdParser cmdParser;
CmdBuffer<32> myBuffer;
CmdCallback<1> cmdCallback;

void setCuentasAdc(CmdParser *parser);
float getTemperatura(int cuentas);

uint32_t ult_conversion_ms;
uint16_t cuentas_adc_manual;

Boton botonUnidad(BUTTON_2, true);
Boton botonCalibracion(BUTTON_1, true);

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
}

void loop() {

    // CLI
    cmdCallback.updateCmdProcessing(&cmdParser, &myBuffer, &Serial);

    if(botonUnidad.fuePresionado()){
        Display.toggleModo();
    }
    
    // Muestreo
    if(millis() - ult_conversion_ms > T_MUESTREO) {
        
        ult_conversion_ms = millis();
        
        int cuentas = cuentas_adc_manual;
        
        Display.setTemp(getTemperatura(cuentas));
    }
    
    // Entrada modo de calibración
    if (botonCalibracion.estaPresionado()){
        float porcentaje_barra = (float)botonCalibracion.getTiempoPresionadoMs()/(float)T_MODO_CALIB;
        Display.mostrarBarraPresionado(porcentaje_barra);
    }
    else {
        Display.quitarBarraPresionado();
    }
    
    // Display
    Display.updateDisplay();

}


float getTemperatura(int cuentas) {

        /*
            Valores para V salida de amplificador operacional /
            entrada del ADC

            max = 3.2 V
            min = 20  mV

            Para trabajar en la zona lineal de los AO

            Valores calculados con MATLAB
            Deben cambiar si cambia alguno de los parámetros del circuito
        */

        double Ka = 2.045385930517057e-05;
        double Kc = 0.499492372400881;
        double alplha = 0.385; // [Ohms/C°]

    double temp_float = ( -100.0 / alplha ) * ( ( 1 - 2*Ka*cuentas - 2*Kc ) / ( 1 - Ka*cuentas - Kc) ); 

    // limitamos los valores a resoluciones de 0.05
    int16_t temp_en_2000_cuentas = 20 * temp_float; // Redondea para abajo
    
    return (float) (temp_en_2000_cuentas / 20.0);
}

void setCuentasAdc(CmdParser *parser) {

    if (parser->getParamCount() != 2) {
        Serial.println("Especificar el valor de cuentas ADC");
        return;
    }

    String param_string = String(parser->getCmdParam(1));
    cuentas_adc_manual = param_string.toInt();
}