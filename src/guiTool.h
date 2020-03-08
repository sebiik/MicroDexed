AudioPlayQueue           queue1;
AudioAnalyzePeak         peak1;
AudioMixer4              delayFbMixer;
AudioMixer4              delayMixer;
#if defined(EXTERNAL_DELAY_RAM)
AudioEffectDelayExternal delay1;
#else
AudioEffectDelay         delay1;
#endif
AudioFilterStateVariable delayFilter;

AudioConnection          patchCord0(queue1, peak1);
AudioConnection          patchCord1(queue1, 0, delayFbMixer, 0);
AudioConnection          patchCord2(delayFbMixer, 0, delayFilter, 0);
AudioConnection          patchCord3(delayFilter, 0, delay1, 0);
AudioConnection          patchCord4(delay1, 0, delayFbMixer, 1);

AudioConnection          patchCord5(queue1, 0, delayMixer, 0);
AudioConnection          patchCord6(delay1, 0, delayMixer, 1);

#if defined(TEENSY_AUDIO_BOARD)
AudioOutputI2S           i2s1;
AudioConnection          patchCord9(delayMixer, 0, i2s1, 0);
AudioConnection          patchCord10(delayMixer, 0, i2s1, 1);
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
