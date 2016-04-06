#include <detpic32.h>

#define DisableUart1RxInterrupt() IEC0bits.U1RXIE = 0;
#define EnableUart1RxInterrupt()  IEC0bits.U1RXIE = 1;
#define DisableUart1TxInterrupt() IEC0bits.U1TXIE = 0;
#define EnableUart1TxInterrupt()  IEC0bits.U1TXIE = 1;
#define BUF_SIZE 8
#define INDEX_MASK (BUF_SIZE - 1)

void comDrv_flushRx(void);
void comDrv_flushTx(void);
void comDrv_putc(char);

int main(void){
  return 0;
}

typedef struct{
  unsigned char data[BUF_SIZE];
  unsigned int head;
  unsigned int tail;
  unsigned int count;
} circularBuffer;

volatile circularBuffer rxb;
volatile circularBuffer txb;

void comDrv_flushRx(void){
  int i = 0;
  for (i = 0; i < BUF_SIZE; i++){
    rxb.data[i] = '\0';
  }
  rxb.head = 0;
  rxb.tail = 0;
  rxb.count = 0;
}

void comDrv_flushTx(void){
  int i = 0;
  for (i = 0; i < BUF_SIZE; i++){
    txb.data[i] = '\0';
  }
  txb.head = 0;
  txb.tail = 0;
  txb.count = 0;
}

void comDrv_putc(char c){
  while(txb.count == BUF_SIZE);
  txb.data[txb.tail] = c;
  txb.tail = (txb.tail + 1) & INDEX_MASK;
  DisableUart1TxInterrupt();
  txb.count++;
  EnableUart1TxInterrupt();
}
