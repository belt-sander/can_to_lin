#include "lin_bus.h"
#include <FlexCAN_T4.h>
FlexCAN_T4<CAN3, RX_SIZE_256, TX_SIZE_16> Can0;

#define NUM_RX_MAILBOXES 1
#define NUM_TX_MAILBOXES 1

unsigned int BASE_CAN_ADDRESS = 0x777;
static CAN_message_t msg1;

LIN lin;
int lin_cs = 32;
int lin_fault = 28;
int led1 = 13;

uint8_t buffer_state_a[] = {0xc0, 0x00, 0x00, 0x00, 0x31, 0x00, 0xff, 0x00};
uint8_t can_data;

void readFrame(const CAN_message_t &frame) {
  Serial.println("--- Incoming user request message ---");
  Serial.print("OVERRUN: ");
  Serial.print(frame.flags.overrun);
  Serial.print(" LEN: ");
  Serial.print(frame.len);
  Serial.print(" EXT: ");
  Serial.print(frame.flags.extended);
  Serial.print(" TS: ");
  Serial.print(frame.timestamp);
  Serial.print(" ID: ");
  Serial.print(frame.id, HEX);
  Serial.print(" Buffer: ");
  for (uint8_t i = 0; i < frame.len; i++) {
    Serial.print(frame.buf[i], HEX);
    Serial.print(" ");
  }
  Serial.println();
}

void setup() {
  pinMode(lin_cs, OUTPUT);
  digitalWrite(lin_cs, HIGH);
  pinMode(lin_fault, INPUT);
  lin.begin(&Serial3, 19200);

  delay(100);
  pinMode(led1, OUTPUT);
  digitalWrite(led1, HIGH);

  Serial.begin(115200);
  Serial.println("LIN has been enabled");

  Can0.begin();
  Can0.setBaudRate(1000000);
  Can0.setMaxMB(NUM_RX_MAILBOXES + NUM_TX_MAILBOXES);

  for (int i = 0; i < NUM_RX_MAILBOXES; i++) {
    Can0.setMB((FLEXCAN_MAILBOX)i, RX, STD);
  }
  for (int i = 0; i < NUM_TX_MAILBOXES; i++) {
    Can0.setMB((FLEXCAN_MAILBOX)i, TX, STD);
  }

  Can0.setMBFilter(REJECT_ALL);
  Can0.enableMBInterrupts();
  Can0.setMBFilter(MB1, 0x666);
  Can0.onReceive(MB1, readFrame);

  Serial.println("--- FlexCAN_T4 MB Status ---");
  Can0.mailboxStatus();
  Serial.println("CAN has been enabled");
  Serial.println("");
}

void constructFrame() {
  msg1.id = BASE_CAN_ADDRESS;
  msg1.len = 8;
  msg1.buf[0] = 0;
  msg1.buf[1] = 0;
  msg1.buf[2] = 0;
  msg1.buf[3] = 0;
  msg1.buf[4] = 0;
  msg1.buf[5] = 0;
  msg1.buf[6] = 0;
  msg1.buf[7] = 0;
}

void loop() {
  constructFrame();
  lin.order(0x31, buffer_state_a, 8);
  Serial.println("you're doing great.");
  Can0.write(msg1);
  delay(100);
}