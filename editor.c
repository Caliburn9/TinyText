#include <windows.h>
#include <stdio.h>

#include "editor.h"

void runProgram() {
    HANDLE hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);
    HANDLE hStdIn = GetStdHandle(STD_INPUT_HANDLE);

    char* screenBuffer = NULL;
    char* textBuffer = NULL;
    COORD screenSize = getScreenSize();
    int textSize = 128; // temp

    int running = 1;

    clearScreen(hStdOut);

    initScreen(&screenBuffer, screenSize);

    initTextBuffer(&textBuffer, textSize);

    // Enable raw mode
    enableRawMode(hStdIn);

    // debug for testing output
    for (int i = 0; i < textSize; i++) {
        textBuffer[i] = 'a';
    }

    // Editor loop
    while(running) {
        writeTextToScreen(screenBuffer, screenSize.X, screenSize.Y, textBuffer, textSize);

        drawScreen(screenBuffer, screenSize.X, screenSize.Y);
    }

    free(screenBuffer);
    free(textBuffer);
}

void exitProgram() {
    HANDLE hStdIn = GetStdHandle(STD_INPUT_HANDLE);

    // Exit raw mode
    exitRawMode(hStdIn);
}

void enableRawMode(HANDLE hConsole) {
    DWORD dwMode;

    GetConsoleMode(hConsole, &dwMode);

    dwMode &= ~(ENABLE_LINE_INPUT | ENABLE_ECHO_INPUT | ENABLE_PROCESSED_INPUT);

    SetConsoleMode(hConsole, dwMode);
}

void exitRawMode(HANDLE hConsole) {
    DWORD dwMode;

    GetConsoleMode(hConsole, &dwMode);

    dwMode |= (ENABLE_LINE_INPUT | ENABLE_ECHO_INPUT | ENABLE_PROCESSED_INPUT);

    SetConsoleMode(hConsole, dwMode);
}

void clearScreen(HANDLE hConsole) {
    COORD coordScreen = { 0, 0 };    // Home for the cursor
    DWORD cCharsWritten;
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    DWORD dwConSize;

    // Get the number of character cells in the current buffer.
    if (!GetConsoleScreenBufferInfo(hConsole, &csbi))
    {
        return;
    }

    dwConSize = csbi.dwSize.X * csbi.dwSize.Y;

    // Fill the entire screen with blanks.
    if (!FillConsoleOutputCharacter(hConsole,        // Handle to console screen buffer
                                    (TCHAR)' ',      // Character to write to the buffer
                                    dwConSize,       // Number of cells to write
                                    coordScreen,     // Coordinates of first cell
                                    &cCharsWritten)) // Receive number of characters written
    {
        return;
    }

    // Set the buffer's attributes accordingly.
    if (!FillConsoleOutputAttribute(hConsole,         // Handle to console screen buffer
                                    csbi.wAttributes, // Character attributes to use
                                    dwConSize,        // Number of cells to set attribute
                                    coordScreen,      // Coordinates of first cell
                                    &cCharsWritten))  // Receive number of characters written
    {
        return;
    }

    // Put the cursor at its home coordinates.
    SetConsoleCursorPosition(hConsole, coordScreen);
}

void initScreen(char** screenBuffer, COORD screenSize) {
    *screenBuffer = (char*)malloc(screenSize.X * screenSize.Y * sizeof(char));
    if (*screenBuffer == NULL) {
        fprintf(stderr, "Failed to allocate memory for screen buffer\n");
        exit(1); // Handle allocation failure
    }

    // Debug Output
    //printf("Screen initialized successfully");

    //free(screenBuffer); // temp
}

COORD getScreenSize() {
    unsigned int screenWidth = getscreenWidth();
    unsigned int screenHeight = getscreenHeight();

    // Edge case handling
    if (screenWidth == 0 || screenHeight == 0) {
        screenWidth = 80; // Default width
        screenHeight = 24; // Default height
    }

    COORD screenSize = { screenWidth, screenHeight };

    return screenSize;
}

void initTextBuffer(char** textBuffer, int textSize) {
    *textBuffer = (char*)malloc(textSize * sizeof(char));
    if (*textBuffer == NULL) {
        fprintf(stderr, "Failed to allocate memory for text buffer\n");
        exit(1); // Handle allocation failure
    }
}

unsigned int getscreenWidth() {
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    if (GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi)) {
        return (unsigned int)csbi.srWindow.Right - csbi.srWindow.Left + 1;
    }
    return 0;
}

unsigned int getscreenHeight() {
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    if (GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi)) {
        return (unsigned int)csbi.srWindow.Bottom - csbi.srWindow.Top + 1;
    }
    return 0;
}

/*
    issues:
    doesnt account for text wrapping
*/
void writeTextToScreen(char* screenBuffer, int screenWidth, int screenHeight, char* textBuffer, int textBufferLen) {
    int screenBufferLen = screenWidth * screenHeight;

    for (int i = 0; i < screenBufferLen; i++) {
        if (i < textBufferLen) {
            screenBuffer[i] = textBuffer[i];
        } else {
            screenBuffer[i] = ' ';
        }
    } 
}

/*
    issues: 
    redraws ENTIRE screen, focus on refreshing only updated parts
    suffers from output flickering
    no cursor handling
*/
void drawScreen(char* screenBuffer, int screenWidth, int screenHeight) {
    int screenBufferLen = screenWidth * screenHeight;

    for (int i = 0; i < screenBufferLen; i++) {
        printf("%c", screenBuffer[i]);
        if ((i + 1) % screenWidth == 0) {
            printf("\n");
        }
    }
}