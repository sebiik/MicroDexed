#include <Arduino.h>
#include "mixer2.h"
#include "utility/dspinst.h"

#if defined(__ARM_ARCH_7EM__)
#define MULTI_UNITYGAIN 65536

static void applyGain(int16_t *data, int32_t mult)
{
	uint32_t *p = (uint32_t *)data;
	const uint32_t *end = (uint32_t *)(data + AUDIO_BLOCK_SAMPLES);

	do {
		uint32_t tmp32 = *p; // read 2 samples from *data
		int32_t val1 = signed_multiply_32x16b(mult, tmp32);
		int32_t val2 = signed_multiply_32x16t(mult, tmp32);
		val1 = signed_saturate_rshift(val1, 16, 0);
		val2 = signed_saturate_rshift(val2, 16, 0);
		*p++ = pack_16b_16b(val2, val1);
	} while (p < end);
}

static void applyGainThenAdd(int16_t *data, const int16_t *in, int32_t mult)
{
	uint32_t *dst = (uint32_t *)data;
	const uint32_t *src = (uint32_t *)in;
	const uint32_t *end = (uint32_t *)(data + AUDIO_BLOCK_SAMPLES);

	if (mult == MULTI_UNITYGAIN) {
		do {
			uint32_t tmp32 = *dst;
			*dst++ = signed_add_16_and_16(tmp32, *src++);
			tmp32 = *dst;
			*dst++ = signed_add_16_and_16(tmp32, *src++);
		} while (dst < end);
	} else {
		do {
			uint32_t tmp32 = *src++; // read 2 samples from *data
			int32_t val1 = signed_multiply_32x16b(mult, tmp32);
			int32_t val2 = signed_multiply_32x16t(mult, tmp32);
			val1 = signed_saturate_rshift(val1, 16, 0);
			val2 = signed_saturate_rshift(val2, 16, 0);
			tmp32 = pack_16b_16b(val2, val1);
			uint32_t tmp32b = *dst;
			*dst++ = signed_add_16_and_16(tmp32, tmp32b);
		} while (dst < end);
	}
}

#elif defined(KINETISL)
#define MULTI_UNITYGAIN 256

static void applyGain(int16_t *data, int32_t mult)
{
	const int16_t *end = data + AUDIO_BLOCK_SAMPLES;

	do {
		int32_t val = *data * mult;
		*data++ = signed_saturate_rshift(val, 16, 0);
	} while (data < end);
}

static void applyGainThenAdd(int16_t *dst, const int16_t *src, int32_t mult)
{
	const int16_t *end = dst + AUDIO_BLOCK_SAMPLES;

	if (mult == MULTI_UNITYGAIN) {
		do {
			int32_t val = *dst + *src++;
			*dst++ = signed_saturate_rshift(val, 16, 0);
		} while (dst < end);
	} else {
		do {
			int32_t val = *dst + ((*src++ * mult) >> 8); // overflow possible??
			*dst++ = signed_saturate_rshift(val, 16, 0);
		} while (dst < end);
	}
}
#endif

void AudioMixer2::update(void) {
	audio_block_t *in, *out=NULL;
	unsigned int channel;

	for (channel=0; channel < 2; channel++) {
		if (!out) {
			out = receiveWritable(channel);
			if (out) {
				int32_t mult = multiplier[channel];
				if (mult != MULTI_UNITYGAIN) applyGain(out->data, mult);
			}
		} else {
			in = receiveReadOnly(channel);
			if (in) {
				applyGainThenAdd(out->data, in->data, multiplier[channel]);
				release(in);
			}
		}
	}
	if (out) {
		transmit(out);
		release(out);
	}
}

// void AudioAmplifier::update(void) {
// 	audio_block_t *block;
// 	int32_t mult = multiplier;
//
// 	if (mult == 0) {
// 		// zero gain, discard any input and transmit nothing
// 		block = receiveReadOnly(0);
// 		if (block) release(block);
// 	} else if (mult == MULTI_UNITYGAIN) {
// 		// unity gain, pass input to output without any change
// 		block = receiveReadOnly(0);
// 		if (block) {
// 			transmit(block);
// 			release(block);
// 		}
// 	} else {
// 		// apply gain to signal
// 		block = receiveWritable(0);
// 		if (block) {
// 			applyGain(block->data, mult);
// 			transmit(block);
// 			release(block);
// 		}
// 	}
// }
