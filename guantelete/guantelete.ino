#include <avr/io.h> 
#include <avr/interrupt.h> 

uint16_t syncPhaseAcc; 
uint16_t syncPhaseInc; 
uint16_t grainPhaseAcc; 
uint16_t grainPhaseInc; 
uint16_t grainAmp; 
uint8_t grainDecay; 
uint16_t grain2PhaseAcc; 
uint16_t grain2PhaseInc; 
uint16_t grain2Amp; 
uint8_t grain2Decay; 


#define GRAIN_FREQ_CONTROL   (1) 
#define GRAIN_DECAY_CONTROL  (2) 
#define GRAIN2_FREQ_CONTROL  (3) 
#define GRAIN2_DECAY_CONTROL (4) 
#define PULSE_PIN       4 

#define PWM_PIN       3 
#define PWM_VALUE     OCR2B 
#define PWM_INTERRUPT TIMER2_OVF_vect 

// Smooth logarithmic mapping 
// 
uint16_t antilogTable[] = { 
64830,64132,63441,62757,62081,61413,60751,60097,59449,58809,58176,57549,56929,56316,55709,55109, 
54515,53928,53347,52773,52204,51642,51085,50535,49991,49452,48920,48393,47871,47356,46846,46341, 
45842,45348,44859,44376,43898,43425,42958,42495,42037,41584,41136,40693,40255,39821,39392,38968, 
38548,38133,37722,37316,36914,36516,36123,35734,35349,34968,34591,34219,33850,33486,33125,32768 
}; 
uint16_t mapPhaseInc(uint16_t input) { 
  return (antilogTable[input & 0x3f]) >> (input >> 6); 
} 

// Stepped chromatic mapping 
// 
uint16_t midiTable[] = { 
17,18,19,20,22,23,24,26,27,29,31,32,34,36,38,41,43,46,48,51,54,58,61,65,69,73, 
77,82,86,92,97,103,109,115,122,129,137,145,154,163,173,183,194,206,218,231, 
244,259,274,291,308,326,346,366,388,411,435,461,489,518,549,581,616,652,691, 
732,776,822,871,923,978,1036,1097,1163,1232,1305,1383,1465,1552,1644,1742, 
1845,1955,2071,2195,2325,2463,2610,2765,2930,3104,3288,3484,3691,3910,4143, 
4389,4650,4927,5220,5530,5859,6207,6577,6968,7382,7821,8286,8779,9301,9854, 
10440,11060,11718,12415,13153,13935,14764,15642,16572,17557,18601,19708,20879, 
22121,23436,24830,26306 
}; 


int pines[] = {1,2,3,4};
int estadoPines[] = {1,1,1,1};
int antirebotes[] = {0,0,0,0};

/*
Notas a MIDI:
-------------
    F2   41
    G2   43
    A2   45  ( 110hz)
*/
int notas[] = {36,36,41,43,46,48,51};

/*
void setup2() {
  for(int i=0;i<4;i++){
    pinMode(pines[i], INPUT_PULLUP);
  }
}

void loop2() {
  
  for(int i=0;i<4;i++){
    int nuevoEstadoPin = digitalRead(pines[i]);
    if (estadoPines[i] != nuevoEstadoPin ){
      antirebotes[i] = antirebotes[i] + 1;
      if(antirebotes[i] > 200 ){
        estadoPines[i] = nuevoEstadoPin;
        antirebotes[i] = 0;
      }
    }else{
        antirebotes[i]= 0;
    }
  }

}
*/

uint16_t grainFreqCtrl; 
uint16_t grainDecayCtrl; 
uint16_t grain2FreqCtrl; 
uint16_t grain2DecayCtrl; 


void audioOn() { 
  // Set up PWM to 31.25kHz, phase accurate 
  TCCR2A = _BV(COM2B1) | _BV(WGM20); 
  TCCR2B = _BV(CS20); 
  TIMSK2 = _BV(TOIE2);
} 

void setup() { 
  pinMode(PWM_PIN,OUTPUT); 
  pinMode(PULSE_PIN,OUTPUT);
  digitalWrite(PULSE_PIN, 0);
  audioOn(); 
  Serial.begin(9600);
} 


int patches[][4][2]={
    {{0,363},{572,237},{158,366},{9,0}},
    {{0,0},{148,149},{392,120},{7,12}},
    {{589,65},{0,0},{851,855},{729,356}},
    {{405,0},{25,25},{0,0},{342,342}}
  };

bool debug = 0;

int index = 0;
long lastMillis = 0;

void loop() { 

  long curMillis = millis();
  if((curMillis-lastMillis)>500){
    if(syncPhaseInc >0){
      syncPhaseInc =0;
    }else{
      syncPhaseInc = midiTable[notas[index]]; 
      index++;
      index %=7;

    }
    lastMillis = curMillis;
  }




  if(debug){
    uint16_t gFreqCtrl = analogRead(GRAIN_FREQ_CONTROL); 
    uint16_t gDecayCtrl= analogRead(GRAIN_DECAY_CONTROL); 
    uint16_t g2FreqCtrl = analogRead(GRAIN2_FREQ_CONTROL); 
    uint16_t g2DecayCtrl = analogRead(GRAIN2_DECAY_CONTROL); 
  
    if(    (diff(gFreqCtrl,grainFreqCtrl)>10) 
        || (diff(gDecayCtrl,grainDecayCtrl)>10 )
        || (diff(g2FreqCtrl,grain2FreqCtrl)>10)
        || (diff(g2DecayCtrl,grain2DecayCtrl)>10) ){
      grainPhaseInc  = mapPhaseInc(gFreqCtrl) / 2; 
      grainFreqCtrl = gFreqCtrl;
      grainDecay     = gDecayCtrl / 8; 
      grainDecayCtrl = gDecayCtrl;
      grain2PhaseInc = mapPhaseInc(g2FreqCtrl) / 2; 
      grain2FreqCtrl = g2FreqCtrl;
      grain2Decay     = g2DecayCtrl / 4; 
      grain2DecayCtrl = g2DecayCtrl;
      Serial.print("F ");
      Serial.print(grainFreqCtrl);
      Serial.print(" D ");
      Serial.print(grainDecayCtrl);
      Serial.print("  F2 ");
      Serial.print(grain2FreqCtrl);
      Serial.print("  D2 ");
      Serial.println(grain2DecayCtrl);
    }
  
  }else{
    uint16_t ctrl = analogRead(GRAIN_FREQ_CONTROL); 
  
    uint16_t gFreqCtrl = map(ctrl,0,1023,patches[0][0][0],patches[0][0][1]); 
    uint16_t gDecayCtrl= map(ctrl,0,1023,patches[0][1][0],patches[0][1][1]); 
    uint16_t g2FreqCtrl = map(ctrl,0,1023,patches[0][2][0],patches[0][2][1]); 
    uint16_t g2DecayCtrl = map(ctrl,0,1023,patches[0][3][0],patches[0][3][1]); 

    grainPhaseInc  = mapPhaseInc(gFreqCtrl) / 2; 
    grainDecay     = gDecayCtrl / 8; 
    grain2PhaseInc = mapPhaseInc(g2FreqCtrl) / 2; 
    grain2Decay     = g2DecayCtrl / 4; 


  };

  
  
} 

uint16_t diff(uint16_t a, uint16_t  b){
  return (a > b) ? a - b : b - a;
}



SIGNAL(PWM_INTERRUPT) 
{ 
  uint8_t value; 
  uint16_t output; 

  syncPhaseAcc += syncPhaseInc; 
  if (syncPhaseAcc < syncPhaseInc) { 
    // Time to start the next grain 
    grainPhaseAcc = 0; 
    grainAmp = 0x7fff; 
    grain2PhaseAcc = 0; 
    grain2Amp = 0x7fff; 
  } 

  // Increment the phase of the grain oscillators 
  grainPhaseAcc += grainPhaseInc; 
  grain2PhaseAcc += grain2PhaseInc; 

  // Convert phase into a triangle wave 
  value = (grainPhaseAcc >> 7) & 0xff; 
  if (grainPhaseAcc & 0x8000) value = ~value; 
  // Multiply by current grain amplitude to get sample 
  output = value * (grainAmp >> 8); 

  // Repeat for second grain 
  value = (grain2PhaseAcc >> 7) & 0xff; 
  if (grain2PhaseAcc & 0x8000) value = ~value; 
  output += value * (grain2Amp >> 8); 

  // Make the grain amplitudes decay by a factor every sample  (exponential decay) 
  grainAmp -= (grainAmp >> 8) * grainDecay; 
  grain2Amp -= (grain2Amp >> 8) * grain2Decay; 

  // Scale output to the available range, clipping if necessary 
  output >>= 9; 
  if (output > 255) output = 255; 

  // Output to PWM (this is faster than using analogWrite) 
  PWM_VALUE = output; 
}



