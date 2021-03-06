#include "config.h"
#include <Audio.h>
// #include <Wire.h>
// #include <SPI.h>
#include <SD.h>
#include <MIDI.h>
// #include <EEPROM.h>
#include "EEPROMAnything.h"
#include "midi_devices.hpp"
// #include <limits.h>
#include "dexed.h"
#include "dexed_sysex.h"
// #include "PluginFx.h"

// TODO follow codeberg continue from f0cbc

#ifdef I2C_DISPLAY
#include "UI.h"
// #include <Bounce.h>
// #include "Encoder4.h"
#include <hd44780.h>
#include <hd44780ioClass/hd44780_I2Clcd.h> // include i/o class header
#endif

#include "defines.h"
#include "adc.h"
// #include "mixer2.h"
// #include "effect_modulated_delay_stereo.h"
#include "variables.h"
#include "GUITool.h"


/******************************** Setup ********************************/
void setup() {

    #ifdef I2C_DISPLAY
    lcd.begin(16, 2);
    lcd.noBlink();
    lcd.noCursor();
    lcd.backlight();
    lcd.noAutoscroll();
    lcd.lineWrap();
    lcd.clear();
    lcd.display();
    lcd.print(F("MicroDexed"));
    lcd.write(VERSION);
    lcd.setCursor(0, 1);
    lcd.print(F("sebiiksbcs edit "));
    delay(500);

    pinMode(BUT_L_PIN, INPUT_PULLUP);
    pinMode(BUT_R_PIN, INPUT_PULLUP);
    #endif

    //while (!Serial) ; // wait for Arduino Serial Monitor
    Serial.begin(SERIAL_SPEED);
    delay(100);

    Serial.println(F("MicroDexed based on https://github.com/asb2m10/dexed"));
    Serial.println(F("(c)2018,2019 H. Wirtz <wirtz@parasitstudio.de>"));
    Serial.println(F("edited 2019,2020 by sebiiksbcs"));

    Serial.print(F("Version: "));
    Serial.println(VERSION);
    Serial.println(F("<setup start>"));

    initial_values_from_eeprom();

    setup_midi_devices();

    AudioNoInterrupts();
    AudioMemory(AUDIO_MEM);

    #ifdef TEENSY_AUDIO_BOARD
    sgtl5000_1.enable(); sgtl5000_1.dacVolumeRampLinear();
    sgtl5000_1.unmuteHeadphone(); sgtl5000_1.unmuteLineout();
    sgtl5000_1.autoVolumeDisable(); // turn off AGC
    sgtl5000_1.volume(0.8,0.8); // Headphone volume
    sgtl5000_1.lineOutLevel(SGTL5000_LINEOUT_LEVEL);
    sgtl5000_1.audioPostProcessorEnable();
    sgtl5000_1.autoVolumeControl(1, 1, 1, 0.9, 0.01, 0.05);
    sgtl5000_1.autoVolumeEnable();
    sgtl5000_1.enhanceBassEnable();
    sgtl5000_1.enhanceBass(1.0, 0.2, 1, 2); // //seb Configures the bass enhancement by setting the levels of the original stereo signal and the bass-enhanced mono level which will be mixed together. The high-pass filter may be enabled (0) or bypassed (1).
    Serial.println(F("Teensy Audio Board enabled."));
    #elif defined(TGA_AUDIO_BOARD)
    // wm8731_1.enable();
    // wm8731_1.volume(1.0);
    // Serial.println(F("TGA board enabled."));
    #elif defined(TEENSY_DAC)
    // Serial.println(F("Internal DAC enabled."));
    #elif defined(TEENSY_DAC_SYMMETRIC)
    // invMixer.gain(0,-1.f);
    // Serial.println(F("Internal DAC using symmetric outputs enabled."));
    #else
    // Serial.println(F("PT8211 enabled."));
    #endif

    set_volume(configuration.vol, configuration.pan);

    // start SD card
    SPI.setMOSI(SDCARD_MOSI_PIN);
    SPI.setSCK(SDCARD_SCK_PIN);
    if (!SD.begin(SDCARD_CS_PIN)) {
        Serial.println(F("Couldn't find SD card."));
        strcpy(bank_name, "Default");
        strcpy(voice_name, "Default");
    } else {
        Serial.println(F("SD card found."));
        sd_card_available = true;

        // read all bank names
        max_loaded_banks = get_bank_names();
        strip_extension(bank_names[configuration.bank], bank_name);

        // read all voice name for actual bank
        get_voice_names_from_bank(configuration.bank);
        #ifdef DEBUG
        Serial.print(F("Bank ["));
        Serial.print(bank_names[configuration.bank]);
        Serial.print(F("/"));
        Serial.print(bank_name);
        Serial.println(F("]"));
        for (uint8_t n = 0; n < MAX_VOICES; n++) {
            if (n < 10)
            Serial.print(F(" "));
            Serial.print(F("   "));
            Serial.print(n, DEC);
            Serial.print(F("["));
            Serial.print(voice_names[n]);
            Serial.println(F("]"));
        }
        #endif

        #include "voiceStatic.h"

        // load default SYSEX data
        load_sysex(configuration.bank, configuration.voice);
    }

    #ifdef I2C_DISPLAY
    initADC();
    // init_myButtons();
    // handle_myButtons();

    enc[0].write(map(configuration.vol * 100, 0, 100, 0, ENC_VOL_STEPS));
    enc_val[0] = enc[0].read();
    enc[1].write(configuration.voice);
    enc_val[1] = enc[1].read();
    encSwitch[0].update();
    encSwitch[1].update();

    #endif

    #if defined (DEBUG) && defined (SHOW_CPU_LOAD_MSEC)
    // Initialize processor and memory measurements
    AudioProcessorUsageMaxReset();
    AudioMemoryUsageMaxReset();
    #endif

    #ifdef DEBUG
    Serial.print(F("Bank/Voice from EEPROM ["));
    Serial.print(configuration.bank, DEC);
    Serial.print(F("/"));
    Serial.print(configuration.voice, DEC);
    Serial.println(F("]"));
    show_patch();
    #endif

    Serial.print(F("AUDIO_BLOCK_SAMPLES="));
    Serial.print(AUDIO_BLOCK_SAMPLES);
    Serial.print(F(" (Time per block="));
    Serial.print(1000000 / (SAMPLE_RATE / AUDIO_BLOCK_SAMPLES));
    Serial.println(F("ms)"));

    #if defined (DEBUG) && defined (SHOW_CPU_LOAD_MSEC)
    show_cpu_and_mem_usage();
    #endif

    #ifdef I2C_DISPLAY
    lcd.clear();
    ui_show_main();
    #endif

    AudioInterrupts();
    Serial.println(F("<setup end>"));
}


/******************************** Loop *********************************/
void loop() {

    int16_t* audio_buffer; // pointer to AUDIO_BLOCK_SAMPLES * int16_t
    const uint16_t audio_block_time_us = 1000000 / (SAMPLE_RATE / AUDIO_BLOCK_SAMPLES);

    // Main sound calculation
    if (queue1.available() && fill_audio_buffer > audio_block_time_us - 10) {
        fill_audio_buffer = 0;

        audio_buffer = queue1.getBuffer();

        elapsedMicros t1;
        dexed->getSamples(AUDIO_BLOCK_SAMPLES, audio_buffer);
        if (t1 > audio_block_time_us) // everything greater 2.9ms is a buffer underrun!
        xrun++;
        if (t1 > render_time_max)
        render_time_max = t1;
        if (peak1.available()) {
            if (peak1.read() > 0.99)
            peak++;
        }
        #ifndef TEENSY_AUDIO_BOARD
        for (uint8_t i = 0; i < AUDIO_BLOCK_SAMPLES; i++)
        audio_buffer[i] *= configuration.vol;
        #endif
        queue1.playBuffer();
    }

    // EEPROM update handling
    if (autostore >= AUTOSTORE_MS && active_voices == 0 && eeprom_update_flag == true) {
        // only store configuration data to EEPROM when AUTOSTORE_MS is reached and no voices are activated anymore
        eeprom_update();
    }

    // MIDI input handling
    check_midi_devices();

    // CONTROL-RATE-EVENT-HANDLING
    if (control_rate > CONTROL_RATE_MS) {
        control_rate = 0;

        // Shutdown unused voices
        active_voices = dexed->getNumNotesPlaying();
    }

    // #ifdef I2C_DISPLAY

    if (master_timer >= TIMER_UI_HANDLING_MS_ADC) { //seb ADC handling
        // handle ADC pots //seb
        // readADC();
        // send midi CC if any changed //seb
        // updateCC();
    }

    // UI-HANDLING
    if (master_timer >= TIMER_UI_HANDLING_MS) {
        master_timer -= TIMER_UI_HANDLING_MS;

        #ifdef I2C_DISPLAY
        handle_ui();
        #endif
        // handle custom buttons //seb
        // handle_myButtons();

    }


    // #endif

    #if defined (DEBUG) && defined (SHOW_CPU_LOAD_MSEC)
    if (cpu_mem_millis >= SHOW_CPU_LOAD_MSEC) {
        cpu_mem_millis -= SHOW_CPU_LOAD_MSEC;
        show_cpu_and_mem_usage();
    }
    #endif
}


/* MIDI MESSAGE HANDLER */
void handleNoteOn(byte inChannel, byte inNumber, byte inVelocity) {

    if (checkMidiChannel(inChannel))
    {
        dexed->keydown(inNumber, inVelocity);
        if (voiceCount == 0) filterEnv.noteOn();
        voiceCount++;
    }
}

void handleNoteOff(byte inChannel, byte inNumber, byte inVelocity) {
    if (checkMidiChannel(inChannel))
    {
        dexed->keyup(inNumber);
        voiceCount--;
        if (voiceCount == 0) filterEnv.noteOff();
    }
}

void handleControlChange(byte inChannel, byte inCtrl, byte inValue) {
    if (checkMidiChannel(inChannel)) {
        #ifdef DEBUG
        Serial.print(F("CC#"));
        Serial.print(inCtrl, DEC);
        Serial.print(F(":"));
        Serial.println(inValue, DEC);
        #endif

        float inValueNorm = inValue * DIV127;
        switch (inCtrl) {
            case CC_BANK_SELECT:
            case CC_BANK_SELECT_LSB:
            if (inValue < MAX_BANKS) {
                configuration.bank = inValue;
                get_voice_names_from_bank(configuration.bank);
                load_sysex(configuration.bank, configuration.voice);
                #ifdef I2C_DISPLAY
                handle_ui();
                ui_show_main();
                #endif
            }
            break;

            case CC_MOD_WHEEL:
            dexed->controllers.modwheel_cc = inValue;
            dexed->controllers.refresh();
            break;

            case CC_BREATH_CONTROLLER:
            dexed->controllers.breath_cc = inValue;
            dexed->controllers.refresh();
            break;

            case CC_FOOT_SWITCH:
            dexed->controllers.foot_cc = inValue;
            dexed->controllers.refresh();
            break;

            case CC_VOLUME:
            configuration.vol = inValueNorm;
            set_volume(configuration.vol, configuration.pan);
            break;

            case CC_PAN:
            configuration.pan = inValueNorm;
            set_volume(configuration.vol, configuration.pan);
            break;

            case CC_SUSTAIN:
            dexed->setSustain(inValue > 63);
            if (!dexed->getSustain()) {
                for (uint8_t note = 0; note < dexed->getMaxNotes(); note++) {
                    if (dexed->voices[note].sustained && !dexed->voices[note].keydown) {
                        dexed->voices[note].dx7_note->keyup();
                        dexed->voices[note].sustained = false;
                    }
                }
            }
            break;

            case CC_FILTER_RESO:
            effect_filter_resonance = inValue;
            // dexed->fx.Reso = inValueNorm;
            paraphonicFilter.resonance(0.7 + inValueNorm * 4.7);
            #ifdef I2C_DISPLAY
            handle_ui();
            #endif
            break;

            case CC_FILTER_FREQUENCY:
            effect_filter_cutoff = inValue;
            filterModMixer.gain(0, inValueNorm);
            // dexed->fx.Cutoff = inValueNorm;
            #ifdef I2C_DISPLAY
            handle_ui();
            #endif
            break;

            case CC_FILTER_ENV_MOD:
            effect_filter_env_mod = inValue;
            filterModMixer.gain(1, inValueNorm);
            // dexed->fx.Cutoff = inValueNorm;
            #ifdef I2C_DISPLAY
            handle_ui();
            #endif
            break;

            case CC_DELAY_TIME:
            effect_delay_time = inValue;
            delay1.delay(0, (10 + inValueNorm * (DELAY_MAX_TIME-10)));
            #ifdef I2C_DISPLAY
            handle_ui();
            #endif
            break;

            case CC_DELAY_FEEDBACK:
            effect_delay_feedback = inValue;
            delayFbMixer.gain(1, inValueNorm*0.8);
            #ifdef I2C_DISPLAY
            handle_ui();
            #endif
            break;

            case CC_DELAY_VOLUME: {
                effect_delay_volume = inValue;
                float tempDry = cosf(inValueNorm * HALFPI);
                float tempWet = sinf(inValueNorm * HALFPI);
                delayMixer.gain(DRY_SIGNAL, tempDry);
                delayMixer.gain(WET_SIGNAL, tempWet);
                #ifdef I2C_DISPLAY
                handle_ui();
                #endif
            }
            break;

            case CC_DELAY_FILTER_FREQUENCY:
            effect_delay_filter_frequency = inValue;
            delayFilter.frequency(100 + sq(inValueNorm)*9900);
            break;

            case CC_DELAY_FILTER_RESO:
            effect_delay_filter_resonance = inValue;
            delayFilter.resonance(0.7 + sq(inValueNorm)*0.5);
            break;

            case CC_CHORUS_RATE:
            effect_chorus_rate = inValue;
            modulator.frequency(0.01 + sq(inValueNorm) * 9.99);
            break;

            case CC_CHORUS_DEPTH:
            effect_chorus_depth = inValue;
            modulator.amplitude(0.01 + inValueNorm*0.49);
            break;

            case CC_CHORUS_WAVE:
            effect_chorus_wave = inValue;
            if (inValue < 63) modulator.begin(WAVEFORM_SINE);
            else modulator.begin(WAVEFORM_TRIANGLE);
            break;

            case CC_CHORUS_WET: {
                effect_chorus_on = inValue;
                float tempDry = cosf(inValueNorm * HALFPI);
                float tempWet = sinf(inValueNorm * HALFPI);
                chorusMixer_l.gain(DRY_SIGNAL, tempDry);
                chorusMixer_r.gain(DRY_SIGNAL, tempDry);
                chorusMixer_l.gain(WET_SIGNAL, tempWet);
                chorusMixer_r.gain(WET_SIGNAL, tempWet);
            }
            break;

            case CC_PANIC:
            dexed->panic();
            dexed->notesOff();
            filterEnv.noteOff();
            voiceCount = 0;
            break;

            case CC_RESET_CONTROLLERS:
            dexed->resetControllers();
            break;

            case CC_NOTES_OFF:
            dexed->notesOff();
            filterEnv.noteOff();
            voiceCount = 0;
            break;

            case CC_SET_MONO_MODE:
            if (inValue > 63) dexed->setMonoMode(true);
            else dexed->setMonoMode(false);
            break;

            case CC_SET_POLY_MODE:
            if (inValue > 63) dexed->setMonoMode(false);
            else dexed->setMonoMode(true);
            break;

            case 125: // TODO algorithm test
            dexed->data[103]=inValue/4;
            break;
        }
    }
}

void handleAfterTouch(byte inChannel, byte inPressure) {
    dexed->controllers.aftertouch_cc = inPressure;
    dexed->controllers.refresh();
}

void handlePitchBend(byte inChannel, int inPitch) {
    dexed->controllers.values_[kControllerPitch] = inPitch + 0x2000; // -8192 to +8191 --> 0 to 16383
}

void handleProgramChange(byte inChannel, byte inProgram) {
    if (inProgram < MAX_VOICES)
    {
        configuration.voice = inProgram;
        load_sysex(configuration.bank, configuration.voice);
        #ifdef I2C_DISPLAY
        handle_ui();
        ui_show_main();
        #endif
    }
}

void handleSystemExclusive(byte * sysex, uint len) {
    /*
    SYSEX MESSAGE: Parameter Change
    -------------------------------
    bits    hex  description

    11110000  F0   Status byte - start sysex
    0iiiiiii  43   ID # (i=67; Yamaha)
    0sssnnnn  10   Sub-status (s=1) & channel number (n=0; ch 1)
    0gggggpp  **   parameter group # (g=0; voice, g=2; function)
    0ppppppp  **   parameter # (these are listed in next section)
    Note that voice parameter #'s can go over 128 so
    the pp bits in the group byte are either 00 for
    par# 0-127 or 01 for par# 128-155. In the latter case
    you add 128 to the 0ppppppp byte to compute par#.
    0ddddddd  **   data byte
    11110111  F7   Status - end sysex
    */

    #ifdef DEBUG
    Serial.print(F("SYSEX-Data["));
    Serial.print(len, DEC);
    Serial.print(F("]"));
    for (uint8_t i = 0; i < len; i++) {

        Serial.print(F(" "));
        Serial.print(sysex[i], DEC);
    }
    Serial.println();
    #endif

    if (!checkMidiChannel((sysex[2] & 0x0f) + 1 )) {
        #ifdef DEBUG
        Serial.println(F("SYSEX-MIDI-Channel mismatch"));
        #endif
        return;
    }

    if (sysex[1] != 0x43) { // check for Yamaha sysex
        #ifdef DEBUG
        Serial.println(F("E: SysEx vendor not Yamaha."));
        #endif
        return;
    }

    #ifdef DEBUG
    Serial.print(F("Substatus: ["));
    Serial.print((sysex[2] & 0x70) >> 4);
    Serial.println(F("]"));
    #endif

    // parse parameter change
    if (len == 7) {
        if (((sysex[3] & 0x7c) >> 2) != 0 && ((sysex[3] & 0x7c) >> 2) != 2)
        {
            #ifdef DEBUG
            Serial.println(F("E: Not a SysEx parameter or function parameter change."));
            #endif
            return;
        }
        if (sysex[6] != 0xf7) {
            #ifdef DEBUG
            Serial.println(F("E: SysEx end status byte not detected."));
            #endif

            return;
        }

        sysex[4] &= 0x7f;
        sysex[5] &= 0x7f;

        #ifdef DEBUG
        uint8_t data_index;
        #endif

        if (((sysex[3] & 0x7c) >> 2) == 0) {
            dexed->notesOff();
            dexed->data[sysex[4] + ((sysex[3] & 0x03) * 128)] = sysex[5]; // set parameter
            dexed->doRefreshVoice();
            #ifdef DEBUG
            data_index = sysex[4] + ((sysex[3] & 0x03) * 128);
            #endif
        } else {
            dexed->data[DEXED_GLOBAL_PARAMETER_OFFSET - 63 + sysex[4]] = sysex[5]; // set function parameter
            dexed->controllers.values_[kControllerPitchRange] = dexed->data[DEXED_GLOBAL_PARAMETER_OFFSET + DEXED_PITCHBEND_RANGE];
            dexed->controllers.values_[kControllerPitchStep] = dexed->data[DEXED_GLOBAL_PARAMETER_OFFSET + DEXED_PITCHBEND_STEP];
            dexed->controllers.wheel.setRange(dexed->data[DEXED_GLOBAL_PARAMETER_OFFSET + DEXED_MODWHEEL_RANGE]);
            dexed->controllers.wheel.setTarget(dexed->data[DEXED_GLOBAL_PARAMETER_OFFSET + DEXED_MODWHEEL_ASSIGN]);
            dexed->controllers.foot.setRange(dexed->data[DEXED_GLOBAL_PARAMETER_OFFSET + DEXED_FOOTCTRL_RANGE]);
            dexed->controllers.foot.setTarget(dexed->data[DEXED_GLOBAL_PARAMETER_OFFSET + DEXED_FOOTCTRL_ASSIGN]);
            dexed->controllers.breath.setRange(dexed->data[DEXED_GLOBAL_PARAMETER_OFFSET + DEXED_BREATHCTRL_RANGE]);
            dexed->controllers.breath.setTarget(dexed->data[DEXED_GLOBAL_PARAMETER_OFFSET + DEXED_BREATHCTRL_ASSIGN]);
            dexed->controllers.at.setRange(dexed->data[DEXED_GLOBAL_PARAMETER_OFFSET + DEXED_AT_RANGE]);
            dexed->controllers.at.setTarget(dexed->data[DEXED_GLOBAL_PARAMETER_OFFSET + DEXED_AT_ASSIGN]);
            dexed->controllers.masterTune = (dexed->data[DEXED_GLOBAL_PARAMETER_OFFSET + DEXED_MASTER_TUNE] * 0x4000 << 11) * (1.0 / 12);
            dexed->controllers.refresh();
            #ifdef DEBUG
            data_index = DEXED_GLOBAL_PARAMETER_OFFSET - 63 + sysex[4];
            #endif
        }
        #ifdef DEBUG
        Serial.print(F("SysEx"));
        if (((sysex[3] & 0x7c) >> 2) == 0)
        Serial.print(F(" function"));
        Serial.print(F(" parameter "));
        Serial.print(sysex[4], DEC);
        Serial.print(F(" = "));
        Serial.print(sysex[5], DEC);
        Serial.print(F(", data_index = "));
        Serial.println(data_index, DEC);
        #endif
    }
    #ifdef DEBUG
    else
    Serial.println(F("E: SysEx parameter length wrong."));
    #endif
}

void handleTimeCodeQuarterFrame(byte data) {
    ;
}

void handleAfterTouchPoly(byte inChannel, byte inNumber, byte inVelocity) {
    ;
}

void handleSongSelect(byte inSong) {
    ;
}

void handleTuneRequest(void) {
    ;
}

void handleClock(void) {

    midi_timing_counter++;
    if (midi_timing_counter % 24 == 0)
    {
        midi_timing_quarter = midi_timing_timestep;
        midi_timing_counter = 0;
        midi_timing_timestep = 0;
        // Adjust delay control here
        #ifdef DEBUG
        Serial.print(F("MIDI Clock: "));
        Serial.print(60000 / midi_timing_quarter, DEC);
        Serial.print(F("bpm ("));
        Serial.print(midi_timing_quarter, DEC);
        Serial.println(F("ms per quarter)"));
        #endif
    }
}

void handleStart(void) {
    ;
}

void handleContinue(void) {
    ;
}

void handleStop(void) {
    ;
}

void handleActiveSensing(void) {
    ;
}

void handleSystemReset(void) {
    #ifdef DEBUG
    Serial.println(F("MIDI SYSEX RESET"));
    #endif

    dexed->notesOff();
    dexed->panic();
    dexed->resetControllers();
}


/* MIDI HELPER */
bool checkMidiChannel(byte inChannel) {
    // check for MIDI channel
    if (configuration.midi_channel == MIDI_CHANNEL_OMNI) {
        return (true);
    }
    else if (inChannel != configuration.midi_channel) {
        #ifdef DEBUG
        Serial.print(F("Ignoring MIDI data on channel "));
        Serial.print(inChannel);
        Serial.print(F("(listening on "));
        Serial.print(configuration.midi_channel);
        Serial.println(F(")"));
        #endif
        return (false);
    }
    return (true);
}


/* VOLUME HELPER */
void set_volume(float v, float p) {
    configuration.vol = v;
    configuration.pan = p;

    #ifdef DEBUG

    Serial.print(F("Setting volume: VOL="));
    Serial.print(v, DEC);
    Serial.print(F("["));
    Serial.print(configuration.vol, DEC);
    Serial.print(F("] PAN="));
    Serial.print(F("["));
    Serial.print(configuration.pan, DEC);
    Serial.print(F("] "));
    Serial.print(pow(configuration.vol * sinf(configuration.pan * HALFPI), VOLUME_CURVE), 3);
    Serial.print(F("/"));
    Serial.println(pow(configuration.vol * cosf( configuration.pan * HALFPI), VOLUME_CURVE), 3);
    #endif

    // http://files.csound-tutorial.net/floss_manual/Release03/Cs_FM_03_ScrapBook/b-panning-and-spatialization.html
    #ifdef TEENSY_AUDIO_BOARD
    sgtl5000_1.dacVolume(pow(v * sinf(p * HALFPI), VOLUME_CURVE), pow(v * cosf(p * HALFPI), VOLUME_CURVE));
    #else // assume PT8211
    volume_r.gain(sinf(p * HALFPI));
    volume_l.gain(cosf(p * HALFPI));
    #endif
}


/* EEPROM HELPER */
void initial_values_from_eeprom(void) {

    uint32_t checksum;
    config_t tmp_conf;

    EEPROM_readAnything(EEPROM_START_ADDRESS, tmp_conf);
    checksum = crc32((byte*)&tmp_conf + 4, sizeof(tmp_conf) - 4);

    #ifdef DEBUG
    Serial.print(F("EEPROM checksum: 0x"));
    Serial.print(tmp_conf.checksum, HEX);
    Serial.print(F(" / 0x"));
    Serial.print(checksum, HEX);
    #endif

    if (checksum != tmp_conf.checksum) {
        #ifdef DEBUG
        Serial.print(F(" - mismatch -> initializing EEPROM!"));
        #endif
        eeprom_update();
    } else {
        EEPROM_readAnything(EEPROM_START_ADDRESS, configuration);
        Serial.print(F(" - OK, loading!\n"));
    }
    #ifdef DEBUG
    Serial.println();
    #endif
}

void eeprom_write(void) {

    autostore = 0;
    eeprom_update_flag = true;
}

void eeprom_update(void) {
    eeprom_update_flag = false;
    configuration.checksum = crc32((byte*)&configuration + 4, sizeof(configuration) - 4);
    EEPROM_writeAnything(EEPROM_START_ADDRESS, configuration);
    Serial.println(F("Updating EEPROM with configuration data"));
}

uint32_t crc32(byte * calc_start, uint16_t calc_bytes) {
    const uint32_t crc_table[16] = {
        0x00000000, 0x1db71064, 0x3b6e20c8, 0x26d930ac,
        0x76dc4190, 0x6b6b51f4, 0x4db26158, 0x5005713c,
        0xedb88320, 0xf00f9344, 0xd6d6a3e8, 0xcb61b38c,
        0x9b64c2b0, 0x86d3d2d4, 0xa00ae278, 0xbdbdf21c
    };
    uint32_t crc = ~0L;

    for (byte* index = calc_start ; index < (calc_start + calc_bytes) ; ++index) {
        crc = crc_table[(crc ^ *index) & 0x0f] ^ (crc >> 4);
        crc = crc_table[(crc ^ (*index >> 4)) & 0x0f] ^ (crc >> 4);
        crc = ~crc;
    }

    return (crc);
}


/* DEBUG HELPER */
#if defined (DEBUG) && defined (SHOW_CPU_LOAD_MSEC)
void show_cpu_and_mem_usage(void) {

    Serial.print(F("CPU: "));
    Serial.print(AudioProcessorUsage(), 2);
    Serial.print(F("%   CPU MAX: "));
    Serial.print(AudioProcessorUsageMax(), 2);
    Serial.print(F("%  MEM: "));
    Serial.print(AudioMemoryUsage(), DEC);
    Serial.print(F("   MEM MAX: "));
    Serial.print(AudioMemoryUsageMax(), DEC);
    Serial.print(F("   RENDER_TIME_MAX: "));
    Serial.print(render_time_max, DEC);
    Serial.print(F("   XRUN: "));
    Serial.print(xrun, DEC);
    Serial.print(F("   OVERLOAD: "));
    Serial.print(overload, DEC);
    Serial.print(F("   PEAK: "));
    Serial.print(peak, DEC);
    Serial.print(F(" BLOCKSIZE: "));
    Serial.print(AUDIO_BLOCK_SAMPLES, DEC);
    Serial.print(F(" ACTIVE_VOICES: "));
    Serial.print(active_voices, DEC);
    Serial.println();
    AudioProcessorUsageMaxReset();
    AudioMemoryUsageMaxReset();
    render_time_max = 0;
}
#endif

#ifdef DEBUG
void show_patch(void) {
    uint8_t i;
    char voicename[VOICE_NAME_LEN];

    memset(voicename, 0, sizeof(voicename));
    for (i = 0; i < 6; i++) {
        Serial.println();
        Serial.print(F("OP"));
        Serial.print(6 - i, DEC);
        Serial.println(F(": "));
        Serial.println(F("R1 | R2 | R3 | R4 | L1 | L2 | L3 | L4 | LEV_SCL_BRK_PT | SCL_LEFT_DEPTH | SCL_RGHT_DEPTH"));
        Serial.print(dexed->data[(i * 21) + DEXED_OP_EG_R1], DEC);
        Serial.print(F("   "));
        Serial.print(dexed->data[(i * 21) + DEXED_OP_EG_R2], DEC);
        Serial.print(F("   "));
        Serial.print(dexed->data[(i * 21) + DEXED_OP_EG_R3], DEC);
        Serial.print(F("   "));
        Serial.print(dexed->data[(i * 21) + DEXED_OP_EG_R4], DEC);
        Serial.print(F("   "));
        Serial.print(dexed->data[(i * 21) + DEXED_OP_EG_L1], DEC);
        Serial.print(F("   "));
        Serial.print(dexed->data[(i * 21) + DEXED_OP_EG_L2], DEC);
        Serial.print(F("   "));
        Serial.print(dexed->data[(i * 21) + DEXED_OP_EG_L3], DEC);
        Serial.print(F("   "));
        Serial.print(dexed->data[(i * 21) + DEXED_OP_EG_L4], DEC);
        Serial.print(F("   "));
        Serial.print(dexed->data[(i * 21) + DEXED_OP_LEV_SCL_BRK_PT], DEC);
        Serial.print(F("               "));
        Serial.print(dexed->data[(i * 21) + DEXED_OP_SCL_LEFT_DEPTH], DEC);
        Serial.print(F("               "));
        Serial.println(dexed->data[(i * 21) + DEXED_OP_SCL_RGHT_DEPTH], DEC);
        Serial.println(F("SCL_L_CURVE | SCL_R_CURVE | RT_SCALE | AMS | KVS | OUT_LEV | OP_MOD | FRQ_C | FRQ_F | DETUNE"));
        Serial.print(dexed->data[(i * 21) + DEXED_OP_SCL_LEFT_CURVE], DEC);
        Serial.print(F("            "));
        Serial.print(dexed->data[(i * 21) + DEXED_OP_SCL_RGHT_CURVE], DEC);
        Serial.print(F("            "));
        Serial.print(dexed->data[(i * 21) + DEXED_OP_OSC_RATE_SCALE], DEC);
        Serial.print(F("         "));
        Serial.print(dexed->data[(i * 21) + DEXED_OP_AMP_MOD_SENS], DEC);
        Serial.print(F("    "));
        Serial.print(dexed->data[(i * 21) + DEXED_OP_KEY_VEL_SENS], DEC);
        Serial.print(F("    "));
        Serial.print(dexed->data[(i * 21) + DEXED_OP_OUTPUT_LEV], DEC);
        Serial.print(F("        "));
        Serial.print(dexed->data[(i * 21) + DEXED_OP_OSC_MODE], DEC);
        Serial.print(F("       "));
        Serial.print(dexed->data[(i * 21) + DEXED_OP_FREQ_COARSE], DEC);
        Serial.print(F("      "));
        Serial.print(dexed->data[(i * 21) + DEXED_OP_FREQ_FINE], DEC);
        Serial.print(F("      "));
        Serial.println(dexed->data[(i * 21) + DEXED_OP_OSC_DETUNE], DEC);
        delay(10);
    }
    Serial.println(F("PR1 | PR2 | PR3 | PR4 | PL1 | PL2 | PL3 | PL4"));
    for (i = 0; i < 8; i++) {
        Serial.print(dexed->data[DEXED_VOICE_OFFSET + i], DEC);
        Serial.print(F("    "));
        delay(10);
    }
    Serial.println();
    Serial.print(F("ALG: "));
    Serial.println(dexed->data[DEXED_VOICE_OFFSET + DEXED_ALGORITHM], DEC);
    Serial.print(F("FB: "));
    Serial.println(dexed->data[DEXED_VOICE_OFFSET + DEXED_FEEDBACK], DEC);
    Serial.print(F("OKS: "));
    Serial.println(dexed->data[DEXED_VOICE_OFFSET + DEXED_OSC_KEY_SYNC], DEC);
    Serial.print(F("LFO SPD: "));
    Serial.println(dexed->data[DEXED_VOICE_OFFSET + DEXED_LFO_SPEED], DEC);
    Serial.print(F("LFO_DLY: "));
    Serial.println(dexed->data[DEXED_VOICE_OFFSET + DEXED_LFO_DELAY], DEC);
    Serial.print(F("LFO PMD: "));
    Serial.println(dexed->data[DEXED_VOICE_OFFSET + DEXED_LFO_PITCH_MOD_DEP], DEC);
    Serial.print(F("LFO_AMD: "));
    Serial.println(dexed->data[DEXED_VOICE_OFFSET + DEXED_LFO_AMP_MOD_DEP], DEC);
    Serial.print(F("LFO_SYNC: "));
    Serial.println(dexed->data[DEXED_VOICE_OFFSET + DEXED_LFO_SYNC], DEC);
    Serial.print(F("LFO_WAVEFRM: "));
    Serial.println(dexed->data[DEXED_VOICE_OFFSET + DEXED_LFO_WAVE], DEC);
    Serial.print(F("LFO_PMS: "));
    Serial.println(dexed->data[DEXED_VOICE_OFFSET + DEXED_LFO_PITCH_MOD_SENS], DEC);
    Serial.print(F("TRNSPSE: "));
    Serial.println(dexed->data[DEXED_VOICE_OFFSET + DEXED_TRANSPOSE], DEC);
    Serial.print(F("NAME: "));
    strncpy(voicename, (char *)&dexed->data[DEXED_VOICE_OFFSET + DEXED_NAME], sizeof(voicename) - 1);
    Serial.print(F("["));
    Serial.print(voicename);
    Serial.println(F("]"));
    for (i = DEXED_GLOBAL_PARAMETER_OFFSET; i <= DEXED_GLOBAL_PARAMETER_OFFSET + DEXED_MAX_NOTES; i++) {
        Serial.print(i, DEC);
        Serial.print(F(": "));
        Serial.println(dexed->data[i]);
    }
    Serial.println();
}
#endif
