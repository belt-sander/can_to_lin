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
int counter = 0;

uint8_t buffer_state_a[] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
uint8_t buffer_test[] = {0x30, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
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
  counter++;

  if (counter > 15) {
    counter = 0;
  }

  int bit4 = 1;  // KL15 Ignition enable (ON REQUIRED)
  int bit5 = 1;  // KLX enable (ON REQUIRED)

  uint8_t byte1_first_four_bits = 5;  // Speed controls for intermittent wipe
  /*
   * 1 = slow
   * 5 = low-mid
   * 9 = high-mid
   * 13 = = fast
   */
  int byte1_bit4 = 0;  // Single wipe
  int byte1_bit5 = 0;  // Intermittent wipe
  int byte1_bit6 = 0;  // Slow continous wipe
  int byte1_bit7 = 0;  // Fast continous wipe

  // Add the counter value to the first 4 bits of byte 0 of buffer_state_a
  buffer_state_a[0] = (buffer_state_a[0] & 0xF0) | (counter & 0x0F);

  // Set bit 4 based on bit4 variable
  if (bit4 == 1) {
    buffer_state_a[0] |= (1 << 4);  // Set bit 4
  } else {
    buffer_state_a[0] &= ~(1 << 4);  // Clear bit 4
  }

  // Set bit 5 based on bit5 variable
  if (bit5 == 1) {
    buffer_state_a[0] |= (1 << 5);  // Set bit 5
  } else {
    buffer_state_a[0] &= ~(1 << 5);  // Clear bit 5
  }

  // Set the first four bits of byte 1
  buffer_state_a[1] = (buffer_state_a[1] & 0xF0) | (byte1_first_four_bits & 0x0F);

  // Set bit 4 of byte 1 based on byte1_bit4 variable
  if (byte1_bit4 == 1) {
    buffer_state_a[1] |= (1 << 4);  // Set bit 4
  } else {
    buffer_state_a[1] &= ~(1 << 4);  // Clear bit 4
  }

  // Set bit 5 of byte 1 based on byte1_bit5 variable
  if (byte1_bit5 == 1) {
    buffer_state_a[1] |= (1 << 5);  // Set bit 5
  } else {
    buffer_state_a[1] &= ~(1 << 5);  // Clear bit 5
  }

  // Set bit 6 of byte 1 based on byte1_bit6 variable
  if (byte1_bit6 == 1) {
    buffer_state_a[1] |= (1 << 6);  // Set bit 6
  } else {
    buffer_state_a[1] &= ~(1 << 6);  // Clear bit 6
  }

  // Set bit 7 of byte 1 based on byte1_bit7 variable
  if (byte1_bit7 == 1) {
    buffer_state_a[1] |= (1 << 7);  // Set bit 7
  } else {
    buffer_state_a[1] &= ~(1 << 7);  // Clear bit 7
  }

  lin.order(0x31, buffer_state_a, 8, lin2x);
  constructFrame();
  Can0.write(msg1);
  delay(20);
}
