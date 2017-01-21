/*

  Cortito y al pie:

  
                         pin4:   G2 --------  `+:/.                    
                                     ``       :. `+   `+::  ----- pin5: A2         
                  pin3:   F2 -----  .o.o.     /  `+   +  /             
                                    .s  o`    o  `/   o  +             
                                     d` ./    s  .-  -- `/             
                                     -o  :-  ./  -- `+  :.   -/+.      
                                      y   +` --  `+ +`  o   `/ :-   ----- pin6: C3
                                      /o   / `:  `+ /   /  ./ `+       
                                       s:  ././  `o:.  :. /- `+        
                                        h`  ./.   :/   o +. .+         
                                        +-             -:. `+          
                           -+/--:/.     /`                `o           
          pin2:  D2 -----   ./:   -/    /-                 +           
                               ::` `:-  s`                `/           
                                 //  `:/o`                /`           
                                  ::                      s            
                                   //.                    o            
                                     /-                  /-            
                                      /:                `s             
                                       -/`              +              
                                          :.           /`              
*/


int pines[] = {2,3,4,5,6};
int estadoPines[] = {1,1,1,1,1};
int antirebotes[] = {0,0,0,0,0};

/*
Notas a MIDI:
-------------
    D2   38
    F2   41
    G2   43
    A2   45  ( 110hz)
    C3   48

*/
int notas[] = {38,41,43,45,48};


void setup() {

  for(int i=0;i<5;i++){
    pinMode(pines[i], INPUT_PULLUP);
  }
  
  Serial.begin(9600);

}

void loop() {

  for(int i=0;i<5;i++){
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
  Serial.write(0x90);
  Serial.write(pitch);
  Serial.write(velocity);
}

void noteOff(int pitch) {
  noteOn(pitch, 0);
}


