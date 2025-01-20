#include <windows.h>
#include <stdio.h>

#include "editor.h"

int running = 1;

void runProgram() {
    HANDLE hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);
    HANDLE hStdIn = GetStdHandle(STD_INPUT_HANDLE);

    char* screenBuffer = NULL;
    char* textBuffer = NULL;
    COORD screenSize = getScreenSize();
    int textSize = 128; // temp

    clearScreen(hStdOut);
    resizeCursor(hStdOut, 100);

    initScreen(&screenBuffer, screenSize);
    initTextBuffer(&textBuffer, textSize);

    // Enable raw mode
    enableRawMode(hStdIn);

    // debug for testing output
    for (int i = 0; i < 5; i++) {
        textBuffer[i] = 'a';
    }

    int* foundIndices, numFound;

    // Editor loop
    while (running) {
        handleInput(hStdIn);
        foundIndices = findUpdatedIndices(screenBuffer, screenSize.X, screenSize.Y, textBuffer, textSize, &numFound);
        if (foundIndices == NULL) {
            fprintf(stderr, "Failed to allocate memory for found indices buffer in running loop\n");
            exit(1);
        }
        if (numFound != 0) {
            updateScreenBuffer(screenBuffer, textBuffer, foundIndices, numFound);
        }
        drawScreen(hStdOut, screenBuffer, screenSize.X, screenSize.Y);
    }

    free(foundIndices);
    free(screenBuffer);
    free(textBuffer);
    exit(0);
}

void stopProgram() {
    HANDLE hStdIn = GetStdHandle(STD_INPUT_HANDLE);
    // Exit raw mode
    exitRawMode(hStdIn);
    running = 0;
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

    for (int i = 0; i < screenSize.X * screenSize.Y; i++) {
        (*screenBuffer)[i] = ' ';
    }
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

    for (int i = 0; i < textSize; i++) {
        (*textBuffer)[i] = '\0';
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

int* findUpdatedIndices(char* screenBuffer, int screenWidth, int screenHeight, char* textBuffer, int textBufferLen, int* numFound) {
    int screenBufferLen = screenWidth * screenHeight; 
    int currentCapacity = 128;
    int* temp;
    *numFound = 0;
    int limit = (screenBufferLen < textBufferLen) ? screenBufferLen : textBufferLen;
    int* found = malloc(currentCapacity * sizeof(int));
    if (found == NULL) {
        fprintf(stderr, "Failed to allocate memory for found indices buffer\n");
        exit(1);
    }

    for (int i = 0; i < limit; i++) {
        if (screenBuffer[i] != textBuffer[i]) {
            if ((*numFound) >= currentCapacity) {
                currentCapacity *= 2;
                temp = realloc(found, currentCapacity * sizeof(int));
                if (temp == NULL) {
                    free(found);
                    fprintf(stderr, "Failed to re-allocate memory for found indices buffer\n");
                    exit(1);
                }
                found = temp;   
            }
            found[(*numFound)] = i;
            (*numFound)++;
        }
    }  

    return found;
}

/*
    issues:
    doesnt account for text wrapping
*/
void updateScreenBuffer(char* screenBuffer, char* textBuffer, int* foundIndices, int numFound) {
    for (int i = 0; i < numFound; i++) {
        screenBuffer[foundIndices[i]] = textBuffer[foundIndices[i]];
    } 
}

/*
    issues: 
    no cursor handling 
*/
void drawScreen(HANDLE hConsole, char* screenBuffer, int screenWidth, int screenHeight) {
    int screenBufferLen = screenWidth * screenHeight;

    clearScreen(hConsole);

    COORD cursorPos = { 0, 0 };
    SetConsoleCursorPosition(hConsole, cursorPos);

    DWORD charsWritten;
    WriteConsoleOutputCharacter(hConsole, screenBuffer, screenBufferLen, cursorPos, &charsWritten);
}

void handleInput(HANDLE hConsole) {
    DWORD cNumRead, i;
    INPUT_RECORD irInBuf[128];

    if (!ReadConsoleInput(hConsole, irInBuf, 128, &cNumRead)) {
        fprintf(stderr, "ReadConsoleInput Error\n");
        exit(1);
    }

    for (i = 0; i < cNumRead; i++) {
        switch(irInBuf[i].EventType) {
            case KEY_EVENT:
                handleKeyEvent(irInBuf[i].Event.KeyEvent);
                break;

            default:
                fprintf(stderr, "Unknown event type");
        }
    }
}

/*
    todo:
    inserting keys to textbuffer 
    handle special characters 
*/
void handleKeyEvent(KEY_EVENT_RECORD keyEventRec) {
    // Handle Control Key chords
    if (keyEventRec.dwControlKeyState & (LEFT_CTRL_PRESSED | RIGHT_CTRL_PRESSED)) {
        // Quit
        if (keyEventRec.wVirtualKeyCode == 'Q' || keyEventRec.wVirtualKeyCode == 'q') {
            stopProgram();
        }
        // Save file

        // Load file

        // Copy selection

        // Paste selection

        // Undo

        // Redo
        
    }
    // Handle character ASCII keys
    else if (keyEventRec.uChar.AsciiChar > 32 && keyEventRec.uChar.AsciiChar <= 126) {
        char keyChar = keyEventRec.uChar.AsciiChar;
    }
    // Handle special keys
    else {
        switch (keyEventRec.wVirtualKeyCode)
        {
        case VK_BACK:
            /* code */
            break;
        
        case VK_RETURN:
            /* code */
            break;

        case VK_CAPITAL:
            /* code */
            break;

        case VK_ESCAPE:
            /* code */
            break;

        case VK_SPACE:
            /* code */
            break;

        case VK_LEFT:
            /* code */
            break;
        
        case VK_UP:
            /* code */
            break;

        case VK_DOWN:
            /* code */
            break;

        case VK_RIGHT:
            /* code */
            break;

        default:
            fprintf(stderr, "Unknown Virtual Key Code");
            break;
        }
    }
}

void resizeCursor(HANDLE hConsole, DWORD size) {
    CONSOLE_CURSOR_INFO ci;
    if (GetConsoleCursorInfo(hConsole, &ci)) {
        ci.bVisible = TRUE;
        ci.dwSize = size;
        SetConsoleCursorInfo(hConsole, &ci);
    } else {
        fprintf(stderr, "Failed to retrieve cursor info. Unable to resize cursor to: %d", size);
    }
}

// void insertCharacterToTextBuffer() {

// }