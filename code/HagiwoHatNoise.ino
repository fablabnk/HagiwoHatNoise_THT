#include <avr/io.h>//for custom pwm setting

//noise setting
unsigned int frq = 1000; // 周波数
float duty = 0.5;//pin10 for noise
unsigned int knob_tune = 1;//knob AD
unsigned int knob_tone = 1;//knob AD
unsigned int CV_tone = 1;//CV AD

//decay setting
unsigned int knob_decay = 0;//knob AD
unsigned int CV_decay = 0;//CV AD
unsigned int decay_time = 0;
unsigned int d_duty = 100;//pin3 for decay

int i = 100;//decay wavetable reference
bool before_gate = 0;//0=no signal,1=gate in
bool gate = 0;//0=decay out end , 1=during decay out
long trigTimer = 0;//for generate decay time

//decay wavetable
const static word decay_table[100] PROGMEM = {
 100,  96, 94, 92, 91, 89, 87, 86, 84, 82, 81, 79, 78, 76, 74, 73, 71, 70, 68, 67, 65, 64, 63, 61, 60, 58, 57, 56, 54, 53, 52, 51, 49, 48, 47, 46, 44, 43, 42, 41, 40, 39, 38, 37, 36, 34, 33, 32, 31, 31, 30, 29, 28, 27, 26, 25, 24, 23, 23, 22, 21, 20, 19, 19, 18, 17, 17, 16, 15, 15, 14, 13, 13, 12, 12, 11, 11, 10, 10, 9,  9,  8,  8,  8,  7,  7,  6,  6,  6,  6,  5,  5,  5,  5,  4,  4,  3,  3,  2,  1
};

void setup() {
 pinMode(3, OUTPUT);//decay out
 pinMode(5, INPUT);//gate in
 pinMode(10, OUTPUT);//noise out

 //10pin pwm setting
 TCCR1A = 0b00100001;
 TCCR1B = 0b00010001;//分周比1

 //3pin pwm setting
 TCCR2A = 0b00100001;
 TCCR2B = 0b00001001;//分周比1

 //for development
//  Serial.begin(9600);
}

void loop() {
 before_gate = gate;//for gate signal detect

 //------------------decay time setting---------------


 knob_decay = 1023 - analogRead(3); // inverted due to miswiring of pins 1 and 3 on pots
 
 CV_decay = analogRead(5);
 decay_time = (knob_decay + CV_decay ) * 5;

 //------------------noise setting-----------
 knob_tune = 1023 - analogRead(0); // inverted due to miswiring of pins 1 and 3 on pots
 knob_tone = 1023 - analogRead(1); // inverted due to miswiring of pins 1 and 3 on pots
 CV_tone = analogRead(6);

 //------------------detect gate signal--------------
 gate = digitalRead(5);
 if (before_gate == 0 && gate == 1) {
   i = 0;
 }

 //--------------10pin pwm setting-----------------
 //MAKE SOME NOISE!!

 frq = random(1 + knob_tone * 2 + CV_tone * 2, 2 + knob_tone * 2 + CV_tone * 2 + knob_tune * 2);
 duty = (float)random(1, 999) / 1000;


 // TOP値指定
 OCR1A = (unsigned int)(8000000 / frq );

 // Duty比指定
 OCR1B = (unsigned int)(8000000  / frq * duty);


 //--------------3pin pwm setting-----------------

 // TOP値指定
 OCR2A = (unsigned int)(8000000 / 20000 );

 if (knob_decay <= 970) {//make decay wave
   if (i < 99 && (micros() - trigTimer >= decay_time)) {
     i++;
     d_duty = (pgm_read_word(&(decay_table[i]))) ;
     trigTimer = micros();
   }
 }
 
 else {//decay knob max = constant output ( no decay )
   d_duty = 100;
 }

 // Duty比指定
 OCR2B = (unsigned int)(8000000  / 20000 * d_duty / 300);//300 is magic number

 // for development(Serial communication affects noise generation!)
 //     Serial.print(d_duty);
 //     Serial.print(",");
 //     Serial.print(gate);
 //     Serial.println("");
}