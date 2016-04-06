#include <detpic32.h>
#define BUF_SIZE 8 
#define INDEX_MASK (BUF_SIZE - 1)

#define DisableUart1RxInterrupt() IEC0bits.U1RXIE = 0
#define EnableUart1RxInterrupt() IEC0bits.U1RXIE = 1
#define DisableUart1TxInterrupt() IEC0bits.U1TXIE = 0
#define EnableUart1TxInterrupt() IEC0bits.U1TXIE = 1

typedef struct{
	unsigned char data[BUF_SIZE];
	unsigned int tail,head,count;
}circularBuffer;

volatile circularBuffer txb;
volatile circularBuffer rxb;

void comDrv_flushRx(void){
	rxb.head = rxb.tail = rxb.count = 0;
}

void comDrv_flushTx(void){
	txb.head = txb.tail = txb.count = 0;
}
void comDrv_putc(char ch){
	while(txb.count == BUF_SIZE);
	txb.data[txb.tail] = ch;
	txb.tail = (txb.tail + 1) & INDEX_MASK;
	DisableUart1TxInterrupt();
	txb.count++;
	EnableUart1TxInterrupt();
}
void comDrv_puts(char *s){
	int i = 0;
	while(s[i] != '\0'){
		comDrv_putc(s[i]);
		i++;
	}
}
char comDrv_getc(char *pchar){
	if(rxb.count == 0) return 0;
	DisableUart1RxInterrupt();
	*pchar = rxb.data[rxb.head];
	rxb.count--;
	rxb.head = (rxb.head + 1 ) & INDEX_MASK;
	EnableUart1RxInterrupt();
	return 1;
}
int main(void){
	U1MODEbits.BRGH = 0;
	U1BRG = 10;
	U1MODEbits.PDSEL = 0;
	U1MODEbits.STSEL = 0;
	
	U1STAbits.UTXSEL = 0;
	U1STAbits.URXISEL = 0;

	U1STAbits.UTXEN = 1;
	U1STAbits.URXEN = 1;

	IFS0bits.U1TXIF = 0;
	IFS0bits.U1RXIF = 0;
	IEC0bits.U1TXIE = 0;	
	IEC0bits.U1RXIE = 1;

	IPC6bits.U1IP = 2;
	U1STAbits.UTXEN = 1;
	U1STAbits.URXEN = 1;
	U1MODEbits.ON = 1;
	comDrv_flushRx();
	comDrv_flushTx();
	EnableInterrupts();
	char c;

	comDrv_puts("PIC32 UART Device-driver\n");
	while(1){
		while(comDrv_getc(&c) == 0);
		comDrv_putc(c);
	}
}
void _int_(24) isr_uart1(void){
	if(IFS0bits.U1TXIF == 1){
		if(txb.count > 0){
			U1TXREG = txb.data[txb.head];
			txb.head = (txb.head + 1) & INDEX_MASK;
			txb.count--;
		}
		if(txb.count == 0)
			DisableUart1TxInterrupt();
		IFS0bits.U1TXIF = 0;
	}
	if(IFS0bits.U1RXIF == 1){
		rxb.data[rxb.tail] = U1RXREG;
		rxb.tail = (rxb.tail + 1) & INDEX_MASK;
		if(rxb.count < BUF_SIZE)
			rxb.count++;
		else
			rxb.head = (rxb.head + 1) & INDEX_MASK;
		IFS0bits.U1RXIF = 0;
		
	}
}
