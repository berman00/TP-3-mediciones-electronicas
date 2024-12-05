#include <Arduino.h>

#include "DisplayTemp.hpp"


void setup() {
    
    Display.init();
    Serial.begin(115200);

    Display.setTemp(24);
    Display.updateDisplay();
}

void loop() {
  
    if(Serial.available() > 0){
        Display.setTemp(Serial.parseFloat());
        Display.updateDisplay();
    }
}
