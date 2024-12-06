#include <Arduino.h>

#include "DisplayTemp.hpp"

#define BOTON BUTTON_2

bool botonPresionado();


void setup() {
    
    Display.init();
    Serial.begin(115200);

    Display.setTemp(24);
    Display.updateDisplay();

    // Boton
    pinMode(BOTON, INPUT); // Tiene resistencia pullup en el PCB
}

void loop() {
  
    if(Serial.available() > 0){
        Display.setTemp(Serial.parseFloat());
        Display.updateDisplay();
        Serial.flush();
    }

    if(botonPresionado()){
        Display.toggleModo();
        Display.updateDisplay();
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
