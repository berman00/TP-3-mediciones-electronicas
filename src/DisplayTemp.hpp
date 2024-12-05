#pragma once

#include "TFT_eSPI.h"


class DisplayTemp {
public:

    DisplayTemp() : tft(TFT_eSPI()) {}

    void init(float min_temp = 0, float max_temp = 100, bool boton = false);

    void setTemp(float temp);
    void updateDisplay();

private:

    TFT_eSPI tft;

    float temp;
    float max_temp;
    float min_temp;

    enum modo_t {
        CELSIUS,
        FAHRENHEIT
    };

    modo_t modo = CELSIUS;
    bool boton = false;

};

extern DisplayTemp Display;