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

// Band data
struct BandTable {
    const char *bandName;  // Bandname
    uint8_t bandType;      // Band type (FM, MW or SW)
    uint16_t prefmod;      // Pref. modulation
    uint16_t minimumFreq;  // Minimum frequency of the band
    uint16_t maximumFreq;  // maximum frequency of the band
    uint16_t currentFreq;  // Default frequency or current frequency
    uint8_t currentStep;   // Default step (increment and decrement)
    int lastBFO;           // Last BFO per band
    int lastmanuBFO;       // Last Manual BFO per band using X-Tal
    bool isHam;            // HAM band?
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
    void useBand();

   public:
    static const char *bandModeDesc[5];
    static const char *bandwidthFM[5];
    static const char *bandwidthAM[7];
    static const char *bandwidthSSB[6];

    uint8_t currentMode;  // aktuális mód/modulációs típus (FM, AM, LSB, USB, CW)

    Band(SI4735 &si4735);
    virtual ~Band() = default;
    /**
     *
     */
    void BandInit();

    /**
     *
     */
    void BandSet();

    /**
     * A Band egy rekordjának elkérése az index alapján
     * @param bandIdx a band indexe
     * @return A BandTable rekord referenciája, vagy egy üres rekord, ha nem található
     */
    BandTable &getBandByIdx(uint8_t bandIdx);

    /**
     * A Band indexének elkérése a bandName alapján
     *
     * @param bandName A keresett sáv neve
     * @return A BandTable rekord indexe, vagy -1, ha nem található
     */
    int8_t getBandIdxByBandName(const char *bandName);

    // /**
    //  * A Band egy rekordjának elkérése a bandName alapján
    //  *
    //  * @param bandName A keresett sáv neve
    //  * @return A BandTable rekord referenciája, vagy egy üres rekord, ha nem található
    //  */
    // BandTable &getBandByBandName(const char *bandName);

    /**
     *
     */
    inline const char *getCurrentBandModeDesc() { return bandModeDesc[currentMode]; }

    /**
     *
     */
    inline const char *getCurrentBandWithPstr() {
        const char *p;
        if (currentMode == AM) p = bandwidthAM[config.data.bwIdxAM];
        if (currentMode == LSB or currentMode == USB or currentMode == CW) p = bandwidthSSB[config.data.bwIdxSSB];
        if (currentMode == FM) p = bandwidthFM[config.data.bwIdxFM];

        return p;
    }

    const char **getBandNames(uint8_t &count, bool isHamFilter);
};

// A főprogramban definiálva
extern Band band;

#endif  // __BAND_H