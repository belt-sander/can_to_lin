/*
 * Brief:       A module to allow CAN messages to control a Bosch WDA
 * Author:      Sander
 */ 

#include "lin_bus.h"
#include <FlexCAN_T4.h>
FlexCAN_T4<CAN3, RX_SIZE_256, TX_SIZE_16> Can0;

#define NUM_RX_MAILBOXES 1
#define NUM_TX_MAILBOXES 1

unsigned int BASE_RX_CAN_ADDRESS = 0x777;
unsigned int BASE_TX_DIAG_CAN_ADDRESS = 0x778;
bool DEBUG_MODE = true;
static CAN_message_t msg1;

LIN lin;

int lin_cs = 32;
int lin_fault = 28;
int led1 = 13;

int wda_counter = 0;  // Bosch WDA 0-15 rolling counter
int wda_kl15_enable = 1;  // Bosch WDA KL15 ignition enable (ON REQUIRED)
int wda_klx_enable = 1;  // Bosch WDA KLX enable (ON REQUIRED)

uint8_t wda_intermittent_wipe_speed = 5;  // Bosch WDA speed controls for intermittent wipe
/*
  * 1 = slow
  * 5 = low-mid
  * 9 = high-mid
  * 13 = = fast
  */
int wda_single_wipe_req = 0;  // Bosch WDA single wipe
int wda_intermittent_wipe_req = 0;  // Bosch WDA intermittent wipe
int wda_cont_slow_wipe_req = 0;  // Bosch WDA slow continous wipe
int wda_cont_fast_wipe_req = 0;  // Bosch WDA fast continous wipe

uint8_t can_tx_byte_1 = 0x00;
uint8_t can_tx_byte_2 = 0x00;
uint8_t buffer_data[] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
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

  if (frame.buf[0] == 0x66) {  // Header
    
    if (frame.buf[1] == 0x01) {  // Intermittent wipe
      wda_intermittent_wipe_req = 1;
      wda_single_wipe_req = 0;
      wda_cont_slow_wipe_req = 0;
      wda_cont_fast_wipe_req = 0;
      if (frame.buf[2] == 0x01) {  // Intermittent wipe speed
        wda_intermittent_wipe_speed = 1;
      }
      else if (frame.buf[2] == 0x02) {
        wda_intermittent_wipe_speed = 5;
      }
      else if (frame.buf[2] == 0x03) {
        wda_intermittent_wipe_speed = 9;
      }
      else if (frame.buf[2] == 0x04) {
        wda_intermittent_wipe_speed = 13;
      }
      else{
        wda_intermittent_wipe_speed = 5;  // Default mid-slow
      }
    }

    else if (frame.buf[1] == 0x02) {  // Slow wipe
      wda_intermittent_wipe_req = 0;
      wda_single_wipe_req = 0;
      wda_cont_slow_wipe_req = 1;
      wda_cont_fast_wipe_req = 0;
    }

    else if (frame.buf[1] == 0x03) {  // Fast wipe
      wda_intermittent_wipe_req = 0;
      wda_single_wipe_req = 0;
      wda_cont_slow_wipe_req = 0;
      wda_cont_fast_wipe_req = 1;
    }

    else if (frame.buf[1] == 0x04) {  // Single wipe
      wda_intermittent_wipe_req = 0;
      wda_single_wipe_req = 1;
      wda_cont_slow_wipe_req = 0;
      wda_cont_fast_wipe_req = 0;
    }

    else {  // Goodbye
      wda_intermittent_wipe_req = 0;
      wda_single_wipe_req = 0;
      wda_cont_slow_wipe_req = 0;
      wda_cont_fast_wipe_req = 0;
    } 

    can_tx_byte_1 = frame.buf[1];  // Data to be sent out to logger
    can_tx_byte_2 = frame.buf[2];  // Data to be sent out to logger
  }
}

void setup() {
  pinMode(lin_cs, OUTPUT);
  digitalWrite(lin_cs, HIGH);
  pinMode(lin_fault, INPUT);
  lin.begin(&Serial3, 19200);
  Serial.println("LIN has been enabled");

  pinMode(led1, OUTPUT);
  digitalWrite(led1, HIGH);

  Serial.begin(115200);

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
  Can0.setMBFilter(MB1, BASE_RX_CAN_ADDRESS);
  Can0.onReceive(MB1, readFrame);

  Serial.println("--- FlexCAN_T4 MB Status ---");
  Can0.mailboxStatus();
  Serial.println("CAN has been enabled");
  Serial.println("");
}

void canSendDiagPacket() {
  msg1.id = BASE_TX_DIAG_CAN_ADDRESS;
  msg1.len = 3;
  msg1.buf[0] = 0x66;
  msg1.buf[1] = can_tx_byte_1;
  msg1.buf[1] = can_tx_byte_2;
  
  Can0.write(msg1);
}


void loop() {
  Can0.events();

  wda_counter++;

  if (wda_counter > 15) {
    wda_counter = 0;
  }

  buffer_data[0] = (buffer_data[0] & 0xF0) | (wda_counter & 0x0F);

  if (wda_kl15_enable == 1) {
    buffer_data[0] |= (1 << 4);  // Set bit 4
  } else {
    buffer_data[0] &= ~(1 << 4);  // Clear bit 4
  }

  if (wda_klx_enable == 1) {
    buffer_data[0] |= (1 << 5);  // Set bit 5
  } else {
    buffer_data[0] &= ~(1 << 5);  // Clear bit 5
  }

  buffer_data[1] = (buffer_data[1] & 0xF0) | (wda_intermittent_wipe_speed & 0x0F);

  if (wda_single_wipe_req == 1) {
    buffer_data[1] |= (1 << 4);  // Set bit 4
  } else {
    buffer_data[1] &= ~(1 << 4);  // Clear bit 4
  }

  if (wda_intermittent_wipe_req == 1) {
    buffer_data[1] |= (1 << 5);  // Set bit 5
  } else {
    buffer_data[1] &= ~(1 << 5);  // Clear bit 5
  }

  if (wda_cont_slow_wipe_req == 1) {
    buffer_data[1] |= (1 << 6);  // Set bit 6
  } else {
    buffer_data[1] &= ~(1 << 6);  // Clear bit 6
  }

  if (wda_cont_fast_wipe_req == 1) {
    buffer_data[1] |= (1 << 7);  // Set bit 7
  } else {
    buffer_data[1] &= ~(1 << 7);  // Clear bit 7
  }

  lin.order(0x31, buffer_data, 8, lin2x);
  Serial.println("Raw data to wda: ");
  Serial.println((buffer_data[0] & 0xf0) >> 4);
  Serial.println((buffer_data[0] & 0x0f));  
  Serial.println((buffer_data[1] & 0xf0) >> 4);
  Serial.println((buffer_data[1] & 0x0f));  
  Serial.println();

  canSendDiagPacket();

  delay(20);  // Sleep
}
