/*
 * 
 * Teensy 4.0 CAN FD and LIN-bus demo.
 * 
 * For use with this board:
 * https://www.skpang.co.uk/products/teensy-4-0-can-fd-and-lin-bus-breakout-board-include-teensy-4-0
 * 
 * LIN-bus library must be installed first.
 * https://github.com/MarkusLange/Teensy_3.x_4.x_and_LC_LIN_Master
 * 
 * NCV7430 RGB board:
 * https://www.skpang.co.uk/collections/breakout-boards/products/ncv7430-lin-bus-rgb-led-breakout-baord
 * 
 */

#include "lin_bus.h"
#include <FlexCAN_T4.h>
FlexCAN_T4FD<CAN3, RX_SIZE_256, TX_SIZE_16> FD;


// Create an IntervalTimer object 
IntervalTimer myTimer;

int ledState = LOW;                // ledState used to set the LED
unsigned long interval = 200000;   // interval at which to blinkLED to run every 0.2 seconds

#define SET_LED_CONTROL 0x23
#define SET_LED_COLOUR  0x24

LIN lin(&Serial3, 19200);

int lin_cs = 32;

int led1 = 23;
int lin_fault = 28;
//                              Grp   Grp   Fade  Intense  G     R     B
uint8_t buffer_red[]   = {0xc0, 0x00, 0x00, 0x00, 0x31, 0x00, 0xff, 0x00};
uint8_t buffer_green[] = {0xc0, 0x00, 0x00, 0x00, 0x31, 0xff, 0x00, 0x00};
uint8_t buffer_blue[]  = {0xc0, 0x00, 0x00, 0x00, 0x31, 0x00, 0x00, 0xff};
uint8_t can_data;

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(lin_fault,INPUT);
  pinMode(lin_cs, OUTPUT);
  digitalWrite(lin_cs, HIGH);
  delay(100);
  pinMode(led1,OUTPUT);
  

  Serial.begin(115200);
  Serial.print("NVC7430 RGB Demo");

  init_ncv7430();
  myTimer.begin(blinkLED, interval);

  FD.begin();
  CANFD_timings_t config;
  config.clock = CLK_24MHz;
  config.baudrate =   500000;     // 500kbps Nominal data rate
  config.baudrateFD = 2000000;    // 2000kpbs Data rate
  config.propdelay = 190;
  config.bus_length = 1;
  config.sample = 75;
  FD.setRegions(64);
  FD.setBaudRate(config);


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
