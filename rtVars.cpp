#include "rtVars.h"

#include "defaults.h"

namespace rtv {

// Némítás
bool mute = false;

// Frekvencia kijelzés pozíciója
uint16_t freqDispX = 0;
uint16_t freqDispY = 0;

// Frekvencia lépés
// uint16_t freqstep = 1000;  // frekvencia lépés értéke Hz-ben
// freqstepnr==0 -> freqstep = 1000 Hz;
// freqstepnr==1 -> freqstep = 100 Hz;
// freqstepnr==2 -> freqstep = 10 Hz;
uint8_t freqstepnr = 0;  // A frekvencia kijelző digit száma, itt jelezzük SSB/CW-ben, hogy mi a frekvencia lépés
int freqDec = 0;

// BFO
bool bfoOn = false;
bool bfoTr = false;  // A BFO frekvencia kijelzés miatt a frekvencia méretének az animált csökkentése flag

// Mute
bool muteStat = false;

// Squelch
long squelchDecay = 0;

// Scan
bool SCANbut = false;   // Scan aktív?
bool SCANpause = true;  // LWH - SCANpause must be initialized to a value else the squelch function will

// Seek
bool SEEK = false;

// CW shift
bool CWShift = false;

}  // namespace rtv