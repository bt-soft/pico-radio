#ifndef __STOREBASE_H
#define __STOREBASE_H
#include "EepromManager.h"
#include <Arduino.h>
#include <list>

/**
 * Generikus wrapper ősosztály a mentés és betöltés + CRC számítás funkciókhoz
 */
template <typename T>
class StoreBase {

private:
    // A tárolt adatok CRC32 ellenőrző összege
    uint16_t lastCRC = 0;

protected:
    /**
     * Referencia az adattagra, ez az ős használja
     */
    virtual T &r() = 0;

public:
    /**
     * Tárolt adatok mentése
     */
    virtual void forceSave() {
        EepromManager<T>::save(r());
    }

    /**
     * Tárolt adatok betöltése
     */
    virtual void load() {
        lastCRC = EepromManager<T>::load(r());
    }

    /**
     * Alapértelmezett adatok betöltése
     * Muszáj implementálni a leszármazottban
     */
    virtual void loadDefaults() = 0;

    /**
     * CRC ellenőrzés és mentés indítása, ha szükséges
     */
    virtual void checkSave() final {

        uint16_t crc = calcCRC16((uint8_t *)&r(), sizeof(T));
        if (lastCRC != crc) {
            crc = EepromManager<T>::save(r());
            lastCRC = crc;
            DEBUG("EEPROM save end, crc = %d\n", crc);
        }
    }
};

#endif //__STOREBASE_H
