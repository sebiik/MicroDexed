/*
sebiiksbcs 2020
*/

#ifndef MIXER2_H_
#define MIXER2_H_

#include "AudioStream.h"

class AudioMixer2 : public AudioStream
{
#if defined(__ARM_ARCH_7EM__)
public:
	AudioMixer2(void) : AudioStream(2, inputQueueArray) {
		for (int i=0; i<2; i++) multiplier[i] = 65536;
	}
	virtual void update(void);
	void gain(unsigned int channel, float gain) {
		if (channel >= 2) return;
		if (gain > 32767.0f) gain = 32767.0f;
		else if (gain < -32767.0f) gain = -32767.0f;
		multiplier[channel] = gain * 65536.0f; // TODO: proper roundoff?
	}
private:
	int32_t multiplier[2];
	audio_block_t *inputQueueArray[2];

#elif defined(KINETISL)
public:
	AudioMixer4(void) : AudioStream(2, inputQueueArray) {
		for (int i=0; i<2; i++) multiplier[i] = 256;
	}
	virtual void update(void);
	void gain(unsigned int channel, float gain) {
		if (channel >= 2) return;
		if (gain > 127.0f) gain = 127.0f;
		else if (gain < -127.0f) gain = -127.0f;
		multiplier[channel] = gain * 256.0f; // TODO: proper roundoff?
	}
private:
	int16_t multiplier[2];
	audio_block_t *inputQueueArray[2];
#endif
};

#endif // MIXER2_H_
