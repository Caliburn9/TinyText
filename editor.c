#include <windows.h>
#include <stdio.h>

#include "editor.h"

void runProgram() {
    editorState e;
    HANDLE hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);
    HANDLE hStdIn = GetStdHandle(STD_INPUT_HANDLE);
    int* foundIndices, numFound;

    enableRawMode(hStdIn);
    initEditorState(&e);
    resizeCursor(hStdOut, 100);
    initScreen(hStdOut, &e);
    initTextBuffer(&e);

    // Editor loop
    while (e.isRunning) {
        handleInput(hStdIn, &e);
        foundIndices = findUpdatedIndices(&e, &numFound);
        if (foundIndices == NULL) {
            fprintf(stderr, "Failed to allocate memory for found indices buffer in running loop\n");
            stopProgram(&e);
        }
        if (numFound != 0) {
            updateScreenBuffer(&e, foundIndices, numFound);
        }
        drawScreen(hStdOut, &e);
    }

    free(foundIndices);
    free(e.screenBuffer);
    free(e.textBuffer);
    exit(0);
}

void initEditorState(editorState* state) {
    state->screenBuffer = NULL;
    state->textBuffer = NULL;

    state->screenSize = getScreenSize();
    state->textSize = 128; // temp
    state->textLength = 0;

    state->cursorPosition.X = 0;
    state->cursorPosition.Y = 0;

    state->textPosition.X = 0;
    state->textPosition.Y = 0;

    state->linePositions = (int*)malloc(state->screenSize.Y * sizeof(int));
    if (state->linePositions == NULL) {
        fprintf(stderr, "Failed to allocate memory for buffer\n");
        stopProgram(state);
    } else {
        memset(state->linePositions, 0, state->screenSize.Y * sizeof(int));
    }

    state->isRunning = 1;
}

void stopProgram(editorState* state) {
    HANDLE hStdIn = GetStdHandle(STD_INPUT_HANDLE);
    // Exit raw mode
    exitRawMode(hStdIn);
    state->isRunning = 0;
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

void initScreen(HANDLE hConsole, editorState* state) {
    clearScreen(hConsole);

    state->screenBuffer = (char*)malloc(state->screenSize.X * state->screenSize.Y * sizeof(char));
    if (state->screenBuffer == NULL) {
        fprintf(stderr, "Failed to allocate memory for screen buffer\n");
        stopProgram(state);
    }

    for (int i = 0; i < state->screenSize.X * state->screenSize.Y; i++) {
        (state->screenBuffer)[i] = '\0';
    }

    SetConsoleCursorPosition(hConsole, state->cursorPosition);
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

void initTextBuffer(editorState* state) {
    state->textBuffer = (char*)malloc(state->textSize * sizeof(char));
    if (state->textBuffer == NULL) {
        fprintf(stderr, "Failed to allocate memory for text buffer\n");
        stopProgram(state);
    }

    for (int i = 0; i < state->textSize; i++) {
        (state->textBuffer)[i] = '\0';
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

int* findUpdatedIndices(editorState* state, int* numFound) {
    int screenBufferLen = state->screenSize.X * state->screenSize.Y; 
    size_t currentCapacity = 128;
    *numFound = 0;
    int limit = (screenBufferLen < state->textSize) ? screenBufferLen : state->textSize;
    int* found = malloc(currentCapacity * sizeof(int));
    if (found == NULL) {
        fprintf(stderr, "Failed to allocate memory for found indices buffer\n");
        stopProgram(state);
    }

    for (int i = 0; i < limit; i++) {
        if (state->screenBuffer[i] != state->textBuffer[i]) {
            if ((*numFound) >= currentCapacity) {
                found = (int*)resizeBuffer(found, &currentCapacity, sizeof(int), state);
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
void updateScreenBuffer(editorState* state, int* foundIndices, int numFound) {
    for (int i = 0; i < numFound; i++) {
        state->screenBuffer[foundIndices[i]] = state->textBuffer[foundIndices[i]];
    } 
}

void drawScreen(HANDLE hConsole, editorState* state) {
    int screenBufferLen = state->screenSize.X * state->screenSize.Y;

    clearScreen(hConsole);

    COORD cursorPos = { 0, 0 };
    SetConsoleCursorPosition(hConsole, cursorPos);

    DWORD charsWritten;
    WriteConsoleOutputCharacter(hConsole, state->screenBuffer, screenBufferLen, cursorPos, &charsWritten);

    SetConsoleCursorPosition(hConsole, state->cursorPosition);
}

void handleInput(HANDLE hConsole, editorState* state) {
    DWORD cNumRead, i;
    INPUT_RECORD irInBuf[128];

    if (!ReadConsoleInput(hConsole, irInBuf, 128, &cNumRead)) {
        fprintf(stderr, "ReadConsoleInput Error\n");
        stopProgram(state);
    }

    for (i = 0; i < cNumRead; i++) {
        switch(irInBuf[i].EventType) {
            case KEY_EVENT:
                handleKeyEvent(irInBuf[i].Event.KeyEvent, state);
                break;

            default:
                fprintf(stderr, "Unknown event type");
        }
    }
}

void handleKeyEvent(KEY_EVENT_RECORD keyEventRec, editorState* state) {
    if (keyEventRec.bKeyDown) {
         // Handle Control Key chords
        if (keyEventRec.dwControlKeyState & (LEFT_CTRL_PRESSED | RIGHT_CTRL_PRESSED)) {
            // Quit
            if (keyEventRec.wVirtualKeyCode == 'W' || keyEventRec.wVirtualKeyCode == 'w') {
                stopProgram(state);
            }
            // Save file

            // Load file

            // Copy selection

            // Paste selection

            // Undo

            // Redo
            
        }
        // Handle Shift Key Chords
        else if (keyEventRec.dwControlKeyState == SHIFT_PRESSED) {
            // Capitalize lowercase characters

            // Highlight selection using arrow keys
            
        }
        // Handle character ASCII keys
        else if (keyEventRec.uChar.AsciiChar > 32 && keyEventRec.uChar.AsciiChar <= 126) {
            char keyChar = keyEventRec.uChar.AsciiChar;
            insertCharacterToTextBuffer(state, keyChar);
        }
        // Handle special keys
        else {
            switch (keyEventRec.wVirtualKeyCode)
            {
            case VK_BACK:
                removeCharacterFromTextBuffer(state);
                break;
            
            case VK_RETURN:
                insertCharacterToTextBuffer(state, '\n');
                break;

            case VK_ESCAPE:
                /* code */
                break;

            case VK_SPACE:
                insertCharacterToTextBuffer(state, ' ');
                break;

            case VK_TAB:
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

void* resizeBuffer(void* buffer, size_t* currentSize, size_t elementSize, editorState* state) {
    *currentSize *= 2;
    size_t oldSize = *currentSize / 2;
    void *temp = realloc(buffer, *currentSize * elementSize);
    if (temp == NULL) {
        fprintf(stderr, "Failed to re-allocate memory for buffer\n");
        stopProgram(state);
    }
    // Zero out new memory
    memset(temp + (oldSize * elementSize), 0, (oldSize * elementSize));
    return temp;
}

void insertCharacterToTextBuffer(editorState* state, char c) {
    int lineWidth = state->screenSize.X;
    int index = state->textPosition.Y * lineWidth + state->textPosition.X;

    if (state->textLength + 1 >= state->textSize || index + 1 >= state->textSize) {
        state->textBuffer = (char*)resizeBuffer(state->textBuffer, &state->textSize, sizeof(char), state);
    }

    if (c == '\n') {
        state->textLength++;
        state->linePositions[state->textPosition.Y] = state->textPosition.X;
        state->textPosition.X = 0;
        state->textPosition.Y++;
    } else {
        for (int i = state->textLength; i > index; i--) {
            state->textBuffer[i] = state->textBuffer[i - 1];
        }

        state->textBuffer[index] = c;
        state->textLength++;
        state->textPosition.X++;
    
        if (state->textPosition.X >= lineWidth) {
            state->linePositions[state->textPosition.Y] = state->textPosition.X;
            state->textPosition.X = 0;
            state->textPosition.Y++;
        }
    }
    
    state->cursorPosition = state->textPosition;
}

void removeCharacterFromTextBuffer(editorState* state) {
    if (state->textLength == 0 || (state->cursorPosition.X == 0 && state->cursorPosition.Y == 0)) {
        return;
    }

    int lineWidth = state->screenSize.X;
    int index = state->textPosition.Y * lineWidth + state->textPosition.X;
    int totalLength = getTotalTextBufferLength(state);

    for (int i = index; i < totalLength; i++) {
        state->textBuffer[i] = state->textBuffer[i + 1];
    }

    state->textLength--;
    state->textBuffer[state->textLength] = '\0';
    
    if (state->textPosition.X > 0) {
        state->textPosition.X--;
    } else {
        state->textPosition.Y--;
        state->textPosition.X = state->linePositions[state->textPosition.Y];
    }

    state->cursorPosition = state->textPosition;
}

int getTotalTextBufferLength(editorState* state) {
    int length = 0;
    int lineWidth = state->screenSize.X;

    // Traverse textBuffer up to the cursor's position
    for (int y = 0; y < state->textPosition.Y; y++) {
        // Count full lines
        length += lineWidth;
    }

    // Add characters from the current line (up to cursor X position)
    length += state->textPosition.X;

    return length;
}