#include "FreqScanDisplay.h"

#include <Arduino.h>

#include <cmath>  // std::pow, round, fmod használatához

/**
 * Konstruktor
 */
FreqScanDisplay::FreqScanDisplay(TFT_eSPI &tft, SI4735 &si4735, Band &band) : DisplayBase(tft, si4735, band) {

    DEBUG("FreqScanDisplay::FreqScanDisplay\n");

    // Vektorok inicializálása a megfelelő méretre
    // Kezdetben a spektrum aljával (max Y) töltjük fel, ami a minimális jelet jelenti
    scanValueRSSI.resize(spectrumWidth, spectrumEndY);
    scanValueSNR.resize(spectrumWidth, 0);
    scanMark.resize(spectrumWidth, false);
    scanScaleLine.resize(spectrumWidth, 0);

    // Szkenneléshez releváns gombok definiálása
    DisplayBase::BuildButtonData horizontalButtonsData[] = {
        {"Start", TftButton::ButtonType::Pushable, TftButton::ButtonState::Off},  {"Stop", TftButton::ButtonType::Pushable, TftButton::ButtonState::Disabled},  // Kezdetben tiltva
        {"Pause", TftButton::ButtonType::Toggleable, TftButton::ButtonState::On},  // Kezdetben szünetel
        {"Scale", TftButton::ButtonType::Pushable, TftButton::ButtonState::Off},  {"Back", TftButton::ButtonType::Pushable, TftButton::ButtonState::Off},
    };

    // Létrehozzuk a CSAK ehhez a képernyőhöz tartozó gombokat.
    // A kötelező gombokat (pl. Scan) a Base osztály kezeli, itt nem kellenek.
    buildHorizontalScreenButtons(horizontalButtonsData, ARRAY_ITEM_COUNT(horizontalButtonsData), false);

    // Vertikális gombok (a Base osztályból jönnek a kötelezőek)
    buildVerticalScreenButtons(nullptr, 0, false);  // Nincs egyedi vertikális gombunk, nem kellenek a defaultak sem
}

/**
 * Destruktor
 */
FreqScanDisplay::~FreqScanDisplay() {
    DEBUG("FreqScanDisplay::~FreqScanDisplay\n");
    // A vektorok automatikusan felszabadulnak.
}

/**
 * Képernyő kirajzolása
 */
void FreqScanDisplay::drawScreen() {
    tft.setFreeFont();
    tft.fillScreen(TFT_COLOR_BACKGROUND);
    tft.setTextFont(2);  // Vagy a használni kívánt font

    // Státuszsor kirajzolása (örökölt)
    dawStatusLine();

    // Gombok kirajzolása (örökölt)
    drawScreenButtons();

    // Szkenneléshez szükséges kezdeti értékek beállítása az aktuális sávból
    BandTable &currentBand = band.getCurrentBand();
    startFrequency = currentBand.pConstData->minimumFreq;
    endFrequency = currentBand.pConstData->maximumFreq;

    // Kezdeti lépésköz beállítása
    if (autoScanStep) {
        scanStep = std::max(minScanStep, std::min(maxScanStep, float(endFrequency - startFrequency) / spectrumWidth));
    } else {
        scanStep = (minScanStep + maxScanStep) / 2.0f;
    }

    currentFrequency = startFrequency;     // Kezdő frekvencia beállítása
    posScanFreq = startFrequency;          // Szkennelési frekvencia is a kezdő
    deltaScanLine = spectrumWidth / 2.0f;  // Inicializálás, hogy a bal szél a startFrequency legyen
    currentScanLine = spectrumX;           // Kezdő kurzor pozíció a spektrum bal szélén
    scanEmpty = true;
    scanPaused = true;  // Kezdetben szünetel
    scanning = false;   // Kezdetben nem szkennelünk
    prevTouchedX = -1;
    prevRssiY = spectrumEndY;  // Kezdetben a legalján van

    // Spektrum alapjának és szövegeinek kirajzolása
    drawScanGraph(true);  // true = törölje a korábbi adatokat
    drawScanText(true);   // true = minden szöveget rajzoljon ki

    // Kurzor (kezdeti pozíció) - piros vonal, ha szünetel
    // A kurzor pozícióját a frekvencia alapján számoljuk
    currentScanLine = spectrumX + (currentFrequency - startFrequency) / scanStep - deltaScanLine + (spectrumWidth / 2.0f);
    currentScanLine = constrain(currentScanLine, spectrumX, spectrumEndScanX - 1);
    if (scanPaused) {
        int currentX = static_cast<int>(currentScanLine);
        if (currentX >= spectrumX && currentX < spectrumEndScanX) {
            tft.drawFastVLine(currentX, spectrumY, spectrumHeight, TFT_RED);
        }
    }

    // Aktuális frekvencia kiírása (kezdeti)
    drawScanText(false);  // Ez már a currentFrequency-t írja ki

    // Aktuális RSSI/SNR kiírása (kezdeti)
    displayScanSignal();
}

/**
 * Esemény nélküli display loop -> Szkennelés futtatása
 */
void FreqScanDisplay::displayLoop() {
    // Ha van dialóg, nem csinálunk semmit
    if (pDialog != nullptr) {
        return;
    }

    if (scanning && !scanPaused) {
        // --- Szkennelési logika ---
        int d = 0;

        // Következő pozíció kiszámítása
        posScan = static_cast<int>((posScanFreq - startFrequency) / scanStep - deltaScanLine + (spectrumWidth / 2.0f));
        int xPos = spectrumX + posScan;

        // --- ÚJ: Ellenőrzés, hogy az első posScan kiesik-e a tartományból scanEmpty esetén ---
        if (scanEmpty && (posScan < 0 || posScan >= spectrumWidth)) {
            DEBUG("Initial posScan (%d) out of bounds after scale change, skipping first point.\n", posScan);
            // Nem mérünk/rajzolunk, csak lépünk a következő frekvenciára
            freqUp();
            // scanEmpty igaz marad, a következő ciklusban újra próbálkozunk
        } else {
            // --- Eredeti/Módosított Határellenőrzés ---
            bool setf = false;
            if (!scanEmpty) {  // Csak akkor alkalmazzuk az átugrási logikát, ha már vannak adataink
                // A scanBeginBand és scanEndBand értékeket a drawScanLine számolja ki
                if (posScan >= spectrumWidth || posScan >= scanEndBand) {
                    posScan = scanBeginBand + 1;
                    if (posScan < 0) posScan = 0;  // Biztosítjuk, hogy ne legyen negatív
                    setf = true;
                } else if (posScan < 0 || posScan <= scanBeginBand) {
                    posScan = scanEndBand - 1;
                    if (posScan >= spectrumWidth) posScan = spectrumWidth - 1;  // Biztosítjuk, hogy a határon belül legyen
                    setf = true;
                }
            }
            // --- Határellenőrzés vége ---

            if (setf) {  // Ez az ág csak akkor fut le, ha !scanEmpty és határt léptünk
                // Újrahangolás ugrás miatt
                posScanFreq = startFrequency + static_cast<uint16_t>((posScan - (spectrumWidth / 2.0f) + deltaScanLine) * scanStep);
                setFreq(posScanFreq);
                xPos = spectrumX + posScan;  // xPos frissítése
                // Az ugrás utáni első pontot nem mérjük/rajzoljuk ebben a ciklusban,
                // a következő ciklusban a frissített posScanFreq alapján fogunk mérni.
            } else {  // Ez az ág fut le, ha scanEmpty (és határon belül voltunk) VAGY ha !scanEmpty és nem léptünk határt
                // --- Jelerősség mérése ---
                int rssi_val = getSignal(true);
                int snr_val = getSignal(false);

                // Értékek tárolása a megfelelő indexen
                // Biztosítjuk, hogy posScan érvényes legyen a vektorokhoz
                if (posScan >= 0 && posScan < spectrumWidth) {
                    scanValueRSSI[posScan] = static_cast<uint8_t>(rssi_val);
                    scanValueSNR[posScan] = static_cast<uint8_t>(snr_val);
                    scanMark[posScan] = (scanValueSNR[posScan] >= scanMarkSNR);

                    // Ha ez az első érvényes adatpont, jelezzük, hogy a spektrum már nem üres
                    if (scanEmpty) {
                        scanEmpty = false;
                        DEBUG("First valid scan data point acquired, scanEmpty set to false.\n");
                    }

                    // --- Rajzolás ---
                    // xPos itt már a helyes posScan alapján van
                    drawScanLine(xPos);

                } else {
                    DEBUG("Error: posScan (%d) invalid for vector access in displayLoop.\n", posScan);
                }

                drawScanText(false);  // Frekvencia frissítése

                // --- Következő frekvencia ---
                freqUp();

                posScanLast = posScan;
            }
        }  // --- ÚJ else ág vége (azaz az első posScan rendben volt) ---
    }  // --- if (scanning && !scanPaused) vége ---
}  // --- displayLoop vége ---

/**
 * Rotary encoder esemény lekezelése
 */
bool FreqScanDisplay::handleRotary(RotaryEncoder::EncoderState encoderState) {
    // Ha szünetel a szkennelés, a forgatással a kiválasztott frekvenciát hangoljuk
    if (scanPaused && encoderState.direction != RotaryEncoder::Direction::None) {
        uint16_t step = band.getCurrentBand().varData.currStep;  // Aktuális sáv lépésköze
        if (encoderState.direction == RotaryEncoder::Direction::Up) {
            currentFrequency += step;
        } else {
            currentFrequency -= step;
        }
        // Határok ellenőrzése
        currentFrequency = constrain(currentFrequency, startFrequency, endFrequency);
        setFreq(currentFrequency);  // Rádió hangolása

        // Kurzor mozgatása a spektrumon
        // A currentScanLine most már a frekvenciához igazodik, nem csak X koordináta
        currentScanLine = spectrumX + (currentFrequency - startFrequency) / scanStep - deltaScanLine + (spectrumWidth / 2.0f);
        currentScanLine = constrain(currentScanLine, spectrumX, spectrumEndScanX - 1);

        // Újrarajzolás (teljes grafikon kellhet a kurzor miatt)
        drawScanGraph(false);  // Ne törölje az adatokat, de rajzolja újra a vonalakat
        drawScanText(true);    // Szövegek frissítése
        displayScanSignal();   // RSSI/SNR frissítése

        // Piros kurzor kirajzolása az új helyre
        int currentX = static_cast<int>(currentScanLine);
        if (currentX >= spectrumX && currentX < spectrumEndScanX) {
            tft.drawFastVLine(currentX, spectrumY, spectrumHeight, TFT_RED);
        }

        // Előző érintés törlése, ha volt
        prevTouchedX = -1;

        return true;  // Kezeltük az eseményt
    }
    // TODO: Implementálhatnánk a skála vagy lépésköz váltást is forgatással + gombnyomással
    return false;  // Nem kezeltük az eseményt
}

/**
 * Touch (nem képernyő button) esemény lekezelése
 */
bool FreqScanDisplay::handleTouch(bool touched, uint16_t tx, uint16_t ty) {
    // Csak a spektrum területén lévő érintéseket kezeljük
    if (ty < spectrumY || ty > spectrumEndY || tx < spectrumX || tx > spectrumEndScanX) {
        // Ha az érintés megszűnt, és volt előző jelölés, töröljük
        if (!touched && prevTouchedX != -1) {
            drawScanLine(prevTouchedX, true);  // Eredeti vonal visszaállítása (isErasing = true)
            prevTouchedX = -1;
        }
        return false;
    }

    // Ha fut a szkennelés, az érintés a signalScale-t állítja (sample.cpp logika)
    if (scanning && !scanPaused) {
        if (touched && !scanEmpty) {  // Csak ha van adat
            Utils::beepTick();

            int d = 0;               // screenV nem releváns
            int tmpMax = spectrumY;  // Legmagasabb pont (legalacsonyabb Y érték)
            float tmpMid = 0;
            int count = 0;
            for (int i = 0; i < spectrumWidth; i++) {
                if (scanValueRSSI[i] < spectrumEndY) {  // Csak az érvényes (nem háttér) értékeket vesszük
                    tmpMid += (spectrumEndY - scanValueRSSI[i]);
                    if (scanValueRSSI[i] < tmpMax) tmpMax = scanValueRSSI[i];
                    count++;
                }
            }
            if (count > 0) {
                tmpMid = (spectrumHeight * 0.7f) / (tmpMid / count);                                              // Átlagos magassághoz viszonyított skála
                if ((spectrumEndY - ((spectrumEndY - tmpMax) * tmpMid)) < (spectrumY + spectrumHeight * 0.1f)) {  // Ne legyen túl nagy
                    tmpMid = (spectrumHeight * 0.9f) / float(spectrumEndY - tmpMax);
                }

                if (tmpMid > 0.1f && tmpMid < 10.0f) {                                 // Ésszerű határok között
                    if ((signalScale * tmpMid) > 10.0f) tmpMid = 10.0f / signalScale;  // Max nagyítás korlát
                    if ((signalScale * tmpMid) < 0.1f) tmpMid = 0.1f / signalScale;    // Min nagyítás korlát

                    signalScale *= tmpMid;
                    DEBUG("New signal scale: %.2f\n", signalScale);

                    // RSSI értékek újraszámolása és grafikon újrarajzolása
                    for (int i = 0; i < spectrumWidth; i++) {
                        if (scanValueRSSI[i] < spectrumEndY) {  // Csak az érvényeseket skálázzuk
                            // Visszaalakítjuk az eredeti RSSI-szerű értéket, skálázzuk, majd vissza Y-ná
                            float original_rssi_like = (spectrumEndY - scanValueRSSI[i]) / (signalScale / tmpMid);  // Előző skálával osztunk
                            scanValueRSSI[i] = constrain(spectrumEndY - static_cast<int>(original_rssi_like * signalScale), spectrumY, spectrumEndY);
                        }
                    }
                    drawScanGraph(false);  // Újrarajzolás az új skálával
                    drawScanText(true);    // Szövegek is kellenek
                }
            }
        }
        return true;  // Kezeltük az érintést (skála állítás)
    }

    // --- Frekvencia kiválasztása érintésre (ha szünetel) ---
    if (touched && scanPaused) {
        // 1. Töröljük az előző sárga jelölő vonalat (ha volt)
        if (prevTouchedX != -1) {
            drawScanLine(prevTouchedX, true);  // Visszarajzolja az eredeti oszlopot/vonalat (isErasing = true)
        }

        // 2. Kiszámítjuk az érintett X koordinátának megfelelő frekvenciát
        // Figyelembe vesszük az eltolást (deltaScanLine)
        // Az X koordinátából számolunk indexet, majd abból frekvenciát
        int n = tx - spectrumX;
        float touchedFreqFloat = startFrequency + static_cast<float>((n - (spectrumWidth / 2.0f) + deltaScanLine) * scanStep);
        uint16_t touchedFrequency = static_cast<uint16_t>(touchedFreqFloat);

        // Korlátok közé szorítjuk
        touchedFrequency = constrain(touchedFrequency, startFrequency, endFrequency);

        // 3. Beállítjuk az aktuális frekvenciát és a kurzor X pozícióját
        currentFrequency = touchedFrequency;
        currentScanLine = tx;  // Az érintés X koordinátája lesz a kurzor

        // 4. Ráhangoljuk a rádiót
        setFreq(currentFrequency);

        // 5. Kirajzoljuk a sárga jelölő vonalat az új pozícióra
        int currentX = static_cast<int>(currentScanLine);
        if (currentX >= spectrumX && currentX < spectrumEndScanX) {
            tft.drawFastVLine(currentX, spectrumY, spectrumHeight, TFT_YELLOW);
            prevTouchedX = currentX;  // Elmentjük az új pozíciót
        } else {
            prevTouchedX = -1;  // Ha kívül esik, nincs előző pozíció
        }

        // 6. Frissítjük a kijelzéseket
        displayScanSignal();  // RSSI/SNR
        drawScanText(false);  // Aktuális frekvencia kiírása

        return true;  // Kezeltük az érintést
    }

    return false;  // Nem kezeltük
}

/**
 * Képernyő menügomb esemény feldolgozása
 */
void FreqScanDisplay::processScreenButtonTouchEvent(TftButton::ButtonTouchEvent &event) {
    DEBUG("FreqScanDisplay::processScreenButtonTouchEvent() -> id: %d, label: %s, state: %s\n", event.id, event.label, TftButton::decodeState(event.state));

    if (STREQ("Start", event.label)) {
        startScan();
    } else if (STREQ("Stop", event.label)) {
        stopScan();
    } else if (STREQ("Pause", event.label)) {
        // A gomb állapota már beállt a handleTouch-ban, itt csak reagálunk rá
        scanPaused = (event.state == TftButton::ButtonState::On);
        pauseScan();  // Metódus hívása a szükséges műveletekhez
    } else if (STREQ("Scale", event.label)) {
        changeScanScale();
    } else if (STREQ("Back", event.label)) {
        stopScan();  // Leállítjuk a szkennelést, mielőtt visszalépünk
        // Visszalépés az előző képernyőre (FM vagy AM)
        ::newDisplay = band.getCurrentBandType() == FM_BAND_TYPE ? DisplayBase::DisplayType::fm : DisplayBase::DisplayType::am;
    }
    // TODO: Esetleges további gombok kezelése (pl. Step)
}

// --- Private Helper Methods ---

/**
 * Szkennelés indítása
 */
void FreqScanDisplay::startScan() {
    if (scanning) return;  // Már fut

    DEBUG("Starting scan...\n");
    scanning = true;
    scanPaused = false;             // Indításkor nem szünetel
    scanEmpty = true;               // Új szkennelés, töröljük az adatokat
    scanAGC = config.data.agcGain;  // Mentsük el az AGC állapotát

    // Állítsuk be a "Pause" gombot Off állapotba
    TftButton *pauseButton = findButtonByLabel("Pause");
    if (pauseButton) pauseButton->setState(TftButton::ButtonState::Off);
    // Stop gomb engedélyezése, Start tiltása
    TftButton *startButton = findButtonByLabel("Start");
    if (startButton) startButton->setState(TftButton::ButtonState::Disabled);
    TftButton *stopButton = findButtonByLabel("Stop");
    if (stopButton) stopButton->setState(TftButton::ButtonState::Off);  // Off = enabled but not pushed

    // Kezdő frekvencia és pozíció beállítása
    currentFrequency = startFrequency;
    posScanFreq = startFrequency;
    posScan = 0;
    posScanLast = -1;  // Érvénytelen előző pozíció
    deltaScanLine = spectrumWidth / 2.0f;
    signalScale = 1.5f;        // Alapértelmezett skála
    prevRssiY = spectrumEndY;  // Vonalrajzoláshoz reset

    // AGC kikapcsolása szkenneléshez (sample.cpp logika)
    config.data.agcGain = static_cast<uint8_t>(Si4735Utils::AgcGainMode::Off);
    checkAGC();

    // Spektrum törlése és újrarajzolása
    drawScanGraph(true);
    drawScanText(true);

    setFreq(currentFrequency);  // Első frekvencia beállítása
    si4735.setAudioMute(true);  // Némítás szkennelés alatt

    // Gombok újrarajzolása a frissített állapotokkal
    drawScreenButtons();
}

/**
 * Szkennelés leállítása
 */
void FreqScanDisplay::stopScan() {
    if (!scanning) return;  // Már le van állítva

    DEBUG("Stopping scan...\n");
    scanning = false;
    scanPaused = true;

    // Állítsuk be a "Pause" gombot On állapotba (szünetel)
    TftButton *pauseButton = findButtonByLabel("Pause");
    if (pauseButton) pauseButton->setState(TftButton::ButtonState::On);
    // Stop gomb tiltása, Start engedélyezése
    TftButton *startButton = findButtonByLabel("Start");
    if (startButton) startButton->setState(TftButton::ButtonState::Off);  // Off = enabled
    TftButton *stopButton = findButtonByLabel("Stop");
    if (stopButton) stopButton->setState(TftButton::ButtonState::Disabled);

    // AGC visszaállítása
    config.data.agcGain = scanAGC;
    checkAGC();

    // Hang visszaállítása (ha némítva volt)
    si4735.setAudioMute(rtv::muteStat);  // Vissza az eredeti némítási állapotba

    // Utolsó ismert frekvencia beállítása (ahol a kurzor van)
    setFreq(currentFrequency);

    // Piros kurzor kirajzolása az aktuális frekvenciára
    currentScanLine = spectrumX + (currentFrequency - startFrequency) / scanStep - deltaScanLine + (spectrumWidth / 2.0f);
    currentScanLine = constrain(currentScanLine, spectrumX, spectrumEndScanX - 1);
    int currentX = static_cast<int>(currentScanLine);
    if (currentX >= spectrumX && currentX < spectrumEndScanX) {
        tft.drawFastVLine(currentX, spectrumY, spectrumHeight, TFT_RED);
    }
    displayScanSignal();  // RSSI/SNR frissítése

    DEBUG("Scan stopped at %d kHz\n", currentFrequency);

    // Gombok újrarajzolása a frissített állapotokkal
    drawScreenButtons();
}

/**
 * Szkennelés szüneteltetése/folytatása
 */
void FreqScanDisplay::pauseScan() {
    DEBUG("Scan %s\n", scanPaused ? "paused" : "resumed");
    if (scanPaused) {
        // AGC visszaállítása az eredetire
        config.data.agcGain = scanAGC;
        checkAGC();
        // Hang visszaállítása
        si4735.setAudioMute(rtv::muteStat);
        // Aktuális sáv lépésközének visszaállítása
        uint8_t step = band.getCurrentBand().varData.currStep;
        si4735.setFrequencyStep(step);
        // Frekvencia beállítása a kurzor pozíciójára
        setFreq(currentFrequency);

        // Kurzor pozíciójának kiszámítása
        currentScanLine = spectrumX + (currentFrequency - startFrequency) / scanStep - deltaScanLine + (spectrumWidth / 2.0f);
        currentScanLine = constrain(currentScanLine, spectrumX, spectrumEndScanX - 1);

        // Piros kurzor kirajzolása
        int currentX = static_cast<int>(currentScanLine);
        if (currentX >= spectrumX && currentX < spectrumEndScanX) {
            tft.drawFastVLine(currentX, spectrumY, spectrumHeight, TFT_RED);
            // prevTouchedX = currentX; // Nem kell, mert a piros vonal a kurzor
        } else {
            prevTouchedX = -1;
        }
        displayScanSignal();  // RSSI/SNR frissítése

    } else {  // Folytatás
        // AGC kikapcsolása szkenneléshez
        scanAGC = config.data.agcGain;  // Mentsük el újra, hátha közben változott
        config.data.agcGain = static_cast<uint8_t>(Si4735Utils::AgcGainMode::Off);
        checkAGC();
        // Hang némítása
        si4735.setAudioMute(true);
        // Lépésköz 1kHz-re állítása (vagy a scanStep-nek megfelelőre?) - sample.cpp 1-et használ
        si4735.setFrequencyStep(1);  // Vagy scanStep? sample.cpp 1-et használ
        // Frekvencia beállítása a következő szkennelési pontra
        setFreq(posScanFreq);
        // Piros/Sárga kurzor eltüntetése (eredeti vonal visszaállítása)
        int currentX = static_cast<int>(currentScanLine);
        if (prevTouchedX != -1) {  // Ha volt sárga
            drawScanLine(prevTouchedX);
            prevTouchedX = -1;
        } else if (currentX >= spectrumX && currentX < spectrumEndScanX) {  // Ha volt piros
            drawScanLine(currentX);
        }
        prevRssiY = spectrumEndY;  // Vonalrajzoláshoz reset
    }
}

/**
 * Szkennelési skála (lépésköz) váltása
 */
void FreqScanDisplay::changeScanScale() {
    DEBUG("Changing scan scale...\n");
    bool was_paused = scanPaused;
    if (scanning && !was_paused) {
        scanPaused = true;
        pauseScan();  // Szüneteltetjük a váltáshoz
    }

    float oldScanStep = scanStep;

    // Lépésköz váltása (ciklikusan)
    scanStep *= 2.0f;
    if (scanStep > maxScanStep) {
        scanStep = minScanStep;
    }
    // Biztosítjuk, hogy a minimum alatt ne menjen (lebegőpontos pontatlanság miatt)
    if (scanStep < minScanStep) {
        scanStep = minScanStep;
    }
    DEBUG("New scan step: %.3f kHz\n", scanStep);

    // Kurzor frekvenciájának megtartása mellett deltaScanLine újraszámolása
    // A képernyő közepén lévő frekvencia maradjon ugyanaz
    float freqAtCenter = startFrequency + static_cast<float>(((spectrumWidth / 2.0f) + deltaScanLine) * oldScanStep);
    deltaScanLine = (freqAtCenter - startFrequency) / scanStep - (spectrumWidth / 2.0f);

    // Újrarajzolás új skálával
    scanEmpty = true;          // Újra kell rajzolni a teljes grafikont
    prevRssiY = spectrumEndY;  // Vonalrajzoláshoz reset
    drawScanGraph(true);       // Töröljük és újrarajzoljuk a grafikont
    drawScanText(true);
    setFreq(currentFrequency);  // Frekvencia újra beállítása (kurzor pozícióhoz)

    // Szkennelés folytatása, ha futott
    if (scanning && !was_paused) {
        scanPaused = false;
        pauseScan();
    } else {
        // Ha szünetelt, a kurzort újra ki kell rajzolni pirossal
        currentScanLine = spectrumX + (currentFrequency - startFrequency) / scanStep - deltaScanLine + (spectrumWidth / 2.0f);
        currentScanLine = constrain(currentScanLine, spectrumX, spectrumEndScanX - 1);
        int currentX = static_cast<int>(currentScanLine);
        if (currentX >= spectrumX && currentX < spectrumEndScanX) {
            tft.drawFastVLine(currentX, spectrumY, spectrumHeight, TFT_RED);
        }
        displayScanSignal();
    }
}

/**
 * Spektrum alapjának és skálájának rajzolása
 * @param erase Törölje a korábbi adatokat?
 */
void FreqScanDisplay::drawScanGraph(bool erase) {
    DEBUG("Drawing scan graph (erase: %s)\n", erase ? "true" : "false");
    int d = 0;  // screenV itt nem releváns

    if (erase) {
        tft.fillRect(spectrumX, spectrumY, spectrumWidth, spectrumHeight, TFT_BLACK);  // Háttér törlése
        scanEmpty = true;
        // Vektorok törlése vagy nullázása
        std::fill(scanValueRSSI.begin(), scanValueRSSI.end(), spectrumEndY);  // Max Y érték = min jel
        std::fill(scanValueSNR.begin(), scanValueSNR.end(), 0);
        std::fill(scanMark.begin(), scanMark.end(), false);
        std::fill(scanScaleLine.begin(), scanScaleLine.end(), 0);
        // A posScanFreq-et a startScan/changeScale állítja be
        prevRssiY = spectrumEndY;  // Vonalrajzoláshoz reset
    }

    scanBeginBand = -1;
    scanEndBand = spectrumWidth;
    prevScaleLine = false;

    // Vonalak újrarajzolása (ha nem töröltünk) vagy alap skála rajzolása
    for (int n = 0; n < spectrumWidth; n++) {
        // Az erase logikát most már a drawScanLine kezeli
        drawScanLine(spectrumX + n);
    }

    // --- Sávhatár jelző vonalak rajzolása ---
    if (scanBeginBand > 0 && scanBeginBand < spectrumWidth) {                               // Ha a kezdő határ látható
        tft.drawFastVLine(spectrumX + scanBeginBand, spectrumEndY - 10, 10, TFT_DARKGREY);  // Csak alul egy 10px magas vonal
    }
    if (scanEndBand >= 0 && scanEndBand < spectrumWidth - 1) {                            // Ha a vég határ látható
        tft.drawFastVLine(spectrumX + scanEndBand, spectrumEndY - 10, 10, TFT_DARKGREY);  // Csak alul egy 10px magas vonal
    }
    // ------------------------------------------

    // --- SNR Limit Vonal (Vizuális Jelző) ---
    // Rajzolunk egy vízszintes sötétszürke vonalat az aljához közel, vizuális küszöbjelzőként.
    // Ennek a vonalnak a pozíciója nincs közvetlen matematikai kapcsolatban a scanMarkSNR értékkel.
    uint16_t snrLimitLineY = spectrumEndY - 10;  // Példa: 10 pixelre az aljától
    tft.drawFastHLine(spectrumX, snrLimitLineY, spectrumWidth, TFT_DARKGREY);
    // --- ÚJ RÉSZ VÉGE ---

    // Keret újrarajzolása
    tft.drawRect(spectrumX - 1, spectrumY - 1, spectrumWidth + 2, spectrumHeight + 2, TFT_WHITE);
}

/**
 * Egy spektrumvonal/oszlop rajzolása a megadott X pozícióra
 * @param xPos Az X koordináta a képernyőn
 * @param isErasing Ha true, akkor nem rajzolja újra a kurzort (törlési céllal hívták)
 */
void FreqScanDisplay::drawScanLine(int xPos, bool isErasing) {
    int n = xPos - spectrumX;                 // Index a vektorokban
    if (n < 0 || n >= spectrumWidth) return;  // Érvénytelen index

    int d = 0;  // screenV itt nem releváns

    // Aktuális frekvencia kiszámítása ehhez a pozícióhoz (precízebben)
    double freq_double = static_cast<double>(startFrequency) +
                         (static_cast<double>(n) - (static_cast<double>(spectrumWidth) / 2.0) + static_cast<double>(deltaScanLine)) * static_cast<double>(scanStep);
    // Kerekítés uint16_t-re konvertálás előtt
    uint16_t frq = static_cast<uint16_t>(round(freq_double));

    int16_t colf = TFT_NAVY;   // Háttérszín (gyenge jel)
    int16_t colb = TFT_BLACK;  // Előtérszín (ahol nincs jel)

    // --- Skálavonal típusának meghatározása ---
    if (!scanScaleLine[n] || scanEmpty) {
        scanScaleLine[n] = 0;
        double stepThreshold = scanStep > 0 ? static_cast<double>(scanStep) : 1.0;  // Osztás nulla ellen védve

        // Fő skálavonal (pl. 100kHz-enként)
        if (scanStep <= 100) {
            if (fmod(freq_double, 100.0) < stepThreshold || fmod(freq_double, 100.0) > (100.0 - stepThreshold)) scanScaleLine[n] = 2;
        }
        // Fél skálavonal (pl. 50kHz-enként)
        if (!scanScaleLine[n] && scanStep <= 50) {
            if (fmod(freq_double, 50.0) < stepThreshold || fmod(freq_double, 50.0) > (50.0 - stepThreshold)) scanScaleLine[n] = 3;
        }
        // Tized skálavonal (pl. 10kHz-enként)
        if (!scanScaleLine[n] && scanStep <= 10) {
            if (fmod(freq_double, 10.0) < stepThreshold || fmod(freq_double, 10.0) > (10.0 - stepThreshold)) scanScaleLine[n] = 4;
        }

        if (!scanScaleLine[n]) scanScaleLine[n] = 1;
    }

    // Színek beállítása a skálavonal típusa alapján
    if (scanScaleLine[n] == 2) {         // Fő vonal
        colf = TFT_BLACK;                // Háttér
        colb = TFT_OLIVE;                // Vonal színe
    } else if (scanScaleLine[n] == 3) {  // Fél vonal
        colb = TFT_DARKGREY;             // Halványabb vonal
    } else if (scanScaleLine[n] == 4) {  // Tized vonal
        colb = TFT_DARKGREY;             // Még halványabb
    }

    // --- Szín az SNR alapján (felülírja a skálavonal színét, ha van jel) ---
    if (scanValueSNR[n] > 0 && !scanEmpty) {
        colf = TFT_NAVY + 0x8000;
        if (scanValueSNR[n] < 16) {
            colf += (scanValueSNR[n] * 2048);
        } else {
            colf = 0xFBE0;  // Sárga
            if (scanValueSNR[n] < 24)
                colf += ((scanValueSNR[n] - 16) * 4);
            else
                colf = TFT_RED;
        }
    }

    // --- Sávon kívüli terület indexének jelölése (CSAK AZ INDEXEKET ÁLLÍTJUK) ---
    const double freqTolerance = scanStep * 0.1;  // Tolerancia a lépésköz tizede
    if (freq_double > (static_cast<double>(endFrequency) + freqTolerance)) {
        if (scanEndBand > n) scanEndBand = n;
    } else {
        if (n >= scanEndBand) scanEndBand = spectrumWidth;
    }
    if (freq_double < (static_cast<double>(startFrequency) - freqTolerance)) {
        if (scanBeginBand < n) scanBeginBand = n;
    } else {
        if (n <= scanBeginBand) scanBeginBand = -1;
    }

    // --- Rajzolás ---
    int currentRssiY = scanValueRSSI[n];
    currentRssiY = constrain(currentRssiY, spectrumY, spectrumEndY);

    // 1. Háttér rajzolása (oszlop vagy skálavonal)
    if (colb != TFT_BLACK) {
        if (scanScaleLine[n] == 2)
            tft.drawFastVLine(xPos, spectrumY, spectrumHeight, colb);
        else if (scanScaleLine[n] == 3)
            tft.drawFastVLine(xPos, spectrumY + spectrumHeight / 2, spectrumHeight / 2, colb);
        else if (scanScaleLine[n] == 4)
            tft.drawFastVLine(xPos, spectrumY + spectrumHeight * 3 / 4, spectrumHeight / 4, colb);
        if (currentRssiY < spectrumEndY && !scanEmpty) {
            tft.drawFastVLine(xPos, currentRssiY, spectrumEndY - currentRssiY, colf);
        }
    } else {
        tft.drawFastVLine(xPos, spectrumY, spectrumHeight, colb);
        if (currentRssiY < spectrumEndY && !scanEmpty) {
            tft.drawFastVLine(xPos, currentRssiY, spectrumEndY - currentRssiY, colf);
        }
    }

    // 2. Fő jelvonal (összekötve az előző ponttal)
    if (!scanEmpty) {
        if (n > 0) {
            tft.drawLine(xPos - 1, prevRssiY, xPos, currentRssiY, TFT_SILVER);
        } else {
            tft.drawPixel(xPos, currentRssiY, TFT_SILVER);
        }
    }
    if (!scanEmpty) {
        prevRssiY = currentRssiY;
    }

    // 3. Jelölő (scanMark) kirajzolása
    if (scanMark[n] && !scanEmpty) {
        tft.fillRect(xPos - 1, spectrumY + 5, 3, 5, TFT_YELLOW);
    }

    // 4. Kurzor (piros vagy sárga vonal) rajzolása
    if (!isErasing && scanPaused) {  // Csak akkor rajzol kurzort, ha nem törlési céllal hívták
        if (xPos == prevTouchedX) {  // Sárga érintésjelző
            tft.drawFastVLine(xPos, spectrumY, spectrumHeight, TFT_YELLOW);
        } else if (xPos == static_cast<int>(currentScanLine) && prevTouchedX == -1) {  // Piros kurzor, ha nincs érintés
            tft.drawFastVLine(xPos, spectrumY, spectrumHeight, TFT_RED);
        }
    }
}

/**
 * Frekvencia címkék és egyéb szövegek rajzolása
 * @param all Minden szöveget rajzoljon újra? (true = igen, false = csak a változókat)
 */
void FreqScanDisplay::drawScanText(bool all) {

    // --- A többi szöveg rajzolása  ---
    tft.setTextFont(1);  // Kisebb font a többi szöveghez
    tft.setTextSize(1);
    tft.setTextColor(TFT_SILVER, TFT_BLACK);
    tft.setTextDatum(BC_DATUM);

    // Sáv eleje/vége jelzések...
    if (all || (scanEndBand < (spectrumWidth - 5))) {
        tft.fillRect(spectrumX + scanEndBand + 3, spectrumY - 15, 40, 15, TFT_BLACK);
        if (scanEndBand < (spectrumWidth - 5)) {
            tft.setTextDatum(BL_DATUM);
            tft.drawString("END", spectrumX + scanEndBand + 5, spectrumY - 5);
        }
    }
    if (all || (scanBeginBand > 5)) {
        tft.fillRect(spectrumX + scanBeginBand - 43, spectrumY - 15, 40, 15, TFT_BLACK);
        if (scanBeginBand > 5) {
            tft.setTextDatum(BR_DATUM);
            tft.drawString("BEGIN", spectrumX + scanBeginBand - 5, spectrumY - 5);
        }
    }

    // Skála kezdő és vég frekvenciájának kiírása...
    if (all) {
        uint16_t freqStartVisible = startFrequency + static_cast<uint16_t>((deltaScanLine - (spectrumWidth / 2.0f)) * scanStep);
        uint16_t freqEndVisible = startFrequency + static_cast<uint16_t>((deltaScanLine + (spectrumWidth / 2.0f)) * scanStep);
        freqStartVisible = constrain(freqStartVisible, startFrequency, endFrequency);
        freqEndVisible = constrain(freqEndVisible, startFrequency, endFrequency);

        tft.setTextColor(TFT_GREEN, TFT_BLACK);
        tft.setTextDatum(BL_DATUM);
        tft.fillRect(spectrumX, spectrumEndY + 5, 100, 15, TFT_BLACK);
        tft.drawString(String(freqStartVisible), spectrumX, spectrumEndY + 15);  // " kHz" eltávolítva

        tft.setTextDatum(BR_DATUM);
        tft.fillRect(spectrumEndScanX - 100, spectrumEndY + 5, 100, 15, TFT_BLACK);
        tft.drawString(String(freqEndVisible), spectrumEndScanX, spectrumEndY + 15);  // " kHz" eltávolítva

        // Lépésköz kiírása...
        tft.setTextDatum(BC_DATUM);
        tft.fillRect(spectrumX + spectrumWidth / 2 - 50, spectrumEndY + 5, 100, 15, TFT_BLACK);
        tft.setTextColor(TFT_YELLOW, TFT_BLACK);
        tft.drawString("Step: " + String(scanStep, scanStep < 1.0f ? 3 : 1) + " kHz", spectrumX + spectrumWidth / 2, spectrumEndY + 15);
    }

    // --- Aktuális frekvencia kiírása (MÓDOSÍTOTT RÉSZ) ---
    uint16_t freqToDisplayRaw = (scanning && !scanPaused) ? posScanFreq : currentFrequency;
    String freqStr;

    // Mértékegység és formázás meghatározása
    if (band.getCurrentBandType() == FM_BAND_TYPE) {
        // FM: MHz-ben, 2 tizedesjeggyel
        float freqMHz = freqToDisplayRaw / 100.0f;
        freqStr = String(freqMHz, 2);
    } else {
        // AM/LW/MW/SW: kHz-ben, tizedesjegy nélkül
        freqStr = String(freqToDisplayRaw);
    }

    // Nagyobb font visszaállítása a fő frekvenciához
    tft.setTextFont(2);
    tft.setTextDatum(TL_DATUM);  // Bal felső igazítás

    // "Freq: " címke kiírása (ezt nem töröljük)
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.drawString("Freq: ", 5, 25);

    // Kiszámoljuk a frekvencia érték helyét és töröljük azt (a mértékegység helyét is)
    uint16_t valueStartX = 5 + tft.textWidth("Freq: ");  // Ahol az érték kezdődik
    // Szélesebb törlés, hogy a régi mértékegység is biztosan eltűnjön
    tft.fillRect(valueStartX, 20, 100, 20, TFT_BLACK);  // Törlés 100 px szélességben

    // Új érték kiírása (mértékegység nélkül)
    tft.drawString(freqStr, valueStartX, 25);
}

/**
 * Aktuális RSSI/SNR kiírása a spektrum fölé
 */
void FreqScanDisplay::displayScanSignal() {
    int d = 0;  // screenV nem releváns
    int xPos = static_cast<int>(currentScanLine);
    int n = xPos - spectrumX;

    // Csak akkor írunk ki, ha a kurzor a spektrumon belül van
    bool cursorVisible = (xPos >= spectrumX && xPos < spectrumEndScanX);

    tft.setTextFont(1);  // Explicit kisebb font beállítása
    tft.setTextSize(1);  // Méret beállítása a font után
    tft.setTextDatum(BC_DATUM);
    // Középen fent töröljük a régi értéket
    // A törlési területet kicsit megnöveljük, biztos ami biztos
    tft.fillRect(spectrumX + spectrumWidth / 2 - 60, spectrumY - 15, 120, 15, TFT_BLACK);  // Szélesebb törlés

    if (scanPaused && cursorVisible) {  // Ha szünetel, az aktuális mérést írjuk ki
        si4735.getCurrentReceivedSignalQuality();
        tft.setTextColor(TFT_WHITE, TFT_BLACK);
        // Kicsit szűkebbre vesszük a szövegeket, hogy biztos elférjenek
        tft.drawString("RSSI:" + String(si4735.getCurrentRSSI()), spectrumX + spectrumWidth / 2 - 30, spectrumY - 5);
        tft.setTextColor(TFT_ORANGE, TFT_BLACK);
        tft.drawString("SNR:" + String(si4735.getCurrentSNR()), spectrumX + spectrumWidth / 2 + 30, spectrumY - 5);
    } else if (!scanEmpty && cursorVisible && n >= 0 && n < spectrumWidth) {  // Ha fut és van adat, a tárolt értéket írjuk ki
        // Az RSSI érték visszaalakítása a skálázott Y koordinátából
        int displayed_rssi = static_cast<int>((spectrumEndY - scanValueRSSI[n]) / signalScale);

        tft.setTextColor(TFT_WHITE, TFT_BLACK);
        tft.drawString("RSSI:" + String(displayed_rssi), spectrumX + spectrumWidth / 2 - 30, spectrumY - 5);
        tft.setTextColor(TFT_ORANGE, TFT_BLACK);
        tft.drawString("SNR:" + String(scanValueSNR[n]), spectrumX + spectrumWidth / 2 + 30, spectrumY - 5);
    }
    // Visszaállítjuk az alapértelmezett fontot, ha máshol mást használunk
    // tft.setTextFont(2); // Vagy amit a drawScreen-ben beállítottál
}

/**
 * Jelerősség (RSSI vagy SNR) lekérése (átlagolással)
 * @param rssi True esetén RSSI-t, false esetén SNR-t ad vissza.
 * @return Az átlagolt jelerősség (RSSI esetén már Y koordinátává alakítva).
 */
int FreqScanDisplay::getSignal(bool rssi) {
    int res = 0;
    for (int i = 0; i < countScanSignal; i++) {
        si4735.getCurrentReceivedSignalQuality();  // Új mérés kérése
        if (rssi)
            res += si4735.getCurrentRSSI();
        else
            res += si4735.getCurrentSNR();
        // Rövid várakozás lehet szükséges a mérések között?
        // delayMicroseconds(100); // Opcionális
    }
    res /= countScanSignal;  // Átlagolás

    // Ha RSSI-t kértünk, alakítsuk át Y koordinátává a sample.cpp logika szerint
    if (rssi) {
        // res = 198 + (screenV * 40) - ((res) * signalScale); // screenV itt 0
        res = spectrumEndY - static_cast<int>(static_cast<float>(res) * signalScale);
        res = constrain(res, spectrumY, spectrumEndY);  // Korlátok közé szorítás (Y koordináta!)
    }

    return res;
}

/**
 * Frekvencia beállítása és kapcsolódó műveletek
 * @param f A beállítandó frekvencia (kHz).
 */
void FreqScanDisplay::setFreq(uint16_t f) {
    // Nem engedjük a sávon kívülre állítani
    f = constrain(f, startFrequency, endFrequency);

    posScanFreq = f;  // Tároljuk a szkennelési frekvenciát is
    if (scanPaused) {
        currentFrequency = f;  // Ha szünetel, az aktuális frekvencia is ez lesz
    }

    si4735.setFrequency(f);
    // Az AGC-t csak akkor állítjuk, ha szkennelünk és nem szünetelünk
    if (scanning && !scanPaused) {
        // AGC letiltása (1 = disabled)
        si4735.setAutomaticGainControl(1, 0);  // Explicit letiltás
    }
    // Rövid várakozás a stabilizálódáshoz (a setFrequency már tartalmazhat ilyet)
    // delay(MAX_DELAY_AFTER_SET_FREQUENCY); // Ha szükséges
}

/**
 * Frekvencia léptetése felfelé (a scanStep alapján)
 */
void FreqScanDisplay::freqUp() {
    // Itt nem a si4735.frequencyUp()-ot használjuk, mert a lépésköz a scanStep
    posScanFreq += static_cast<uint16_t>(scanStep);
    if (posScanFreq > endFrequency) {
        posScanFreq = startFrequency;  // Túlcsordulás esetén vissza az elejére
    }
    setFreq(posScanFreq);  // Beállítjuk az új frekvenciát
}
