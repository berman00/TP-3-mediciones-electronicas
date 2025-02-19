#pragma once

#include <Arduino.h>

#define TIEMPO_DEBOUNCE_MS 10

// Clase simple para controlar botones
class Boton {

public:

    Boton(int pin, bool invertido = false) {
        _pin = pin;
        _invertido = invertido;
    }

    void init() {

        pinMode(_pin, INPUT);
        bool presionado = digitalRead(_pin);
        if (_invertido) presionado = !presionado;

        if (presionado) {
            _estado = PRESIONADO;
        }
        else {
            _estado = NO_PRESIONADO;
        }
        _evento = evento_NADA;
        
        _ult_actualizacion_ms = millis();
    }

    bool estaPresionado() {
        actualizar();
        return _estado == PRESIONADO;
    }

    bool estaNoPresionado() {
        actualizar();
        return _estado == NO_PRESIONADO;
    }

    bool fuePresionado() {
        actualizar();
        if (_evento == evento_PRESIONADO) {
            _evento = evento_NADA;
            return true;
        }
        return false;
    }

    bool fueSoltado() {
        actualizar();
        if (_evento == evento_SOLTADO) {
            _evento = evento_NADA;
            return true;
        }
        return false;
    }

    uint64_t getTiempoPresionadoMs() {
        actualizar();
        if (_estado == PRESIONADO){
            return _tiempo_transcurrido_ms;
        }
        else {
            return 0;
        }
    }

    uint64_t getTiempoNoPresionadoMs() {
        actualizar();
        if (_estado == NO_PRESIONADO){
            return _tiempo_transcurrido_ms;
        }
        else {
            return 0;
        }
    }


private:

    void actualizar() {

        bool presionado = digitalRead(_pin);
        if (_invertido) presionado = !presionado;

        switch (_estado) {

        case PRESIONADO:
            _tiempo_transcurrido_ms += millis() - _ult_actualizacion_ms;
            if (!presionado) {
                _estado = DEBOUNCE_SOLTAR;
                _stopwatch_debouce_ms = millis();
            }
            break;
        
        case NO_PRESIONADO:
            _tiempo_transcurrido_ms += millis() - _ult_actualizacion_ms;
            if (presionado){
                _estado = DEBOUNCE_APRETAR;
                _stopwatch_debouce_ms = millis();
            }
            break;

        case DEBOUNCE_SOLTAR:
            if (presionado) {
                _estado = PRESIONADO;
            }
            else if (millis() - _stopwatch_debouce_ms > TIEMPO_DEBOUNCE_MS) {
                _estado = NO_PRESIONADO;
                _evento = evento_SOLTADO;
                _tiempo_transcurrido_ms = 0;
            }
            break;

        case DEBOUNCE_APRETAR:
            if (!presionado) {
                _estado = NO_PRESIONADO;
            }
            else if (millis() - _stopwatch_debouce_ms > TIEMPO_DEBOUNCE_MS) {
                _estado = PRESIONADO;
                _evento = evento_PRESIONADO;
                _tiempo_transcurrido_ms = 0;
            }
            break;
            
        
        default:
            break;
        }

        _ult_actualizacion_ms = millis();

    }

    int _pin;
    bool _invertido = false;

    uint64_t _ult_actualizacion_ms = 0;
    uint64_t _tiempo_transcurrido_ms = 0;
    uint64_t _stopwatch_debouce_ms = 0;

    enum estado_t {
        PRESIONADO,
        NO_PRESIONADO,
        DEBOUNCE_APRETAR,
        DEBOUNCE_SOLTAR
    } _estado;

    enum evento_t {
        evento_PRESIONADO,
        evento_SOLTADO,
        evento_NADA
    } _evento;

};