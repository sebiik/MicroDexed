/* Seb Arduino upload settings:
Board: "Teensy 3.5/3.6"
USB Type: "Serial + MIDI"
CPU Speed: "120 MHz"
Optimize: "Fastest + pure-code"
*/

#ifndef CONFIG_H_INCLUDED
#define CONFIG_H_INCLUDED

#include "midinotes.h"

// ATTENTION! For better latency you have to redefine AUDIO_BLOCK_SAMPLES from
// 128 to 64 in <ARDUINO-IDE-DIR>/cores/teensy3/AudioStream.h

#define VERSION "0.9.4s"

//******************************************************************************
//* DEVICE SETTINGS

//******************************************************************************
//* DEBUG OUTPUT SETTINGS
#define DEBUG 1
#define SERIAL_SPEED 38400
#define SHOW_XRUN 1
#define SHOW_CPU_LOAD_MSEC 3000

// MIDI
#define MIDI_DEVICE_DIN Serial1
// #define MIDI_DEVICE_USB 1
// #define MIDI_DEVICE_USB_HOST 1
#define MIDI_DEVICE_NUMBER 0

// AUDIO
// If nothing is defined PT8211 is used as audio output device!
#define TEENSY_AUDIO_BOARD 1
// If the Teensy Audio Board is equipped with an 23LC1024 RAM chip, use it for the delay
#define EXTERNAL_DELAY_RAM 1 //seb
#ifdef EXTERNAL_DELAY_RAM
#define DELAY_MAX_TIME 1500.0
#endif

//#define TGA_AUDIO_BOARD

// Left and right channel audio signal is presented on pins A21 and A22.
// #define TEENSY_DAC


//******************************************************************************
//* MIDI SETTINGS

// Midi channel 7 for DX-7.. easier to remember
#define DEFAULT_MIDI_CHANNEL 7 //MIDI_CHANNEL_OMNI
// #define MIDI_MERGE_THRU 1
#define DEFAULT_SYSEXBANK 0
#define DEFAULT_SYSEXSOUND 0

//******************************************************************************
//* DEXED AND EFECTS SETTINGS
//#define DEXED_ENGINE DEXED_ENGINE_MODERN // DEXED_ENGINE_MARKI // DEXED_ENGINE_OPL
#define DEXED_ENGINE DEXED_ENGINE_MARKI

// CHORUS parameters
#define CHORUS_SAMPLE_BUFFER (2*AUDIO_BLOCK_SAMPLES)
#define CHORUS_FILTER_CUTOFF_HZ 12000

//******************************************************************************
//* AUDIO SETTINGS
#define VOLUME 0.8
#define VOLUME_CURVE 0.07
#ifndef TEENSY_AUDIO_BOARD
#if AUDIO_BLOCK_SAMPLES == 64
#define AUDIO_MEM 450
#else
#define AUDIO_MEM 225
#endif
#define DELAY_MAX_TIME 600.0
#define REDUCE_LOUDNESS_FACTOR 0.25
#else
#if AUDIO_BLOCK_SAMPLES == 64
#define AUDIO_MEM 900
#else
#define AUDIO_MEM 450
#endif
#define DELAY_MAX_TIME 1500.0
#define REDUCE_LOUDNESS_FACTOR 0.25
#endif
#define SAMPLE_RATE 44100
#define NORMALIZE_DX_VELOCITY 1

//******************************************************************************
//* UI AND DATA-STORE SETTINGS
#define CONTROL_RATE_MS 200
#define TIMER_UI_HANDLING_MS 64
#define TIMER_UI_HANDLING_MS_ADC 16


//******************************************************************************
//* HARDWARE SETTINGS
#define SGTL5000_LINEOUT_LEVEL 29

// SD card on Teensy Audio Shield:
//#define SDCARD_CS_PIN    10
#define SDCARD_MOSI_PIN  7
#define SDCARD_SCK_PIN   14

// Teensy 3.5 & 3.6 SD card
#define SDCARD_CS_PIN    BUILTIN_SDCARD
// #define SDCARD_MOSI_PIN  11  // not actually used
// #define SDCARD_SCK_PIN   13  // not actually used

/* Encoders, buttons settings */
#define ENC_VOL_STEPS 127
#define ENC_FILTER_RES_STEPS 127
#define ENC_FILTER_CUT_STEPS 127
#define ENC_DELAY_TIME_STEPS 127
#define ENC_DELAY_FB_STEPS 127
#define ENC_DELAY_VOLUME_STEPS 127
#define NUM_ENCODER 2
#define ENC_L_PIN_A  3
#define ENC_L_PIN_B  2
#define BUT_L_PIN    4
#define INITIAL_ENC_L_VALUE 0
#define ENC_R_PIN_A  28
#define ENC_R_PIN_B  29
#define BUT_R_PIN    30
#define INITIAL_ENC_R_VALUE 0
#define BUT_DEBOUNCE_MS 10
#define LONG_BUTTON_PRESS 750

// LCD Display
#define I2C_DISPLAY 1
// [I2C] SCL: Pin 19, SDA: Pin 18 (https://www.pjrc.com/teensy/td_libs_Wire.html)
#define LCD_I2C_ADDRESS 0x3E
#define LCD_CHARS 16
#define LCD_LINES 2
#define UI_AUTO_BACK_MS 3000
#define AUTOSTORE_MS 5000
#define AUTOSTORE_FAST_MS 50

// EEPROM address
#define EEPROM_START_ADDRESS 0

#define MAX_BANKS 100
#define MAX_VOICES 32 // voices per bank
#define BANK_NAME_LEN 13 // FAT12 filenames (plus '\0')
#define VOICE_NAME_LEN 11 // 10 (plus '\0')



//******************************************************************************
//* DO NO CHANGE ANYTHING BEYOND IF YOU DON'T KNOW WHAT YOU ARE DOING !!!
// MIDI
#ifdef MIDI_DEVICE_USB
#define USBCON 1
#endif
#if defined(__MK66FX1M0__) // Teensy-3.6
// Teensy-3.6 settings
// #define MIDI_DEVICE_USB_HOST 1
#define MAX_NOTES 16
#else
// Teensy-3.5 settings
#undef MIDI_DEVICE_USB_HOST
#define MAX_NOTES 6
#endif
#define TRANSPOSE_FIX 24

// Audio
#ifdef TGA_AUDIO_BOARD
#define REDUCE_LOUDNESS 2
#endif

// Some optimizations
#define USE_TEENSY_DSP 1
#define SUM_UP_AS_INT 1

// struct for holding the current configuration
struct config_t {
  uint32_t checksum;
  uint8_t bank;
  uint8_t voice;
  float vol;
  float pan;
  uint8_t midi_channel;
};

/* HELPER MACROS */
#define TIME_MS2SAMPLES(x) floor(uint32_t(x) * AUDIO_SAMPLE_RATE / 1000)
#define SAMPLES2TIME_MS(x) float(uint32_t(x) * 1000 / AUDIO_SAMPLE_RATE)

#endif // CONFIG_H_INCLUDED
