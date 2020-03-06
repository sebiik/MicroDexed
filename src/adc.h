#define NUM_ADC_CHANNELS 8
#define NUM_LAYERS 8
#define NUM_CONTROLS (NUM_ADC_CHANNELS * NUM_LAYERS)
#define MOV_AVG 8

const uint8_t ainPinList[12] = {1, 2, 3, 6, 7, 16, 17, 18, 19, 20, 21, 22};
const float EMA_a = 0.125;      //initialize EMA alpha
const uint8_t layerList[NUM_LAYERS] = {11, 21, 31, 41, 51, 61, 71, 81}; // Layer 1: CC11...18, Layer2: CC21...28, etc.

float ainRead[NUM_ADC_CHANNELS];
uint8_t pots[NUM_LAYERS][NUM_ADC_CHANNELS];
uint8_t potsLast[NUM_LAYERS][NUM_ADC_CHANNELS];
uint8_t activeLayer = 0;

void initADC(void) {
  for (uint8_t i = 0; i < NUM_LAYERS; i++) {
    for (uint8_t j = 0; j < NUM_ADC_CHANNELS; j++) {
      potsLast[i][j] = pots[i][j];
    }
  }
}

void readADC(void) {
  // Loop through ADC channels
  for (uint8_t i = 0; i < NUM_ADC_CHANNELS; i++) {
    // Moving Average
    float sensorValues[MOV_AVG];
    float sensorAverage = 0;
    for (uint8_t j = 0; j < MOV_AVG; j++) {
      sensorValues[j] = analogRead(ainPinList[i]);
      sensorAverage += sensorValues[j] * 0.125;
      delayMicroseconds(1);
    }
    sensorAverage = round(map(sensorAverage, 3.0, 1018.0, 0.0, 127.0));
    // If value exceeds threshold, update
    if ((abs(sensorAverage - ainRead[i]) > 0.9)) {
      ainRead[i] += EMA_a * (sensorAverage - ainRead[i]);
      pots[activeLayer][i] = uint8_t(ainRead[i]);
    }
  }
}

void sendCC(uint8_t layer, uint8_t pot, uint8_t ccValue, uint8_t outChannel) {
  midi_serial.sendControlChange(layerList[layer]+pot, ccValue, outChannel);
}

void updateCC(void) {
  for (uint8_t i=0; i<NUM_LAYERS; i++) {
    for (uint8_t j=0; j<NUM_ADC_CHANNELS; j++) {
      if (pots[i][j] != potsLast[i][j]) {
        //    layer, pot, value,      channel
        sendCC(i,    j,   pots[i][j], MIDI_OUT_CHANNEL);
        potsLast[i][j] = pots[i][j];
      }
    }
  }
}
