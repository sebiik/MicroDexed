/*
Audio Library for Teensy 3.X
Copyright (c) 2014, Pete (El Supremo)
Copyright (c) 2019, Holger Wirtz
*/

#ifndef EFFECT_MODULATED_DELAY_STEREO_H_
#define EFFECT_MODULATED_DELAY_STEREO_H_

#include "Arduino.h"
#include "AudioStream.h"
#include "config.h"

/*************************************************************************/
//                A u d i o E f f e c t M o d u l a t e d D e l a y
// Written by Pete (El Supremo) Jan 2014
// 140219 - correct storage class (not static)
// 190527 - added modulation input handling (Aug 2019 by Holger Wirtz)

class AudioEffectModulatedDelayStereo : public AudioStream {
  public:
    AudioEffectModulatedDelayStereo(void):
      AudioStream(3, inputQueueArray) // has 3 inputs
      { }

    boolean begin(short *delayline, uint16_t delay_length);
    virtual void update(void);
    virtual uint16_t get_delay_length(void);

  private:
    audio_block_t *inputQueueArray[3]; // has 3 inputs
    int16_t *_delayline;   // pointer for the circular buffer
    uint16_t _cb_index;    // current write pointer of the circular buffer
    uint16_t _delay_length; // calculated number of samples of the delay
    int16_t cb_mod_index;  // current read pointer with modulation for the circular buffer
    int16_t cb_mod_index2; // current (right channel) read pointer with modulation for the circular buffer
    uint16_t _delay_offset;
};

#endif // EFFECT_MODULATED_DELAY_STEREO_H_
