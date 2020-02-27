AudioPlayQueue           queue1;
AudioAnalyzePeak         peak1;
AudioMixer4              dlyFbMixer;
AudioMixer4              dlyMixer;
AudioConnection          patchCord0(queue1, peak1);
AudioConnection          patchCord1(queue1, 0, dlyFbMixer, 0);
AudioConnection          patchCord2(queue1, 0, dlyMixer, 0);
AudioConnection          patchCord3(delay1, 0, dlyFbMixer, 1);
AudioConnection          patchCord4(delay1, 0, dlyMixer, 2);
AudioConnection          patchCord5(dlyFbMixer, delay1);
AudioConnection          patchCord6(dlyFbMixer, 0, dlyMixer, 1);

#if defined(EXTERNAL_DELAY_RAM)
AudioEffectDelayExternal delay1;
#else
AudioEffectDelay         delay1;
#endif

#if defined(TEENSY_AUDIO_BOARD)
AudioOutputI2S           i2s1;
AudioConnection          patchCord7(dlyMixer, 0, i2s1, 0);
AudioConnection          patchCord8(dlyMixer, 0, i2s1, 1);
AudioControlSGTL5000     sgtl5000_1;
#elif defined(TGA_AUDIO_BOARD)
AudioOutputI2S           i2s1;
AudioAmplifier           volume_r;
AudioAmplifier           volume_l;
AudioConnection          patchCord7(dlyMixer, volume_r);
AudioConnection          patchCord8(dlyMixer, volume_l);
AudioConnection          patchCord9(volume_r, 0, i2s1, 1);
AudioConnection          patchCord10(volume_l, 0, i2s1, 0);
AudioControlWM8731master wm8731_1;
#elif defined(TEENSY_DAC)
AudioOutputAnalogStereo  dacOut;
AudioConnection          patchCord11(volume_r, 0, dacOut, 0);
AudioConnection          patchCord12(volume_l, 0, dacOut, 1);
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
AudioConnection          patchCord7(dlyMixer, 0, volume_master, 0);
AudioConnection          patchCord10(volume_r, 0, pt8211_1, 0);
AudioConnection          patchCord11(volume_l, 0, pt8211_1, 1);
AudioConnection          patchCord8(volume_master, volume_r);
AudioConnection          patchCord9(volume_master, volume_l);
#endif
