#include <detpic32.h>
#define CALIBRATION_VALUE 5000
void delay(unsigned int n_intervals)
{
	volatile unsigned int i;

	for(; n_intervals != 0; n_intervals--)
		for(i = CALIBRATION_VALUE; i != 0; i--)
			;
}

void send2displays(unsigned char value)
{	
	unsigned char digit_low,digit_high;
	static unsigned char displayFlag = 0;
	static const unsigned char display7Scodes[] = {0x77,0x41,0x3B,0x6B,0x4D,0x6E,0x7E,0x43,0x7F,0x6F,0x5F,0x7C,0x36,0x79,0x3E,0x1E};

	digit_low = value & 0x0F;
	digit_high = value>>4;

	//if "displayFlag" is 0 send digit_low to display_low
	if(displayFlag == 0){
		LATBbits.LATB9 = 1;
		LATBbits.LATB8 = 0;
		LATB = LATB & 0xFF00;
		LATB = LATB | display7Scodes[digit_low];
	}
	else{
		LATBbits.LATB9 = 0;
		LATBbits.LATB8 = 1;
		LATB = LATB & 0xFF00;
		LATB = LATB | display7Scodes[digit_high];
	}
	displayFlag = !displayFlag;
}
void setPWM(unsigned int dutyCycle){
	if(dutyCycle >= 0 && dutyCycle <= 100){
		OC1RS = ((PR3+1)*dutyCycle)/100;
	}
}

volatile unsigned int value2display;
void main(void)
{
	const static unsigned char pwmValues[]={3, 15, 40, 90};
	TRISB = TRISB & 0xFC00;
	TRISE = TRISE | 0x00F0;
	TRISBbits.TRISB14 = 1;
	
	AD1PCFGbits.PCFG14 = 0;
	AD1CHSbits.CH0SA = 14;
	AD1CON2bits.SMPI = 7;
	AD1CON1bits.SSRC = 7;
	AD1CON1bits.CLRASAM = 1;
	AD1CON3bits.SAMC = 16;
	AD1CON1bits.ON = 1;

	IPC6bits.AD1IP = 1;
	IEC1bits.AD1IE = 1;
	AD1CON1bits.ASAM = 1;
	IFS1bits.AD1IF = 0;

	T3CONbits.TCKPS = 2;
	PR3 = 49999;
	TMR3 = 0;
	T3CONbits.TON = 1;
	IPC3bits.T3IP = 2;
	IFS0bits.T3IF = 0;
	IEC0bits.T3IE = 1;

	T1CONbits.TCKPS = 3;
	PR1 = 19530;
	TMR1 = 0;
	T1CONbits.TON = 1;
	IPC1bits.T1IP = 2;
	IFS0bits.T1IF = 0;

	/*T2CONbits.TCKPS = 2;
	PR2 = 49999;
	TMR2 = 0;
	T2CONbits.TON = 1;
	IPC2bits.T2IP = 2;
	IFS0bits.T2IF = 0;
	IEC0bits.T2IE = 1;
	*/
	OC1CONbits.OCM = 6;
	OC1CONbits.OCTSEL = 1;
	OC1CONbits.ON = 1;
	
	EnableInterrupts(); // Global Interrupt Enable

	int portVal,dc;
	while(1)
	{
		// Read RE5, RE4 to the variable "portVal"
		if(PORTEbits.RE5 == 0 && PORTEbits.RE4 == 0) portVal = 0;
		else if(PORTEbits.RE5 == 0 && PORTEbits.RE4 == 1) portVal = 1;
		else if(PORTEbits.RE5 == 1 && PORTEbits.RE4 == 0) portVal = 2;
		else portVal = 3;
		switch(portVal)
		{
			case 0: // Measure input voltage
				// Enable T1 interrupts
				IEC0bits.T1IE = 1;
				setPWM(0);
				break;
			case 1: // Freeze
				// Disable T1 interrupts
				IEC0bits.T1IE = 0;
				setPWM(100);
				break;
			case 2: // LED brigthness control
				// Disable T1 interrupts
				IEC0bits.T1IE = 0;
				// Read RE7, RE6 (duty-cycle value) to the variable "dc"			
				if(PORTEbits.RE7 == 0 && PORTEbits.RE6 == 0) dc = 0;			
				else if(PORTEbits.RE7 == 0 && PORTEbits.RE6 == 1) dc = 1;
				else if(PORTEbits.RE7 == 1 && PORTEbits.RE6 == 0) dc = 2;
				else dc = 3;
				setPWM(pwmValues[dc]);
				// Copy duty-cycle value to global variable "value2display"
				value2display = (pwmValues[dc]/10)*16+(pwmValues[dc]%10);
				break;
			default:
				break;
		}
	}
}
void _int_(27) isr_adc(void)
{	
	int j,average = 0;
	// Calculate buffer average (8 samples)
	int *p = (int*) (&ADC1BUF0);
	for(j = 0; j < 8; j++){
		average += p[j*4];
	}
	average = average/8;
	// Calculate voltage amplitude
	average = (average*33+511)/1023;
	// Convert voltage amplitude to decimal. Assign it to "value2display"
	char decimal = (average/10)*16+(average%10);
	value2display = decimal;
	IFS1bits.AD1IF = 0; // Reset AD1IF flag
}
void _int_(4) isr_T1(void)
{
	// Start A/D conversion
	AD1CON1bits.ASAM = 1;
	// Reset T1IF flag
	IFS0bits.T1IF = 0;
}
void _int_(12) isr_T3(void)
{
	// Send "value2display" global variable to displays
	send2displays(value2display);
	// Reset T3IF flag
	IFS0bits.T3IF = 0;
}
