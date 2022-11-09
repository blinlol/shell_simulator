typedef struct Command_ {
    char* name;
    char** args;
    int operator;
    int input;       // file descriptor
    int output;
    int mode;
} Command;

typedef struct CommandList {
    Command* cmd;
    struct CommandList* nextcmd;    
} CommandList;

