#ifndef GUITOOL_H_INCLUDED
#define GUITOOL_H_INCLUDED

#include "effect_modulated_delay_stereo.h"
#include "mixer2.h"

AudioPlayQueue            queue1;
AudioAnalyzePeak          peak1;
AudioMixer2               delayFbMixer;
AudioMixer2               delayMixer;
#if defined(EXTERNAL_DELAY_RAM)
AudioEffectDelayExternal  delay1;
#else
AudioEffectDelay          delay1;
#endif
AudioFilterStateVariable  delayFilter;
AudioEffectWaveshaper     queueWaveshaper;
AudioEffectWaveshaper     delayWaveshaper;

AudioFilterStateVariable  masterFilter;
AudioEffectEnvelope       filterEnv;
AudioSynthWaveformDc      dcOneVolt;
AudioSynthWaveformDc      filterCutoff;
AudioMixer2               filterModMixer;

AudioEffectModulatedDelayStereo chorus1;
AudioMixer2               chorusMixer_l;
AudioMixer2               chorusMixer_r;
AudioFilterBiquad         chorusFilter_l;
AudioFilterBiquad         chorusFilter_r;
AudioSynthWaveform        modulator;
AudioMixer2               inverter;

AudioConnection          patchCord0(dcOneVolt, 0, filterEnv, 0);
AudioConnection          patchCord1(queue1, 0, queueWaveshaper, 0);
AudioConnection          patchCord2(queueWaveshaper, 0, masterFilter, 0);
AudioConnection          patchCord3(masterFilter, 0, peak1, 0);
AudioConnection          patchCord4(dcOneVolt, 0, filterModMixer, 0);
AudioConnection          patchCord5(filterEnv, 0, filterModMixer, 1);
AudioConnection          patchCord6(filterModMixer, 0, masterFilter, 1);

AudioConnection          patchCord7(masterFilter, 0, delayFbMixer, 0);
AudioConnection          patchCord8(delayFbMixer, 0, delayWaveshaper, 0);
AudioConnection          patchCord9(delayWaveshaper, 0, delayFilter, 0);
AudioConnection          patchCord10(delayFilter, 0, delay1, 0);
AudioConnection          patchCord11(delay1, 0, delayFbMixer, 1);

AudioConnection          patchCord12(masterFilter, 0, delayMixer, 0);
AudioConnection          patchCord13(delay1, 0, delayMixer, 1);
AudioConnection          patchCord14(delayMixer, 0, chorus1, 0);
AudioConnection          patchCord16(modulator, 0, chorus1, 1);
AudioConnection          patchCord17(modulator, inverter);
AudioConnection          patchCord18(inverter, 0, chorus1, 2);
AudioConnection          patchCord19(chorus1, 0, chorusFilter_l, 0);
AudioConnection          patchCord20(chorus1, 1, chorusFilter_r, 0);
AudioConnection          patchCord21(delayMixer, 0, chorusMixer_l, 0);
AudioConnection          patchCord22(delayMixer, 0, chorusMixer_r, 0);
AudioConnection          patchCord23(chorusFilter_l, 0, chorusMixer_l, 1);
AudioConnection          patchCord24(chorusFilter_r, 0, chorusMixer_r, 1);

#if defined(TEENSY_AUDIO_BOARD)
AudioOutputI2S           i2s1;
AudioConnection          patchCord25(chorusMixer_l, 0, i2s1, 0);
AudioConnection          patchCord26(chorusMixer_r, 0, i2s1, 1);
AudioControlSGTL5000     sgtl5000_1;

#elif defined(TGA_AUDIO_BOARD)
AudioOutputI2S           i2s1;
AudioAmplifier           volume_r;
AudioAmplifier           volume_l;
AudioConnection          patchCord25(delayMixer, 0, volume_l, 0);
AudioConnection          patchCord26(delayMixer, 0, volume_r, 0);
AudioConnection          patchCord27(volume_l, 0, i2s1, 0);
AudioConnection          patchCord28(volume_r, 0, i2s1, 1);
AudioControlWM8731master wm8731_1;

#elif defined(TEENSY_DAC)
AudioOutputAnalogStereo  dacOut;
AudioConnection          patchCord25(volume_l, 0, dacOut, 0);
AudioConnection          patchCord26(volume_r, 0, dacOut, 1);

#elif defined(TEENSY_DAC_SYMMETRIC)
AudioOutputAnalogStereo  dacOut;
AudioMixer4              invMixer;
AudioConnection          patchCord25(volume_l, 0, dacOut  , 0);
AudioConnection          patchCord26(volume_l, 0, invMixer, 0);
AudioConnection          patchCord27(invMixer, 0, dacOut  , 1);

#else
AudioOutputPT8211        pt8211_1;
AudioAmplifier           volume_master;
AudioAmplifier           volume_r;
AudioAmplifier           volume_l;
AudioConnection          patchCord25(delayMixer, 0, volume_master, 0);
AudioConnection          patchCord26(volume_master, 0, volume_l, 0);
AudioConnection          patchCord27(volume_master, 0, volume_r, 0);
AudioConnection          patchCord28(volume_l, 0, pt8211_1, 0);
AudioConnection          patchCord28(volume_r, 0, pt8211_1, 1);
#endif

#endif //GUITOOL_H_INCLUDED
