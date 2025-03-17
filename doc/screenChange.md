# Kód Elemzés - TFT-SPI Pico Rádió Projekt

## Bevezetés

Ez a dokumentum a TFT-SPI Pico Rádió projekt kódjának részletes elemzését tartalmazza, különös tekintettel a képernyőváltás biztonságos megvalósítására. A projekt célja, hogy egy Pico mikrokontrollerrel és egy TFT kijelzővel egy rádió kezelőfelületét hozza létre.

## A Képernyőváltás Biztonságos Megoldása

A projekt egyik központi eleme a különböző képernyők közötti váltás (pl. FM, AM, FreqScan). A fejlesztő egy nagyon elegáns és biztonságos megoldást alkalmazott, melynek főbb elemei az alábbiak:

### 1. `displayChangeType` Globális Változó

*   **Definíció:** A `pico-radio.ino` fájlban definiált `DisplayBase::DisplayType` típusú enum változó.
*   **Szerepe:** Jelzőként működik, tárolja, hogy milyen típusú képernyőre kell átváltani.
*   **Kezdőérték:** Induláskor `DisplayBase::DisplayType::FmDisplayType` (FM képernyő).
* **Elérhetőség**: A `DisplayBase.h` fájlban `extern` deklarációval érhető el más fájlokból.

### 2. `changeDisplay()` Függvény

*   **Definíció:** A `pico-radio.ino` fájlban definiált globális függvény.
*   **Szerepe:** A tényleges képernyőváltást végzi.
*   **Hívása:** A `loop()` függvény elején kerül meghívásra, **de csak akkor**, ha a `displayChangeType` értéke nem `DisplayBase::DisplayType::noneDisplayType`.
*   **Működése:**
    1.  Törli az előző képernyőt (`delete pDisplay;`).
    2.  Létrehozza az új képernyőt (`new FmDisplay(tft);`, `new AmDisplay(tft);`, `new FreqScanDisplay(tft);`).
    3.  Meghívja az új képernyő `drawScreen()` metódusát.
    4. A végén a `displayChangeType` értékét `DisplayBase::DisplayType::noneDisplayType`-ra állítja.

### 3. `processScreenButtonTouchEvent()` Metódusok

*   **Definíció:** Az `FmDisplay.cpp`, `AmDisplay.cpp`, `FreqScanDisplay.cpp` fájlokban definiált metódusok.
*   **Szerepe:** A képernyőn lévő gombok megnyomásakor fut le.
*   **Működése:** Beállítja a `displayChangeType` értékét a megfelelő `DisplayBase::DisplayType`-ra, ha egy képernyőváltást kiváltó gombot nyomtak meg.

### 4. `loop()` Függvény (Fő Ciklus)

*   **Szerepe:** Az Arduino program fő ciklusa.
*   **Működése:**
    1.  A ciklus elején vizsgálja, hogy a `displayChangeType` értéke `DisplayBase::DisplayType::noneDisplayType`-e.
    2.  Ha nem, akkor meghívja a `changeDisplay()` függvényt.
    3.  A `try-catch` blokkban meghívja a `pDisplay->loop(encoderState)` metódust.

## A Biztonság Kulcselemei

A megoldás biztonságát a következő kulcselemek biztosítják:

*   **Központi Képernyőváltás Kezelő:** A `changeDisplay()` függvény egy központi helyen kezeli a képernyőváltást. Ez elkerüli a többszörös törlés és létrehozás hibáit.
*   **`displayChangeType` Jelző:** A `displayChangeType` változó csak jelzi az igényt a képernyőváltásra. A tényleges váltás csak a `loop()` függvény elején történik.
*   **Szinkronizáció:** A `displayChangeType` változó figyelése a `loop()`-ban biztosítja, hogy a váltás mindig szinkronizáltan történjen.
*   **A képernyőváltás nem rekurzív:** A `changeDisplay()` függvény soha nem hívja meg önmagát.
* **A `changeDisplay` csak egyszer fut le**: A `displayChangeType` értékének visszaállítása után nem fog újra lefutni a `changeDisplay()`.
* **Nincs versenyhelyzet**: Mivel a képernyőváltást mindig a fő `loop()` függvény kezeli, nem lesznek versenyhelyzetek.
* **Nincs memória szivárgás**: A `delete pDisplay` mindig lefut a létrehozás előtt.

## Kódstruktúra és Osztályok

### `DisplayBase` Osztály

*   **Szerepe:** Az összes képernyő alaposztálya, közös funkcionalitásokat tartalmaz.
*   **Tagváltozók:**
    *   `tft`: A TFT kijelző objektuma.
    *   `pDialog`: A dialógus objektumra mutató pointer.
    *   `screenButtons`: A képernyőgombok tömbjére mutató pointer.
    *   `screenButtonsCount`: A képernyőgombok száma.
    *   `screenButtonTouchEvent`, `dialogButtonResponse`: A megnyomott gombok eseményei.
*   **Metódusok:**
    *   `drawScreen()`: A képernyő kirajzolását végző absztrakt metódus.
    *   `processScreenButtonTouchEvent()`: A képernyőgombok eseményeinek feldolgozása.
    *   `processDialogButtonResponse()`: A dialógusgombok eseményeinek feldolgozása.
    *   `loop()`: A fő loop függvényből meghívott metódus, ami kezeli a touch eseményeket, a rotary-t, és a gombokat.
    * `handleRotary()`, `handleTouch()`: Absztrakt metódusok a rotary és touch események kezelésére.

### `FmDisplay`, `AmDisplay`, `FreqScanDisplay` Osztályok

*   **Szerep:** A különböző képernyők megvalósításai, amelyek öröklik a `DisplayBase` osztályt.
*   **Metódusok:**
    *   `drawScreen()`: A képernyő tartalmának kirajzolása.
    *   `processScreenButtonTouchEvent()`: A képernyőgombok eseményeinek kezelése.
    * `handleRotary()`, `handleTouch()`: A rotary és touch események kezelése.

### `pico-radio.ino` (Fő Fájl)

*   **Szerepe:** Az Arduino program fő fájlja.
*   **Tartalma:**
    *   `setup()`: Az inicializálási műveletek.
    *   `loop()`: A fő ciklus, amely a program futását vezérli.
    *   `changeDisplay()`: A képernyőváltást végző függvény.
    *   `displayChangeType`: A képernyőváltást jelző változó.
    * Globális objektumok, mint `tft`, `rotaryEncoder`, `config`.
* **Includok:**
    * `<Arduino.h>`, `<TFT_eSPI.h>`
    * `"utils.h"`, `"RotaryEncoder.h"`, `"Config.h"`, `"PicoMemoryInfo.h"`
    * `"AmDisplay.h"`, `"FmDisplay.h"`, `"FreqScanDisplay.h"`

### `*.cpp` fájlok

* **Tartalom**: Az osztályok metódusainak definíciói.
* **Includok**:
    * `FmDisplay.h`, `AmDisplay.h`, `FreqScanDisplay.h`, stb.
    * `<Arduino.h>`
* A `.cpp` fájlok nem includolják egymást.

### `*.h` fájlok

* **Tartalom**: Az osztályok definíciói.
* **Includok**:
    * `<Arduino.h>`, `<TFT_eSPI.h>`
    * `"DialogBase.h"`, `"IDialogParent.h"`, `"IGuiEvents.h"`, `"TftButton.h"`, `"utils.h"`, stb.
* A `.h` fájlok nem includolják egymást.

## Összegzés

A fejlesztő egy nagyon átgondolt és biztonságos megoldást implementált a képernyőváltásra. A `displayChangeType` változó és a `changeDisplay()` függvény közös használatával, valamint a `loop()` függvényben történő szinkronizálással hatékonyan kezeli a képernyők közötti váltást. A kód strukturált, moduláris, és jól követi az objektumorientált programozás elveit. A program hibátlanul működik.

A megoldás egyszerű, de hatékony, és elkerüli a bonyolultabb, hibalehetőséget rejtő megközelítéseket. A kód átlátható, karbantartható, és könnyen bővíthető új képernyőtípusokkal.

