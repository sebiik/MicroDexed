// Init effects
delay1.delay(0, mapfloat(effect_delay_time, 10, ENC_DELAY_TIME_STEPS, 0.0, DELAY_MAX_TIME));
delayFilter.frequency(2000);
delayFilter.resonance(0.7);
delayFbMixer.gain(DRY_SIGNAL, 1.0);
delayFbMixer.gain(WET_SIGNAL, mapfloat(effect_delay_feedback, 0, ENC_DELAY_FB_STEPS, 0.0, 0.8)); // delay feedback
delayMixer.gain(DRY_SIGNAL, 1.0); // TODO do the sin/cos calculation
delayMixer.gain(WET_SIGNAL, mapfloat(effect_delay_volume, 0, ENC_DELAY_VOLUME_STEPS, 0.0, 1.0)); // dry/wet signal (1.0 = full wet)
// dexed->fx.Gain =  1.0;
// dexed->fx.Reso = 1.0;// - float(effect_filter_resonance) / ENC_FILTER_RES_STEPS;
// dexed->fx.Cutoff = 0.5;// - float(effect_filter_cutoff) / ENC_FILTER_CUT_STEPS;

queueWaveshaper.shape(waveshape,9);
delayWaveshaper.shape(waveshape,9);
dcOneVolt.amplitude(1.0);
masterFilter.octaveControl(7);
masterFilter.frequency(40);
masterFilter.resonance(0.7);
filterModMixer.gain(0, 1);
filterModMixer.gain(1, 1);
filterEnv.releaseNoteOn(1);
filterEnv.attack(0.1);
filterEnv.decay(500);
filterEnv.sustain(0.5);
filterEnv.release(10000);

inverter.gain(0,-1.0);
chorus1.begin(chorus_delayline, CHORUS_SAMPLE_BUFFER);
modulator.begin(WAVEFORM_SINE);
modulator.frequency(0.5);
modulator.amplitude(0.25);
chorusFilter_r.setLowpass(0, CHORUS_FILTER_CUTOFF_HZ, 0.707);
chorusFilter_l.setLowpass(0, CHORUS_FILTER_CUTOFF_HZ, 0.707);
chorusMixer_l.gain(DRY_SIGNAL, 1.0);
chorusMixer_r.gain(DRY_SIGNAL, 1.0);
chorusMixer_l.gain(WET_SIGNAL, 0);
chorusMixer_r.gain(WET_SIGNAL, 0);
