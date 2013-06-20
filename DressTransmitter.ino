/*
 Copyright (C) 2012 James Coliz, Jr. <maniacbug@ymail.com>
 
 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 version 2 as published by the Free Software Foundation.
 */

/**
 * Simplest possible example of using RF24Network
 *
 * TRANSMITTER NODE
 * Every 2 seconds, send a payload to the receiver node.
 */

#include <RF24Network.h>
#include <RF24.h>
#include <SPI.h>

// nRF24L01(+) radio attached using Getting Started board
RF24 radio(9,10);

RF24Network network(radio);// Network uses that radio
const uint16_t this_node = 1;// Address of our node
const uint16_t other_node = 0;// Address of the other node
const unsigned long interval = 1; //ms// How often to send 'hello world to the other unit

// When did we last send?
unsigned long last_sent;

// Structure of our payload
struct payload_t
{
    unsigned long ms;
    byte mode;
    byte eq[7];
} payload;

int animation = 0;

int analogPin = 0; // MSGEQ7 OUT
int strobePin = 4; // MSGEQ7 STROBE
int resetPin = 5; // MSGEQ7 RESET
int spectrumValue[7];
const int filterValue = 80;

void setup(void)
{
    Serial.begin(57600);
    
    SPI.begin();
    radio.setDataRate(RF24_250KBPS);
    radio.begin();
    radio.setDataRate(RF24_250KBPS);
    network.begin(/*channel*/ 90, /*node address*/ this_node);
    
    pinMode(5, OUTPUT);
    pinMode(6, OUTPUT);
    digitalWrite(6, HIGH);
    digitalWrite(5,LOW);
    
    pinMode(analogPin, INPUT);
    pinMode(strobePin, OUTPUT);
    pinMode(resetPin, OUTPUT);
    
    // Set analogPin's reference voltage
    analogReference(DEFAULT); // 5V
    
    // Set startup values for pins
    digitalWrite(resetPin, LOW);
    digitalWrite(strobePin, HIGH);
}

void checkEQ()
{
    // Set reset pin low to enable strobe
    digitalWrite(resetPin, HIGH);
    digitalWrite(resetPin, LOW);
    
    // Get all 7 spectrum values from the MSGEQ7
    for (int i = 0; i < 7; i++)
    {
        digitalWrite(strobePin, LOW);
        delayMicroseconds(30); // Allow output to settle
        
        spectrumValue[i] = analogRead(analogPin);
        spectrumValue[i] = constrain(spectrumValue[i], filterValue, 1023);        // Constrain any value above 1023 or below filterValue
        spectrumValue[i] = map(spectrumValue[i], filterValue, 1023, 0, 255);        // Remap the value to a number between 0 and 255
        payload.eq[i] = (byte)spectrumValue[i];
        
        digitalWrite(strobePin, HIGH);
        
    }    
}

int iMode = 0;
int getModeFromSerial() {
    // if there's any serial available, read it:
    while (Serial.available() > 0) {
        // look for the next valid integer in the incoming serial stream:
        iMode = Serial.parseInt();
        Serial.print("switching to:");Serial.println(iMode);
        // look for the newline. That's the end of your
        // sentence:
    }
    return iMode;
}

void loop(void)
{
    static unsigned long counter = 0;
    checkEQ();
    // Pump the network regularly
    network.update();
    
    // If it's time to send a message, send it!
    unsigned long now = millis();
    if ( now - last_sent >= interval  )
    {
        last_sent = now;
        
        //Serial.print("Sending...");
        payload.ms = counter++;
        payload.mode = getModeFromSerial();
        
        RF24NetworkHeader header(other_node);
        bool ok = network.write(header,&payload,sizeof(payload));
        
        if (ok)
        {
            Serial.print("ok: ");
            for(int i = 0;i<7;i++){
                Serial.print(payload.eq[i]);
                Serial.print(" ");
            }
            Serial.println();
        }
        else
            Serial.println("failed.");
        
    }
    
    /*
    if(Serial.available())
    {
        char c = toupper(Serial.read());
        switch(c)
        {
            case 'D':
                radio.printDetails();
                break;
        }
    }
    */
    
    
}
// vim:ai:cin:sts=2 sw=2 ft=cpp
