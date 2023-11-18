#include "encoder.h"
#include "sdcard_opt.h"



//these pins can not be changed 2/3 are special pins
uint8_t encoder_pin1 = 2;
uint8_t encoder_pin2 = 3;
uint8_t encoder_pin3 = 14;//push PC0


volatile int last_encoded = 0;

volatile int16_t encoder_value = 0;
volatile int8_t encoder_push = 0;
volatile int8_t encoder_changed = 0;

ISR (PCINT0_vect)
{
	// handle pin change interrupt for pin D8 to D13 here
} // end of PCINT0_vect
 
ISR (PCINT1_vect)
{
	static unsigned long temp = 0;
	if(millis() - temp < 300) //
		return;
	temp = millis();
	
	int enc_push = digitalRead(encoder_pin3);
	if(enc_push == LOW){
		//Serial.println("enc_push");
		encoder_push = 1;			
	}else {
		sd_detectpin = digitalRead(SDPIN_DETECT);	
	}
	encoder_changed = 1;
} // end of PCINT1_vect
 
ISR (PCINT2_vect)
{
	// handle pin change interrupt for pin D0 to D7 here
} // end of PCINT2_vect


void updateEncoder()
{
	int MSB = digitalRead(encoder_pin1); //MSB = most significant bit
	int LSB = digitalRead(encoder_pin2); //LSB = least significant bit

	int encoded = (MSB << 1) |LSB; //converting the 2 pin value to single number 
	int sum = (last_encoded << 2) | encoded; //adding it to the previous encoded value 
	if(sum == 0b1101 || sum == 0b0100 || sum == 0b0010 || sum == 0b1011) encoder_value ++;
	if(sum == 0b1110 || sum == 0b0111 || sum == 0b0001 || sum == 0b1000) encoder_value --; 
	last_encoded = encoded; //store this value for next time 

	encoder_changed = 1;
}

void encoder_init(void)
{
	pinMode(encoder_pin1, INPUT);
	pinMode(encoder_pin2, INPUT);

	digitalWrite(encoder_pin1, HIGH); //turn pullup resistor on
	digitalWrite(encoder_pin2, HIGH); //turn pullup resistor on

	//call updateEncoder() when any high/low changed seen
	//on interrupt 0 (pin 2), or interrupt 1 (pin 3)
	attachInterrupt(0, updateEncoder, CHANGE);
	attachInterrupt(1, updateEncoder, CHANGE);

	// pin change interrupt (example for pin A0)
	PCMSK1 |= (bit (PCINT8) | bit(PCINT9)); // PC0 PC1
	PCIFR |= bit (PCIF1); // clear any outstanding interrupts
	PCICR |= bit (PCIE1); // enable pin change interrupts for D8 to D13
}
