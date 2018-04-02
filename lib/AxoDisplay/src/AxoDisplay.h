/*
 * AxoDisplay.h - Display driver for the Teency Axoloti controller
 * Copyright 2018 Christian Roring
 */

#ifndef AXO_DISPLAY_H
#define AXO_DISPLAY_H

#define MAX_PATCHES 48 // Max number of patches
#define MAX_PATCH_NAME_LENGTH 14 // Max length of the patch names

#define MAX_CONTROLLERS 8 // Max number of controllers
#define MAX_CONTROLLER_NAME_LENGTH 4 // Max number of controllers

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

    void setControllers(char (*)[MAX_CONTROLLERS][MAX_CONTROLLER_NAME_LENGTH], byte (*)[MAX_CONTROLLERS], byte);
    void displayControllerBank(byte);
protected:
    int hexCharToInt(byte);
    void displayPatchName(byte);
    void displayControllerName(byte, byte);
    void displayControllerValue(byte, byte);

    byte currentPatch;
    byte numPatches;
    char (*patches)[MAX_PATCHES][MAX_PATCH_NAME_LENGTH];

    char (*controllerNames)[MAX_CONTROLLERS][MAX_CONTROLLER_NAME_LENGTH];
    byte (*controllerValues)[MAX_CONTROLLERS];
    byte numControllers;
};

#endif
