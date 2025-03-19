#include "Band.h"

#include <patch_full.h>  // SSB patch for whole SSBRX full download

#include "rtVars.h"

// Band tábla
BandTable bandTable[] = {
    {"FM", FM_BAND_TYPE, FM, 6400, 10800, 9390, 10, 0, 0},     //  FM          0   //93.9MHz Petőfi
    {"LW", LW_BAND_TYPE, AM, 100, 514, 198, 9, 0, 0},          //  LW          1
    {"MW", MW_BAND_TYPE, AM, 514, 1800, 540, 9, 0, 0},         //  MW          2   // 540kHz Kossuth
    {"800M", LW_BAND_TYPE, AM, 280, 470, 284, 1, 0, 0},        // Ham  800M    3
    {"630M", SW_BAND_TYPE, LSB, 470, 480, 475, 1, 0, 0},       // Ham  630M    4
    {"160M", SW_BAND_TYPE, LSB, 1800, 2000, 1850, 1, 0, 0},    // Ham  160M    5
    {"120M", SW_BAND_TYPE, AM, 2000, 3200, 2400, 5, 0, 0},     //      120M    6
    {"90M", SW_BAND_TYPE, AM, 3200, 3500, 3300, 5, 0, 0},      //       90M    7
    {"80M", SW_BAND_TYPE, LSB, 3500, 3900, 3630, 1, 0, 0},     // Ham   80M    8
    {"75M", SW_BAND_TYPE, AM, 3900, 5300, 3950, 5, 0, 0},      //       75M    9
    {"60M", SW_BAND_TYPE, USB, 5300, 5900, 5375, 1, 0, 0},     // Ham   60M   10
    {"49M", SW_BAND_TYPE, AM, 5900, 7000, 6000, 5, 0, 0},      //       49M   11
    {"40M", SW_BAND_TYPE, LSB, 7000, 7500, 7074, 1, 0, 0},     // Ham   40M   12
    {"41M", SW_BAND_TYPE, AM, 7200, 9000, 7210, 5, 0, 0},      //       41M   13
    {"31M", SW_BAND_TYPE, AM, 9000, 10000, 9600, 5, 0, 0},     //       31M   14
    {"30M", SW_BAND_TYPE, USB, 10000, 10200, 10099, 1, 0, 0},  // Ham   30M   15
    {"25M", SW_BAND_TYPE, AM, 10200, 13500, 11700, 5, 0, 0},   //       25M   16
    {"22M", SW_BAND_TYPE, AM, 13500, 14000, 13700, 5, 0, 0},   //       22M   17
    {"20M", SW_BAND_TYPE, USB, 14000, 14500, 14074, 1, 0, 0},  // Ham   20M   18
    {"19M", SW_BAND_TYPE, AM, 14500, 17500, 15700, 5, 0, 0},   //       19M   19
    {"17M", SW_BAND_TYPE, AM, 17500, 18000, 17600, 5, 0, 0},   //       17M   20
    {"16M", SW_BAND_TYPE, USB, 18000, 18500, 18100, 1, 0, 0},  // Ham   16M   21
    {"15M", SW_BAND_TYPE, AM, 18500, 21000, 18950, 5, 0, 0},   //       15M   22
    {"14M", SW_BAND_TYPE, USB, 21000, 21500, 21074, 1, 0, 0},  // Ham   14M   23
    {"13M", SW_BAND_TYPE, AM, 21500, 24000, 21500, 5, 0, 0},   //       13M   24
    {"12M", SW_BAND_TYPE, USB, 24000, 25500, 24940, 1, 0, 0},  // Ham   12M   25
    {"11M", SW_BAND_TYPE, AM, 25500, 26100, 25800, 5, 0, 0},   //       11M   26
    {"CB", SW_BAND_TYPE, AM, 26100, 28000, 27200, 1, 0, 0},    // CB band     27
    {"10M", SW_BAND_TYPE, USB, 28000, 30000, 28500, 1, 0, 0},  // Ham   10M   28
    {"SW", SW_BAND_TYPE, AM, 100, 30000, 15500, 5, 0, 0}       // Whole SW    29
};

/**
 * A Band egy rekordjának elkérése az index alapján
 */

BandTable &Band::getBandByIdx(uint8_t bandIdx) { return bandTable[bandIdx]; }

/**
 * SSB patch betöltése
 */
void Band::loadSSB() {

    // Ha már be van töltve, akkor nem megyünkn tovább
    if (ssbLoaded) {
        return;
    }

    si4735.reset();
    si4735.queryLibraryId();  // Is it really necessary here? I will check it.
    si4735.patchPowerUp();
    delay(50);

    si4735.setI2CFastMode();  // Recommended
    // si4735.setI2CFastModeCustom(500000); // It is a test and may crash.
    si4735.downloadPatch(ssb_patch_content, sizeof(ssb_patch_content));
    si4735.setI2CStandardMode();  // goes back to default (100KHz)
    delay(50);

    // Parameters
    // AUDIOBW - SSB Audio bandwidth; 0 = 1.2KHz (default); 1=2.2KHz; 2=3KHz; 3=4KHz; 4=500Hz; 5=1KHz;
    // SBCUTFLT SSB - side band cutoff filter for band passand low pass filter ( 0 or 1)
    // AVC_DIVIDER  - set 0 for SSB mode; set 3 for SYNC mode.
    // AVCEN - SSB Automatic Volume Control (AVC) enable; 0=disable; 1=enable (default).
    // SMUTESEL - SSB Soft-mute Based on RSSI or SNR (0 or 1).
    // DSP_AFCDIS - DSP AFC Disable or enable; 0=SYNC MODE, AFC enable; 1=SSB MODE, AFC disable.
    si4735.setSSBConfig(config.data.bwIdxSSB, 1, 0, 1, 0, 1);
    delay(25);
    ssbLoaded = true;
}

/**
 * Band beállítása
 */
void Band::useBand() {

    BandTable currentBand = bandTable[config.data.bandIdx];

    switch (currentBand.bandType) {

        case LW_BAND_TYPE:
        case MW_BAND_TYPE:
        case SW_BAND_TYPE:

            switch (currentBand.bandType) {
                case SW_BAND_TYPE:
                    currentBand.currentStep = config.data.ssIdxAM;
                    si4735.setTuneFrequencyAntennaCapacitor(1);
                    break;
                default:
                    currentBand.currentStep = config.data.ssIdxMW;
                    si4735.setTuneFrequencyAntennaCapacitor(0);
                    break;
            }

            if (ssbLoaded) {
                si4735.setSSB(currentBand.minimumFreq, currentBand.maximumFreq, currentBand.currentFreq, currentBand.currentStep, currentMode);
                si4735.setSSBBfo(config.data.currentBFO + config.data.currentBFOmanu);
                // SSB ONLY 1KHz stepsize
                bandTable[config.data.bandIdx].currentStep = 1;
                si4735.setFrequencyStep(1);

            } else {
                si4735.setAM(currentBand.minimumFreq, currentBand.maximumFreq, currentBand.currentFreq, currentBand.currentStep);
                rtv::bfoOn = false;
            }
            break;

        case FM_BAND_TYPE:
            ssbLoaded = false;
            rtv::bfoOn = false;
            currentBand.currentStep = config.data.ssIdxFM;
            si4735.setTuneFrequencyAntennaCapacitor(0);
            si4735.setFM(currentBand.minimumFreq, currentBand.maximumFreq, currentBand.currentFreq, currentBand.currentStep);
            si4735.setFMDeEmphasis(1);
            si4735.RdsInit();
            si4735.setRdsConfig(1, 2, 2, 2, 2);
            si4735.setSeekFmSpacing(10);
            si4735.setSeekFmLimits(bandTable[0].minimumFreq, bandTable[0].maximumFreq);  // FM band limits, a Band táblában a 0. indexü elem
            si4735.setSeekAmRssiThreshold(50);
            si4735.setSeekAmSrnThreshold(20);
            si4735.setSeekFmRssiThreshold(5);
            si4735.setSeekFmSrnThreshold(5);
            break;

        default:
            DEBUG("Hiba: Le nem kezelt bandType: %d\n", currentBand.bandType);
            return;
    }
}

/**
 * Sávszélesség beállítása
 */
void Band::setBandWidth() {

    if (currentMode == LSB || currentMode == USB) {
        si4735.setSSBAudioBandwidth(config.data.bwIdxSSB);

        // If audio bandwidth selected is about 2 kHz or below, it is recommended to set Sideband Cutoff Filter to 0.
        if (config.data.bwIdxSSB == 0 or config.data.bwIdxSSB == 4 or config.data.bwIdxSSB == 5) {
            // Band pass filter to cutoff both the unwanted side band and high frequency components > 2.0 kHz of the wanted side band. (default)
            si4735.setSSBSidebandCutoffFilter(0);

        } else {
            // Low pass filter to cutoff the unwanted side band.
            si4735.setSSBSidebandCutoffFilter(1);
        }
    }

    if (currentMode == AM) {
        /**
         * @ingroup group08 Set bandwidth
         * @brief Selects the bandwidth of the channel filter for AM reception.
         * @details The choices are 6, 4, 3, 2, 2.5, 1.8, or 1 (kHz). The default bandwidth is 2 kHz. It works only in AM / SSB (LW/MW/SW)
         * @see Si47XX PROGRAMMING GUIDE; AN332 (REV 1.0); pages 125, 151, 277, 181.
         * @param AMCHFLT the choices are:   0 = 6 kHz Bandwidth
         *                                   1 = 4 kHz Bandwidth
         *                                   2 = 3 kHz Bandwidth
         *                                   3 = 2 kHz Bandwidth
         *                                   4 = 1 kHz Bandwidth
         *                                   5 = 1.8 kHz Bandwidth
         *                                   6 = 2.5 kHz Bandwidth, gradual roll off
         *                                   7–15 = Reserved (Do not use).
         * @param AMPLFLT Enables the AM Power Line Noise Rejection Filter.
         */
        si4735.setBandwidth(config.data.bwIdxAM, 0);

    } else if (currentMode == FM) {
        /**
         * @brief Sets the Bandwith on FM mode
         * @details Selects bandwidth of channel filter applied at the demodulation stage. Default is automatic which means the device automatically selects proper channel filter.
         * <BR>
         * @details | Filter  | Description |
         * @details | ------- | -------------|
         * @details |    0    | Automatically select proper channel filter (Default) |
         * @details |    1    | Force wide (110 kHz) channel filter |
         * @details |    2    | Force narrow (84 kHz) channel filter |
         * @details |    3    | Force narrower (60 kHz) channel filter |
         * @details |    4    | Force narrowest (40 kHz) channel filter |
         *
         * @param filter_value
         */
        si4735.setFmBandwidth(config.data.bwIdxFM);
    }
}

/**
 * AGC beállítása
 */
void Band::checkAGC() {

    si4735.getAutomaticGainControl();

    if (si4735.isAgcEnabled()) {

        if (config.data.AGCgain == 2) {
            si4735.setAutomaticGainControl(1, rtv::currentAGCgain);

        } else if (config.data.AGCgain == 0) {
            si4735.setAutomaticGainControl(1, 0);  // disabled
        }

    } else if (config.data.AGCgain == 1) {

        si4735.setAutomaticGainControl(0, 0);  // enabled
    }
}

/**
 * Band inicializálása
 */
void Band::BandInit() {

    if (bandTable[config.data.bandIdx].bandType == FM_BAND_TYPE) {
        DEBUG("Start in FM\n");
        si4735.setup(PIN_SI4735_RESET, FM_BAND_TYPE);
        si4735.setFM();
    } else {
        DEBUG("Start in AM\n");
        si4735.setup(PIN_SI4735_RESET, MW_BAND_TYPE);
        si4735.setAM();
    }
}

/**
 * Band beállítása
 */
void Band::BandSet() {
    if (bandTable[config.data.bandIdx].bandType == FM_BAND_TYPE) {
        currentMode = FM;  // only mod FM in FM band
    }

    if ((currentMode == AM) or (currentMode == FM)) {
        ssbLoaded = false;  // FIXME: Ez kell? Band váltás után megint be kell tölteni az SSB-t?
    }

    if ((currentMode == LSB) or (currentMode == USB)) {
        if (ssbLoaded == false) {
            loadSSB();
        }
    }

    useBand();
    setBandWidth();
    checkAGC();

    currentMode = bandTable[config.data.bandIdx].prefmod;
}
