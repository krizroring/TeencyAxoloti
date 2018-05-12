/*
 * AxoDisplay.cpp - Display driver for the Teency Axoloti controller
 * Copyright 2018 Christian Roring
 */
#include "Arduino.h"
#include <LiquidCrystalFast.h>
#include "AxoDisplay.h"

#define DISP_CURRENT  "* "
#define DISP_SELECTED "> "
#define DISP_NEXT     "* "
#define DISP_NONE     "  "
#define DISP_VALUE_CLEAR "   "
#define DISP_CONTROLLER_WIDTH 4

LiquidCrystalFast lcd(12, 11, 18, 17, 16, 15, 14);

AxoDisplay::AxoDisplay(void){
};

// Inits the display
void AxoDisplay::init() {
    lcd.begin(16, 2);
    lcd.clear();
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

// Displays the selected patch and the next (if available)
void AxoDisplay::displaySelectedPatch(byte _index) {
    lcd.clear();
    lcd.setCursor(0, 0);

    if(_index == currentPatch) {
        lcd.write(DISP_CURRENT);
    } else {
        lcd.write(DISP_SELECTED);
    }
    // First row
    displayPatchName(_index);

    if((_index + 1) < numPatches) {
        lcd.setCursor(0, 1);

        if(_index + 1 == currentPatch) {
            lcd.write(DISP_NEXT);
        } else {
            lcd.write(DISP_NONE);
        }

        displayPatchName(_index + 1);
    }
}
// Send the patch name in 14 bytes
void AxoDisplay::displayPatchName(byte _index) {
    for(byte i = 0; i < MAX_PATCH_NAME_LENGTH; i++) {
        lcd.write((*patches)[_index][i]);
    }
}

// Sets the pointes to the patch name table
void AxoDisplay::setControllers(char (*_controllerNames)[MAX_CONTROLLERS][MAX_CONTROLLER_NAME_LENGTH],
                byte (*_controllerValues)[MAX_CONTROLLERS], byte _numControllers) {
    controllerNames = _controllerNames;
    controllerValues = _controllerValues;
    numControllers = _numControllers;
}

// Displays the current bank of controllers
void AxoDisplay::displayControllerBank(byte _bank) {
    lcd.clear();

    startIndex = _bank * DISP_CONTROLLER_WIDTH;

    for (byte i = 0; i< DISP_CONTROLLER_WIDTH; i++) {
        if(i + startIndex < numControllers) {
            displayControllerName(i + startIndex);
            displayControllerValue(i + startIndex);
        }
    }
}

// Updates the controller value
void AxoDisplay::updateControllerValue(byte _index) {
    if(_index >= startIndex && _index < startIndex+ DISP_CONTROLLER_WIDTH) {
        lcd.setCursor((_index % DISP_CONTROLLER_WIDTH)  * DISP_CONTROLLER_WIDTH, 1);
        lcd.write(DISP_VALUE_CLEAR);

        displayControllerValue(_index);
    }
}

// Displays a controller name
void AxoDisplay::displayControllerName(byte _index) {
    lcd.setCursor((_index % DISP_CONTROLLER_WIDTH) * DISP_CONTROLLER_WIDTH, 0);
    for(byte i = 0; i < MAX_CONTROLLER_NAME_LENGTH; i++) {
        lcd.write((*controllerNames)[_index][i]);
    }
}

// Displays a controller name
void AxoDisplay::displayControllerValue(byte _index) {
    byte value = (*controllerValues)[_index];

    if(value < 10) {
        lcd.setCursor(((_index % DISP_CONTROLLER_WIDTH) * DISP_CONTROLLER_WIDTH) + 2, 1);
    } else if(value < 100) {
        lcd.setCursor(((_index % DISP_CONTROLLER_WIDTH) * DISP_CONTROLLER_WIDTH) + 1, 1);
    } else {
        lcd.setCursor((_index % DISP_CONTROLLER_WIDTH) * DISP_CONTROLLER_WIDTH, 1);
    }

    lcd.print(value, DEC);
}
