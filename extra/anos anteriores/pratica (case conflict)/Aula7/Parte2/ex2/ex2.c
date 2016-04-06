#include<detpic32.h>
void setPWM(unsigned int dutyCycle)
{	
	if(dutyCycle >= 0 && dutyCycle <= 100){
		OC1RS = ((PR3+1)*dutyCycle)/100;
	}
}
void main(void){
	T3CONbits.TCKPS = 2;
	PR3 = 49999; 
	TMR3 = 0; // Reset timer T3 count register
	T3CONbits.TON = 1; // Enable timer T3 (must be the last command of the
	// timer configuration sequence)
	OC1CONbits.OCM = 6; // PWM mode on OCx; fault pin disabled
	OC1CONbits.OCTSEL =1;// Use timer T3 as the time base for PWM generation
	int duty;
	//printStr("Insert Duty Cycle (0-100): \n");
	duty = 10;
	setPWM(duty); // Ton constant
	OC1CONbits.ON = 1; // Enable OC1 module
	while(1) ;
}
