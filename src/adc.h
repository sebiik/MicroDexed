#ifndef ADC_H_INCLUDED
#define ADC_H_INCLUDED

#define NUM_ADC_CHANNELS 8
#define NUM_LAYERS 8
#define NUM_CONTROLS (NUM_ADC_CHANNELS * NUM_LAYERS)
#define MOV_AVG 8

const uint8_t ainPinList[12] = {1, 2, 3, 6, 7, 16, 17, 18, 19, 20, 21, 22};
const float EMA_a = 0.125;      //initialize EMA alpha

float ainRead[NUM_ADC_CHANNELS];
uint8_t potValues[NUM_CONTROLS];
uint8_t potValuesLast[NUM_CONTROLS];
uint8_t activeLayer = 0;

void initADC(void) {
  for (uint8_t i = 0; i < NUM_CONTROLS; i++) {
    potValuesLast[i] = potValues[i];
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
      potValues[(activeLayer*8) + i] = uint8_t(ainRead[i]);
    }
  }
}

void sendCC(uint8_t ccNumber, uint8_t ccValue, uint8_t outChannel) {
  midi_serial.sendControlChange(ccNumber, ccValue, outChannel);
}

void updateCC(void) {
  for (uint8_t i=0; i<NUM_CONTROLS; i++) {
      if (potValues[i] != potValuesLast[i]) {
        //    pot, value,           channel
        sendCC(i,  potValues[i], MIDI_OUT_CHANNEL);
        potValuesLast[i] = potValues[i];
      }
    }
}

#endif /* ADC_H_INCLUDED */
