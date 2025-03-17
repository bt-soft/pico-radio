# `pico-radio.ino` - Fő Programfájl Dokumentáció
(Gemini Code Assist)

## Bevezetés

A `pico-radio.ino` fájl a TFT-SPI Pico Rádió projekt fő programfájlja. Ez a fájl tartalmazza az Arduino `setup()` és `loop()` függvényeit, valamint a globális változókat, a képernyőváltás logikáját és a fő program működését irányító kódot.

## `#include` Direktívák

A fájl elején a következő include direktívák találhatók:

*   `<Arduino.h>`: Az Arduino platform alapvető függvényeinek és definícióinak beillesztése.
*   `"utils.h"`: A projekt segédfüggvényeinek definícióit tartalmazó header fájl beillesztése.
*   `<TFT_eSPI.h>`: A TFT kijelző kezelésére szolgáló TFT_eSPI könyvtár beillesztése.
*   `"RotaryEncoder.h"`: A forgó encoder kezelésére szolgáló osztály definícióját tartalmazó header fájl beillesztése.
*   `"Config.h"`: A konfigurációs adatok kezelésére szolgáló osztály definícióját tartalmazó header fájl beillesztése.
*   `"PicoMemoryInfo.h"`: A memória információk lekérdezésére szolgáló osztály definícióját tartalmazó header fájl beillesztése.
*   `"AmDisplay.h"`: Az AM képernyő kezeléséért felelős osztály definícióját tartalmazó header fájl beillesztése.
*   `"FmDisplay.h"`: Az FM képernyő kezeléséért felelős osztály definícióját tartalmazó header fájl beillesztése.
* `"FreqScanDisplay.h"`: A frekvencia scan képernyő kezeléséért felelős osztály definícióját tartalmazó header fájl beillesztése.

## Globális Változók

A fájlban a következő globális változók vannak definiálva:

*   `TFT_eSPI tft;`: A TFT kijelző objektumának létrehozása. Ezen keresztül érhető el a kijelző vezérlése.
*   `RotaryEncoder rotaryEncoder = RotaryEncoder(PIN_ENCODER_CLK, PIN_ENCODER_DT, PIN_ENCODER_SW);`: A forgó encoder objektumának létrehozása, a megfelelő lábkiosztással.
*   `Config config;`: A konfigurációs adatok tárolására és kezelésére szolgáló objektum létrehozása.
*   `DisplayBase *pDisplay = nullptr;`: Pointer, amely az aktuálisan megjelenített képernyő objektumára mutat. Kezdetben `nullptr`, mert induláskor még nincs képernyő megjelenítve.
*   `DisplayBase::DisplayType displayChangeType = DisplayBase::DisplayType::FmDisplayType;`: A képernyőváltás jelzésére szolgáló globális változó. Kezdeti értéke `DisplayBase::DisplayType::FmDisplayType`, ami azt jelenti, hogy a program indulásakor az FM képernyő jelenik meg. A többi képernyő `.cpp` fájljaiban tudják módosítani a gombnyomás esetén, hogy melyik képernyőre kell váltani. A `loop()` függvényben ellenőrizzük ezt a változót.

## Globális Függvények

### `changeDisplay()`

*   **Szerepe:** Az aktuális képernyő objektum lecserélése egy másik képernyő objektumra.
*   **Hívása:** A `loop()` függvényből hívódik meg, ha a `displayChangeType` értéke nem `DisplayBase::DisplayType::noneDisplayType`.
*   **Működése:**
    1.  **Előző Képernyő Törlése:** Ha a `pDisplay` pointer nem `nullptr`, akkor a `delete pDisplay;` utasítással törli az előző képernyő objektumot, felszabadítva a lefoglalt memóriát.
    2.  **Új Képernyő Létrehozása:** A `switch` utasítás segítségével a `displayChangeType` értékének megfelelően létrehozza a megfelelő új képernyő objektumot (`FmDisplay`, `AmDisplay` vagy `FreqScanDisplay`) a `new` operátorral, és a `pDisplay` pointert erre az új objektumra állítja be.
    3.  **Képernyő Kirajzolása:** Meghívja az új képernyő objektum `drawScreen()` metódusát, hogy megjelenítse a képernyő tartalmát.
    4.  **Jelzés Törlése:** A `displayChangeType` értékét `DisplayBase::DisplayType::noneDisplayType`-ra állítja, hogy a `loop()` függvény a következő ciklusban már ne hívja meg újra a `changeDisplay()` függvényt.

### `setup()`

*   **Szerepe:** A program indulásakor lefutó inicializációs függvény.
*   **Működése:**
    1.  **Soros Port:** Ha debug mód van beállítva (`__DEBUG` definíció), inicializálja a soros portot 115200 baud sebességgel, és beállítja a beépített LED kimenetként.
    2.  **Beeper:** Beállítja a beeper kimeneti lábát (`PIN_BEEPER`) és alacsony szintre állítja.
    3.  **TFT LED:** Beállítja a TFT LED háttérvilágítás kimeneti lábát (`PIN_DISPLAY_LED`) és kikapcsolja.
    4.  **Rotary Encoder:** Beállítja a forgó encoder tulajdonságait (dupla kattintás engedélyezve, gyorsulás engedélyezve).
    5.  **TFT Inicializálás:** Inicializálja a TFT kijelzőt, beállítja a forgatást (1-es érték), és feketére tölti a képernyőt.
    6.  **Konfiguráció:** Ha bekapcsoláskor nyomva tartják a rotary gombját, akkor törli a konfigurációs adatokat és visszaáll az alapértelmezettre, ha nem, akkor betölti a konfigot.
    7.  **TFT Kalibráció:** Ellenőrzi, hogy a TFT kijelző kalibrálva van-e. Ha nincs, akkor elindítja a kalibrációs folyamatot.
    8. **TFT Touch**: A TFT touch screen beállítása.
    9.  **Kezdő Képernyő:** Meghívja a `changeDisplay()` függvényt, hogy megjelenítse a kezdőképernyőt (FM).
    10. **Beeper**: Egy csippanással jelzi, hogy a program elindult.

### `loop()`

*   **Szerepe:** A program fő ciklusa, amely folyamatosan fut az Arduino működése alatt.
*   **Működése:**
    1.  **Képernyőváltás Ellenőrzés:** Megvizsgálja, hogy a `displayChangeType` értéke nem `DisplayBase::DisplayType::noneDisplayType`-e. Ha nem, akkor meghívja a `changeDisplay()` függvényt, hogy elvégezze a képernyőváltást.
    2.  **Rotary Encoder Service:** Periodikusan (5 ms-enként) meghívja a `rotaryEncoder.service()` függvényt a forgó encoder eseményeinek kezelésére.
    3.  **EEPROM Mentés Ellenőrzés:** Periodikusan (5 percenként) meghívja a `config.checkSave()` függvényt a konfigurációs adatok EEPROM-ba mentésének ellenőrzésére.
    4.  **Memória Információk:** Ha debug módban van (`__DEBUG`), periodikusan (20 másodpercenként) meghívja a `debugMemoryInfo()` függvényt a memória állapotának kiírására.
    5.  **Rotary Gomb Kezelés:** Beolvassa a forgó encoder állapotát a `rotaryEncoder.read()` függvény segítségével. Ha a rotary gombot folyamatosan nyomva tartják, akkor debug üzenetet ír ki, és visszatér a függvényből, mintha kikapcsolna.
    6.  **Aktuális Képernyő Kezelése:** A `try-catch` blokkban meghívja az aktuális képernyő objektum (`pDisplay`) `loop()` metódusát (`pDisplay->loop(encoderState)`), továbbítva a forgó encoder állapotát.
    7.  **Kivételkezelés:** Ha a `pDisplay->loop()` függvényben kivétel keletkezik, akkor a `catch` blokkokban kezeli azokat, csippant egy hiba jelzést, és megjeleníti a hibaüzenetet a kijelzőn.

## Összegzés

A `pico-radio.ino` fájl a TFT-SPI Pico Rádió projekt központi eleme. Ebben a fájlban található a program belépési pontja, az inicializációs logika, a fő ciklus és a képernyőváltás mechanizmusa. A fájl globális változókat használ a különböző modulok közötti kommunikációra, és jól strukturált kódot tartalmaz, amely könnyen karbantartható és bővíthető.



# `DisplayBase.h` - Alap Képernyő Osztály Dokumentáció

## Bevezetés

A `DisplayBase.h` fájl a TFT-SPI Pico Rádió projekt alap képernyő osztályát, a `DisplayBase`-t definiálja. Ez az absztrakt alaposztály szolgál az összes többi képernyő osztály (pl. `FmDisplay`, `AmDisplay`, `FreqScanDisplay`) őséül. Tartalmazza a közös funkcionalitásokat, mint a képernyőgombok kezelése, a dialógusok kezelése, a touch és rotary események továbbítása.

## `#include` Direktívák

A fájl elején a következő include direktívák találhatók:

*   `<Arduino.h>`: Az Arduino platform alapvető függvényeinek és definícióinak beillesztése.
*   `<TFT_eSPI.h>`: A TFT kijelző kezelésére szolgáló TFT_eSPI könyvtár beillesztése.
*   `"DialogBase.h"`: A dialógusok kezelésére szolgáló alaposztály definícióját tartalmazó header fájl beillesztése.
*   `"IDialogParent.h"`: Az `IDialogParent` interfész definícióját tartalmazó header fájl beillesztése.
*   `"IGuiEvents.h"`: Az `IGuiEvents` interfész definícióját tartalmazó header fájl beillesztése.
*   `"TftButton.h"`: A gomb osztály definícióját tartalmazó header fájl beillesztése.
*   `"utils.h"`: A projekt segédfüggvényeinek definícióit tartalmazó header fájl beillesztése.

## Elődefiniált Makrók

A fájl a következő makrókat definiálja:

*   `SCRN_MENU_BTN_ID_START`: A képernyő menügombok kezdő ID-je. (`50`)
*   `SCREEN_BTNS_X_START`: A gombok kezdő X koordinátája. (`5`)
*   `SCREEN_BTNS_Y_START`: A gombok kezdő Y koordinátája. (`250`)
*   `SCRN_BTN_H`: A gombok magassága. (`30`)
*   `SCRN_BTN_W`: A gombok szélessége. (`70`)
*   `SCREEN_BTNS_GAP`: A gombok közötti távolság. (`10`)
*   `SCREEN_BUTTONS_PER_ROW`: Egy sorban hány gomb van. (`6`)
*   `SCREEN_BTN_ROW_SPACING`: A gombok sorai közötti távolság. (`5`)
*   `SCREEN_BTNS_X(n)`: A gombok X koordinátájának kiszámítása az `n` sorszám alapján.
* `SCREEN_COMPS_REFRESH_TIME_MSEC`: A képernyő változó adatok frissítési ciklusideje. (`500`)

## `DisplayBase` Osztály

### `DisplayType` Enum

*   **Szerepe:** A lehetséges képernyőtípusokat definiálja.
*   **Tagok:**
    *   `noneDisplayType`: Nincs kijelző.
    *   `FmDisplayType`: FM kijelző.
    *   `AmDisplayType`: AM kijelző.
    * `FreqScanDisplayType`: Frekvencia scan kijelző

### `BuildButtonData` Struktúra

*   **Szerepe:** A képernyőgombok létrehozásához szükséges adatokat tárolja.
*   **Tagok:**
    *   `const char *label`: A gomb címkéje (szövege).
    *   `TftButton::ButtonType type`: A gomb típusa (`Pushable`, `Toggleable`).
    *   `TftButton::ButtonState state`: A gomb állapota (`Off`, `On`, `Disabled`).

### Tagváltozók

*   `TftButton **screenButtons = nullptr;`: Pointer a dinamikusan létrehozott gombok tömbjére.
*   `uint8_t screenButtonsCount = 0;`: A dinamikusan létrehozott gombok száma.
* `TftButton::ButtonTouchEvent screenButtonTouchEvent = TftButton::noTouchEvent;`: A lenyomott képernyő menügomb adatai.
* `TftButton::ButtonTouchEvent dialogButtonResponse = TftButton::noTouchEvent;`: A dialógban megnyomott gomb adatai.
*   `TFT_eSPI &tft;`: Referencia a TFT kijelző objektumra.
*   `DialogBase *pDialog = nullptr;`: Pointer a képernyőn megjelenő dialóg objektumára.

### Protected Metódusok

*   `inline uint16_t getAutoX(uint8_t index)`:
    *   **Szerepe:** A képernyőgombok X koordinátáját számítja ki automatikusan.
    * **Működése**: Ha nem férnek el egy sorban a gombok, akkor nyit egy új sort.
*   `inline uint16_t getAutoY(uint8_t index)`:
    *   **Szerepe:** A képernyőgombok Y koordinátáját számítja ki automatikusan.
    * **Működése**: A gombok több sorban is elhelyezkedhetnek, az alsó sor a képernyő aljához igazodik.
*   `inline void buildScreenButtons(BuildButtonData buttonsData[], uint8_t buttonsDataLength, uint8_t startId)`:
    *   **Szerepe:** Létrehozza a képernyő menügombjait.
    *   **Paraméterek:**
        *   `BuildButtonData buttonsData[]`: A gombok adatait tartalmazó tömb.
        *   `uint8_t buttonsDataLength`: A tömb hossza.
        *   `uint8_t startId`: A gombok ID-jének kezdőértéke.
*   `inline void drawScreenButtons()`:
    *   **Szerepe:** Kirajzolja a képernyő menügombjait.

### Public Metódusok

*   `DisplayBase(TFT_eSPI &tft)`:
    *   **Szerepe:** Konstruktor.
    *   **Paraméterek:**
        *   `TFT_eSPI &tft`: Referencia a TFT kijelző objektumra.
*   `virtual ~DisplayBase()`:
    *   **Szerepe:** Destruktor.
    *   **Működése:** Törli a képernyőgombokat és a dialógust.
* `inline void setDialogResponse(TftButton::ButtonTouchEvent event) override`:
    * **Szerepe**: A dialog által átadott megnyomott gomb adatai.
    * **Paraméter**: `TftButton::ButtonTouchEvent`: A megnyomott gomb adatai.
    * **Működése**:  Az IDialogParent-ből jön, a dialóg hívja, ha nyomtak rajta valamit.
* `inline bool isDialogResponseCancelOrCloseX() override`:
    * **Szerepe**: Megvizsgálja, hogy Cancelt vagy 'X'-et nyomtak-e a dialógon.
    * **Visszatérési érték**: `true`, ha 'Cancel'-t vagy 'X'-et nyomtak, `false` egyébként.
*   `virtual void drawScreen() = 0`:
    *   **Szerepe:** Absztrakt metódus, a képernyő tartalmának kirajzolása. A leszármazott osztályokban kell implementálni.
*   `virtual void processScreenButtonTouchEvent(TftButton::ButtonTouchEvent &event) = 0`:
    *   **Szerepe:** Absztrakt metódus, a képernyőgombok eseményeinek feldolgozása. A leszármazott osztályokban kell implementálni.
*   `virtual void processDialogButtonResponse(TftButton::ButtonTouchEvent &event) = 0`:
    *   **Szerepe:** Absztrakt metódus, a dialógusgombok eseményeinek feldolgozása. A leszármazott osztályokban kell implementálni.
* `virtual void loop(RotaryEncoder::EncoderState encoderState) final`:
    * **Szerepe**: Arduino loop hívás.
    * **Paraméter**: `RotaryEncoder::EncoderState`: Az encorder és a gomb állapota.
    * **Működése**:
        *   Kezeli a touch és a rotary eseményeket.
        * Ha van az előző körből feldolgozandó esemény, akkor azzal foglalkozik először.
        * A rotary eseményeket a `pDialog` (ha van) vagy a `handleRotary()` kezeli.
        * Ha nincs rotary esemény, akkor a touch események vizsgálata történik.
        * Ha van dialóg, de még nincs dialogButtonResponse, akkor meghívja a dialóg touch handlerét.
        * Ha nincs dialóg, de vannak képernyő menügombok és még nincs scrrenButton esemény, akkor azok kapják meg a touch adatokat.
        * A `screenButtonTouchEvent` vagy `dialogButtonResponse` továbbítása.
        * Ha nincs screenButton touch event, de nyomtak valamit, akkor azt továbbítja a képernyőnek a `handleTouch()` függvénnyel.
* `virtual bool handleRotary(RotaryEncoder::EncoderState encoderState) = 0`:
    * **Szerepe**: Absztrakt metódus a rotary esemény kezelésére. A leszármazott osztályokban kell implementálni.
* `virtual bool handleTouch(bool touched, uint16_t tx, uint16_t ty) = 0`:
    * **Szerepe**: Absztrakt metódus a Touch esemény kezelésére. A leszármazott osztályokban kell implementálni.

### Globális változó

*   `extern DisplayBase::DisplayType displayChangeType;`: Deklarálja a `pico-radio.ino`-ban definiált globális változót. Ezzel a globális változóval jelzik a képernyőváltást.

## Összegzés

A `DisplayBase.h` fájl definiálja a projekt alap képernyő osztályát. Ez az absztrakt osztály tartalmazza a képernyők közös tulajdonságait, metódusait, a gombkezelést, a dialógus kezelést és az eseménykezelést. A leszármazott osztályokban kell implementálni a képernyő tartalmának kirajzolását, valamint a gombokra és eseményekre reagáló metódusokat. A `DisplayBase` osztály egy központi eleme a kódnak, és felelős a képernyők közötti egységes viselkedés biztosításáért.



# `FmDisplay.h` és `FmDisplay.cpp` - FM Képernyő Osztály Dokumentáció

## Bevezetés

Az `FmDisplay.h` és `FmDisplay.cpp` fájlok a TFT-SPI Pico Rádió projekt FM képernyőjének osztályát definiálják és implementálják. Az `FmDisplay` osztály a `DisplayBase` osztályból származik, így örökli annak tulajdonságait és metódusait. Az FM képernyő felelős az FM módhoz tartozó felhasználói felület megjelenítéséért, a gombnyomások, a rotary encoder események és a dialógusok kezeléséért.

## `FmDisplay.h`

### `#include` Direktívák

*   `#include "DisplayBase.h"`: A `DisplayBase` osztály definíciójának beillesztése.

### `FmDisplay` Osztály

*   **Származtatás:** `public DisplayBase`
*   **Szerepe:** Az FM képernyő logikájának implementálása.
*   **Metódusok:**
    *   `FmDisplay(TFT_eSPI &tft)`:
        *   **Szerepe:** Konstruktor.
        *   **Paraméterek:**
            *   `TFT_eSPI &tft`: Referencia a TFT kijelző objektumra.
        * **Működése**: Meghívja az ősosztály konstruktorát. Létrehozza a képernyőn található gombokat, a `BuildButtonData` tömb alapján.
    *   `~FmDisplay()`:
        *   **Szerepe:** Destruktor.
        * **Működése**: Meghívja az ősosztály destruktorát.
    *   `virtual void drawScreen() override`:
        *   **Szerepe:** Az FM képernyő tartalmának kirajzolása.
        *   **Felülírás:** Felülírja a `DisplayBase` osztály `drawScreen()` absztrakt metódusát.
        * **Működése**:
            * Megjeleníti a képernyő gombokat (`DisplayBase::drawScreenButtons();`).
            * Megjelenít egy szöveget, ami jelzi, hogy az FM display látható.
    *   `virtual bool handleRotary(RotaryEncoder::EncoderState encoderState) override`:
        *   **Szerepe:** A forgó encoder eseményeinek kezelése az FM képernyőn.
        *   **Paraméterek:**
            *   `RotaryEncoder::EncoderState encoderState`: A forgó encoder aktuális állapota.
        *   **Felülírás:** Felülírja a `DisplayBase` osztály `handleRotary()` absztrakt metódusát.
        * **Működése**: Kiírja a rotary encoder változásait a Serial portra, ha van.
        *   **Visszatérési érték:** `false`, mert az FM képernyő nem kezeli a forgó encodert.
    *   `virtual bool handleTouch(bool touched, uint16_t tx, uint16_t ty) override`:
        *   **Szerepe:** A Touch esemény kezelése az FM képernyőn.
        *   **Paraméterek:**
            *   `bool touched`: `true`, ha történt érintés.
            * `uint16_t tx`, `uint16_t ty`: A touch X és Y koordinátái.
        *   **Felülírás:** Felülírja a `DisplayBase` osztály `handleTouch()` absztrakt metódusát.
        * **Működése**: Nem kezeli a Touch eseményeket.
        *   **Visszatérési érték:** `false`, mert az FM képernyő nem kezeli a Touch eseményeket.
    *   `virtual void processScreenButtonTouchEvent(TftButton::ButtonTouchEvent &event) override`:
        *   **Szerepe:** A képernyőgombok eseményeinek feldolgozása az FM képernyőn.
        *   **Paraméterek:**
            *   `TftButton::ButtonTouchEvent &event`: A gombnyomás eseményének adatai.
        *   **Felülírás:** Felülírja a `DisplayBase` osztály `processScreenButtonTouchEvent()` absztrakt metódusát.
        * **Működése**:
            * Vizsgálja, hogy melyik gombot nyomták meg.
            * Az "AM", "Scan" gombok esetén módosítja a `::displayChangeType` globális változót, hogy a `pico-radio.ino` `loop()` függvényében lévő `changeDisplay` függvény meghívódjon, és átváltson az AM vagy a FreqScan képernyőre.
            * A többi gomb esetén létrehozza a megfelelő dialóg objektumot (`MessageDialog`, `MultiButtonDialog`, `ValueChangeDialog`).
    *   `virtual void processDialogButtonResponse(TftButton::ButtonTouchEvent &event) override`:
        *   **Szerepe:** A dialógusgombok eseményeinek feldolgozása az FM képernyőn.
        *   **Paraméterek:**
            *   `TftButton::ButtonTouchEvent &event`: A gombnyomás eseményének adatai.
        *   **Felülírás:** Felülírja a `DisplayBase` osztály `processDialogButtonResponse()` absztrakt metódusát.
        * **Működése**:
            * Törli a dialógot.
            * Újra kirajzolja a képernyőt.

## `FmDisplay.cpp`

### `#include` Direktívák

*   `#include "FmDisplay.h"`: Az `FmDisplay` osztály definícióját tartalmazó header fájl beillesztése.
*   `#include <Arduino.h>`: Az Arduino alapvető függvényeinek beillesztése.
*   `#include "MessageDialog.h"`: Az `MessageDialog` osztály definíciójának beillesztése.
*   `#include "MultiButtonDialog.h"`: A `MultiButtonDialog` osztály definíciójának beillesztése.
*   `#include "ValueChangeDialog.h"`: A `ValueChangeDialog` osztály definíciójának beillesztése.

### Konstruktor: `FmDisplay::FmDisplay(TFT_eSPI &tft)`

*   **Szerepe:** Az `FmDisplay` objektum inicializálása.
*   **Működése:**
    1.  Meghívja a `DisplayBase` ősosztály konstruktorát.
    2.  Definiálja a képernyőn megjelenő gombok adatait a `buttonsData` tömbben:
        *   `"AM"`: AM képernyőre váltó gomb.
        *   `"Scan"`: FreqScan képernyőre váltó gomb.
        *   `"Popup"`: `MessageDialog` dialógus megnyitása.
        *   `"Multi"`: `MultiButtonDialog` dialógus megnyitása.
        *   `"b-Val"`: `ValueChangeDialog` dialógus megnyitása (`ledState` értékének módosításához).
        *   `"i-Val"`: `ValueChangeDialog` dialógus megnyitása (`volume` értékének módosításához).
        *   `"f-Val"`: `ValueChangeDialog` dialógus megnyitása (`temperature` értékének módosításához).
        *   `"Pause"`: Toggle gomb, melynek az alapértelmezett állapota "On".
        *   `"Reset"`: Push gomb, melynek az alapértelmezett állapota "Disabled".
    3.  Meghívja a `DisplayBase::buildScreenButtons()` metódust a gombok létrehozására, a megadott adatokkal.

### Destruktor: `FmDisplay::~FmDisplay()`

*   **Szerepe:** Az `FmDisplay` objektum felszabadítása.
* **Működése**: Meghívja a `DisplayBase` ősosztály destruktorát.

### Metódusok Implementációi

A metódusok implementációi a `FmDisplay.h`-ban leírtak szerint működnek.

## Globális változók

*  `ledState`: A `ValueChangeDialog`-ban használt változó.
* `volume`: A `ValueChangeDialog`-ban használt változó.
* `temperature`: A `ValueChangeDialog`-ban használt változó.

## Összegzés

Az `FmDisplay` osztály implementálja az FM képernyő logikáját. Felelős a képernyő tartalmának kirajzolásáért, a gombnyomásokért, a rotary eseményekért és a dialógok kezeléséért. A kód jól strukturált, moduláris, és jól követi az objektumorientált programozás elveit. Az `FmDisplay` osztály a `DisplayBase` osztályból származik, így örökli annak funkcionalitásait, és csak az FM képernyőre specifikus viselkedéseket implementálja.




# `AmDisplay.h` és `AmDisplay.cpp` - AM Képernyő Osztály Dokumentáció

## Bevezetés

Az `AmDisplay.h` és `AmDisplay.cpp` fájlok a TFT-SPI Pico Rádió projekt AM képernyőjének osztályát definiálják és implementálják. Az `AmDisplay` osztály a `DisplayBase` osztályból származik, így örökli annak tulajdonságait és metódusait. Az AM képernyő felelős az AM módhoz tartozó felhasználói felület megjelenítéséért, a gombnyomások és a rotary encoder események kezeléséért.

## `AmDisplay.h`

### `#include` Direktívák

*   `#include "DisplayBase.h"`: A `DisplayBase` osztály definíciójának beillesztése.

### `AmDisplay` Osztály

*   **Származtatás:** `public DisplayBase`
*   **Szerepe:** Az AM képernyő logikájának implementálása.
*   **Metódusok:**
    *   `AmDisplay(TFT_eSPI &tft)`:
        *   **Szerepe:** Konstruktor.
        *   **Paraméterek:**
            *   `TFT_eSPI &tft`: Referencia a TFT kijelző objektumra.
        * **Működése**: Meghívja az ősosztály konstruktorát. Létrehozza a képernyőn található gombokat, a `BuildButtonData` tömb alapján.
    *   `~AmDisplay()`:
        *   **Szerepe:** Destruktor.
        * **Működése**: Meghívja az ősosztály destruktorát.
    *   `virtual void drawScreen() override`:
        *   **Szerepe:** Az AM képernyő tartalmának kirajzolása.
        *   **Felülírás:** Felülírja a `DisplayBase` osztály `drawScreen()` absztrakt metódusát.
        * **Működése**:
            * Megjeleníti a képernyő gombokat (`DisplayBase::drawScreenButtons();`).
            * Megjelenít egy szöveget, ami jelzi, hogy az AM display látható.
    *   `virtual bool handleRotary(RotaryEncoder::EncoderState encoderState) override`:
        *   **Szerepe:** A forgó encoder eseményeinek kezelése az AM képernyőn.
        *   **Paraméterek:**
            *   `RotaryEncoder::EncoderState encoderState`: A forgó encoder aktuális állapota.
        *   **Felülírás:** Felülírja a `DisplayBase` osztály `handleRotary()` absztrakt metódusát.
        *   **Visszatérési érték:** `false`, mert az AM képernyő nem kezeli a forgó encodert.
    *   `virtual bool handleTouch(bool touched, uint16_t tx, uint16_t ty) override`:
        *   **Szerepe:** A Touch esemény kezelése az AM képernyőn.
        *   **Paraméterek:**
            *   `bool touched`: `true`, ha történt érintés.
            * `uint16_t tx`, `uint16_t ty`: A touch X és Y koordinátái.
        *   **Felülírás:** Felülírja a `DisplayBase` osztály `handleTouch()` absztrakt metódusát.
        *   **Visszatérési érték:** `false`, mert az AM képernyő nem kezeli a Touch eseményeket.
    *   `virtual void processScreenButtonTouchEvent(TftButton::ButtonTouchEvent &event) override`:
        *   **Szerepe:** A képernyőgombok eseményeinek feldolgozása az AM képernyőn.
        *   **Paraméterek:**
            *   `TftButton::ButtonTouchEvent &event`: A gombnyomás eseményének adatai.
        *   **Felülírás:** Felülírja a `DisplayBase` osztály `processScreenButtonTouchEvent()` absztrakt metódusát.
        * **Működése**:
            * Vizsgálja, hogy melyik gombot nyomták meg.
            * Az "FM" vagy "Scan" gombok esetén módosítja a `::displayChangeType` globális változót, hogy a `pico-radio.ino` `loop()` függvényében lévő `changeDisplay` függvény meghívódjon, és átváltson az FM vagy a FreqScan képernyőre.
    *   `virtual void processDialogButtonResponse(TftButton::ButtonTouchEvent &event) override`:
        *   **Szerepe:** A dialógusgombok eseményeinek feldolgozása az AM képernyőn.
        *   **Paraméterek:**
            *   `TftButton::ButtonTouchEvent &event`: A gombnyomás eseményének adatai.
        *   **Felülírás:** Felülírja a `DisplayBase` osztály `processDialogButtonResponse()` absztrakt metódusát.
        * **Működése**: Jelenleg nincs implementálva.

## `AmDisplay.cpp`

### `#include` Direktívák

*   `#include "AmDisplay.h"`: Az `AmDisplay` osztály definícióját tartalmazó header fájl beillesztése.
* `#include <Arduino.h>`: Az Arduino alapvető függvényeinek beillesztése.

### Konstruktor: `AmDisplay::AmDisplay(TFT_eSPI &tft)`

*   **Szerepe:** Az `AmDisplay` objektum inicializálása.
*   **Működése:**
    1.  Meghívja a `DisplayBase` ősosztály konstruktorát.
    2.  Definiálja a képernyőn megjelenő gombok adatait a `buttonsData` tömbben:
        *   `"FM"`: FM képernyőre váltó gomb.
        * `"Scan"`: FreqScan képernyőre váltó gomb.
    3.  Meghívja a `DisplayBase::buildScreenButtons()` metódust a gombok létrehozására, a megadott adatokkal.

### Destruktor: `AmDisplay::~AmDisplay()`

*   **Szerepe:** Az `AmDisplay` objektum felszabadítása.
* **Működése**: Meghívja a `DisplayBase` ősosztály destruktorát.

### Metódusok Implementációi

A metódusok implementációi a `AmDisplay.h`-ban leírtak szerint működnek.

## Összegzés

Az `AmDisplay` osztály implementálja az AM képernyő logikáját. Felelős a képernyő tartalmának kirajzolásáért, a gombnyomásokért. A kód jól strukturált, moduláris, és jól követi az objektumorientált programozás elveit. Az `AmDisplay` osztály a `DisplayBase` osztályból származik, így örökli annak funkcionalitásait, és csak az AM képernyőre specifikus viselkedéseket implementálja.

---

# `FreqScanDisplay.h` és `FreqScanDisplay.cpp` - Frekvencia Szkennelő Képernyő Osztály Dokumentáció

## Bevezetés

A `FreqScanDisplay.h` és `FreqScanDisplay.cpp` fájlok a TFT-SPI Pico Rádió projekt frekvencia szkennelő képernyőjének osztályát definiálják és implementálják. A `FreqScanDisplay` osztály a `DisplayBase` osztályból származik, így örökli annak tulajdonságait és metódusait. A frekvencia szkennelő képernyő felelős a frekvencia szkenneléshez tartozó felhasználói felület megjelenítéséért, a gombnyomások és a rotary encoder események kezeléséért.

## `FreqScanDisplay.h`

### `#include` Direktívák

*   `#include "DisplayBase.h"`: A `DisplayBase` osztály definíciójának beillesztése.

### `FreqScanDisplay` Osztály

*   **Származtatás:** `public DisplayBase`
*   **Szerepe:** A frekvencia szkennelő képernyő logikájának implementálása.
*   **Metódusok:**
    *   `FreqScanDisplay(TFT_eSPI &tft)`:
        *   **Szerepe:** Konstruktor.
        *   **Paraméterek:**
            *   `TFT_eSPI &tft`: Referencia a TFT kijelző objektumra.
        * **Működése**: Meghívja az ősosztály konstruktorát. Létrehozza a képernyőn található gombokat, a `BuildButtonData` tömb alapján.
    *   `~FreqScanDisplay()`:
        *   **Szerepe:** Destruktor.
        * **Működése**: Meghívja az ősosztály destruktorát.
    *   `virtual void drawScreen() override`:
        *   **Szerepe:** A frekvencia szkennelő képernyő tartalmának kirajzolása.
        *   **Felülírás:** Felülírja a `DisplayBase` osztály `drawScreen()` absztrakt metódusát.
        * **Működése**:
            * Megjeleníti a képernyő gombokat (`DisplayBase::drawScreenButtons();`).
            * Megjelenít egy szöveget, ami jelzi, hogy a frekvencia scan display látható.
    *   `virtual bool handleRotary(RotaryEncoder::EncoderState encoderState) override`:
        *   **Szerepe:** A forgó encoder eseményeinek kezelése a frekvencia szkennelő képernyőn.
        *   **Paraméterek:**
            *   `RotaryEncoder::EncoderState encoderState`: A forgó encoder aktuális állapota.
        *   **Felülírás:** Felülírja a `DisplayBase` osztály `handleRotary()` absztrakt metódusát.
        *   **Visszatérési érték:** `false`, mert a frekvencia szkennelő képernyő nem kezeli a forgó encodert.
    *   `virtual bool handleTouch(bool touched, uint16_t tx, uint16_t ty) override`:
        *   **Szerepe:** A Touch esemény kezelése a frekvencia szkennelő képernyőn.
        *   **Paraméterek:**
            *   `bool touched`: `true`, ha történt érintés.
            * `uint16_t tx`, `uint16_t ty`: A touch X és Y koordinátái.
        *   **Felülírás:** Felülírja a `DisplayBase` osztály `handleTouch()` absztrakt metódusát.
        *   **Visszatérési érték:** `false`, mert a frekvencia szkennelő képernyő nem kezeli a Touch eseményeket.
    *   `virtual void processScreenButtonTouchEvent(TftButton::ButtonTouchEvent &event) override`:
        *   **Szerepe:** A képernyőgombok eseményeinek feldolgozása a frekvencia szkennelő képernyőn.
        *   **Paraméterek:**
            *   `TftButton::ButtonTouchEvent &event`: A gombnyomás eseményének adatai.
        *   **Felülírás:** Felülírja a `DisplayBase` osztály `processScreenButtonTouchEvent()` absztrakt metódusát.
        * **Működése**:
            * Vizsgálja, hogy melyik gombot nyomták meg.
            * Az "FM" vagy "AM" gombok esetén módosítja a `::displayChangeType` globális változót, hogy a `pico-radio.ino` `loop()` függvényében lévő `changeDisplay` függvény meghívódjon, és átváltson az FM vagy az AM képernyőre.
    *   `virtual void processDialogButtonResponse(TftButton::ButtonTouchEvent &event) override`:
        *   **Szerepe:** A dialógusgombok eseményeinek feldolgozása a frekvencia szkennelő képernyőn.
        *   **Paraméterek:**
            *   `TftButton::ButtonTouchEvent &event`: A gombnyomás eseményének adatai.
        *   **Felülírás:** Felülírja a `DisplayBase` osztály `processDialogButtonResponse()` absztrakt metódusát.
        * **Működése**: Jelenleg nincs implementálva.

## `FreqScanDisplay.cpp`

### `#include` Direktívák

*   `#include "FreqScanDisplay.h"`: A `FreqScanDisplay` osztály definícióját tartalmazó header fájl beillesztése.
* `#include <Arduino.h>`: Az Arduino alapvető függvényeinek beillesztése.

### Konstruktor: `FreqScanDisplay::FreqScanDisplay(TFT_eSPI &tft)`

*   **Szerepe:** A `FreqScanDisplay` objektum inicializálása.
*   **Működése:**
    1.  Meghívja a `DisplayBase` ősosztály konstruktorát.
    2.  Definiálja a képernyőn megjelenő gombok adatait a `buttonsData` tömbben:
        *   `"FM"`: FM képernyőre váltó gomb.
        * `"AM"`: AM képernyőre váltó gomb.
    3.  Meghívja a `DisplayBase::buildScreenButtons()` metódust a gombok létrehozására, a megadott adatokkal.

### Destruktor: `FreqScanDisplay::~FreqScanDisplay()`

*   **Szerepe:** A `FreqScanDisplay` objektum felszabadítása.
* **Működése**: Meghívja a `DisplayBase` ősosztály destruktorát.

### Metódusok Implementációi

A metódusok implementációi a `FreqScanDisplay.h`-ban leírtak szerint működnek.

## Összegzés

A `FreqScanDisplay` osztály implementálja a frekvencia szkennelő képernyő logikáját. Felelős a képernyő tartalmának kirajzolásáért, a gombnyomásokért. A kód jól strukturált, moduláris, és jól követi az objektumorientált programozás elveit. A `FreqScanDisplay` osztály a `DisplayBase` osztályból származik, így örökli annak funkcionalitásait, és csak a frekvencia szkennelő képernyőre specifikus viselkedéseket implementálja.



# `RotaryEncoder.h` - Forgó Encoder Osztály Dokumentáció

## Bevezetés

A `RotaryEncoder.h` fájl a TFT-SPI Pico Rádió projekt forgó encoderének kezelésére szolgáló `RotaryEncoder` osztályt definiálja. Az osztály segítségével könnyen kezelhetők a forgó encoder eseményei, mint a forgatás irányának és a gomb lenyomásának érzékelése.

## `#include` Direktívák

A fájl elején az alábbi include direktíva található:

*   `<Arduino.h>`: Az Arduino platform alapvető függvényeinek és definícióinak beillesztése.

## `RotaryEncoder` Osztály

### Enumok

*   `Direction`: A forgás irányát definiáló enum.
    *   `None`: Nincs forgás.
    *   `Up`: Forgatás felfelé (óramutató járásával megegyező).
    *   `Down`: Forgatás lefelé (óramutató járásával ellentétes).
*   `ButtonState`: A gomb állapotát definiáló enum.
    *   `Open`: A gomb nincs lenyomva.
    *   `Pressed`: A gomb éppen le van nyomva.
    *   `Held`: A gomb lenyomva van tartva.
    *   `Released`: A gomb felengedve.
    *   `Clicked`: Rövid gombnyomás (egy kattintás).
    *   `DoubleClicked`: Dupla kattintás.
* `EncoderState`: A forgó encorder és a gomb együttes állapotát adja meg.
    * `direction`: A forgás iránya.
    * `buttonState`: A gomb állapota.

### Tagváltozók

*   `int pinClk;`: A forgó encoder CLK lábának száma.
*   `int pinDt;`: A forgó encoder DT lábának száma.
*   `int pinSw;`: A forgó encoder SW (gomb) lábának száma.
*   `Direction direction;`: Az utolsó forgás iránya.
*   `ButtonState buttonState;`: A gomb utolsó állapota.
*   `bool doubleClickEnabled;`: Dupla kattintás engedélyezése.
*   `bool accelerationEnabled;`: Gyorsulás engedélyezése.

### Konstruktorok

*   `RotaryEncoder(int pinClk, int pinDt, int pinSw)`:
    *   **Szerepe:** Az osztály konstruktora.
    *   **Paraméterek:**
        *   `int pinClk`: A CLK láb száma.
        *   `int pinDt`: A DT láb száma.
        *   `int pinSw`: A SW (gomb) láb száma.
    * **Működése**: inicializálja a tagváltozókat.

### Metódusok

*   `void setDoubleClickEnabled(bool enabled)`:
    *   **Szerepe:** A dupla kattintás érzékelésének engedélyezése vagy tiltása.
    *   **Paraméterek:**
        *   `bool enabled`: `true` esetén engedélyezi, `false` esetén tiltja a dupla kattintást.
*   `void setAccelerationEnabled(bool enabled)`:
    *   **Szerepe:** A gyorsulás (több impulzus gyors forgatás esetén) érzékelésének engedélyezése vagy tiltása.
    *   **Paraméterek:**
        *   `bool enabled`: `true` esetén engedélyezi, `false` esetén tiltja a gyorsulást.
*   `void service()`:
    *   **Szerepe:** A forgó encoder állapotának frissítése. Ezt a függvényt periodikusan hívni kell a program fő ciklusában.
* `EncoderState read()`:
    * **Szerepe**: Visszaadja az encorder és a gomb aktuális állapotát.
    * **Visszatérési érték**: Az aktuális állapot egy `EncoderState` struktúrában.

## Összegzés

A `RotaryEncoder` osztály egyszerű és hatékony módon kezeli a forgó encoder eseményeit. A `service()` metódus periodikus hívásával és a `read()` függvény meghívásával könnyen leolvasható a forgás iránya és a gomb állapota. A dupla kattintás és gyorsulás érzékelésének lehetősége tovább növeli az osztály funkcionalitását.



# `Config.h` - Konfigurációs Osztály Dokumentáció

## Bevezetés

A `Config.h` fájl a TFT-SPI Pico Rádió projekt konfigurációs adatainak tárolására, betöltésére, mentésére és kezelésére szolgáló `Config` osztályt definiálja. Az osztály segítségével az EEPROM-ban tárolt beállítások könnyen kezelhetők, és az alapértelmezett értékek visszaállíthatók.

## `#include` Direktívák

A fájl elején a következő include direktívák találhatók:

*   `<Arduino.h>`: Az Arduino platform alapvető függvényeinek és definícióinak beillesztése.
*   `<EEPROM.h>`: Az EEPROM (Electrically Erasable Programmable Read-Only Memory) kezelésére szolgáló könyvtár beillesztése.
*   `"utils.h"`: A segédfüggvényeket tartalmazó header fájl beillesztése.

## `Config` Osztály

### `ConfigData` Struktúra

*   **Szerepe:** A konfigurációs adatokat tárolja.
*   **Tagváltozók:**
    *   `uint8_t eepromVersion = 1;`: Az EEPROM tartalom verziószáma.
    *   `uint16_t configSize = sizeof(ConfigData);`: A konfigurációs adatok mérete (bájtban).
    *   `uint8_t tftCalibrateData[8] = {0, 0, 0, 0, 0, 0, 0, 0};`: A TFT érintőképernyő kalibrációs adatai.
    *   `uint8_t beeperVolume = 10;`: A beeper hangereje.

### Tagváltozók

*   `ConfigData data;`: A konfigurációs adatok tárolására szolgáló struktúra.
*   `uint32_t lastEepromSaveTime = 0;`: Az utolsó EEPROM mentés időpontja.

### Metódusok

*   `Config()`:
    *   **Szerepe:** Konstruktor.
    * **Működése**: inicializálja a `lastEepromSaveTime` változót.
*   `void load()`:
    *   **Szerepe:** A konfigurációs adatok betöltése az EEPROM-ból.
    *   **Működése:**
        1.  Ellenőrzi, hogy az EEPROM-ban tárolt verziószám megegyezik-e a kódban definiált verziószámmal.
        2.  Ha megegyezik, akkor betölti az adatokat a `data` struktúrába.
        3. Ha nem egyezik meg, akkor törli az EEPROM tartalmát, és az `loadDefaults()` metódus meghívásával betölti az alapértelmezett értékeket.
*   `void loadDefaults()`:
    *   **Szerepe:** Az alapértelmezett konfigurációs adatok betöltése.
    *   **Működése:**
        1.  Beállítja a `data` struktúra tagváltozóit az alapértelmezett értékekre.
        2.  Törli az EEPROM-ot.
        3.  Elmenti a `data` struktúra tartalmát az EEPROM-ba.
*   `void checkSave()`:
    *   **Szerepe:** Ellenőrzi, hogy a konfigurációs adatok megváltoztak-e, és szükség van-e mentésre az EEPROM-ba.
    *   **Működése:**
        1.  Ellenőrzi, hogy eltelt-e a `EEPROM_SAVE_MIN_INTERVAL` ideje az utolsó mentés óta.
        2.  Ha eltelt, akkor meghívja a `save()` metódust.
*   `void save()`:
    *   **Szerepe:** A konfigurációs adatok mentése az EEPROM-ba.
    *   **Működése:**
        1.  Elmenti a `data` struktúra tartalmát az EEPROM-ba.
        2.  Beállítja a `lastEepromSaveTime` változót az aktuális időre.

### Makrók

*   `EEPROM_SAVE_MIN_INTERVAL`: A minimális időtartam két EEPROM mentés között (30 másodperc).

## Összegzés

A `Config` osztály a projekt konfigurációs adatainak kezelésére szolgál. Az osztály segítségével könnyen kezelhetők az EEPROM-ban tárolt beállítások, és az alapértelmezett értékek visszaállíthatók. A `load()`, `loadDefaults()`, `checkSave()` és `save()` metódusok biztosítják a konfigurációs adatok betöltését, mentését és ellenőrzését.





# `utils.h` - Segédfüggvények Dokumentáció

## Bevezetés

A `utils.h` fájl a TFT-SPI Pico Rádió projekt segédfüggvényeit definiálja. Ez a header fájl olyan általános célú függvényeket tartalmaz, amelyeket a projekt különböző részein használnak.

## `#include` Direktívák

A fájl elején a következő include direktívák találhatók:

*   `<Arduino.h>`: Az Arduino platform alapvető függvényeinek és definícióinak beillesztése.
*   `<TFT_eSPI.h>`: A TFT kijelző kezelésére szolgáló TFT_eSPI könyvtár beillesztése.
* `<stdarg.h>`: A változó számú paraméterek kezeléséhez szükséges függvények.

## Makrók

* `STREQ(s1,s2)`: A két string összehasonlítására szolgáló makró.

## Függvények

*   `void beepTick()`:
    *   **Szerepe:** Egy rövid hangjelzés (csippanás) lejátszása a beeper segítségével.
    *   **Működése:** Beállítja a `PIN_BEEPER` lábat magas szintre egy rövid ideig, majd visszaállítja alacsony szintre.
*   `void beepError()`:
    *   **Szerepe:** Egy hosszabb, hibát jelző hangjelzés (sípolás) lejátszása a beeper segítségével.
    *   **Működése:** Beállítja a `PIN_BEEPER` lábat magas szintre egy hosszabb ideig, majd visszaállítja alacsony szintre.
*   `void debugWaitForSerial(TFT_eSPI &tft)`:
    *   **Szerepe:** Várakozás a soros port megnyitására, debug célokra.
    *   **Paraméterek:**
        *   `TFT_eSPI &tft`: Referencia a TFT kijelző objektumra, ha szükséges a kiírás.
    *   **Működése:** A `DEBUG` makróval jeleníti meg, hogy a soros portra vár.
*   `void tftTouchCalibrate(TFT_eSPI &tft, uint8_t calibrateData[8])`:
    *   **Szerepe:** A TFT érintőképernyő kalibrációs folyamatának elindítása.
    *   **Paraméterek:**
        *   `TFT_eSPI &tft`: Referencia a TFT kijelző objektumra.
        *   `uint8_t calibrateData[8]`: A kalibrációs adatok tárolására szolgáló tömb.
    *   **Működése:** Meghívja a `tft.calibrateTouch()` függvényt a kalibráció elvégzéséhez, és a kapott adatokat a `calibrateData` tömbbe menti.
*   `bool isZeroArray(uint8_t arr[])`:
    *   **Szerepe:** Ellenőrzi, hogy egy tömb minden eleme nulla-e.
    *   **Paraméterek:**
        * `uint8_t arr[]`: A vizsgálandó tömb.
    *   **Visszatérési érték:** `true`, ha minden elem nulla, `false` egyébként.
* `void displayException(TFT_eSPI &tft, const char *format, ...)`:
    * **Szerepe**: Hibaüzenet megjelenítése a kijelzőn.
    * **Paraméterek**:
        * `TFT_eSPI &tft`: A TFT kijelző objektuma.
        * `const char *format`: A megjelenítendő szöveg, printf formátumban.
        * `...`: A printf formátumhoz tartozó további argumentumok.
    * **Működése**: Megjeleníti a kapott szöveget a képernyő közepén, piros színnel.

## Összegzés

A `utils.h` fájl hasznos segédfüggvényeket tartalmaz, amelyeket a projekt különböző részein lehet használni. A csippanó hangjelzések, a soros portra való várakozás, a TFT kalibráció és az általános tömbkezelő függvények mind növelik a kód olvashatóságát és karbantarthatóságát. A változó paraméterkezeléssel pedig dinamikus hibaüzeneteket lehet megjeleníteni.


# `DialogBase.h`, `IDialogParent.h`, `IGuiEvents.h`, `TftButton.h` - GUI Kezelő Osztályok és Interfészek Dokumentációja

## Bevezetés

Ez a dokumentum a TFT-SPI Pico Rádió projekt grafikus felhasználói felületéhez (GUI) kapcsolódó osztályok és interfészek dokumentációját tartalmazza. Ezek a fájlok a dialógusok, a gombok és az eseménykezelés alapvető építőköveit definiálják.

## `IDialogParent.h` - Dialógus Szülő Interfész

### `#include` Direktívák

*   `#include "TftButton.h"`: A `TftButton` osztály definíciójának beillesztése.

### `IDialogParent` Interfész

*   **Szerepe:** Az interfész olyan osztályok számára készült, amelyek dialógusokat tudnak kezelni. Az osztályoknak implementálniuk kell a metódusokat.
*   **Metódusok:**
    *   `virtual void setDialogResponse(TftButton::ButtonTouchEvent event) = 0`:
        *   **Szerepe:** A dialógus által megnyomott gomb eseményének átadása a szülő osztálynak.
        * **Paraméterek**: `TftButton::ButtonTouchEvent event`: A megnyomott gomb adatait tartalmazó esemény.
    *   `virtual bool isDialogResponseCancelOrCloseX() = 0`:
        *   **Szerepe:** Megvizsgálja, hogy a dialógusban a "Cancel" vagy a bezárás ("X") gombot nyomták-e meg.
        *   **Visszatérési érték:** `true`, ha a "Cancel" vagy "X" gombot nyomták meg, `false` egyébként.

## `IGuiEvents.h` - GUI Események Interfész

### `#include` Direktívák

* `#include "RotaryEncoder.h"`: A `RotaryEncoder` osztály definíciójának beillesztése.

### `IGuiEvents` Interfész

*   **Szerepe:** Olyan osztályok számára készült, amelyek GUI eseményeket, például Rotary és Touch eseményeket tudnak kezelni. Az osztályoknak implementálniuk kell a metódusokat.
* **Metódusok**:
    * `virtual bool handleRotary(RotaryEncoder::EncoderState encoderState) = 0`:
        * **Szerepe**: A rotary esemény kezelése.
        * **Paraméterek**: `RotaryEncoder::EncoderState encoderState`: Az encorder állapota.
        * **Visszatérési érték**: `true`, ha kezelte az eseményt, `false` egyébként.
    * `virtual bool handleTouch(bool touched, uint16_t tx, uint16_t ty) = 0`:
        * **Szerepe**: A touch esemény kezelése.
        * **Paraméterek**:
            * `bool touched`: `true`, ha történt érintés.
            * `uint16_t tx`, `uint16_t ty`: A touch X és Y koordinátái.
        * **Visszatérési érték**: `true`, ha kezelte az eseményt, `false` egyébként.

## `DialogBase.h` - Dialógus Alap Osztály

### `#include` Direktívák

*   `<Arduino.h>`: Az Arduino platform alapvető függvényeinek beillesztése.
*   `<TFT_eSPI.h>`: A TFT kijelző kezelésére szolgáló TFT_eSPI könyvtár beillesztése.
*   `"IDialogParent.h"`: Az `IDialogParent` interfész definíciójának beillesztése.
* `"TftButton.h"`: A `TftButton` osztály definíciójának beillesztése.
* `utils.h`: A segédfüggvényeket tartalmazó header fájl beillesztése.

### Makrók

*   `DLG_CLOSE_BUTTON_ID`: A dialógus bezárás gombjának ID-je. (`40`)
*   `DLG_CANCEL_BUTTON_ID`: A dialógus "Cancel" gombjának ID-je. (`41`)
*   `DLG_BTN_H`: A dialógus gombok magassága. (`30`)
*   `DLG_BTN_W`: A dialógus gombok szélessége. (`70`)
* `DLG_TXT_COLOR`: A dialog szövegszíne. (`TFT_WHITE`)
* `DLG_BG_COLOR`: A dialog háttérszíne. (`TFT_BLACK`)
* `DLG_BORDER_COLOR`: A dialog keretszíne. (`TFT_BLUE`)

### `DialogBase` Osztály

*   **Származtatás:** `public IGuiEvents`
*   **Szerepe:** A dialógusok alaposztálya. Tartalmazza a dialógusok közös tulajdonságait és metódusait.
*   **Tagváltozók:**
    *   `IDialogParent *dialogParent`: Pointer a dialógust létrehozó szülő objektumra.
    *   `TFT_eSPI &tft`: Referencia a TFT kijelző objektumra.
    *   `uint16_t x`: A dialógus X koordinátája.
    *   `uint16_t y`: A dialógus Y koordinátája.
    *   `uint16_t w`: A dialógus szélessége.
    *   `uint16_t h`: A dialógus magassága.
*   **Metódusok:**
    *   `DialogBase(IDialogParent *dialogParent, TFT_eSPI &tft, uint16_t x, uint16_t y, uint16_t w, uint16_t h)`:
        *   **Szerepe:** Konstruktor.
        *   **Paraméterek:**
            *   `IDialogParent *dialogParent`: Pointer a dialógust létrehozó szülő objektumra.
            *   `TFT_eSPI &tft`: Referencia a TFT kijelző objektumra.
            *   `uint16_t x`, `uint16_t y`: A dialógus X és Y koordinátái.
            *   `uint16_t w`, `uint16_t h`: A dialógus szélessége és magassága.
    *   `virtual ~DialogBase()`:
        *   **Szerepe:** Destruktor.
    *   `virtual void draw() = 0`:
        *   **Szerepe:** Absztrakt metódus, a dialógus tartalmának kirajzolása.
    *   `virtual bool handleRotary(RotaryEncoder::EncoderState encoderState) override`:
        *   **Szerepe:** A rotary esemény kezelése a dialógusban.
        * **Paraméterek**: `RotaryEncoder::EncoderState encoderState`: Az encorder állapota.
        * **Működése**: Nem csinál semmit.
        *   **Visszatérési érték:** `false`, mert a dialóg nem kezeli a rotary eseményeket.
    * `virtual bool handleTouch(bool touched, uint16_t tx, uint16_t ty) override`:
        * **Szerepe**: A touch esemény kezelése a dialógusban.
        * **Paraméterek**:
            * `bool touched`: `true`, ha történt érintés.
            * `uint16_t tx`, `uint16_t ty`: A touch X és Y koordinátái.
        *   **Visszatérési érték:** `false`, ha nem kezelte a touch eseményt, `true` egyébként.
        * **Működése**: Visszaadja, hogy volt-e valamilyen eseménye a dialogon belül.

## `TftButton.h` - Gomb Osztály

### `#include` Direktívák

*   `<Arduino.h>`: Az Arduino platform alapvető függvényeinek beillesztése.
*   `<TFT_eSPI.h>`: A TFT kijelző kezelésére szolgáló TFT_eSPI könyvtár beillesztése.

### Makrók

*   `BTN_TXT_COLOR`: A gomb szövegszíne (`TFT_WHITE`).
*   `BTN_BG_COLOR`: A gomb háttérszíne (`TFT_BLACK`).
*   `BTN_BORDER_COLOR`: A gomb keretszíne (`TFT_BLUE`).

### Enumok

*   `ButtonType`: A gomb típusát definiáló enum.
    *   `Pushable`: Nyomógomb.
    *   `Toggleable`: Kapcsoló gomb.
*   `ButtonState`: A gomb állapotát definiáló enum.
    *   `Off`: Kikapcsolva.
    *   `On`: Bekapcsolva.
    *   `Disabled`: Letiltva.
*   `ButtonTouchEvent`: A gomb eseményeit jelző struktúra.
*   `noTouchEvent`: Nincs esemény.

### `TftButton` Osztály

*   **Szerepe:** A TFT kijelzőn megjelenő gombok kezelésére szolgáló osztály.
*   **Tagváltozók:**
    *   `id`: A gomb egyedi azonosítója.
    *   `x`, `y`: A gomb bal felső sarkának koordinátái.
    *   `w`, `h`: A gomb szélessége és magassága.
    *   `label`: A gomb címkéje (szövege).
    *   `type`: A gomb típusa (`Pushable`, `Toggleable`).
    * `state`: A gomb állapota.
    *   `tft`: Referencia a TFT kijelző objektumra.
*   **Metódusok:**
    *   `TftButton(uint8_t id, TFT_eSPI &tft, uint16_t x, uint16_t y, uint16_t w, uint16_t h, const char *label, ButtonType type, ButtonState state)`:
        *   **Szerepe:** Konstruktor.
        *   **Paraméterek:**
            *   `uint8_t id`: A gomb azonosítója.
            *   `TFT_eSPI &tft`: Referencia a TFT kijelző objektumra.
            *   `uint16_t x`, `uint16_t y`: A gomb bal felső sarkának koordinátái.
            *   `uint16_t w`, `uint16_t h`: A gomb szélessége és magassága.
            *   `const char *label`: A gomb címkéje.
            *   `ButtonType type`: A gomb típusa.
            * `ButtonState state`: A gomb állapota.
    *   `void draw()`:
        *   **Szerepe:** A gomb kirajzolása a TFT kijelzőre.
    *   `bool handleTouch(bool touched, uint16_t tx, uint16_t ty)`:
        *   **Szerepe:** A touch esemény kezelése a gomb területén.
        *   **Paraméterek:**
            * `bool touched`: `true`, ha volt érintés.
            * `uint16_t tx`, `uint16_t ty`: A touch koordinátái.
        * **Visszatérési érték**: Ha volt gombnyomás, akkor `true`, egyébként `false`.
    * `ButtonTouchEvent buildButtonTouchEvent()`:
        * **Szerepe**: Létrehozza a gombnyomás eseményt.
        * **Visszatérési érték**: A gombnyomás adatait tartalmazó objektum.
    * `static const char * decodeState(ButtonState state)`:
        * **Szerepe**: Visszaadja a gomb állpotának szöveges értékét.

## Összegzés

A `DialogBase.h`, `IDialogParent.h`, `IGuiEvents.h`, és `TftButton.h` fájlok a TFT-SPI Pico Rádió projekt grafikus felhasználói felületének építőköveit tartalmazzák. Az `IDialogParent` és `IGuiEvents` interfészek, valamint a `DialogBase` és `TftButton` osztályok segítségével könnyen hozhatók létre dialógusok és gombok, valamint kezelhetők a touch és rotary események. A kód jól strukturált, moduláris, és követi az objektumorientált programozás elveit.



# `MessageDialog.h`, `MultiButtonDialog.h`, `ValueChangeDialog.h` - Dialógus Osztályok Dokumentációja

## Bevezetés

Ez a dokumentum a TFT-SPI Pico Rádió projekt dialógus ablakainak implementációját tartalmazó osztályok dokumentációját foglalja össze. Ezek a fájlok különböző típusú dialógusok definícióit tartalmazzák, mint az egyszerű üzenetdialógus (`MessageDialog`), a többgombos dialógus (`MultiButtonDialog`) és az értékváltoztató dialógus (`ValueChangeDialog`).

## `MessageDialog.h` - Üzenet Dialógus Osztály

### `#include` Direktívák

*   `#include "DialogBase.h"`: A `DialogBase` osztály definíciójának beillesztése.

### `MessageDialog` Osztály

*   **Származtatás:** `public DialogBase`
*   **Szerepe:** Egyszerű üzenetdialógus megjelenítése két gombbal.
*   **Tagváltozók:**
    *   `const char *title`: A dialógus címe.
    *   `const char *msg`: A megjelenítendő üzenet.
    *   `const char *okButtonLabel`: Az OK gomb címkéje.
    *   `const char *cancelButtonLabel`: A Cancel gomb címkéje.
*   **Metódusok:**
    *   `MessageDialog(IDialogParent *dialogParent, TFT_eSPI &tft, uint16_t w, uint16_t h, const char *title, const char *msg, const char *okButtonLabel, const char *cancelButtonLabel)`:
        *   **Szerepe:** Konstruktor.
        *   **Paraméterek:**
            *   `IDialogParent *dialogParent`: Pointer a dialógust létrehozó szülő objektumra.
            *   `TFT_eSPI &tft`: Referencia a TFT kijelző objektumra.
            *   `uint16_t w`, `uint16_t h`: A dialógus szélessége és magassága.
            *   `const char *title`: A dialógus címe.
            *   `const char *msg`: A megjelenítendő üzenet.
            *   `const char *okButtonLabel`: Az OK gomb címkéje.
            *   `const char *cancelButtonLabel`: A Cancel gomb címkéje.
    *   `virtual ~MessageDialog()`:
        *   **Szerepe:** Destruktor.
    *   `virtual void draw() override`:
        *   **Szerepe:** A dialógus tartalmának kirajzolása.
        *   **Felülírás:** Felülírja a `DialogBase` osztály `draw()` absztrakt metódusát.
    * `virtual bool handleTouch(bool touched, uint16_t tx, uint16_t ty) override`:
        * **Szerepe**: A touch esemény kezelése a dialógusban.
        * **Paraméterek**:
            * `bool touched`: `true`, ha történt érintés.
            * `uint16_t tx`, `uint16_t ty`: A touch X és Y koordinátái.
        *   **Visszatérési érték:** `false`, ha nem kezelt eseményt, `true`, ha kezelt.
        *   **Felülírás:** Felülírja a `DialogBase` osztály `handleTouch()` metódusát.

## `MultiButtonDialog.h` - Többgombos Dialógus Osztály

### `#include` Direktívák

*   `#include "DialogBase.h"`: A `DialogBase` osztály definíciójának beillesztése.

### `MultiButtonDialog` Osztály

*   **Származtatás:** `public DialogBase`
*   **Szerepe:** Több gombot tartalmazó dialógus megjelenítése.
*   **Tagváltozók:**
    *   `const char *title`: A dialógus címe.
    *   `const char **buttonLabels`: A gombok címkéit tartalmazó tömb.
    *   `int buttonsCount`: A gombok száma.
*   **Metódusok:**
    *   `MultiButtonDialog(IDialogParent *dialogParent, TFT_eSPI &tft, uint16_t w, uint16_t h, const char *title, const char **buttonLabels, int buttonsCount)`:
        *   **Szerepe:** Konstruktor.
        *   **Paraméterek:**
            *   `IDialogParent *dialogParent`: Pointer a dialógust létrehozó szülő objektumra.
            *   `TFT_eSPI &tft`: Referencia a TFT kijelző objektumra.
            *   `uint16_t w`, `uint16_t h`: A dialógus szélessége és magassága.
            *   `const char *title`: A dialógus címe.
            *   `const char **buttonLabels`: A gombok címkéit tartalmazó tömb.
            *   `int buttonsCount`: A gombok száma.
    *   `virtual ~MultiButtonDialog()`:
        *   **Szerepe:** Destruktor.
    *   `virtual void draw() override`:
        *   **Szerepe:** A dialógus tartalmának kirajzolása.
        *   **Felülírás:** Felülírja a `DialogBase` osztály `draw()` absztrakt metódusát.
    * `virtual bool handleTouch(bool touched, uint16_t tx, uint16_t ty) override`:
        * **Szerepe**: A touch esemény kezelése a dialógusban.
        * **Paraméterek**:
            * `bool touched`: `true`, ha történt érintés.
            * `uint16_t tx`, `uint16_t ty`: A touch X és Y koordinátái.
        *   **Visszatérési érték:** `false`, ha nem kezelt eseményt, `true`, ha kezelt.
        *   **Felülírás:** Felülírja a `DialogBase` osztály `handleTouch()` metódusát.

## `ValueChangeDialog.h` - Érték Változtató Dialógus Osztály

### `#include` Direktívák

*   `#include "DialogBase.h"`: A `DialogBase` osztály definíciójának beillesztése.
* `<string.h>`: string függvényekhez.
* `<stdint.h>`: fix méretű int típusokhoz.

### `ValueChangeDialog` Osztály

*   **Származtatás:** `public DialogBase`
*   **Szerepe:** Érték megváltoztatására szolgáló dialógus megjelenítése.
*   **Tagváltozók:**
    *   `const char *title`: A dialógus címe.
    *   `const char *label`: Az érték címkéje.
    * `bool isFloat`: A változó lebegőpontos-e.
    * `union`: A változó értéke.
       * `bool *pBool`: Bool változó pointer.
       * `int *pInt`: Int változó pointer.
       * `float *pFloat`: Float változó pointer.
    *   `int32_t minInt`: A minimális int érték.
    *   `int32_t maxInt`: A maximális int érték.
    * `float minFloat`: A minimális float érték.
    * `float maxFloat`: A maximális float érték.
    * `float step`: A float változás lépésköze.
*   **Metódusok:**
    *   `ValueChangeDialog(IDialogParent *dialogParent, TFT_eSPI &tft, uint16_t w, uint16_t h, const char *title, const char *label, bool *value)`:
        *   **Szerepe:** Konstruktor, Bool változóhoz.
    *   `ValueChangeDialog(IDialogParent *dialogParent, TFT_eSPI &tft, uint16_t w, uint16_t h, const char *title, const char *label, int *value, int32_t minValue = 0, int32_t maxValue = 100, int32_t step = 1)`:
        *   **Szerepe:** Konstruktor, Int változóhoz.
    * `ValueChangeDialog(IDialogParent *dialogParent, TFT_eSPI &tft, uint16_t w, uint16_t h, const char *title, const char *label, float *value, float minValue, float maxValue, float step)`:
        *   **Szerepe:** Konstruktor, Float változóhoz.
    *   `virtual ~ValueChangeDialog()`:
        *   **Szerepe:** Destruktor.
    *   `virtual void draw() override`:
        *   **Szerepe:** A dialógus tartalmának kirajzolása.
        *   **Felülírás:** Felülírja a `DialogBase` osztály `draw()` absztrakt metódusát.
    * `virtual bool handleTouch(bool touched, uint16_t tx, uint16_t ty) override`:
        * **Szerepe**: A touch esemény kezelése a dialógusban.
        * **Paraméterek**:
            * `bool touched`: `true`, ha történt érintés.
            * `uint16_t tx`, `uint16_t ty`: A touch X és Y koordinátái.
        *   **Visszatérési érték:** `false`, ha nem kezelt eseményt, `true`, ha kezelt.
        *   **Felülírás:** Felülírja a `DialogBase` osztály `handleTouch()` metódusát.
    * `bool incValue()`:
        * **Szerepe**: Növeli az értéket.
    * `bool decValue()`:
        * **Szerepe**: Csökkenti az értéket.
    * `void setValue()`:
        * **Szerepe**: Beállítja az értéket a kijelzőn.

## Összegzés

A `MessageDialog.h`, `MultiButtonDialog.h`, és `ValueChangeDialog.h` fájlokban definiált osztályok a TFT-SPI Pico Rádió projekt különböző típusú dialógusainak megvalósítását tartalmazzák. Ezek az osztályok a `DialogBase` osztályból származnak, így öröklik annak közös funkcióit. A dialógus osztályok segítségével könnyen hozhatók létre üzenetek megjelenítésére, gombokkal ellátott választásokra, vagy értékek módosítására szolgáló felületek. A kód jól strukturált, moduláris, és követi az objektumorientált programozás elveit.




# `PicoMemoryInfo.h` - Memória Információk Dokumentációja

## Bevezetés

A `PicoMemoryInfo.h` fájl a TFT-SPI Pico Rádió projekt memóriainformációinak lekérdezésére és kiírására szolgáló függvényt definiálja. Ezzel a függvénnyel debug módban lekérdezhetők a memória aktuális állapota, a szabad memória és a minimális szabad memória információk.

## `#include` Direktívák

A fájl elején a következő include direktíva található:

*   `<Arduino.h>`: Az Arduino platform alapvető függvényeinek és definícióinak beillesztése.

## Függvények

*   `void debugMemoryInfo()`:
    *   **Szerepe:** Kiírja a memória aktuális állapotát a soros portra, debug célokra.
    *   **Működése:**
        1.  Lekérdezi a pillanatnyi szabad memória méretét a `rp2040.getFreeHeap()` függvény segítségével.
        2.  Lekérdezi a minimális szabad memória méretét a `rp2040.getMinFreeHeap()` függvény segítségével.
        3.  Lekérdezi a memória fragmentationt.
        4. Kiírja a soros portra a fenti információkat az alábbi formátumban:
            ```
            Heap: Free %lu, Min Free %lu, Fragmentation %02.02f%%
            ```

## Összegzés

A `PicoMemoryInfo.h` fájl egy egyszerű segédfüggvényt tartalmaz a memória állapotának kiírására. A `debugMemoryInfo()` függvény debug módban segíti a fejlesztőt a memória használatának ellenőrzésében, a memória szivárgások és más memóriakezelési hibák felderítésében.


