/*

  Cortito y al pie:

  
                         pin2:   G2 --------  `+:/.                    
                                     ``       :. `+   `+::  ----- pin3: A2         
                  pin1:   F2 -----  .o.o.     /  `+   +  /             
                                    .s  o`    o  `/   o  +             
                                     d` ./    s  .-  -- `/             
                                     -o  :-  ./  -- `+  :.   -/+.      
                                      y   +` --  `+ +`  o   `/ :-   ----- pin4: C3
                                      /o   / `:  `+ /   /  ./ `+       
                                       s:  ././  `o:.  :. /- `+        
                                        h`  ./.   :/   o +. .+         
                                        +-             -:. `+          
                           -+/--:/.     /`                `o           
            GND:  --------   ./:   -/    /-                 +           
                               ::` `:-  s`                `/           
                                 //  `:/o`                /`           
                                  ::                      s            
                                   //.                    o            
                                     /-                  /-            
                                      /:                `s             
                                       -/`              +              
                                          :.           /`              
*/

#include "SendOnlySoftwareSerial.h"

SendOnlySoftwareSerial midiSerial(0);

int pines[] = {1,2,3,4};
int estadoPines[] = {1,1,1,1};
int antirebotes[] = {0,0,0,0};

/*
Notas a MIDI:
-------------
    F2   41
    G2   43
    A2   45  ( 110hz)
    C3   48

*/
int notas[] = {41,43,45,48};

void setup() {

  for(int i=0;i<4;i++){
    pinMode(pines[i], INPUT_PULLUP);
  }
  
  midiSerial.begin(31250);

}

void loop() {
  
  for(int i=0;i<4;i++){
    int nuevoEstadoPin = digitalRead(pines[i]);
    
    if (estadoPines[i] != nuevoEstadoPin ){
      antirebotes[i] = antirebotes[i] + 1;
  
      if(antirebotes[i] > 200 ){
        cambiarEstadoPin(i,nuevoEstadoPin);
      }
    }else{
        antirebotes[i]= 0;
    }
  }

}


void cambiarEstadoPin(int pin, int nuevoEstado) {
  estadoPines[pin] = nuevoEstado;
  antirebotes[pin] = 0;
  if(estadoPines[pin] == 0){
    noteOn(notas[pin], 100);
  }else{
    noteOff(notas[pin]);
  }
}



//  plays a MIDI note.
void noteOn(int pitch, int velocity) {
  midiSerial.write(0x90);
  midiSerial.write(pitch);
  midiSerial.write(velocity);
}

void noteOff(int pitch) {
  noteOn(pitch, 0);
}


