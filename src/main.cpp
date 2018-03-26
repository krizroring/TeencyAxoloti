// Defines
#define HWSERIAL Serial2 // RX2 TX2
#define BUTTON_PIN 5
#define MAX_PATCHES 48 // Max num ber of patches
#define MAX_PATCH_NAME_LENGTH 14 // Max length of the patch names
#define SERIAL_INPUT_LENGTH 16 // Length of the serial input messages

// Serial protocol commands
#define AXO_GET_PATCHES 0x50
#define AXO_LOAD_PATCH 0x4C
#define AXO_SEP 0x2F
#define AXO_ESC 0x1B
#define AXO_ZERO 0x20

#include <Arduino.h>
#include "AxoDisplay.h"
#include <Wire.h>
#include <Rotary.h>

// Library's
Rotary r = Rotary(4,3);
AxoDisplay axoDisplay;
boolean isMenu = false;

//Button debounce
byte buttonState = 0;
byte debounce = 0;
elapsedMillis elapsedButtonMillis;

// Vars Serial
char incomingBytes[16]; // Buffer for the serial in
byte outgoingBytes[4]; // Buffer for the serial out

// Vars patchtable
byte patchCount = 0; // Number of active patches
byte currentPatch = 0; // Current loaded patch
byte selectedIndex = 0; // Current selected pach

char patchTable[MAX_PATCHES][MAX_PATCH_NAME_LENGTH] = {}; // Storage for the patch names

//Pointers to the action functions
// void (*buttonAction)(int);
void (*endTransmission)();

// Send a 2 byte command to the axoloti
void sendAxolotiRequest(byte request, byte val1, byte val2) {
    HWSERIAL.write(request);
    HWSERIAL.write(AXO_SEP);
    HWSERIAL.write(val1);
    HWSERIAL.write(val2);
}

// reads the serial buffer
void readSerialBuffer() {
    for (int i = 0; i < 16; i++) {
        incomingBytes[i] = HWSERIAL.read();
    }
}

// Gets the patch names from the Axoloti
void getPatchNames() {
    for ( int i = 0; i < MAX_PATCHES; i++ )
    {
        std::fill_n(patchTable[i], MAX_PATCH_NAME_LENGTH, 0x20);
    }

    patchCount = 0;
    sendAxolotiRequest(AXO_GET_PATCHES, AXO_ZERO, AXO_ZERO);
}

// Pushes a patch name to the patch table
void setPatchNames() {
    for (int i = 0; i < MAX_PATCH_NAME_LENGTH; i++)
    {
        patchTable[patchCount][i] = incomingBytes[i + 2];
    }
    patchCount++;
}

// Updates the display with the new patch names
void setPatchNamesToDisplay() {
    axoDisplay.setPatches(&patchTable, patchCount);
    axoDisplay.setCurrentPatch(currentPatch);
    axoDisplay.displaySelectedPatch(selectedIndex);

    isMenu = true;
}

// Loads a patch by index
void loadPatchByIndex(){
    if(selectedIndex != currentPatch) {
        sendAxolotiRequest(AXO_LOAD_PATCH, AXO_ZERO, selectedIndex);
    }
}

void patchLoaded() {
    currentPatch = incomingBytes[15];

    axoDisplay.setCurrentPatch(currentPatch);
    axoDisplay.displaySelectedPatch(currentPatch);

    isMenu = true;
}

void noop() {
};

// Init function
void setup(){
    // Debug
    Serial.begin(9600);

    // IO setup
    HWSERIAL.begin(31250);
    pinMode(BUTTON_PIN, INPUT);

    // Init's the LCD display
    axoDisplay.init();
}

// Main loop
void loop() {
    // listen for serial data
    if (HWSERIAL.available() > 15) {
        readSerialBuffer();

        if(incomingBytes[1] == AXO_SEP) {
            switch (incomingBytes[0]) {
            case AXO_GET_PATCHES: {
                endTransmission = &setPatchNamesToDisplay;
                setPatchNames();
                break;
            }
            case AXO_LOAD_PATCH: {
                endTransmission = &noop;
                patchLoaded();
                break;
            }
            case AXO_ESC: {
                // end of transmission
                (*endTransmission)();
                break;
            }
            }
        }
    }

    // Process the rotary encoder
    unsigned char result = r.process();

    if(result != DIR_NONE) {
        if(isMenu) {
            if (result == DIR_CW) {
                if(selectedIndex + 1 < patchCount) {
                    selectedIndex++;
                    axoDisplay.displaySelectedPatch(selectedIndex);
                }
            } else {
                if(selectedIndex - 1 >= 0) {
                    selectedIndex--;
                    axoDisplay.displaySelectedPatch(selectedIndex);
                }
            }
        } else {
            if (result == DIR_CW) {


            } else {


            }
        }

    }
    // TODO lock action until serial is complete
    // Button debounce
    if (elapsedButtonMillis > 50) {
        buttonState = digitalRead(BUTTON_PIN);

        if (buttonState == LOW && debounce == 0) {
            if(!isMenu) {
                getPatchNames();
            } else {
                loadPatchByIndex();
            }

            debounce = 1;
        }
        if (buttonState == HIGH && debounce == 1) {
            debounce = 0;
        }

        elapsedButtonMillis = 0;
    }
}
