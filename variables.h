Dexed* dexed = new Dexed(SAMPLE_RATE);
bool sd_card_available = false;
uint32_t xrun = 0;
uint32_t overload = 0;
uint32_t peak = 0;
uint16_t render_time_max = 0;
uint8_t max_loaded_banks = 0;
char bank_name[BANK_NAME_LEN];
char voice_name[VOICE_NAME_LEN];
char bank_names[MAX_BANKS][BANK_NAME_LEN];
char voice_names[MAX_VOICES][VOICE_NAME_LEN];
elapsedMillis autostore;
uint8_t midi_timing_counter = 0; // 24 per qarter
elapsedMillis midi_timing_timestep;
uint16_t midi_timing_quarter = 0;
elapsedMillis long_button_pressed;
uint8_t effect_filter_cutoff = 0;
uint8_t effect_filter_resonance = 0;
uint8_t effect_delay_time = 0;
uint8_t effect_delay_feedback = 0;
uint8_t effect_delay_volume = 0;
bool effect_delay_sync = 0;
elapsedMicros fill_audio_buffer;
elapsedMillis control_rate;
uint8_t active_voices = 0;
#ifdef SHOW_CPU_LOAD_MSEC
elapsedMillis cpu_mem_millis;
#endif
config_t configuration = {0xffff, 0, 0, VOLUME, 0.5f, DEFAULT_MIDI_CHANNEL};
bool eeprom_update_flag = false;
const float DIV127 = (1.0 / 127.0);
