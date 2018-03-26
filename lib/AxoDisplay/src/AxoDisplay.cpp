/*
 * AxoDisplay.cpp - Display driver for the Teency Axoloti controller
 * Copyright 2018 Christian Roring
 */
#include "Arduino.h"
#include <LiquidCrystalFast.h>
#include "AxoDisplay.h"

#define DISP_CURRENT  ">*"
#define DISP_SELECTED "> "
#define DISP_NEXT     " *"
#define DISP_NONE     "  "

LiquidCrystalFast lcd(12, 11, 18, 17, 16, 15, 14);

AxoDisplay::AxoDisplay(void){
};

// Inits the display
void AxoDisplay::init() {
    lcd.begin(16, 2);
    lcd.clear();
}

// Debug code
void AxoDisplay::debug() {
    lcd.setCursor(4, 0);
    lcd.write("Axoloti");
}

// Sets the pointes to the patch name table
void AxoDisplay::setPatches(char (*_patches)[MAX_PATCHES][MAX_PATCH_NAME_LENGTH], byte _numPatches) {
    patches = _patches;
    numPatches = _numPatches;
}

// Sets the current loaded patch index
void AxoDisplay::setCurrentPatch(byte _index) {
    currentPatch = _index;
}

// Displays the selected patch and the nex (if available)
void AxoDisplay::displaySelectedPatch(byte _index) {
    lcd.clear();
    lcd.setCursor(0, 0);

    if(_index == currentPatch) {
        lcd.write(DISP_CURRENT);
    } else {
        lcd.write(DISP_SELECTED);
    }
    // First row
    sendPatchName(_index);

    if((_index + 1) < numPatches) {
        lcd.setCursor(0, 1);

        if(_index + 1 == currentPatch) {
            lcd.write(DISP_NEXT);
        } else {
            lcd.write(DISP_NONE);
        }

        sendPatchName(_index + 1);
    }
}
// Send the patch name in 14 bytes
void AxoDisplay::sendPatchName(byte _index) {
    for(byte i = 0; i < MAX_PATCH_NAME_LENGTH; i++) {
        lcd.write((*patches)[_index][i]);
    }
}
