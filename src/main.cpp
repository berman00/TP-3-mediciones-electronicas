#include <Arduino.h>

#include "DisplayTemp.hpp"

#define BOTON BUTTON_2

#define T_MUESTREO 1000 // ms

bool botonPresionado();

uint32_t ult_conversion_ms;

void setup() {

    // Para usar con bateria
    pinMode(15, OUTPUT);
    digitalWrite(15, HIGH);
    
    Display.init();
    Serial.begin(115200);

    Display.setTemp(0.0f);
    Display.updateDisplay();

    // Canal analógico
    pinMode(A0, INPUT);
    analogReadResolution(12);

    // Boton
    pinMode(BOTON, INPUT); // Tiene resistencia pullup en el PCB

    // Tiempo
    ult_conversion_ms = millis();
}

void loop() {

    Display.updateDisplay();
  
    if(millis() - ult_conversion_ms > T_MUESTREO) {

        ult_conversion_ms = millis();

        int temp = analogRead(A0);
        Display.setTemp(temp/10.0f);
    }

    if(botonPresionado()){
        Display.toggleModo();
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
