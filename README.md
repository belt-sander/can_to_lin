# CAN to LIN Module

## T4 based CAN to LIN module for control of LIN-based devices via CAN commands

### Description

This module allows a user to send a CAN message on the ID specified in the project by the `BASE_RX_CAN_ADDRESS` parameter, enabling control of the type and speed of the Bosch WDA LIN wiper. The module also publishes the received CAN state from the user on the CAN message ID specified by `BASE_TX_CAN_ADDRESS`.

NOTE: This all assumes that you use the wonderful product sold by SKPang, here:
https://www.skpang.co.uk/products/teensy-4-0-can-fd-and-lin-bus-breakout-board-include-teensy-4-0

### User CAN TX Function

This function will receive the following input:

- **Identifier:** `0x777` (Default `BASE_RX_CAN_ADDRESS`)
- **Byte 0:** `0x66` (Static Header)
- **Byte 1 Options:**
  - `0x01` - Intermittent Wipe
  - `0x02` - Slow Continuous Wipe
  - `0x03` - Fast Continuous Wipe
  - `0x04` - Single Wipe Event
- **Byte 2 Options:**
  - `0x01` - Intermittent Wipe Speed Control, Slow
  - `0x02` - Intermittent Wipe Speed Control, Medium Slow
  - `0x03` - Intermittent Wipe Speed Control, Medium Fast
  - `0x04` - Intermittent Wipe Speed Control, Fast

### User CAN RX Function

This function will transmit the following data:

- **Byte 0:** `0x66` (Static Header)
- **Byte 1:** Wipe State
- **Byte 2:** Intermittent Wipe Speed State

### Example 1

**Turn the wiper on full speed.** Send the following message:

| CAN ID | Byte 0 | Byte 1 | Byte 2 |
|--------|--------|--------|--------|
| `0x777` | `0x66` | `0x03` | `0x00` |

### Example 2

**Turn the wiper on intermittent wipe at medium slow speed.** Send the following message:

| CAN ID | Byte 0 | Byte 1 | Byte 2 |
|--------|--------|--------|--------|
| `0x777` | `0x66` | `0x01` | `0x02` |
