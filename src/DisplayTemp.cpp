#include "DisplayTemp.hpp"

#include "Arduino.h"
#include "Wire.h"

void DisplayTemp::init(float min_temp, float max_temp, bool boton){

    tft.init();
    tft.setRotation(1);
    tft.fillScreen(TFT_BLACK);
    tft.setTextDatum(MR_DATUM); // Mido desde el medio a la derecha para alinear el texto a la derecha100

    this->min_temp = min_temp;
    this->max_temp = max_temp;
    this->boton = boton;

    // Si selecciona boton se pueden cambiar de celcius a farenheit usando el boton
    if (boton){
        pinMode(BUTTON_2, INPUT);
    }
}

void DisplayTemp::setTemp(float temp){
    this->temp = temp;
}

void DisplayTemp::updateDisplay(){

    tft.fillScreen(TFT_BLACK);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);

    // Formateo string con valor para mejor visualización
    char  temp_str[6];

    sprintf(temp_str, "%6.2f", temp);

    int font = 7;
    int posx = 200;
    int posy = tft.height()/2; // Para que este centrado

    tft.drawString(
        temp_str,
        posx,
        posy,
        font
    );

}


DisplayTemp Display = DisplayTemp();