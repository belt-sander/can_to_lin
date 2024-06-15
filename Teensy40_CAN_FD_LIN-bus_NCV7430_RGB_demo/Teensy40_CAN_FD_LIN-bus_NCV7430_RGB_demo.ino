#include "lin_bus.h"
#include <FlexCAN_T4.h>
FlexCAN_T4<CAN3, RX_SIZE_256, TX_SIZE_16> Can0;

#define NUM_RX_MAILBOXES 1
#define NUM_TX_MAILBOXES 1

int ledState = LOW;                // ledState used to set the LED

LIN lin(&Serial3, 19200);

int lin_cs = 32;
int led1 = 23;

uint8_t buffer_state_a[] = {0xc0, 0x00, 0x00, 0x00, 0x31, 0x00, 0xff, 0x00};
uint8_t buffer_state_b[] = {0xc0, 0x00, 0x00, 0x00, 0x31, 0xff, 0x00, 0x00};
uint8_t buffer_state_c[] = {0xc0, 0x00, 0x00, 0x00, 0x31, 0x00, 0x00, 0xff};
uint8_t can_data;

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(lin_cs, OUTPUT);
  digitalWrite(lin_cs, HIGH);
  delay(100);
  pinMode(led1,OUTPUT);
  
  Serial.begin(115200);
  myTimer.begin(blinkLED, interval);

  Can0.begin();
  Can0.setMaxMB(NUM_RX_MAILBOXES + NUM_TX_MAILBOXES);

  for (int i = 0; i<NUM_RX_MAILBOXES; i++){
      Can0.setMB((FLEXCAN_MAILBOX)i,RX,STD);
  }
  for (int i = 0; i<NUM_TX_MAILBOXES; i++){
      Can0.setMB((FLEXCAN_MAILBOX)i,TX,STD);
  }

  Can0.setMBFilter(REJECT_ALL);
  Can0.enableMBInterrupts();
  Can0.setMBFilter(MB1, 0x666);
  Can0.onReceive(MB1, canReadConfig);

  Serial.println("--- FlexCAN_T4 MB Status ---");
  Can0.mailboxStatus();
  Serial.println("");


  FD.setMBFilter(ACCEPT_ALL);
  FD.setMBFilter(MB13, 0x1);
  FD.setMBFilter(MB12, 0x1, 0x3);
  FD.setMBFilterRange(MB8, 0x1, 0x04);
  FD.enableMBInterrupt(MB8);
  FD.enableMBInterrupt(MB12);
  FD.enableMBInterrupt(MB13);
  FD.enhanceFilter(MB8);
  FD.enhanceFilter(MB10);
  FD.distribute();
  FD.mailboxStatus();

  
}
void sendframe()
{
  CANFD_message_t msg;
  msg.len = 64;           // Set frame length to 64 bytes
  msg.id = 0x321;
  msg.seq = 1;
  msg.buf[0] = can_data++;       
  msg.buf[1] = 1;
  msg.buf[2] = 2;
  msg.buf[3] = 3;
  msg.buf[4] = 4;  
  msg.buf[5] = 5;
  msg.buf[6] = 6;
  msg.buf[7] = 7;
  FD.write( msg);
}


void loop() {
  // Red
  Serial.println("red");
  set_nvc7430_color(buffer_red);
  delay(500);
  
  // Green
  Serial.println("green");
  set_nvc7430_color(buffer_green);
  delay(500);
  
  // Blue
  Serial.println("blue");
  set_nvc7430_color(buffer_blue);
  delay(500);
}

void init_ncv7430(void) {
  uint8_t control_buffer[] = {0xc0, 0x00, 0x00, 0x7f};
  
  lin.order(SET_LED_CONTROL, control_buffer, 4);
}

void set_nvc7430_color(byte* message) {
  lin.order(SET_LED_COLOUR, message, 8);
}

void blinkLED() {
  ledState = !ledState;
  
  digitalWrite(LED_BUILTIN, ledState);
  digitalWrite(led1, ledState);
  sendframe();
}
