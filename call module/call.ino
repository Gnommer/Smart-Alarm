#include <SPI.h>
#include <Dhcp.h>
#include <Dns.h>
#include <Ethernet.h>
#include <EthernetClient.h>
#include <Temboo.h>
#include "TembooAccount.h" // Contains Temboo account information

byte ethernetMACAddress[] = ETHERNET_SHIELD_MAC;
EthernetClient client;

// The number of times to trigger the action if the condition is met
// We limit this so you won't use all of your Temboo calls while testing
int maxCalls = 10;

// The number of times this Choreo has been run so far in this sketch
int calls = 0;

int inputPin = 2;
int outputPin = 6;

void setup() {
  Serial.begin(9600);
  
  // For debugging, wait until the serial console is connected
  delay(4000);
  while(!Serial);

  Serial.print("DHCP:");
  if (Ethernet.begin(ethernetMACAddress) == 0) {
    Serial.println("FAIL");
    while(true);
  }
  Serial.println("OK");
  delay(5000);

  // Initialize pins
  pinMode(inputPin, INPUT);
  pinMode(outputPin, OUTPUT);

  Serial.println("Setup complete.\n");
}

void loop() {
  int sensorValue = digitalRead(inputPin);
  Serial.println("Sensor: " + String(sensorValue));

  if (sensorValue == HIGH) {
    if (calls < maxCalls) {
      Serial.println("\nTriggered! Calling CaptureTextToSpeechPrompt Choreo...");
      runCaptureTextToSpeechPrompt(sensorValue);
      calls++;
    } else {
      Serial.println("\nTriggered! Skipping to save Temboo calls. Adjust maxCalls as required.");
    }
  }
  delay(250);
}

void runCaptureTextToSpeechPrompt(int sensorValue) {
  TembooChoreo CaptureTextToSpeechPromptChoreo(client);

  // Set Temboo account credentials
  CaptureTextToSpeechPromptChoreo.setAccountName(TEMBOO_ACCOUNT);
  CaptureTextToSpeechPromptChoreo.setAppKeyName(TEMBOO_APP_KEY_NAME);
  CaptureTextToSpeechPromptChoreo.setAppKey(TEMBOO_APP_KEY);

  // Set profile to use for execution
  CaptureTextToSpeechPromptChoreo.setProfile("charan");
  // Identify the Choreo to run
  CaptureTextToSpeechPromptChoreo.setChoreo("/Library/Nexmo/Voice/CaptureTextToSpeechPrompt");

  // Run the Choreo
  unsigned int returnCode = CaptureTextToSpeechPromptChoreo.run();

  // A return code of zero means everything worked
  if (returnCode == 0) {
    while (CaptureTextToSpeechPromptChoreo.available()) {
      String name = CaptureTextToSpeechPromptChoreo.readStringUntil('\x1F');
      name.trim();

      if (name == "CallbackData") {
        if (CaptureTextToSpeechPromptChoreo.findUntil("1", "\x1E")) {
          digitalWrite(outputPin, HIGH);
          CaptureTextToSpeechPromptChoreo.find("\x1E");
        }
         if (CaptureTextToSpeechPromptChoreo.findUntil("2", "\x2E")) {
          digitalWrite(outputPin, LOW);
          CaptureTextToSpeechPromptChoreo.find("\x1E");
        }
      }
      else {
        CaptureTextToSpeechPromptChoreo.find("\x1E");
      }
    }
  }

  CaptureTextToSpeechPromptChoreo.close();
}
