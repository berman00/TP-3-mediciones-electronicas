#include "DisplayTemp.hpp"

#include "Arduino.h"
#include "Wire.h"
#include <stdint.h>
#include <math.h>

uint16_t interpolarColor(uint16_t color1, uint16_t color2, float porcentaje);


void DisplayTemp::init(float min_temp, float max_temp){

    tft.init();
    tft.setRotation(1);
    tft.fillScreen(TFT_BLACK);
    tft.setTextDatum(MR_DATUM); // Mido desde el medio a la derecha para alinear el texto a la derecha100

    this->min_temp = min_temp;
    this->max_temp = max_temp;
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

    // Color entre azul y rojo que varia segun temp
    uint16_t color_temp = interpolarColor(
        TFT_BLUE, TFT_RED, ((temp - min_temp) / (max_temp - min_temp)));
    
    // Rectangulo alrrededor de la patalla con el color de la temperatura
    uint16_t grosor_marco = 5;
    for (int i = 0; i < grosor_marco; i++){
        tft.drawRect(
            i,              // x pos
            i,              // y pos
            tft.width() - i * 2,   // ancho
            tft.height() - i * 2,  // alto
            color_temp
        );
    }

    // Gradiente entre marco y fondo negro
    uint16_t grosor_gradiente = 20;
    for (int i = 0; i < grosor_gradiente; i++){
        uint16_t color_gradiente = interpolarColor(
            color_temp, TFT_BLACK, ((float)(i) / (float)(grosor_gradiente))
        );

        int j = grosor_marco + i;

        tft.drawRect(
            j,              // x pos
            j,              // y pos
            tft.width() - j * 2,   // ancho
            tft.height() - j * 2,  // alto
            color_gradiente
        );
    }

    uint16_t grosor_borde = grosor_marco + grosor_gradiente;

    // Lleno con negro el fondo para borrar el numero
    tft.fillRect(
        grosor_borde,              // x pos
        grosor_borde,              // y pos
        tft.width() - grosor_borde * 2,   // ancho
        tft.height() - grosor_borde * 2,  // alto
        TFT_BLACK
    );
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