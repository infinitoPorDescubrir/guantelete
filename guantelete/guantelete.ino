#include <avr/io.h> 
#include <avr/interrupt.h> 
#include <EEPROM.h>

int pines[] = {1,2,3};
int estadoPines[] = {1,1,1};
int antirebotes[] = {0,0,0};
int notas[] = {308,259,231,194,173,154,129,0}; //last index is silence

#define PWM_PIN   0
#define PATCH_COUNT 4
byte patchIndex = 0;


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


int patches[PATCH_COUNT][4][2]={
    {{0,363},{572,237},{158,366},{9,0}},
    {{0,0},{148,149},{392,120},{7,12}},
    {{589,65},{0,0},{851,855},{729,356}},
    {{405,0},{25,25},{0,0},{342,342}}
  };

void setup() { 
  pinMode(2,OUTPUT); 
  pinMode(1,OUTPUT); 
  pinMode(PWM_PIN,OUTPUT); 
  setupTimer();
  initPatch();
  for(int i=0;i<3;i++){
    //pinMode(pines[i], INPUT_PULLUP);
  }

} 



void initPatch(){
  int addr = 0;

  patchIndex = 1;
  return;

  // read patchIndex from EEPROM
  byte i = EEPROM.read(addr);
  //ensure bounds
  i %= PATCH_COUNT;
  patchIndex = i;

  //move to next patch for next run
  EEPROM.write(addr, (i+1));

}



volatile long m = 0;
void loop() { 


  for(int i=0;i<3;i++){
    int nuevoEstadoPin = digitalRead(pines[i]);
    if (estadoPines[i] != nuevoEstadoPin ){
      antirebotes[i] = antirebotes[i] + 1;
      if(antirebotes[i] > 150 ){
        estadoPines[i] = nuevoEstadoPin;
        antirebotes[i] = 0;
      }
    }else{
        antirebotes[i]= 0;
    }
  }

  int index = estadoPines[0]*4+estadoPines[1]*2+estadoPines[2];
  syncPhaseInc =notas[index];
  
  uint16_t ctrl = analogRead(A2); 

  syncPhaseInc =notas[abs((m/10000)%8)];
  ctrl = abs((m/1000)%1000-500);


  uint16_t gFreqCtrl = map(ctrl,0,1023,patches[patchIndex][0][0],patches[patchIndex][0][1]); 
  uint16_t gDecayCtrl= map(ctrl,0,1023,patches[patchIndex][1][0],patches[patchIndex][1][1]); 
  uint16_t g2FreqCtrl = map(ctrl,0,1023,patches[patchIndex][2][0],patches[patchIndex][2][1]); 
  uint16_t g2DecayCtrl = map(ctrl,0,1023,patches[patchIndex][3][0],patches[patchIndex][3][1]); 

  grainPhaseInc  = mapPhaseInc(gFreqCtrl) / 2; 
  grainDecay     = gDecayCtrl / 8; 
  grain2PhaseInc = mapPhaseInc(g2FreqCtrl) / 2; 
  grain2Decay     = g2DecayCtrl / 4; 
  
} 


void setupTimer()
{
  //_BV(CTC1) |  | _BV(CS13)
  //TCCR1 =  _BV(PWM1A) | _BV(COM1A0) | _BV(COM1A1) | _BV(CS10);
  //TIMSK |= _BV(TOIE1); 

  // connect pwm to pin on timer 1, channel A
  //TCCR0A |=_BV(COM0A1);
  //TCCR0A &=~_BV(COM0A0);
  //OCR0A =  120;

  // connect pwm to pin on timer 1, channel A
  //TCCR1 |=_BV(COM1A1);
  //TCCR1 &=~_BV(COM1A0);
  //OCR1A =  120;
  analogWrite(1,120);

  
}

/*
volatile long l=0;

// TIMER1 overflow interrupt service routine
// called whenever TCNT1 overflows
SIGNAL (TIMER1_OVF_vect)
{
  uint8_t value; 
  uint16_t output; 


    digitalWrite(2,(l/10000)%2);  
    l++;

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
  OCR0A =  output;

  
}



*/
