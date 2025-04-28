#ifndef EDITOR_H 
#define EDITOR_H

typedef struct editorState {
    char* screenBuffer; // data written to screen
    char* textBuffer; // actual data 
    
    COORD screenSize; // width and height of screen buffer
    size_t textSize; // size of text buffer
    int textLength; // total number of characters in the text buffer

    COORD cursorPosition; // cursor position in the screen buffer
    COORD textPosition; // cursor position in the text buffer
    int* linePositions; // cursor position in each line
    
    int isRunning; // 1 if program is running, 0 if not
} editorState;

void runProgram();

void initEditorState(editorState* state);

void stopProgram(editorState* state);

void enableRawMode(HANDLE hConsole);

void exitRawMode(HANDLE hConsole);

void clearScreen(HANDLE hConsole);

void initScreen(HANDLE hConsole, editorState* state);

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

void* resizeBuffer(void* buffer, size_t* currentSize, size_t elementSize, editorState* state);

void insertCharacterToTextBuffer(editorState* state, char c);

void removeCharacterFromTextBuffer(editorState* state);

#endif