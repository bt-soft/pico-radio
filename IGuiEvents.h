#ifndef __IGUIEVENTS_H
#define __IGUIEVENTS_H

#include "RotaryEncoder.h"

class IGuiEvents {

public:
    // Virtuális destruktor a származtatott osztályok megfelelő kezeléséhez
    virtual ~IGuiEvents() = default;

    /**
     * Rotary encoder esemény lekezelése
     */
    virtual void handleRotary(RotaryEncoder::EncoderState encoderState) = 0;

    /**
     * Touch esemény lekezelése
     */
    virtual void handleTouch(bool touched, uint16_t tx, uint16_t ty) = 0;
};

#endif // __IGUIEVENTS_H
