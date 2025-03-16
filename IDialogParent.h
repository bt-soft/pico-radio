#ifndef __IDIALOGPARENT_H
#define __IDIALOGPARENT_H

#include "TftButton.h"

/**
 *
 */
class IDialogParent {

   public:
    /**
     * Dialog button adat átadása a szülő képernyőnek
     * (Nem referenciát adunk át, mert a dialóg megszűniuk majd!!)
     */
    virtual void setDialogResponse(TftButton::ButtonTouchEvent event) = 0;

    /**
     * A dialog button adatainak lekérése
     * (A ValueChangeDialog használja, hogy meg tudja állapítani, hogy 'X'-et (cancel-t) nyomtak-e?
     */
    virtual TftButton::ButtonTouchEvent getDialogResponse() = 0;
};

#endif  // __IDIALOGPARENT_H