
#include <Arduino.h>
#include <limits.h>
#include "config.h"
#include "dexed.h"
#include "dexed_sysex.h"
#include "UI.h"

#ifdef I2C_DISPLAY // selecting sounds by encoder, button and display

elapsedMillis ui_back_to_main;


void handle_ui(void) {
  if (ui_back_to_main >= UI_AUTO_BACK_MS && (ui_state != UI_MAIN && ui_state != UI_EFFECTS_FILTER && ui_state != UI_EFFECTS_DELAY)) {
    enc[0].write(map(configuration.vol * 100, 0, 100, 0, ENC_VOL_STEPS));
    enc_val[0] = enc[0].read();
    ui_show_main();
  }

  if (autostore >= AUTOSTORE_MS && (ui_main_state == UI_MAIN_VOICE_SELECTED || ui_main_state == UI_MAIN_BANK_SELECTED)) {
    ui_show_main();
    switch (ui_main_state) {
      case UI_MAIN_VOICE_SELECTED:
        ui_main_state = UI_MAIN_VOICE;
        break;
        case UI_MAIN_BANK_SELECTED:
        ui_main_state = UI_MAIN_BANK;
      break;
    }
  }

  for (uint8_t i = 0; i < NUM_ENCODER; i++) {
    encSwitch[i].update();

    if (encSwitch[i].fallingEdge())
    long_button_pressed = 0;

    if (encSwitch[i].risingEdge()) {
      uint32_t button_released = long_button_pressed;

      // if (button_released > LONG_BUTTON_PRESS) {
      if (button_released > LONG_BUTTON_PRESS) {
        // long pressing of button detected
        #ifdef DEBUG
        Serial.print(F("Long button pressing detected for button "));
        Serial.println(i, DEC);
        #endif

        switch (i) {
          case 0: // long press for left button
          break;
          case 1: // long press for right button
          switch (ui_state) {
            case UI_MAIN:
            ui_main_state = UI_MAIN_FILTER_CUT;
            enc[i].write(effect_filter_cutoff);
            enc_val[i] = enc[i].read();
            ui_show_effects_filter();
            break;
            case UI_EFFECTS_FILTER:
            ui_main_state = UI_MAIN_DELAY_TIME;
            enc[i].write(effect_delay_time);
            enc_val[i] = enc[i].read();
            ui_show_effects_delay();
            break;
            case UI_EFFECTS_DELAY:
            ui_main_state = UI_MAIN_VOICE;
            enc[i].write(configuration.voice);
            enc_val[i] = enc[i].read();
            ui_show_main();
            break;
          }
          break;
        }
      }
      else {
        switch (i) {
          case 0: // short press for left button
          switch (ui_state) {
            case UI_MAIN:
            enc[i].write(map(configuration.vol * 100, 0, 100, 0, ENC_VOL_STEPS));
            enc_val[i] = enc[i].read();
            ui_show_volume();
            break;
            case UI_VOLUME:
            enc[i].write(configuration.midi_channel);
            enc_val[i] = enc[i].read();
            ui_show_midichannel();
            break;
            case UI_MIDICHANNEL:
            enc[i].write(map(configuration.vol * 100, 0, 100, 0, ENC_VOL_STEPS));
            enc_val[i] = enc[i].read();
            ui_show_main();
            break;
          }
          break;
          case 1: // short press for right button
          switch (ui_state) {
            case UI_MAIN:
            switch (ui_main_state) {
              case UI_MAIN_BANK:
              case UI_MAIN_BANK_SELECTED:
              ui_main_state = UI_MAIN_VOICE;
              enc[i].write(configuration.voice);
              enc_val[i] = enc[i].read();
              break;
              case UI_MAIN_VOICE:
              case UI_MAIN_VOICE_SELECTED:
              ui_main_state = UI_MAIN_BANK;
              enc[i].write(configuration.bank);
              enc_val[i] = enc[i].read();
              break;
            }
            ui_show_main();
            break;
            case UI_EFFECTS_FILTER:
            case UI_EFFECTS_DELAY:
            switch (ui_main_state) {
              case UI_MAIN_FILTER_CUT:
              ui_main_state = UI_MAIN_FILTER_RES;
              enc[i].write(effect_filter_resonance);
              enc_val[i] = enc[i].read();
              ui_show_effects_filter();
              break;
              case UI_MAIN_FILTER_RES:
              ui_main_state = UI_MAIN_FILTER_CUT;
              enc[i].write(effect_filter_cutoff);
              enc_val[i] = enc[i].read();
              ui_show_effects_filter();
              break;
              case UI_MAIN_DELAY_TIME:
              ui_main_state = UI_MAIN_DELAY_FEEDBACK;
              enc[i].write(effect_delay_feedback);
              enc_val[i] = enc[i].read();
              ui_show_effects_delay();
              break;
              case UI_MAIN_DELAY_VOLUME:
              ui_main_state = UI_MAIN_DELAY_TIME;
              enc[i].write(effect_delay_time);
              enc_val[i] = enc[i].read();
              ui_show_effects_delay();
              break;
              case UI_MAIN_DELAY_FEEDBACK:
              ui_main_state = UI_MAIN_DELAY_VOLUME;
              enc[i].write(effect_delay_volume);
              enc_val[i] = enc[i].read();
              ui_show_effects_delay();
              break;
            }
            break;
          }
        }
      }
      #ifdef DEBUG
      Serial.print(F("Button "));
      Serial.println(i, DEC);
      #endif
    }

    if (enc[i].read() == enc_val[i])
      continue;
    else {
      switch (i) {
        case 0: // left encoder moved
        switch (ui_state) {
          case UI_MAIN:
          case UI_VOLUME:
          if (enc[i].read() <= 0)
          enc[i].write(0);
          else if (enc[i].read() >= ENC_VOL_STEPS)
          enc[i].write(ENC_VOL_STEPS);
          set_volume(float(map(enc[i].read(), 0, ENC_VOL_STEPS, 0, 100)) / 100, configuration.pan);
          eeprom_write();
          ui_show_volume();
          break;
          case UI_MIDICHANNEL:
          if (enc[i].read() <= 0)
          enc[i].write(0);
          else if (enc[i].read() >= 16)
          enc[i].write(16);
          configuration.midi_channel = enc[i].read();
          eeprom_write();
          ui_show_midichannel();
          break;
        }
        break;
        case 1: // right encoder moved
        switch (ui_state) {
          case UI_VOLUME:
          ui_state = UI_MAIN;
          enc[1].write(configuration.voice);
          ui_show_main();
          break;
          case UI_MAIN:
          switch (ui_main_state) {
            case UI_MAIN_BANK:
            ui_main_state = UI_MAIN_BANK_SELECTED;
            case UI_MAIN_BANK_SELECTED:
            if (enc[i].read() <= 0)
            enc[i].write(0);
            else if (enc[i].read() > max_loaded_banks - 1)
            enc[i].write(max_loaded_banks - 1);
            configuration.bank = enc[i].read();
            get_voice_names_from_bank(configuration.bank);
            load_sysex(configuration.bank, configuration.voice);
            eeprom_write();
            break;
            case UI_MAIN_VOICE:
            ui_main_state = UI_MAIN_VOICE_SELECTED;
            case UI_MAIN_VOICE_SELECTED:
            if (enc[i].read() <= 0) {
              if (configuration.bank > 0) {
                enc[i].write(MAX_VOICES - 1);
                configuration.bank--;
                get_voice_names_from_bank(configuration.bank);
              } else
                enc[i].write(0);
            }
            else if (enc[i].read() > MAX_VOICES - 1) {
              if (configuration.bank < MAX_BANKS - 1) {
                enc[i].write(0);
                configuration.bank++;
                get_voice_names_from_bank(configuration.bank);
              }
              else
                enc[i].write(MAX_VOICES - 1);
            }
            configuration.voice = enc[i].read();
            load_sysex(configuration.bank, configuration.voice);
            eeprom_write();
            break;
          }
          ui_show_main();
          break;
          case UI_EFFECTS_FILTER:
          switch (ui_main_state) {
            case UI_MAIN_FILTER_CUT:
              if (enc[i].read() <= 0)
                enc[i].write(0);
              else if (enc[i].read() > ENC_FILTER_CUT_STEPS)
                enc[i].write(ENC_FILTER_CUT_STEPS);
              effect_filter_cutoff = enc[i].read();
              // dexed->fx.Cutoff = 1.0 - float(effect_filter_cutoff) / ENC_FILTER_CUT_STEPS;
              #ifdef DEBUG
              Serial.print(F("Setting filter cutoff to: "));
              Serial.println(1.0 - float(effect_filter_cutoff) / ENC_FILTER_CUT_STEPS, 5);
              #endif
            break;
            case UI_MAIN_FILTER_RES:
              if (enc[i].read() <= 0)
                enc[i].write(0);
              else if (enc[i].read() > ENC_FILTER_RES_STEPS)
                enc[i].write(ENC_FILTER_RES_STEPS);
              effect_filter_resonance = enc[i].read();
              // dexed->fx.Reso = 1.0 - float(effect_filter_resonance) / ENC_FILTER_RES_STEPS;
              #ifdef DEBUG
              Serial.print(F("Setting filter resonance to: "));
              Serial.println(1.0 - float(effect_filter_resonance) / ENC_FILTER_RES_STEPS, 5);
              #endif
            break;
          }
          ui_show_effects_filter();
          break;
          case UI_EFFECTS_DELAY:
          switch (ui_main_state) {
            case UI_MAIN_DELAY_TIME:
            if (enc[i].read() <= 0)
            enc[i].write(0);
            else if (enc[i].read() > ENC_DELAY_TIME_STEPS)
            enc[i].write(ENC_DELAY_TIME_STEPS);
            effect_delay_time = enc[i].read();;
            delay1.delay(0, mapfloat(effect_delay_time, 10, ENC_DELAY_TIME_STEPS, 0.0, DELAY_MAX_TIME));
            #ifdef DEBUG
            Serial.print(F("Setting delay time to: "));
            Serial.println(map(effect_delay_time, 10, ENC_DELAY_TIME_STEPS, 0, DELAY_MAX_TIME));
            #endif
            break;
            case UI_MAIN_DELAY_FEEDBACK:
            if (enc[i].read() <= 0)
            enc[i].write(0);
            else if (enc[i].read() > ENC_DELAY_FB_STEPS)
            enc[i].write(ENC_DELAY_FB_STEPS);
            effect_delay_feedback = enc[i].read();
            delayFbMixer.gain(1, mapfloat(float(effect_delay_feedback), 0, ENC_DELAY_FB_STEPS, 0.0, 0.8));
            #ifdef DEBUG
            Serial.print(F("Setting delay feedback to: "));
            Serial.println(mapfloat(float(effect_delay_feedback), 0, ENC_DELAY_FB_STEPS, 0.0, 1.0));
            #endif
            break;
            case UI_MAIN_DELAY_VOLUME:
            if (enc[i].read() <= 0)
            enc[i].write(0);
            else if (enc[i].read() > ENC_DELAY_VOLUME_STEPS)
            enc[i].write(ENC_DELAY_VOLUME_STEPS);
            effect_delay_volume = enc[i].read();
            float tmp_vol = mapfloat(effect_delay_volume, 0, ENC_DELAY_VOLUME_STEPS, 0.0, 1.0);
            //delayMixer.gain(0, 1.0 - mapfloat(effect_delay_volume, 0, ENC_DELAY_VOLUME_STEPS, 0.0, 1.0)); // delay tap1 signal (with added feedback)
            // delayMixer.gain(0, 1.0 - tmp_vol); // delay tap1 signal (with added feedback)
            // delayMixer.gain(1, tmp_vol); // delay tap1 signal (with added feedback)
            // delayMixer.gain(2, tmp_vol); // delay tap1 signal
            #ifdef DEBUG
            Serial.print(F("Setting delay volume to: "));
            Serial.println(effect_delay_volume);
            #endif
            break;
          }
          ui_show_effects_delay();
          break;
        }
        break;
      }
      #ifdef DEBUG
      Serial.print(F("Encoder "));
      Serial.print(i, DEC);
      Serial.print(F(": "));
      Serial.println(enc[i].read(), DEC);
      #endif
    }
    enc_val[i] = enc[i].read();
  }
}

void ui_show_main(void) {
  if (ui_state != UI_MAIN)
    lcd.clear();


  // print bank number and name
  lcd.setCursor(0, 1);
  lcd.write("B");
  if (configuration.bank < 10) lcd.write(" ");
  strip_extension(bank_names[configuration.bank], bank_name);
  lcd.print(configuration.bank);
  if (ui_main_state == UI_MAIN_BANK || ui_main_state == UI_MAIN_BANK_SELECTED) {
    lcd.setCursor(4, 1);
    lcd.write("[");
    lcd.print(bank_name);
    lcd.write("]");
  }
  else {
    lcd.setCursor(4, 1);
    lcd.write(" ");
    lcd.print(bank_name);
    lcd.write(" ");
  }

  // print patch number and name
  lcd.setCursor(0, 0);
  lcd.write("P");
  if (configuration.voice < 10) lcd.write(" ");
  lcd.print(configuration.voice);
  if (ui_main_state == UI_MAIN_VOICE || ui_main_state == UI_MAIN_VOICE_SELECTED) {
    lcd.setCursor(4, 0);
    lcd.write("[");
    lcd.print(voice_names[configuration.voice]);
    lcd.write("]");
  }
  else {
    lcd.setCursor(4, 0);
    lcd.write(" ");
    lcd.print(voice_names[configuration.voice]);
    lcd.write(" ");
  }

  ui_state = UI_MAIN;
}

void ui_show_volume(void) {
  ui_back_to_main = 0;

  if (ui_state != UI_VOLUME) {
    lcd.clear();
    lcd.write("Volume");
  }

  lcd.setCursor(LCD_CHARS - 3, 0);
  lcd.print(int(configuration.vol * 100));
  if (configuration.vol == 0.0) {
    lcd.setCursor(0, 1);
    lcd.write(" ");
  } else {
    if (configuration.vol < (float(LCD_CHARS) / 100)) {
      lcd.setCursor(0, 1);
      lcd.write("*");
    } else {
      uint8_t mapVol = map(configuration.vol * 100, 0, 100, 0, LCD_CHARS);
      for (uint8_t i = 0; i < mapVol; i++) {
        lcd.setCursor(i, 1);
        lcd.write("*");
      }
      for (uint8_t i = mapVol; i < LCD_CHARS; i++) {
        lcd.setCursor(i, 1);
        lcd.write(" ");
      }
    }
  }

  ui_state = UI_VOLUME;
}

void ui_show_midichannel(void) {
  ui_back_to_main = 0;

  if (ui_state != UI_MIDICHANNEL) {
    lcd.clear();
    lcd.write("MIDI Channel");
  }

  if (configuration.midi_channel == MIDI_CHANNEL_OMNI) {
    lcd.setCursor(0, 1);
    lcd.write("OMNI");
  } else {
    lcd.setCursor(0, 1);
    if (configuration.midi_channel < 10) lcd.write(" ");
    lcd.print(configuration.midi_channel);
    if (configuration.midi_channel == 1) {
      lcd.setCursor(2, 1);
      lcd.write("  ");
    }
  }

  ui_state = UI_MIDICHANNEL;
}

void ui_show_effects_filter(void) {
  if (ui_state != UI_EFFECTS_FILTER) {
    lcd.clear();
    lcd.write("Filter          ");
    lcd.setCursor(0, 1);
    lcd.write("Cut:");
    lcd.setCursor(8, 1);
    lcd.write("Res:");
  }

  lcd.setCursor(5, 1);
  uint8_t tempLcdCutoff = map(effect_filter_cutoff, 0, ENC_FILTER_CUT_STEPS, 0, 99);
  if (tempLcdCutoff < 10) lcd.write(" ");
  lcd.print(tempLcdCutoff);
  lcd.setCursor(13, 1);
  uint8_t tempLcdResonance = map(effect_filter_resonance, 0, ENC_FILTER_RES_STEPS, 0, 99);
  if (tempLcdResonance < 10) lcd.write(" ");
  lcd.print(tempLcdResonance);

  if (ui_main_state == UI_MAIN_FILTER_CUT) {
    lcd.setCursor(4, 1); lcd.write("[");
    lcd.setCursor(7, 1); lcd.write("]");
  }
  else {
    lcd.setCursor(4, 1); lcd.write(" ");
    lcd.setCursor(7, 1); lcd.write(" ");
  }

  if (ui_main_state == UI_MAIN_FILTER_RES) {
    lcd.setCursor(12, 1); lcd.write("[");
    lcd.setCursor(15, 1); lcd.write("]");
  }
  else {
    lcd.setCursor(12, 1); lcd.write(" ");
    lcd.setCursor(15, 1); lcd.write(" ");
  }

  ui_state = UI_EFFECTS_FILTER;
}

void ui_show_effects_delay(void) {
  if (ui_state != UI_EFFECTS_DELAY) {
    lcd.clear();
    lcd.write("Delay T:      ms");
    lcd.setCursor(0, 1);
    lcd.write("FB:     Vol:");
  }

  lcd.setCursor(9, 0);
  uint16_t tempLcdDelayTime = map(effect_delay_time, 0, ENC_DELAY_TIME_STEPS, 0, DELAY_MAX_TIME);
  if (tempLcdDelayTime < 1000) lcd.write(" ");
  if (tempLcdDelayTime < 100) lcd.write(" ");
  if (tempLcdDelayTime < 10) lcd.write(" ");
  lcd.print(tempLcdDelayTime);

  lcd.setCursor(4, 1);
  uint8_t tempLcdDelayFeedback = map(effect_delay_feedback, 0, ENC_DELAY_FB_STEPS, 0, 99);
  if (tempLcdDelayFeedback < 10) lcd.write(" ");
  lcd.print(tempLcdDelayFeedback);
  lcd.setCursor(13,1);
  uint8_t tempLcdDelayVolume = map(effect_delay_volume, 0, ENC_DELAY_VOLUME_STEPS, 0, 99);
  if (tempLcdDelayVolume < 10) lcd.write(" ");
  lcd.print(tempLcdDelayVolume);

  if (ui_main_state == UI_MAIN_DELAY_TIME) {
    lcd.setCursor(8, 0); lcd.write("[");
    lcd.setCursor(13, 0); lcd.write("]");
  } else {
    lcd.setCursor(8, 0); lcd.write(" ");
    lcd.setCursor(13, 0); lcd.write(" ");
  }

  if (ui_main_state == UI_MAIN_DELAY_FEEDBACK) {
    lcd.setCursor(3, 1); lcd.write("[");
    lcd.setCursor(6, 1); lcd.write("]");
  } else {
    lcd.setCursor(3, 1); lcd.write(" ");
    lcd.setCursor(6, 1); lcd.write(" ");
  }

  if (ui_main_state == UI_MAIN_DELAY_VOLUME) {
    lcd.setCursor(12, 1); lcd.write("[");
    lcd.setCursor(15, 1); lcd.write("]");
  } else {
    lcd.setCursor(12, 1); lcd.write(" ");
    lcd.setCursor(15, 1); lcd.write(" ");
  }

  ui_state = UI_EFFECTS_DELAY;
}
#endif

float mapfloat(float val, float in_min, float in_max, float out_min, float out_max) {
  return (val - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}
