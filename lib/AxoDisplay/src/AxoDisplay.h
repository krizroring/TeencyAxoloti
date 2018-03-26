/*
 * AxoDisplay.h - Display driver for the Teency Axoloti controller
 * Copyright 2018 Christian Roring
 */

#ifndef AXO_DISPLAY_H
#define AXO_DISPLAY_H

#define MAX_PATCHES 48 // Max num ber of patches
#define MAX_PATCH_NAME_LENGTH 14 // Max length of the patch names

#include "LiquidCrystalFast.h"
#include "Arduino.h"

class AxoDisplay
{
public:
    AxoDisplay(void);

    void init();
    void setPatches(char (*)[MAX_PATCHES][MAX_PATCH_NAME_LENGTH], byte);
    void setCurrentPatch(byte);
    void displaySelectedPatch(byte);

    void debug();

protected:
    int hexCharToInt(byte);
    void sendPatchName(byte);

    byte currentPatch;
    byte numPatches;
    char (*patches)[48][14];
};

#endif
