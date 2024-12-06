#pragma once

#include "TFT_eSPI.h"


class DisplayTemp {
public:

    enum modo_t {
        CELSIUS,
        FAHRENHEIT
    };

    DisplayTemp() : tft(TFT_eSPI()) {}

    void init(float min_temp = 0, float max_temp = 100);

    void setTemp(float temp);
    void updateDisplay();
    void setModo(modo_t modo);

private:

    TFT_eSPI tft;

    float temp;
    float max_temp;
    float min_temp;

    modo_t modo = CELSIUS;

    void dibujarNumero();
    void dibujarGradiente();

};

extern DisplayTemp Display;