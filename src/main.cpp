#include <Arduino.h>

#include "DisplayTemp.hpp"

#define BOTON   BUTTON_2
#define PIN_ADC A0

#define T_MUESTREO 1000 // ms


#include <CmdParser.hpp>
#include <CmdBuffer.hpp>
#include <CmdCallback.hpp>

CmdParser cmdParser;
CmdBuffer<32> myBuffer;
CmdCallback<1> cmdCallback;

void setCuentasAdc(CmdParser *parser);
bool botonPresionado();
float getTemperatura();

uint32_t ult_conversion_ms;
uint16_t cuentas_adc_manual;

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

    // Boton
    pinMode(BOTON, INPUT); // Tiene resistencia pullup en el PCB

    // Tiempo
    ult_conversion_ms = millis();
}

void loop() {

    // CLI
    cmdCallback.updateCmdProcessing(&cmdParser, &myBuffer, &Serial);

    // Display
    Display.updateDisplay();
    
    if(botonPresionado()){
        Display.toggleModo();
    }

    // Muestreo
    if(millis() - ult_conversion_ms > T_MUESTREO) {

        ult_conversion_ms = millis();

        Display.setTemp(getTemperatura());
    }


}

bool botonPresionado(){

    static bool boton_anterior = false;
    bool boton_actual = !digitalRead(BOTON); // Es un pullup
    bool ret = false;

    // Hago un debounce cuando suelto el boton
    static uint32_t tiempo_debounce = 0;
    static bool debounce = false;

    if (debounce){
        if (millis() - tiempo_debounce > 50){
            debounce = false;
        }
        return false;
    }

    if(!boton_actual && boton_anterior) {
        debounce = true;
        tiempo_debounce = millis();
    }

    else if (!boton_anterior && boton_actual){
        ret = true;
    }

    boton_anterior = boton_actual;
    return ret;
}

float getTemperatura() {

    int16_t cuentas = cuentas_adc_manual;

    constexpr int16_t max_cuentas = 0xFFF; // ADC de 12 bits
    constexpr int16_t offset = 6;          // cuando la temp es 0 C la salida del AO de instrumental es 5mv y no puede llegar nunca a 0V

    constexpr float max_temp = 100.0;

    return ((cuentas - offset) * max_temp) / ((float)max_cuentas);
}

void setCuentasAdc(CmdParser *parser) {

    if (parser->getParamCount() != 2) {
        Serial.println("Especificar el valor de cuentas ADC");
        return;
    }

    String param_string = String(parser->getCmdParam(1));
    cuentas_adc_manual = param_string.toInt();
}