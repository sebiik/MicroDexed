#ifndef GUITOOL_H_INCLUDED
#define GUITOOL_H_INCLUDED

// #include "mixer2.h"

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

AudioEffectModulatedDelay chorus_l;
AudioEffectModulatedDelay chorus_r;
AudioMixer2               chorusMixer_l;
AudioMixer2               chorusMixer_r;
AudioFilterBiquad         chorusFilter_l;
AudioFilterBiquad         chorusFilter_r;
AudioSynthWaveform        modulator;
AudioMixer2               inverter;

AudioConnection          patchCord70(dcOneVolt, 0, filterEnv, 0);
AudioConnection          patchCord71(queue1, 0, queueWaveshaper, 0);
AudioConnection          patchCord72(queueWaveshaper, 0, masterFilter, 0);
AudioConnection          patchCord0(masterFilter, 0, peak1, 0);
AudioConnection          patchCord73(dcOneVolt, 0, filterModMixer, 0);
AudioConnection          patchCord74(filterEnv, 0, filterModMixer, 1);
AudioConnection          patchCord75(filterModMixer, 0, masterFilter, 1);

AudioConnection          patchCord1(masterFilter, 0, delayFbMixer, 0);
AudioConnection          patchCord212(delayFbMixer, 0, delayWaveshaper, 0);
AudioConnection          patchCord303(delayWaveshaper, 0, delayFilter, 0);
AudioConnection          patchCord3(delayFilter, 0, delay1, 0);
AudioConnection          patchCord313(delay1, 0, delayFbMixer, 1);
// additional delay taps
// AudioConnection          patchCord314(delay1, 1, delayFbMixer, 2);
// AudioConnection          patchCord315(delay1, 2, delayFbMixer, 3);


AudioConnection          patchCord5(masterFilter, 0, delayMixer, 0);
AudioConnection          patchCord6(delay1, 0, delayMixer, 1);
AudioConnection          patchCord7(delayMixer, 0, chorus_l, 0);
AudioConnection          patchCord8(delayMixer, 0, chorus_r, 0);
AudioConnection          patchCord21(modulator, 0, chorus_l, 1);
AudioConnection          patchCord22(modulator, inverter);
AudioConnection          patchCord23(inverter, 0, chorus_r, 1);
AudioConnection          patchCord9(chorus_l, 0, chorusFilter_l, 0);
AudioConnection          patchCord10(chorus_r, 0, chorusFilter_r, 0);
AudioConnection          patchCord297(delayMixer, 0, chorusMixer_l, 0);
AudioConnection          patchCord298(delayMixer, 0, chorusMixer_r, 0);
AudioConnection          patchCord11(chorusFilter_l, 0, chorusMixer_l, 1);
AudioConnection          patchCord12(chorusFilter_r, 0, chorusMixer_r, 1);

#if defined(TEENSY_AUDIO_BOARD)
AudioOutputI2S           i2s1;
AudioConnection          patchCord13(chorusMixer_l, 0, i2s1, 0);
AudioConnection          patchCord14(chorusMixer_r, 0, i2s1, 1);
AudioControlSGTL5000     sgtl5000_1;
#elif defined(TGA_AUDIO_BOARD)
AudioOutputI2S           i2s1;
AudioAmplifier           volume_r;
AudioAmplifier           volume_l;
AudioConnection          patchCord9(delayMixer, 0, volume_l, 0);
AudioConnection          patchCord10(delayMixer, 0, volume_r, 0);
AudioConnection          patchCord11(volume_l, 0, i2s1, 0);
AudioConnection          patchCord12(volume_r, 0, i2s1, 1);
AudioControlWM8731master wm8731_1;
#elif defined(TEENSY_DAC)
AudioOutputAnalogStereo  dacOut;
AudioConnection          patchCord11(volume_l, 0, dacOut, 0);
AudioConnection          patchCord12(volume_r, 0, dacOut, 1);
#elif defined(TEENSY_DAC_SYMMETRIC)
AudioOutputAnalogStereo  dacOut;
AudioMixer4              invMixer;
AudioConnection          patchCord11(volume_l, 0, dacOut  , 0);
AudioConnection          patchCord12(volume_l, 0, invMixer, 0);
AudioConnection          patchCord13(invMixer, 0, dacOut  , 1);
#else
AudioOutputPT8211        pt8211_1;
AudioAmplifier           volume_master;
AudioAmplifier           volume_r;
AudioAmplifier           volume_l;
AudioConnection          patchCord9(delayMixer, 0, volume_master, 0);
AudioConnection          patchCord10(volume_master, 0, volume_l, 0);
AudioConnection          patchCord11(volume_master, 0, volume_r, 0);
AudioConnection          patchCord12(volume_l, 0, pt8211_1, 0);
AudioConnection          patchCord13(volume_r, 0, pt8211_1, 1);
#endif

#endif //GUITOOL_H_INCLUDED
