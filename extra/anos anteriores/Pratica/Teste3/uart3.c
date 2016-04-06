#include <detpic32.h>

void putc(char ch){
	while(U1STAbits.UTXBF == 1);
	U1TXREG = ch;
}

char getc(void){
	if(U1STAbits.OERR == 1) U1STAbits.OERR = 0;
	while(U1STAbits.URXDA != 1);
	return U1RXREG;
}

void main(void){
	U1MODEbits.BRGH = 0;
	U1BRG = 10;
	
	U1MODEbits.PDSEL = 0;
	U1MODEbits.STSEL = 0;

	U1STAbits.UTXEN = 1;
	U1STAbits.URXEN = 1;
	
	U1STAbits.UTXSEL = 0;
	U1STAbits.URXISEL = 0;

	IFS0.U1TXIF = 0;
	IFS0.U1RXIF = 0;

	
	U1MODEbits.ON = 1;

	while(1){
	
	}
}
