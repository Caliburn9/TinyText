#ifndef EDITOR_H 
#define EDITOR_H

typedef struct editorState {
    char* screenBuffer;
    char* textBuffer;
    
    COORD screenSize;
    int textSize;

    COORD cursorPosition;
    COORD textPosition;
    
    int isRunning;
} editorState;

void runProgram();

void stopProgram(editorState* state);

void enableRawMode(HANDLE hConsole);

void exitRawMode(HANDLE hConsole);

void clearScreen(HANDLE hConsole);

void initScreen(editorState* state);

COORD getScreenSize();

void initTextBuffer(editorState* state);

unsigned int getscreenWidth();

unsigned int getscreenHeight();

int* findUpdatedIndices(editorState* state, int* numFound); 

void updateScreenBuffer(editorState* state, int* foundIndices, int numFound);

void drawScreen(HANDLE hConsole, editorState* state);

void handleInput(HANDLE hConsole, editorState* state);

void handleKeyEvent(KEY_EVENT_RECORD keyEventRec, editorState* state) ;

void resizeCursor(HANDLE hConsole, DWORD size);

#endif