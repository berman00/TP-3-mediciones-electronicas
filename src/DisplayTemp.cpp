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
    actualizar_temp = true;
}

void DisplayTemp::setUnidad(unidad_t unidad){
    this->unidad = unidad;
    actualizar_temp = true;
}

void DisplayTemp::toggleUnidad(){
    this->unidad = (unidad == CELSIUS) ? FAHRENHEIT : CELSIUS;
    actualizar_temp = true;
}

void DisplayTemp::setModo(modo_t modo) {
    this->modo = modo;
    cambiar_modo = true;
}

void DisplayTemp::mostrarBarraPresionado(float porcentaje) {
    barra_porcentaje = porcentaje;
    mostrar_barra_presionado = true;
}

void DisplayTemp::quitarBarraPresionado() {
    mostrar_barra_presionado = false;
}

void DisplayTemp::mostrarCalibracion(submodo_t submodo, float pos_aguja) {
    this->submodo = submodo;
    this->pos_aguja = pos_aguja;
}

void DisplayTemp::updateDisplay(){

    int grosor_marco = 7;
    uint16_t color_marco;

    // Si se cambio el modo borrar toda la pantalla para que no queden restos
    if (cambiar_modo) {
        cambiar_modo = false;
        tft.fillRect(
            grosor_marco,              // x pos
            grosor_marco,              // y pos
            tft.width() - grosor_marco * 2,   // ancho
            tft.height() - grosor_marco * 2,  // alto
            TFT_BLACK
        );
    }

    switch (modo) {
    case Disp_MEDICION:
        if (mostrar_barra_presionado) {
            dibujarBarraPresionado(grosor_marco, barra_porcentaje);
        }
        else {
            tft.fillRect(
                grosor_marco + 3,
                grosor_marco + 3,
                200,
                tft.fontHeight(2),
                TFT_BLACK
            );
        }
        // Color entre azul y rojo que varia segun temp
        color_marco = interpolarColor(TFT_BLUE, TFT_RED, ((temp - min_temp) / (max_temp - min_temp)));
        if(actualizar_temp) {
            actualizar_temp = false;
            dibujarNumero(grosor_marco);
            dibujarUnidad();
        }
        break;

    case Disp_CALIBRACION:
        dibujarAgujaCalibracion(pos_aguja, grosor_marco);
        int posx = grosor_marco + 10;
        int posy = tft.height()/2 + 10;
        tft.setTextColor(TFT_WHITE, TFT_BLACK);
        tft.setTextDatum(TL_DATUM);
        tft.setTextPadding(tft.width() - posx - grosor_marco);
        switch (submodo) {
        case Disp_CALIB_POTE:
            tft.drawString("Offset", posx, posy, 4);
            posy += tft.fontHeight(4);
            tft.drawString("Usar resistencia patron", posx, posy,2);
            break;

        case Disp_CALIB_GANANCIA:
            tft.drawString("Ganancia", posx, posy,4);
            posy += tft.fontHeight(4);
            tft.drawString("Medir agua hirviendo (100 C)", posx, posy,2);
            break;
        
        default:
            break;
        }

        color_marco = TFT_GREEN;
        break;
    }
    

    dibujarMarco(grosor_marco, color_marco);
    dibujarTitulo(grosor_marco);
}

#define FUENTE_NUM 7
#define FUENTE_UNIDAD &FreeSans24pt7b

void DisplayTemp::dibujarNumero(int grosor_marco){


    // Manejo de valores fuera del rango
    float temp_mostrada;
    if (temp>max_temp){
        temp_mostrada = max_temp;
        tft.setTextColor(TFT_RED, TFT_BLACK);
    }
    else if (temp<min_temp){
        temp_mostrada = min_temp;
        tft.setTextColor(TFT_RED, TFT_BLACK);
    }
    else{
        temp_mostrada = temp;
        tft.setTextColor(TFT_WHITE, TFT_BLACK);
    }

    // Cambio de unidades
    if (unidad == FAHRENHEIT){
        temp_mostrada = 32 + (temp_mostrada * 1.8);
    }

    // Numero

    tft.setTextFont(FUENTE_NUM); // Digitos 7 segmentos
    tft.setTextDatum(MR_DATUM); // Mido desde el medio a la derecha para alinear el texto a la derecha
    
    // Formateo string con valor para mejor visualización
    char  temp_str[20];
    sprintf(temp_str, "%.2f", temp_mostrada); // Tengo q dejar bastente espacio de padding para que se borren los caracteres entre actualizaciones
    
    int posx = alineacion_x;
    int posy = tft.height()/2; // Para que este centrado
    tft.setTextPadding(alineacion_x-grosor_marco);

    tft.drawString(temp_str, posx, posy);
}

void DisplayTemp::dibujarUnidad(){
    
    // Unidad
    tft.setFreeFont(FUENTE_UNIDAD);
    tft.setTextDatum(BL_DATUM);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.setTextPadding(tft.textWidth("C")); // C es la letra mas grande

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

    char unidad_str[4];

    switch(unidad){
    case CELSIUS:
        sprintf(unidad_str, "C"); 
        break;
    case FAHRENHEIT:
        sprintf(unidad_str, "F"); 
        break;
    }

    tft.drawString(unidad_str, unidad_x, unidad_y);
}

void DisplayTemp::dibujarMarco(int grosor, uint16_t color){
    

    for (int i = 0; i < grosor; i++){
        tft.drawRect(
            i,              // x pos
            i,              // y pos
            tft.width() - i * 2,   // ancho
            tft.height() - i * 2,  // alto
            color
        );
    }
}

void DisplayTemp::dibujarTitulo(int grosor_marco){
    
    tft.setTextFont(2);
    tft.setTextDatum(BR_DATUM);
    tft.setTextColor(TFT_ORANGE);

    int dist_esquina = grosor_marco + 3;

    int posx = tft.width() - dist_esquina;
    int posy = tft.height() - dist_esquina;

    tft.drawString("TP3 MEDICIONES - Bellini | Berman | Saitta", posx, posy);
}

void DisplayTemp::dibujarBarraPresionado(int grosor_marco, float porcentaje){

    tft.setTextFont(2);
    tft.setTextDatum(TL_DATUM);
    tft.setTextColor(TFT_WHITE);

    int dist_esquina = grosor_marco + 3;

    int posx = dist_esquina;
    int posy = dist_esquina;

    int largo_texto = tft.drawString("calibrar", posx, posy);
    int largo_barra = 100;
    int espacio = 3;
    posx = posx + largo_texto + espacio;
    
    int pixeles_completado =  porcentaje < 1.0 ? porcentaje * largo_barra : largo_barra;

    tft.fillRect(posx, posy, pixeles_completado, tft.fontHeight(), TFT_WHITE);
    tft.drawRect(posx, posy, largo_barra, tft.fontHeight(), TFT_WHITE);
}

void DisplayTemp::dibujarAgujaCalibracion(float pos_aguja, int grosor_marco){
    
    int posx;
    int posy;

    // Barra de referencia
    int dist_de_marco = 10;
    int ancho_barra = tft.width()-2*grosor_marco-2*dist_de_marco;
    tft.setTextFont(4);
    tft.setTextDatum(TC_DATUM);
    tft.setTextColor(TFT_WHITE, TFT_ORANGE);
    tft.setTextPadding(ancho_barra); // ancho de la barra

    posx = tft.width()/2;
    posy = tft.height()/3;

    tft.drawString("<< | | | | | | ^ | | | | | | >>", posx, posy);

    // Aguja
    int ancho_aguja = tft.textWidth("v");
    tft.setTextDatum(BC_DATUM);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.setTextPadding(ancho_aguja + 2); // Agrego un poco de padding por las dudas

    int rango = ancho_barra - ancho_aguja;
    int pos_en_rango = (int)((float)((pos_aguja + 1.0) / 2.0) * rango);
    if (pos_en_rango > rango) pos_en_rango = rango;
    else if (pos_en_rango < 0) pos_en_rango = 0;
    posx = grosor_marco + dist_de_marco + pos_en_rango + ancho_aguja/2;
    tft.drawString("v", posx, posy);

    // Rectangulos para borrar agujas anteriores
    int alto = tft.fontHeight();
    posy = posy - alto; // Datum arriba a la izq

    // Rectangulo izq
    posx = grosor_marco;
    int ancho = dist_de_marco + pos_en_rango - 1;
    tft.fillRect(posx, posy, ancho, alto, TFT_BLACK);

    // Rectangulo der
    posx = grosor_marco + dist_de_marco + pos_en_rango + ancho_aguja + 1;
    ancho = tft.width() - posx - grosor_marco;
    tft.fillRect(posx, posy, ancho, alto, TFT_BLACK);


}

// Funciones de ayuda


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