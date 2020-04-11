/*
Audio Library for Teensy 3.X
Copyright (c) 2014, Pete (El Supremo)
Copyright (c) 2019, Holger Wirtz
*/

#include <Arduino.h>
#include <Audio.h>
#include "effect_modulated_delay_stereo.h"
#include "config.h"

extern config_t configuration;

/******************************************************************/

// Based on;      A u d i o E f f e c t D e l a y
// Written by Pete (El Supremo) Jan 2014
// 140529 - change to handle mono stream - change modify() to voices()
// 140219 - correct storage class (not static)
// 190527 - added modulation input (by Holger Wirtz)

#ifndef ARM_Q15_TO_FLOAT
#define ARM_Q15_TO_FLOAT
// pasted function from arm_math.h due to include problems
void arm_q15_to_float(q15_t * pSrc, float32_t * pDst, uint32_t blockSize) {
  q15_t *pIn = pSrc;                             /* Src pointer */
  uint32_t blkCnt;                               /* loop counter */

#ifndef ARM_MATH_CM0_FAMILY

  /* Run the below code for Cortex-M4 and Cortex-M3 */

  /*loop Unrolling */
  blkCnt = blockSize >> 2u;

  /* First part of the processing with loop unrolling.  Compute 4 outputs at a time.
   ** a second loop below computes the remaining 1 to 3 samples. */
  while(blkCnt > 0u)
  {
    /* C = (float32_t) A / 32768 */
    /* convert from q15 to float and then store the results in the destination buffer */
    *pDst++ = ((float32_t) * pIn++ / 32768.0f);
    *pDst++ = ((float32_t) * pIn++ / 32768.0f);
    *pDst++ = ((float32_t) * pIn++ / 32768.0f);
    *pDst++ = ((float32_t) * pIn++ / 32768.0f);

    /* Decrement the loop counter */
    blkCnt--;
  }

  /* If the blockSize is not a multiple of 4, compute any remaining output samples here.
   ** No loop unrolling is used. */
  blkCnt = blockSize % 0x4u;

#else

  /* Run the below code for Cortex-M0 */

  /* Loop over blockSize number of values */
  blkCnt = blockSize;

#endif /* #ifndef ARM_MATH_CM0_FAMILY */

  while(blkCnt > 0u)
  {
    /* C = (float32_t) A / 32768 */
    /* convert from q15 to float and then store the results in the destination buffer */
    *pDst++ = ((float32_t) * pIn++ / 32768.0f);

    /* Decrement the loop counter */
    blkCnt--;
  }
}
#endif // ARM_Q15_TO_FLOAT

boolean AudioEffectModulatedDelayStereo::begin(short *delayline, uint16_t d_length) {
#if 0
  Serial.print(F("AudioEffectModulatedDelayStereo.begin(modulated-delay line length = "));
  Serial.print(d_length);
  Serial.println(F(")"));
#endif

  _delayline = NULL;
  _delay_length = 0;
  _cb_index = 0;
  _delay_offset = 0;

  if (delayline == NULL)
    return (false);
  if (d_length < 10)
    return (false);

  _delayline = delayline;
  _delay_length = d_length;
  memset(_delayline, 0, _delay_length * sizeof(int16_t));
  _delay_offset = _delay_length >> 1 ;
  return (true);
}

uint16_t AudioEffectModulatedDelayStereo::get_delay_length(void) {
  return (_delay_length);
}

void AudioEffectModulatedDelayStereo::update(void) {
  audio_block_t *block, *block2;
  audio_block_t *modulation, *modulation2;

  if (_delayline == NULL)
    return;

  block = receiveWritable(0);
  block2 = allocate();

  modulation = receiveReadOnly(1);
  modulation2 = receiveReadOnly(2);

  if (block && modulation) {
    int16_t *bp;
    int16_t *bp2;
    int16_t cb_mod_index_neighbor;
    int16_t cb_mod_index_neighbor2;
    float *mp;
    float *mp2;
    float mod_index;
    float mod_index2;
    float mod_number;
    float mod_number2;
    float mod_fraction;
    float mod_fraction2;
    float modulation_f32[AUDIO_BLOCK_SAMPLES];
    float modulation2_f32[AUDIO_BLOCK_SAMPLES];

    bp = block->data;
    bp2 = block2->data;
    arm_q15_to_float(modulation->data, modulation_f32, AUDIO_BLOCK_SAMPLES);
    mp = modulation_f32;
    arm_q15_to_float(modulation2->data, modulation2_f32, AUDIO_BLOCK_SAMPLES);
    mp2 = modulation2_f32; //TOOO try ~modulation_f32 or -modulation_f32

    for (uint16_t i = 0; i < AUDIO_BLOCK_SAMPLES; i++) {

      // write data into circular buffer (delayline)
      if (_cb_index >= _delay_length)
        _cb_index = 0;
      _delayline[_cb_index] = *bp;

      // calculate the modulation-index as a floating point number for interpolation
      mod_index = *mp * _delay_offset;
      mod_index2 = *mp2 * _delay_offset;

      // split float of mod_index into integer (= mod_number) and fraction part
      mod_fraction = modff(mod_index, &mod_number);
      mod_fraction2 = modff(mod_index2, &mod_number2);

      // calculate modulation index into circular buffer
      cb_mod_index = _cb_index - (_delay_offset + mod_number);
      cb_mod_index2 = _cb_index - (_delay_offset + mod_number2);

      // check for negative offsets and correct them
      if (cb_mod_index < 0)
        cb_mod_index += _delay_length;
      if (cb_mod_index2 < 0)
        cb_mod_index2 += _delay_length;

      if (cb_mod_index == _delay_length - 1)
        cb_mod_index_neighbor = 0;
      else
        cb_mod_index_neighbor = cb_mod_index + 1;

      if (cb_mod_index2 == _delay_length - 1)
        cb_mod_index_neighbor2 = 0;
      else
        cb_mod_index_neighbor2 = cb_mod_index2 + 1;

      *bp = round(float(_delayline[cb_mod_index]) * mod_fraction + float(_delayline[cb_mod_index_neighbor]) * (1.0 - mod_fraction));
      *bp2 = round(float(_delayline[cb_mod_index2]) * mod_fraction2 + float(_delayline[cb_mod_index_neighbor2]) * (1.0 - mod_fraction2));

      // push the pointers forward
      bp++; // next audio data
      bp2++; // next audio data
      mp++; // next modulation data
      mp2++; // next modulation data
      _cb_index++; // next circular buffer index
    }
  }

  if (modulation)
    release(modulation);

  if (modulation2)
    release(modulation2);

  if (block) {
    transmit(block, 0);
    release(block);
  }
  if (block2) {
    transmit(block2, 1);
    release(block2);
  }
}
