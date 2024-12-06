#include "DisplayTemp.hpp"

#include "Arduino.h"
#include "Wire.h"
#include <stdint.h>

uint16_t interpolarColor(uint16_t color1, uint16_t color2, float porcentaje);


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

    
    dibujarGradiente();
    dibujarNumero();

}

void DisplayTemp::dibujarNumero(){
        tft.setTextColor(TFT_WHITE);

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

void DisplayTemp::dibujarGradiente(){
    
    for(int i = 0; i < tft.width(); i++){
        
        tft.drawFastVLine(
            i,              // x pos
            0,              // y pos inicial
            tft.height(),   // y pos final
            interpolarColor(TFT_BLUE, TFT_RED, ((float)i)/((float)tft.width()))
        );
    }
}


// Funciones para ayudar


uint16_t interpolarColor(uint16_t color1, uint16_t color2, float porcentaje) {

    // Los colores unsan formato RGB 565.
    // quiere decir 
    // RED      >>  5 bits mas significativos
    // GREEN    >>  6 bits del medio
    // BLUE     >>  5 bits menos significativos

    // si es menor que 0 devuelve el color 1
    if (porcentaje <= 0) {
        return color1;
    }
    // si es mayor que 1 devuelve el color 2
    if (porcentaje >= 1) {
        return color2;
    }

    // color 1
    uint8_t r1 = (color1 >> 11) & 0x1F;
    uint8_t g1 = (color1 >> 5) & 0x3F;
    uint8_t b1 = color1 & 0x1F;

    // color 2
    uint8_t r2 = (color2 >> 11) & 0x1F;
    uint8_t g2 = (color2 >> 5) & 0x3F;
    uint8_t b2 = color2 & 0x1F;

    // resultado
    uint8_t r = r1 + (r2 - r1) * porcentaje;
    uint8_t g = g1 + (g2 - g1) * porcentaje;
    uint8_t b = b1 + (b2 - b1) * porcentaje;

    return (r << 11) | (g << 5) | b;
}

DisplayTemp Display = DisplayTemp();