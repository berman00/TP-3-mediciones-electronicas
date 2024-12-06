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
    
    this->min_temp = min_temp;
    this->max_temp = max_temp;
}

void DisplayTemp::setTemp(float temp){
    this->temp = temp;
    necesario_actualizar = true;
}

void DisplayTemp::setModo(modo_t modo){
    this->modo = modo;
    necesario_actualizar = true;
}

void DisplayTemp::toggleModo(){
    this->modo = (modo == CELSIUS) ? FAHRENHEIT : CELSIUS;
    necesario_actualizar = true;
}

void DisplayTemp::updateDisplay(){

    // esperar a q transcurra el tiempo de actualizacion
    if(!necesario_actualizar) {
        return; // Solo actualizar si es necesario
    }

    necesario_actualizar = false;

    int grosor_marco = 7;

    // Lleno con negro el fondo para borrar el numero
    tft.fillRect(
        grosor_marco,              // x pos
        grosor_marco,              // y pos
        tft.width() - grosor_marco * 2,   // ancho
        tft.height() - grosor_marco * 2,  // alto
        TFT_BLACK
    );

    dibujarMarco(grosor_marco);
    dibujarNumero();
    dibujarUnidad();

}

#define FUENTE_NUM 7
#define FUENTE_UNIDAD &FreeSans24pt7b

void DisplayTemp::dibujarNumero(){


    // Manejo de valores fuera del rango
    float temp_mostrada;
    if (temp>max_temp){
        temp_mostrada = max_temp;
        tft.setTextColor(TFT_RED);
    }
    else if (temp<min_temp){
        temp_mostrada = min_temp;
        tft.setTextColor(TFT_RED);
    }
    else{
        temp_mostrada = temp;
        tft.setTextColor(TFT_WHITE);
    }

    // Cambio de unidades
    if (modo == FAHRENHEIT){
        temp_mostrada = 32 + (temp_mostrada * 1.8);
    }

    // Numero

    tft.setTextFont(FUENTE_NUM); // Digitos 7 segmentos
    tft.setTextDatum(MR_DATUM); // Mido desde el medio a la derecha para alinear el texto a la derecha100

    // Formateo string con valor para mejor visualización
    char  temp_str[6];
    sprintf(temp_str, "%.2f", temp_mostrada);

    int posx = alineacion_x;
    int posy = tft.height()/2; // Para que este centrado

    tft.drawString(temp_str, posx, posy);
}

void DisplayTemp::dibujarUnidad(){
    
    // Unidad
    tft.setFreeFont(FUENTE_UNIDAD);
    tft.setTextDatum(BL_DATUM);
    tft.setTextColor(TFT_WHITE);

    // posicion del reglo
    int reglon_y = (tft.height() + tft.fontHeight(FUENTE_NUM)) / 2;


    // Circulo de grados
    // no esta incluido en las tipografias
    int radio = 7;
    int grosor_circ = 3;
    int centro_x = alineacion_x + radio + 5;
    int centro_y = reglon_y - tft.fontHeight() + radio * 2;

    tft.fillCircle(centro_x, centro_y, radio, 0xFFFFFF); // No se porque pero esta funcion usa RGB de 24 bits en vez de 16;
    tft.fillCircle(centro_x, centro_y, radio-grosor_circ, TFT_BLACK);

    
    // celcius o farenheit
    int unidad_x = centro_x + (radio * 2) - 10;
    int unidad_y = reglon_y + 9; // bajo un poco la letra para que este sobre el reglon

    char unidad_str[2];

    switch(modo){
    case CELSIUS:
        sprintf(unidad_str, "C"); 
        break;
    case FAHRENHEIT:
        sprintf(unidad_str, "F"); 
        break;
    }

    tft.drawString(unidad_str, unidad_x, unidad_y);
}

void DisplayTemp::dibujarMarco(int grosor){

    // Color entre azul y rojo que varia segun temp
    uint16_t color_temp = interpolarColor(
        TFT_BLUE, TFT_RED, ((temp - min_temp) / (max_temp - min_temp)));
    

    for (int i = 0; i < grosor; i++){
        tft.drawRect(
            i,              // x pos
            i,              // y pos
            tft.width() - i * 2,   // ancho
            tft.height() - i * 2,  // alto
            color_temp
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