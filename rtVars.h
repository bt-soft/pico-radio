#ifndef __RTVARS_H
#define __RTVARS_H

#include <cstdint>

/**
 *
 */
namespace rtv {

// Némítás
extern bool mute;

// Frekvencia kijelzés pozíciója
extern uint16_t freqDispX;
extern uint16_t freqDispY;

// BFO
extern bool bfoOn;

// Mute
#define AUDIO_MUTE_ON true
#define AUDIO_MUTE_OFF false
extern bool muteStat;

// Squelch
#define SQUELCH_DECAY_TIME 500
#define MIN_SQUELCH 0
#define MAX_SQUELCH 50
extern long squelchDecay;

// Scan
extern bool SCANbut;
extern bool SCANpause;

// Seek
extern bool SEEK;

// CW shift
extern bool CWShift;

}  // namespace rtv

#endif  //__RTVARS_H
