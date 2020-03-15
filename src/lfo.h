// Low frequency oscillator, compatible with DX7

#ifndef LFO_H_INCLUDED
#define LFO_H_INCLUDED

class Lfo {
 public:
  static void init(FRAC_NUM sample_rate);
  void reset(const uint8_t params[6]);

  // result is 0..1 in Q24
  int32_t getsample();

  // result is 0..1 in Q24
  int32_t getdelay();

  void keydown();
 private:
  static uint32_t unit_;

  uint32_t phase_;  // Q32
  uint32_t delta_;
  uint8_t waveform_;
  uint8_t randstate_;
  bool sync_;

  uint32_t delaystate_;
  uint32_t delayinc_;
  uint32_t delayinc2_;
};

#endif // LFO_H_INCLUDED
