#ifndef EDITOR_H 
#define EDITOR_H

void runProgram();

void exitProgram();

void enableRawMode(HANDLE hConsole);

void exitRawMode(HANDLE hConsole);

void clearScreen(HANDLE hConsole);

void initScreen(char** screenBuffer, COORD screenSize);

COORD getScreenSize();

void initTextBuffer(char** textBuffer, int textSize);

unsigned int getscreenWidth();

unsigned int getscreenHeight();

void writeTextToScreen(char* screenBuffer, int screenWidth, int screenHeight, char* textBuffer, int textBufferLen);

void drawScreen(char* screenBuffer, int screenWidth, int screenHeight);

void handleInput(HANDLE hConsole);

void handleKeyEvent(KEY_EVENT_RECORD keyEventRec);

#endif