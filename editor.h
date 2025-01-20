#ifndef EDITOR_H 
#define EDITOR_H

void runProgram();

void stopProgram();

void enableRawMode(HANDLE hConsole);

void exitRawMode(HANDLE hConsole);

void clearScreen(HANDLE hConsole);

void initScreen(char** screenBuffer, COORD screenSize);

COORD getScreenSize();

void initTextBuffer(char** textBuffer, int textSize);

unsigned int getscreenWidth();

unsigned int getscreenHeight();

int* findUpdatedIndices(char* screenBuffer, int screenWidth, int screenHeight, char* textBuffer, int textBufferLen, int* numFound); 

void updateScreenBuffer(char* screenBuffer, char* textBuffer, int* foundIndices, int numFound);

void drawScreen(HANDLE hConsole, char* screenBuffer, int screenWidth, int screenHeight);

void handleInput(HANDLE hConsole);

void handleKeyEvent(KEY_EVENT_RECORD keyEventRec);

void resizeCursor(HANDLE hConsole, DWORD size);

#endif