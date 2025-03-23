#include "rtVars.h"

#include "defaults.h"

namespace rtv {

// Némítás
bool mute = false;

// Frekvencia kijelzés pozíciója
uint16_t freqDispX = 0;
uint16_t freqDispY = 0;

// BFO
bool bfoOn = false;

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