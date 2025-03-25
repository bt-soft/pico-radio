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

// Modulation types
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
    uint8_t lastBFO;      // Utolsó BFO érték
    uint8_t lastmanuBFO;  // Utolsó manuális BFO érték X-Tal segítségével
};

// A kombinált struktúra az állandó és változó adatok összekapcsolásával
struct BandTable {
    const BandTableConst *pConstData;  // PROGMEM-beli állandó adatok
    BandTableVar varData;              // RAM-ban tárolt változó adatok
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

    static const char *bandWidthFM[5];
    static const char *bandWidthAM[7];
    static const char *bandWidthSSB[6];

    // Frequency Step
    static const char *stepSizeAM[4];
    static const char *stepSizeFM[3];

    Band(SI4735 &si4735);
    virtual ~Band() = default;
    /**
     *
     */
    void BandInit(bool sysStart = false);

    /**
     *
     */
    void BandSet();

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
     *
     */
    inline const char *getCurrentBandWithPstr() {
        const char *p;
        uint8_t currMod = getCurrentBand().varData.currMod;
        if (currMod == AM) p = bandWidthAM[config.data.bwIdxAM];
        if (currMod == LSB or currMod == USB or currMod == CW) p = bandWidthSSB[config.data.bwIdxSSB];
        if (currMod == FM) p = bandWidthFM[config.data.bwIdxFM];

        return p;
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

// A főprogramban definiálva
extern Band band;

#endif  // __BAND_H