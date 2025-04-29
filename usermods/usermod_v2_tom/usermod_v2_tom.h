#pragma once

#include "wled.h"

// These flash-stored strings help reduce RAM usage.
static const char _dmxName[]    PROGMEM = "dmxOutput";
static const char _channelStr[] PROGMEM = "channel";
static const char _valueStr[]   PROGMEM = "value";

// DMX configuration constants
static const uint32_t DMX_BAUD     = 250000;  // DMX standard baud rate
static const uint8_t  DMX_CHANNELS = 5;     // Maximum DMX channels

// Define the RS485 DE/RE control pin (adjust to your wiring)
static const int RS485_DE_RE_PIN = 4;

class DMXOutputUsermod : public Usermod {
  private:
    // Data buffer storing DMX channel values (0–255 for each channel).
    uint8_t dmxData[DMX_CHANNELS];

    /**
     * sendDMXFrame() enables the RS485 transmitter, sends a DMX break and frame,
     * then disables the transmitter.
     *
     * Note: For a production system you may need more precise timing to generate
     * a DMX break and mark-after-break, but this example uses a simple delay.
     */
    void sendDMXFrame() {
      // Enable RS485 transmitter
      digitalWrite(RS485_DE_RE_PIN, HIGH);
      
      // Generate a DMX break: pull line LOW briefly.
      // A proper DMX break is typically ≥88 µs; adjust delayMicroseconds if needed.
      Serial2.flush();
      delayMicroseconds(100);
      
      // Send the DMX start code (0) followed by channel data.
      Serial2.write((uint8_t)0);  // DMX start code
      Serial2.write(dmxData, DMX_CHANNELS);
      Serial2.flush();
      
      // Disable transmitter to allow shared bus operation.
      digitalWrite(RS485_DE_RE_PIN, LOW);
    }

  public:
    // setup() is called once after boot. WiFi is not yet connected at this point.
    void setup() override {
      // Configure the RS485 control pin and start Serial2 at DMX baud (8N2).
      pinMode(RS485_DE_RE_PIN, OUTPUT);
      digitalWrite(RS485_DE_RE_PIN, LOW);
      Serial2.begin(DMX_BAUD, SERIAL_8N2);

      // Clear DMX data buffer (all channels off).
      memset(dmxData, 0, DMX_CHANNELS);
      DEBUG_PRINTLN(F("DMXOutputUsermod: setup complete"));
    }

    // loop() is called continuously.
    void loop() override {
      // Nothing needs to run continuously; DMX frames are sent only when JSON state is updated.
    }

    /**
     * readFromJsonState() is called when the JSON state is received from a client.
     *
     * Expected JSON format:
     * {
     *   "dmxOutput": {
     *     "channel": <number>,   // 1-indexed DMX channel to update
     *     "value": <number>      // value 0–255 for that channel
     *   }
     * }
     *
     * When the proper keys are found, update the internal DMX data and send a DMX frame.
     */
    void readFromJsonState(JsonObject& root) override {
      // Debug message to confirm this function is being called
      DEBUG_PRINTLN(F("DMXOutputUsermod: readFromJsonState called"));
      
      JsonObject dmxObj = root[FPSTR(_dmxName)];
      if (dmxObj.isNull()) {
        DEBUG_PRINTLN(F("DMXOutputUsermod: No DMX data in JSON"));
        return;  // No DMX-related data received.
      }
      
      DEBUG_PRINTLN(F("DMXOutputUsermod: Found DMX object in JSON"));
      
      if (dmxObj.containsKey(FPSTR(_channelStr)) && dmxObj.containsKey(FPSTR(_valueStr))) {
        int channel = dmxObj[FPSTR(_channelStr)].as<int>();
        int value   = dmxObj[FPSTR(_valueStr)].as<int>();
    
        DEBUG_PRINTF("DMXOutputUsermod: Updating channel %d to value %d\n", channel, value);
    
        // DMX channels are 1-indexed.
        if (channel < 1 || channel > DMX_CHANNELS) {
          DEBUG_PRINTLN(F("DMXOutputUsermod: Channel out of bounds"));
          return;
        }
    
        // Update the buffer (adjusted for zero-based index) and constrain value.
        dmxData[channel - 1] = constrain(value, 0, 255);
        sendDMXFrame();
        DEBUG_PRINTLN(F("DMXOutputUsermod: DMX update sent"));
      } else {
        DEBUG_PRINTLN(F("DMXOutputUsermod: Missing channel or value keys"));
      }
    }

    /**
     * addToJsonState() adds the current DMX information to the state output.
     * This example outputs the first channel's status as an example.
     * In a production system you might output more details.
     */
    void addToJsonState(JsonObject& root) override {
      JsonObject dmxObj = root.createNestedObject(FPSTR(_dmxName));
      // For demonstration, output the first channel.
      dmxObj[FPSTR(_channelStr)] = 1;
      dmxObj[FPSTR(_valueStr)] = dmxData[0];
    }

    // getId() must return a unique identifier for this usermod.
    uint16_t getId() override {
      return 65002;
    }
};

// Instantiate and register the usermod.
DMXOutputUsermod dmxOutputUsermod;
