#ifndef __BAND_H
#define __BAND_H

#include <SI4735.h>

#include "Config.h"
#include "rtVars.h"

// Band index
#define FM_BAND_TYPE 0
#define MW_BAND_TYPE 1
#define SW_BAND_TYPE 2
#define LW_BAND_TYPE 3

// DeModulation types
#define FM 0
#define LSB 1
#define USB 2
#define AM 3
#define CW 4

// BandTable állandó része (PROGMEM-ben tároljuk)
struct BandTableConst {
    const char *bandName;  // Sáv neve (PROGMEM mutató)
    uint8_t bandType;      // Sáv típusa (FM, MW, LW vagy SW)
    uint16_t prefMod;      // Preferált moduláció (AM, FM, USB, LSB, CW)
    uint16_t minimumFreq;  // A sáv minimum frekvenciája
    uint16_t maximumFreq;  // A sáv maximum frekvenciája
    uint16_t defFreq;      // Alapértelmezett frekvencia vagy aktuális frekvencia
    uint8_t defStep;       // Alapértelmezett lépésköz (növelés/csökkentés)
    bool isHam;            // HAM sáv-e?
};

// BandTable változó része (RAM-ban tároljuk)
struct BandTableVar {
    uint16_t currFreq;    // Aktuális frekvencia
    uint8_t currStep;     // Aktuális lépésköz (növelés/csökkentés)
    uint8_t currMod;      // Aktuális mód/modulációs típus (FM, AM, LSB, USB, CW)
    uint16_t antCap;      // Antenna Tuning Capacitor
    int16_t lastBFO;      // Utolsó BFO érték
    int16_t lastmanuBFO;  // Utolsó manuális BFO érték X-Tal segítségével
};

// A kombinált struktúra az állandó és változó adatok összekapcsolásával
struct BandTable {
    const BandTableConst *pConstData;  // PROGMEM-beli állandó adatok
    BandTableVar varData;              // RAM-ban tárolt változó adatok
};

// Sávszélesség struktúra
struct BandWidth {
    const char *label;  // Megjelenítendő felirat
    uint8_t index;      // Az si4735-nek átadandó index
};

/**
 * Band class
 */
class Band {
   private:
    // Si4735 referencia
    SI4735 &si4735;

    // SSB betöltve?
    bool ssbLoaded = false;

    void setBandWidth();
    void loadSSB();

   public:
    // BandMode description
    static const char *bandModeDesc[5];

    // Sávszélesség struktúrák tömbjei
    static const BandWidth bandWidthFM[5];
    static const BandWidth bandWidthAM[7];
    static const BandWidth bandWidthSSB[6];

    // Frequency Step
    static const char *stepSizeAM[4];
    static const char *stepSizeFM[3];

    Band(SI4735 &si4735);
    virtual ~Band() = default;
    /**
     *
     */
    void bandInit(bool sysStart = false);

    /**
     * Band beállítása
     * @param loadPrefDeMod prefereált demodulációt betöltsük?
     */
    void bandSet(bool loadPrefDeMod = false);

    /**
     *
     */
    void useBand();

    /**
     * A Band egy rekordjának elkérése az index alapján
     * @param bandIdx a band indexe
     * @return A BandTableVar rekord referenciája, vagy egy üres rekord, ha nem található
     */
    BandTable &getBandByIdx(uint8_t bandIdx);

    /**
     *
     */
    inline BandTable &getCurrentBand() { return getBandByIdx(config.data.bandIdx); }

    /**
     * A Band indexének elkérése a bandName alapján
     *
     * @param bandName A keresett sáv neve
     * @return A BandTable rekord indexe, vagy -1, ha nem található
     */
    int8_t getBandIdxByBandName(const char *bandName);

    /**
     * Aktuális mód/modulációs típus (FM, AM, LSB, USB, CW)
     */
    inline const char *getCurrentBandModeDesc() { return bandModeDesc[getCurrentBand().varData.currMod]; }

    /**
     * A lehetséges AM demodulációs módok kigyűjtése
     */
    inline const char **getAmDemodulationModes(uint8_t &count) {
        // count = sizeof(bandModeDesc) / sizeof(bandModeDesc[0]) - 1;
        count = ARRAY_ITEM_COUNT(bandModeDesc) - 1;
        return &bandModeDesc[1];
    }

    /**
     * Az aktuális sávszélesség labeljének lekérdezése
     * @return A sávszélesség labelje, vagy nullptr, ha nem található
     */
    const char *getCurrentBandWidthLabel() {
        const char *p;
        uint8_t currMod = getCurrentBand().varData.currMod;
        if (currMod == AM) p = getCurrentBandWidthLabelByIndex(bandWidthAM, config.data.bwIdxAM);
        if (currMod == LSB or currMod == USB or currMod == CW) p = getCurrentBandWidthLabelByIndex(bandWidthSSB, config.data.bwIdxSSB);
        if (currMod == FM) p = getCurrentBandWidthLabelByIndex(bandWidthFM, config.data.bwIdxFM);

        return p;
    }

    /**
     * Sávszélesség tömb labeljeinek visszaadása
     * @param bandWidth A sávszélesség tömbje
     * @param count A tömb elemeinek száma
     * @return A label-ek tömbje
     */
    template <size_t N>
    const char **getBandWidthLabels(const BandWidth (&bandWidth)[N], size_t &count) {
        count = N;  // A tömb mérete
        static const char *labels[N];
        for (size_t i = 0; i < N; i++) {
            labels[i] = bandWidth[i].label;
        }
        return labels;
    }

    /**
     * A sávszélesség labeljének lekérdezése az index alapján
     * @param bandWidth A sávszélesség tömbje
     * @param index A keresett sávszélesség indexe
     * @return A sávszélesség labelje, vagy nullptr, ha nem található
     */
    template <size_t N>
    const char *getCurrentBandWidthLabelByIndex(const BandWidth (&bandWidth)[N], uint8_t index) {
        for (size_t i = 0; i < N; i++) {
            if (bandWidth[i].index == index) {
                return bandWidth[i].label;  // Megtaláltuk a labelt
            }
        }
        return nullptr;  // Ha nem található
    }

    /**
     * A sávszélesség indexének lekérdezése a label alapján
     * @param bandWidth A sávszélesség tömbje
     * @param label A keresett sávszélesség labelje
     * @return A sávszélesség indexe, vagy -1, ha nem található
     */
    template <size_t N>
    int8_t getBandWidthIndexByLabel(const BandWidth (&bandWidth)[N], const char *label) {
        for (size_t i = 0; i < N; i++) {
            if (strcmp(label, bandWidth[i].label) == 0) {
                return bandWidth[i].index;  // Megtaláltuk az indexet
            }
        }
        return -1;  // Ha nem található
    }

    inline const char *getCurrentBandName() { return (const char *)pgm_read_ptr(&getCurrentBand().pConstData->bandName); }

    inline uint8_t getCurrentBandType() { return getCurrentBand().pConstData->bandType; }

    inline uint16_t getCurrentBandMinimumFreq() { return getCurrentBand().pConstData->minimumFreq; }

    inline uint16_t getCurrentBandMaximumFreq() { return getCurrentBand().pConstData->maximumFreq; }

    inline uint16_t getCurrentBandDefaultFreq() { return getCurrentBand().pConstData->defFreq; }

    inline uint8_t getCurrentBandDefaultStep() { return getCurrentBand().pConstData->defStep; }

    inline bool getCurrentBandIsHam() { return getCurrentBand().pConstData->isHam; }

    /**
     *
     */
    const char **getBandNames(uint8_t &count, bool isHamFilter);
};

#endif  // __BAND_H