#include "directives.c"
#include "types.c"
#include "command_list.c"


int EndOfInput=0, EndOfShell=0;



void sig_handler(int signum){
    kill(getpid(), signum);
}

char* scanstring(){
    /* scan from stdin to dest string */
    int len=0, block=10, blocksize=10;
    char c, *dest;

    dest=(char*) calloc(block, sizeof(char));
    c=getchar();
    while (c!=EOF && c!='\n' && c!='\0'){
        if (len>=block-1){
            block+=blocksize;
            dest=(char*) realloc(dest, block*sizeof(char));
        }
        dest[len]=c;
        c=getchar();
        len++;
    }
    if (c==EOF){
        EndOfInput=1;
    }
    dest[len]='\0';
    return dest;
}

int typeofword(char* word, int len){
    char *tempstr=malloc(MAXLEN*sizeof(char));
    int i, ans;
    for (i=0;word[i]!='\0' && i<len;i++){
        tempstr[i]=word[i];
    }
    tempstr[i]='\0';
    if (strcmp(tempstr, "|")==0) { ans=CONVEYOR; }
    else if (strcmp(tempstr, "||")==0) { ans=OR; }
    else if (strcmp(tempstr, "&&")==0) { ans=AND; }
    else if (strcmp(tempstr, "&")==0) { ans=AMPERSAND; }
    else { ans=WORD; }
    free(tempstr);
    return ans;
}

Command* simplecommandparser(char* str, int len){
    /* parsing str to command name, args and return cmd */
    if (str==NULL || len==0){
        perror("str is empty");
        return NULL;
    }
    while (*str==' ' || *str=='\t'){
        str++;
        len--;
    }
    
    char *name=calloc(MAXLEN, sizeof(char));
    int i, j;
    for (i=0;str[i]!=' ' && i<len;i++){
        if (i>MAXLEN){
            perror("too big command name");
            return NULL;
        }
        name[i]=str[i];
    }
    name[i]='\0';

    for (;str[i]==' ' && i<len;i++){}
    int argslen=0;
    char **args=NULL, *arg=NULL;

    args=(char**)realloc(args, (argslen+1)*sizeof(char*));
    args[argslen]=(char*)calloc(MAXLEN, sizeof(char));
    strcpy(args[argslen], name);
    argslen++;

    for (;i<len;){
        arg=(char*)calloc(MAXLEN, sizeof(char));
        for (j=0;str[i]!=' ' && i<len;j++){
            if (j>MAXLEN){
                perror("too big command argname");
                return NULL;
            }
            arg[j]=str[i];
            i++;
        }
        arg[j]='\0';
        while (str[i]==' ' && i<len){ i++; }
        if (strcmp(arg, "")!=0){
            args=(char**)realloc(args, (argslen+1)*sizeof(char*));
            args[argslen]=arg;
            argslen++;
        }
    }
    if (args==NULL) {
        args=(char**)malloc(sizeof(char*));
    }
    args[argslen]=NULL;
    Command* cmd=init_cmd(name, args);
    return cmd;
}

CommandList* commandparser(char *str){
    /* parsing input string to commands and operators, and return CommandList */
    if (str == NULL){
        perror("str is NULL pointer");
        return NULL;
    }

    int wordlen=0, cmdstrlen=0, inword=0, incmdstr=0, wordtype;
    char* cmdstr=NULL, *word=NULL;
    CommandList *ans=NULL;
    Command *cmd=NULL;
    while (*str!='\0'){
        if (*str==' ' || *str=='\t'){ 
            if (inword==0 || word==NULL) {}
            else {
                wordtype=typeofword(word, wordlen);
                if (wordtype!=WORD){
                    cmdstrlen-=wordlen;
                    cmd=simplecommandparser(cmdstr, cmdstrlen);
                    if (wordtype==AMPERSAND){ set_cmdmode(cmd, BACKGROUND_MODE); }
                    else { set_cmdoperator(cmd, wordtype); }
                    
                    ans = add_cmd(ans, cmd);
                    cmdstr=NULL;
                    cmdstrlen=0;
                }
                word=NULL;
                wordlen=0;
            }
        }
        else{
            if (word == NULL || inword==0){ 
                word=str; 
                inword=1;
            }
            if (cmdstr==NULL) { 
                cmdstr=str; 
                incmdstr=1;
            }
            wordlen++;
        }
        if (cmdstr!=NULL){ cmdstrlen++; }
        str++;
    }

    if (cmdstr!=NULL){
        wordtype=-1;
        if (word!=NULL){ wordtype=typeofword(word, wordlen); }
        if (wordtype!=WORD && wordtype!=-1) { cmdstrlen-=wordlen; }

        cmd=simplecommandparser(cmdstr, cmdstrlen);

        if (wordtype!=WORD && wordtype!=-1){
            if (wordtype==AMPERSAND) { set_cmdmode(cmd, BACKGROUND_MODE); }
            else { set_cmdoperator(cmd, wordtype); }
        }
        ans=add_cmd(ans, cmd);
    }

    return ans;
}






int run_cmd(Command *cmd, int *fd_to_close1, int *fd_to_close2){
    /* fd_to_close - int array, that close in son proccess, 0 - end of array
       stdin = cmd->input, stdout = cmd->output */
    
    if (cmd==NULL) { return -1; }
    int chpid, status, i;
    chpid=fork();
    if (chpid==0){
        dup2(cmd->input, DEFAULT_INPUT);  
        dup2(cmd->output, DEFAULT_OUTPUT);
        if (fd_to_close1!=NULL){
            for (i=0;fd_to_close1[i]!=0;i++){
                if (fd_to_close1[i]!=-1) { close(fd_to_close1[i]); }
            }
        }
        if (fd_to_close2!=NULL){
            for (i=0;fd_to_close2[i]!=0;i++){
                if (fd_to_close2[i]!=-1) { close(fd_to_close2[i]); }
            }
        }

        execvp(cmd->name, cmd->args);
    }
    else{
        waitpid(chpid, NULL, 0);
    }
}

int run_cmdlist(CommandList *cmdlist){
    CommandList *cur=cmdlist,  *convstart=NULL, *convend=NULL, *curconv=NULL;
    Command *cmd=NULL, *nextcmd=NULL;
    int pipes[3], inconveyor=0, cmdreturn, convlen=0, i;

/* find io redirect */
/* find &, () */

    while (cur!=NULL){
        if (cur->cmd!=NULL){
            cmd=cur->cmd;
            if (cur->nextcmd!=NULL){ 
                nextcmd=cur->nextcmd->cmd; 
            }
            if (inconveyor==0 && cmd->operator!=CONVEYOR){
                cmdreturn = run_cmd(cmd, NULL, NULL);
            }
            else if (inconveyor==0 && cmd->operator==CONVEYOR){
                convstart=cur;
                convend=cur;
                inconveyor=1;
                pipe(pipes);
                set_cmdoutput(cmd, pipes[1]);
                set_cmdinput(nextcmd, pipes[0]);
                convlen++;
            }
            else if (inconveyor==1 && cmd->operator==CONVEYOR){
                convend=cur;
                set_cmdoutput(cmd, pipes[1]);
                set_cmdinput(nextcmd, pipes[0]);
                convlen++;
            }
            else if (inconveyor==1 && cmd->operator!=CONVEYOR){
                curconv=convstart;
                i=0;
                while (curconv!=NULL && curconv!=convend){
                    if (i==0 && curconv->cmd!=NULL) { 
                        cmdreturn = run_cmd(curconv->cmd, pipes, NULL); }                    
                    else if (curconv->cmd!=NULL){ 
                        cmdreturn = run_cmd(curconv->cmd, pipes, NULL); }
                    curconv=curconv->nextcmd;
                }
                if (curconv->cmd!=NULL){ 
                    cmdreturn = run_cmd(curconv->cmd, pipes, NULL); }
                inconveyor=0;
                convend=NULL; convstart=NULL; 
                close(pipes[0]); close(pipes[1]);
                pipes[0]=-1; pipes[1]=-1;
                convlen=0;    
            }
        }
        cur=cur->nextcmd;
    }
    



    // while (cur!=NULL){
    //     if (cur->cmd!=NULL){
    //         cmd=cur->cmd;
    //         if (cur->nextcmd!=NULL){ 
    //             nextcmd=cur->nextcmd->cmd; 
    //         }
    //         if (inconveyor==0 && cmd->operator!=CONVEYOR){
    //             cmdreturn = run_cmd(cmd, NULL, NULL);
    //         }
    //         else if (inconveyor==0 && cmd->operator==CONVEYOR){
    //             convstart=cur;
    //             convend=cur;
    //             inconveyor=1;
    //             pipe(pipes[convlen]);
    //             set_cmdoutput(cmd, pipes[convlen][1]);
    //             set_cmdinput(nextcmd, pipes[convlen][0]);
    //             convlen++;
    //         }
    //         else if (inconveyor==1 && cmd->operator==CONVEYOR){
    //             convend=cur;
    //             set_cmdoutput(cmd, pipes[convlen][1]);
    //             set_cmdinput(nextcmd, pipes[convlen][0]);
    //             convlen++;
    //         }
    //         else if (inconveyor==1 && cmd->operator!=CONVEYOR){
    //             curconv=convstart;
    //             i=0;
    //             while (curconv!=NULL && curconv!=convend){
    //                 if (i==0 && curconv->cmd!=NULL) { 
    //                     cmdreturn = run_cmd(curconv->cmd, pipes[i], NULL); }                    
    //                 else if (curconv->cmd!=NULL){ 
    //                     cmdreturn = run_cmd(curconv->cmd, pipes[i-1], pipes[i]); }
    //                 curconv=curconv->nextcmd;
    //             }
    //             if (curconv->cmd!=NULL){ 
    //                 cmdreturn = run_cmd(curconv->cmd, pipes[i-1], NULL); }
    //             inconveyor=0;
    //             convend=NULL; convstart=NULL; 
    //             for (i=0;i<convlen;i++){
    //                 close(pipes[i][0]); close(pipes[i][1]);
    //                 pipes[i][0]=-1; pipes[i][1]=-1;
    //             }
    //             convlen=0;    
    //         }
    //     }
    //     cur=cur->nextcmd;
    // }
    

    


    // while (cur!=NULL){
    //     if (cur->cmd!=NULL){
    //         cmd=cur->cmd;
    //         if (cur->nextcmd!=NULL){ 
    //             nextcmd=cur->nextcmd->cmd; 
    //         }
    //         if (cmd->operator==CONVEYOR){
    //             if (inconveyor==0){
    //                 inconveyor=1;
    //                 convstart=cur;
    //                 convend=cur;
    //                 pipe(pipes);
    //             }
    //             set_cmdoutput(cmd, pipes[1]);
    //             if (nextcmd!=NULL){ set_cmdinput(nextcmd, pipes[0]); }
    //             // run_cmd(cmd, pipes);
    //         }
    //         else if (inconveyor==1){
    //             /* run conveyor */
    //             curconv=convstart;
    //             while (curconv!=NULL && curconv!=convend){
    //                 if (curconv->cmd!=NULL){ run_cmd(curconv->cmd, pipes); }
    //                 curconv=curconv->nextcmd;
    //             }
    //             if (curconv->cmd!=NULL){ run_cmd(curconv->cmd, pipes); }
    //             inconveyor=0;
    //             convend=NULL; convstart=NULL; 
    //             close(pipes[0]); close(pipes[1]);
    //             pipes[0]=-1; pipes[1]=-1;
    //         }
    //         else { 
    //             run_cmd(cmd, pipes);
    //         }
    //     }
    //     cur=cur->nextcmd;
    // }
}





int main(int argc, char **argv){
    signal(SIGINT, sig_handler);
    char *inpstr=NULL;
    CommandList *list=NULL;
    
    while (!EndOfShell){
        printf(">>");
        inpstr = scanstring();
        if (inpstr==NULL){ EndOfInput=1; }
        if (EndOfInput){ EndOfShell=1; }
        
        if (inpstr!=NULL && !EndOfShell){
            list=commandparser(inpstr);

            run_cmdlist(list);
            // int a[3]={-1, -1, 0};
            // run_cmd(list->cmd, a);

            free(inpstr);
            free_cmdlist(list);
        }
        
    }

 
}