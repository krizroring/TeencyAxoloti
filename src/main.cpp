// Defines
#define HWSERIAL Serial2 // RX2 TX2
#define BUTTON_PIN 5
#define SERIAL_INPUT_LENGTH 16 // Length of the serial input messages

// Defined in AxoDisplay
#ifndef AXO_DISPLAY_H
#define MAX_PATCHES 48 // Max num ber of patches
#define MAX_PATCH_NAME_LENGTH 14 // Max length of the patch names
#define MAX_CONTROLLERS 8 // Maximum number of controllers
#define MAX_CONTROLLER_NAME_LENGTH 4 //Maximum length of a controller name (inc space at the end)
#endif

// Serial protocol commands
#define AXO_GET_PATCHES 0x50
#define AXO_LOAD_PATCH 0x4C
#define AXO_GET_INDEX 0x49
#define AXO_SET_CONTROLLER 0x43
#define AXO_CHANGE_CONTROLLER 0x63
#define AXO_SET_BANK 0x42
#define AXO_SEP 0x2F
#define AXO_ESC 0x1B
#define AXO_ZERO 0x20

#define STATE_PATCHES 0x00
#define STATE_CONTROLLER 0x01
#define STATE_SAVE 0x02
#define STATE_BACK 0x03

#include <Arduino.h>
#include "AxoDisplay.h"
#include <Wire.h>
#include <Rotary.h>

// Library's
Rotary r = Rotary(4,3);
AxoDisplay axoDisplay;

byte menuState = STATE_PATCHES;
// 0x00 = patches
// 0x01 = patch controllers
// 0x02 = patch save
// 0x03 = pach back

//Button debounce
byte buttonState = 0;
byte debounce = 0;
elapsedMillis elapsedButtonMillis;

// Vars Serial
char incomingBytes[16]; // Buffer for the serial in
byte outgoingBytes[4]; // Buffer for the serial out

// Vars patchtable
boolean hasPatches = false;
byte patchCount = 0; // Number of active patches
byte currentPatch = 0; // Current loaded patch
byte selectedIndex = 0; // Current selected pach

char patchTable[MAX_PATCHES][MAX_PATCH_NAME_LENGTH] = {}; // Storage for the patch names

char controllerNames[MAX_CONTROLLERS][MAX_CONTROLLER_NAME_LENGTH] = {}; // Array for the controller names
byte controllerValues[MAX_CONTROLLERS] = {}; // array for the controller value's

byte controllerCount = 0;
byte currentControllerBank = 0;

//Pointers to the action functions
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

// Gets the current patch index
void getPatchIndex() {
    currentPatch = 0;

    sendAxolotiRequest(AXO_GET_INDEX, AXO_ZERO, AXO_ZERO);
}

// Gets the controllers for the current patch
void getControllers() {
    for ( int i = 0; i < MAX_CONTROLLERS; i++ )
    {
        std::fill_n(controllerNames[i], MAX_CONTROLLER_NAME_LENGTH, 0x20);
    }

    std::fill_n(controllerValues, MAX_CONTROLLERS, 0);

    controllerCount = 0;
    currentControllerBank = 0;

    sendAxolotiRequest(AXO_SET_CONTROLLER, AXO_ZERO, AXO_ZERO);
}

// SETTERS

// Pushes a patch name to the patch table
void setPatchNames() {
    for (int i = 0; i < MAX_PATCH_NAME_LENGTH; i++)
    {
        patchTable[patchCount][i] = incomingBytes[i + 2];
    }
    patchCount++;
}

// Sets the current patch index
void setPatchIndex() {
    if(incomingBytes[15] <= patchCount) {
        currentPatch = incomingBytes[15];
    }

    selectedIndex = currentPatch;

    axoDisplay.setCurrentPatch(currentPatch);
    axoDisplay.displaySelectedPatch(selectedIndex);

    menuState = STATE_PATCHES;
}

// Saves the controller names anvalues to the array's
void setControllers() {
    for ( int i = 0; i < MAX_CONTROLLER_NAME_LENGTH; i++ ) {
        controllerNames[controllerCount][i] = incomingBytes[i + 2];
    }

    controllerValues[controllerCount] = incomingBytes[15]; // last byte
    controllerCount++;
}

// Change the value of 1 controller
void controllerChanged() {
    controllerValues[(int)incomingBytes[14]] = incomingBytes[15]; // last byte

    if ( menuState == STATE_CONTROLLER ) {
        axoDisplay.updateControllerValue(incomingBytes[14]);
    }
}

// Set the patch names to the display
void patchNamesSet() {
    axoDisplay.setPatches(&patchTable, patchCount);
    hasPatches = true;

    getPatchIndex();
}

// Shows the controllers to the display
void controllersSet() {
    axoDisplay.setControllers(&controllerNames, &controllerValues, controllerCount);
    axoDisplay.displayControllerBank(currentControllerBank);
    menuState = STATE_CONTROLLER;
}

// Loads a patch by index
void loadPatchByIndex(){
    if(selectedIndex != currentPatch) {
        sendAxolotiRequest(AXO_LOAD_PATCH, AXO_ZERO, selectedIndex);
    }
}

// Callback from the current loaded patch index
void patchLoaded() {
    if (hasPatches == false) {
        getPatchNames();
    } else {
        getPatchIndex();
    }
}

// send the current bank index to the Axoloti
void sendCurrentBank() {
    sendAxolotiRequest(AXO_SET_BANK, AXO_ZERO, currentControllerBank);
    axoDisplay.displayControllerBank(currentControllerBank);
};

// Noop Noop
void noop() {
};

// Used for controlling the patch menu with the rotary
void rotaryMenu(int result) {
    if (result == DIR_CW) {
        if(selectedIndex + 1 < patchCount) {
            selectedIndex++;
        }
    } else {
        if(selectedIndex - 1 >= 0) {
            selectedIndex--;
        }
    }

    axoDisplay.displaySelectedPatch(selectedIndex);
}

// Used for controlling the patch controls with the rotary
void rotaryController(int result) {
    if (result == DIR_CW) {
        if((currentControllerBank + 1) * 4 < controllerCount ) {
            currentControllerBank++;
        } else {

        }
    } else {
        if(currentControllerBank -1 >= 0 ) {
            currentControllerBank--;
        }
    }

    sendCurrentBank();
}

// Init function
void setup(){
    // Debug
    Serial.begin(9600);

    // IO setup
    HWSERIAL.begin(31250);
    pinMode(BUTTON_PIN, INPUT);

    // Init's the LCD display
    axoDisplay.init();

    sendAxolotiRequest(AXO_LOAD_PATCH, AXO_ZERO, selectedIndex);
}

// Main loop
void loop() {
    // listen for serial data
    if (HWSERIAL.available() > 15) {
        readSerialBuffer();

        if(incomingBytes[1] == AXO_SEP) {
            switch (incomingBytes[0]) {
                case AXO_LOAD_PATCH:
                    endTransmission = &patchLoaded;
                    break;
                case AXO_GET_PATCHES:
                    endTransmission = &patchNamesSet;
                    setPatchNames();
                    break;
                case AXO_GET_INDEX:
                    endTransmission = &getControllers;
                    setPatchIndex();
                    break;
                case AXO_SET_CONTROLLER:
                    endTransmission = &controllersSet;
                    setControllers();
                    break;
                case AXO_CHANGE_CONTROLLER:
                    endTransmission = &noop;
                    controllerChanged();
                    break;
                case AXO_ESC:
                    (*endTransmission)();
                    break;
                default:
                    // safety
                    endTransmission = &noop;
                    break;
            }
        }
    }

    // Process the rotary encoder
    unsigned char result = r.process();

    if(result != DIR_NONE) {
        switch (menuState) {
            case STATE_PATCHES:
                rotaryMenu(result);
                break;
            case STATE_CONTROLLER:
                rotaryController(result);
                break;
            default:
                break;
        }
    }

    // TODO lock action until serial is complete
    // Button debounce
    if (elapsedButtonMillis > 50) {
        buttonState = digitalRead(BUTTON_PIN);

        if (buttonState == LOW && debounce == 0) {
            switch (menuState) {
                case STATE_PATCHES:
                    loadPatchByIndex();
                    break;
                case STATE_CONTROLLER:
                    break;
                default:
                    break;
                }

            debounce = 1;
        }
        if (buttonState == HIGH && debounce == 1) {
            debounce = 0;
        }

        elapsedButtonMillis = 0;
    }
}
