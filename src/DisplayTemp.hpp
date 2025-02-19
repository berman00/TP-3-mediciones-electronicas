#pragma once

#include "TFT_eSPI.h"
    
enum unidad_t {
    CELSIUS,
    FAHRENHEIT
};

enum modo_t {
    Disp_MEDICION,
    Disp_CALIBRACION
};

class DisplayTemp {
public:

    DisplayTemp() : tft(TFT_eSPI()) {}

    void init(float min_temp = 0, float max_temp = 100);

    void setTemp(float temp);
    void updateDisplay();
    void setUnidad(unidad_t unidad);
    void toggleUnidad();
    void setModo(modo_t modo);
    void mostrarBarraPresionado(float porcentaje);
    void quitarBarraPresionado();

private:

    TFT_eSPI tft;

    float temp;
    float max_temp;
    float min_temp;

    unidad_t unidad = CELSIUS;
    modo_t modo = Disp_MEDICION;

    bool mostrar_barra_presionado = false;
    float barra_porcentaje;

    // punto en el eje x a partir de donde se alinea el numero
    // el valor queda a la izquierda y la unidad a la derecha
    static constexpr int alineacion_x = 210;

    bool actualizar_temp = true;
    bool cambiar_modo = true;

    void dibujarNumero();
    void dibujarMarco(int grosor, uint16_t color);
    void dibujarUnidad();
    void dibujarTitulo(int grosor_marco);
    void dibujarBarraPresionado(int grosor_marco, float porcentaje);

};

extern DisplayTemp Display;