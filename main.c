#include "directives.c"
#include "types.c"
#include "command_list.c"


int EndOfInput=0, EndOfShell=0, onestr_returnstatus=0;
char *progname;

void sig_handler(int signum){
    // kill(getpid(), signum);
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
    for (int i=len;dest[i]==' ' || dest[i]=='\0';i--){
        if (i<0) break;
        dest[i]='\0';
    }
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
    else if (strcmp(tempstr, "<")==0) { ans=LEFT_ARROW; }
    else if (strcmp(tempstr, ">")==0) { ans=RIGHT_ARROW; }
    else if (strcmp(tempstr, ">>")==0) { ans=RIGHT_DOUBLE_ARROW; }
    else if (strcmp(tempstr, ";")==0) { ans=SEMICOLON; }
    else if (strcmp(tempstr, "(0)")==0) { ans=ZERO_IN_BRACKETS; }
    else if (strcmp(tempstr, "(1)")==0) { ans=ONE_IN_BRACKETS; }
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

    args=(char**)calloc((argslen+3), sizeof(char*));
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
            args=(char**)realloc(args, (argslen+3)*sizeof(char*));
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

    int wordlen=0, cmdstrlen=0, inword=0, incmdstr=0, wordtype, inIOredirect=0;
    char* cmdstr=NULL, *word=NULL;
    CommandList *ans=NULL;
    Command *cmd=NULL;
    int endwhile=0;
    while (!endwhile){
        if (*str==' ' || *str=='\t' || *str=='\0'){ 
            if (inword==0 || word==NULL) {}
            else {
                wordtype=typeofword(word, wordlen);
                if (inIOredirect){
                    if (wordlen>0){
                        cmd=simplecommandparser(cmdstr, cmdstrlen);
                        set_cmdoperator(cmd, inIOredirect);
                        ans=add_cmd(ans, cmd);
                        inIOredirect=0;
                        cmdstr=NULL;
                        cmdstrlen=0;
                    }
                }
                else if (wordtype==RIGHT_ARROW || wordtype==LEFT_ARROW || wordtype==RIGHT_DOUBLE_ARROW){
                    if (cmdstrlen>wordlen){
                        cmdstrlen-=wordlen;
                        cmd=simplecommandparser(cmdstr, cmdstrlen);
                        ans = add_cmd(ans, cmd);
                    }
                    cmdstr=NULL;
                    cmdstrlen=0;
                    inIOredirect=wordtype;
                }
                else if (wordtype==SEMICOLON){
                    if (cmdstrlen>wordlen){
                        cmdstrlen-=wordlen;
                        cmd=simplecommandparser(cmdstr, cmdstrlen);
                        ans = add_cmd(ans, cmd);
                    }
                    cmdstr=NULL;
                    cmdstrlen=0;

                    char* name=(char*)calloc(MAXLEN, sizeof(char));
                    char** args=(char**)calloc(1, sizeof(char*));
                    args[0]=NULL;
                    cmd=init_cmd(name, args);
                    set_cmdoperator(cmd, wordtype);
                    ans=add_cmd(ans, cmd);
                }
                else if (wordtype==AMPERSAND){
                    if (cmdstrlen>wordlen){
                        cmdstrlen-=wordlen;
                        cmd=simplecommandparser(cmdstr, cmdstrlen);
                        ans = add_cmd(ans, cmd);
                    }
                    cmdstr=NULL;
                    cmdstrlen=0;

                    char* name=(char*)calloc(MAXLEN, sizeof(char));
                    char** args=(char**)calloc(1, sizeof(char*));
                    args[0]=NULL;
                    cmd=init_cmd(name, NULL);
                    set_cmdoperator(cmd, wordtype);
                    ans=add_cmd(ans, cmd);
                }
                
                else if (wordtype==AND){
                    if (cmdstrlen>wordlen){
                        cmdstrlen-=wordlen;
                        cmd=simplecommandparser(cmdstr, cmdstrlen);
                        ans = add_cmd(ans, cmd);
                    }
                    cmdstr=NULL;
                    cmdstrlen=0;

                    char* name=(char*)calloc(MAXLEN, sizeof(char));
                    char** args=(char**)calloc(1, sizeof(char*));
                    args[0]=NULL;
                    cmd=init_cmd(name, NULL);
                    set_cmdoperator(cmd, wordtype);
                    ans=add_cmd(ans, cmd);
                }
                else if (wordtype==OR){
                    if (cmdstrlen>wordlen){
                        cmdstrlen-=wordlen;
                        cmd=simplecommandparser(cmdstr, cmdstrlen);
                        ans = add_cmd(ans, cmd);
                    }
                    cmdstr=NULL;
                    cmdstrlen=0;

                    char* name=(char*)calloc(MAXLEN, sizeof(char));
                    char** args=(char**)calloc(1, sizeof(char*));
                    args[0]=NULL;
                    cmd=init_cmd(name, NULL);
                    set_cmdoperator(cmd, wordtype);
                    ans=add_cmd(ans, cmd);
                }

                else if (wordtype==ZERO_IN_BRACKETS || wordtype==ONE_IN_BRACKETS){
                    if (cmdstrlen>wordlen){
                        cmdstrlen-=wordlen;
                        cmd=simplecommandparser(cmdstr, cmdstrlen);
                        ans = add_cmd(ans, cmd);
                    }
                    cmdstr=NULL;
                    cmdstrlen=0;

                    char* name=(char*)calloc(MAXLEN, sizeof(char));
                    char** args=(char**)calloc(1, sizeof(char*));
                    args[0]=NULL;
                    cmd=init_cmd(name, NULL);
                    set_cmdoperator(cmd, wordtype);
                    ans=add_cmd(ans, cmd);
                }

                else if (wordtype!=WORD){
                    cmdstrlen-=wordlen;
                    cmd=simplecommandparser(cmdstr, cmdstrlen);
                    set_cmdoperator(cmd, wordtype); 
                    ans = add_cmd(ans, cmd);
                    cmdstr=NULL;
                    cmdstrlen=0;
                }
                else if (wordtype==WORD && *str=='\0'){
                    cmd=simplecommandparser(cmdstr, cmdstrlen);                    
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
        if (*str=='\0'){ endwhile=1; }
        else { str++; } 
    }

    return ans;
}

int run_cmd(Command *cmd, int pipes[][2], int pipeslen, int indclose, int cmdtype){
    /*cmdtype: fstconv, lastconv, reg 
      returns childs pid */    
    if (cmd==NULL) { return -1; }
    int chpid, status, i;
    chpid=fork();
    if (chpid==0){
        dup2(cmd->input, DEFAULT_INPUT);  
        dup2(cmd->output, DEFAULT_OUTPUT);

        if (cmdtype==FIRST_CONV_CMD && cmd->input!=DEFAULT_INPUT){ close(cmd->input); }
        else if (cmdtype==LAST_CONV_CMD && cmd->output!=DEFAULT_OUTPUT){ close(cmd->output); }
        if (pipes!=NULL){
            for (i=0;i<pipeslen;i++){
                close(pipes[i][0]); 
                close(pipes[i][1]); 
            }
        }
        execvp(cmd->name, cmd->args);
    }
    else {
        if (cmdtype==FIRST_CONV_CMD && cmd->input!=DEFAULT_INPUT){ close(cmd->input); }
        else if (cmdtype==LAST_CONV_CMD && cmd->output!=DEFAULT_OUTPUT){ close(cmd->output); }
        if (pipes!=NULL && indclose>0){
            close(pipes[indclose-1][0]);
        }
        if (pipes!=NULL && indclose<pipeslen){
            close(pipes[indclose][1]);
        }
        return chpid;
    }
}

int run_conveyor(CommandList* startConv, CommandList* endConv){
    /* conveyor - [startConv, endConv] 
    0 - success, 1 - fail */
    CommandList *curentConv=startConv;
    Command* cmd;
    if (endConv!=NULL){
        endConv=endConv->nextcmd;
    }

    int convlen=0;
    while (curentConv!=endConv){
        convlen++;
        curentConv=curentConv->nextcmd;
    }
    
    int pipes[convlen-1][2];
    curentConv=startConv;
    for (int i=0;i<convlen-1;i++){
        pipe(pipes[i]);
        if (curentConv!=NULL){
            cmd=curentConv->cmd;
            if (cmd!=NULL){
                cmd->output=pipes[i][1];
            }
            if (curentConv->nextcmd!=NULL && curentConv->nextcmd->cmd!=NULL){
                cmd=curentConv->nextcmd->cmd;
                cmd->input=pipes[i][0];
            }
            curentConv=curentConv->nextcmd;
        }
    }

    curentConv=startConv; 
    int i=0, cmdtype=REGULAR_CMD, pids[convlen];
    while (curentConv!=endConv && curentConv!=NULL){
        if (curentConv==startConv) { cmdtype=FIRST_CONV_CMD; }
        else if (curentConv->nextcmd==endConv) { cmdtype=LAST_CONV_CMD; }
        else { cmdtype=REGULAR_CMD; }
        cmd=curentConv->cmd;
        pids[i]=run_cmd(cmd, pipes, convlen-1, i, cmdtype);
        i++;
        curentConv=curentConv->nextcmd;
    }

    int status, child;
    for (i=0;i<convlen;i++){
        child=wait(&status);
        if (child==pids[convlen-1]){
            if (WIFEXITED(status) && WEXITSTATUS(status)==0){
                return 0;
            }
            else{
                return 1;
            }
        }
    }

}

int redirection(Command* io, Command* start, Command* end){
    if (io==NULL || start==NULL || end==NULL){ return -1; }
    if (io->operator==LEFT_ARROW){
        int fd=open(io->name, O_RDONLY);
        if (fd==-1){
            perror("No such file");
            return -1;
        }
        set_cmdinput(start, fd);
    }
    else if (io->operator==RIGHT_ARROW || RIGHT_DOUBLE_ARROW){
        int fd=-1;
        if (io->operator==RIGHT_ARROW){
            fd=open(io->name, O_WRONLY | O_CREAT | O_TRUNC, 0777);
        }
        else if (io->operator==RIGHT_DOUBLE_ARROW){
            fd=open(io->name, O_WRONLY | O_APPEND, 0777);
        }
        if (fd==-1){
            perror("error with file");
            return -1;
        }
        set_cmdoutput(end, fd);
    }
    else { return -1; }
    return 0;
}
CommandList* set_list_ioredirect(CommandList *cmdlist){
    CommandList *cur=cmdlist, *start=NULL, *end=NULL, *ans=cmdlist;
    Command* iredirect=NULL, *oredirect=NULL, *cmd=NULL;

    while (cur!=NULL){
        cmd=cur->cmd;
        if (cmd!=NULL){
            if (cmd->operator==RIGHT_ARROW || cmd->operator==RIGHT_DOUBLE_ARROW){
                oredirect=cmd;
            }
            else if (cmd->operator==LEFT_ARROW){
                iredirect=cmd;
            }
            else if (cmd->operator==AMPERSAND || cmd->operator==OR || cmd->operator==AND || cmd->operator==SEMICOLON){
                if (iredirect!=NULL && start!=NULL && end!=NULL){
                    redirection(iredirect, start->cmd, end->cmd);
                }
                if (oredirect!=NULL && start!=NULL && end!=NULL){
                    redirection(oredirect, start->cmd, end->cmd);
                }
                start=NULL;
                end=NULL;
                iredirect=NULL;
                oredirect=NULL;
            }

            else {
                if (start==NULL){ 
                    start=cur;
                    end=cur;
                }
                else {
                    end=cur;
                }
            }
        }
        cur=cur->nextcmd; 
    }
    
    if (iredirect!=NULL && start!=NULL && end!=NULL){
        redirection(iredirect, start->cmd, end->cmd);
    }
    if (oredirect!=NULL && start!=NULL && end!=NULL){
        redirection(oredirect, start->cmd, end->cmd);
    }
    
    ans=del_cmds_with_operator(cmdlist, RIGHT_ARROW);
    ans=del_cmds_with_operator(ans, LEFT_ARROW);
    ans=del_cmds_with_operator(ans, RIGHT_DOUBLE_ARROW);
    return ans;
}


CommandList* run_cmdlist(CommandList *cmdlist){
    CommandList *cur;
    Command *cmd=NULL, *nextcmd=NULL;

    cmdlist=set_list_ioredirect(cmdlist);
    CommandList *startConv=NULL, *endConv=NULL;
    cur=cmdlist;
    int inconveyor=0, cmdret=-1;
    while (cur!=NULL){
        cmd=cur->cmd;
        if (cmd==NULL){
            perror("NULL cmd");
            cur=cur->nextcmd;
            continue;
        }

        int check = cmd->operator==AND && cmdret==1 || cmd->operator==OR && cmdret==0;
        if (check){
            while (cur!=NULL && cur->cmd!=NULL && cur->cmd->operator!=SEMICOLON){
                cur=cur->nextcmd;
            }
            cmdret=-1;
            inconveyor=0;
            startConv=NULL;
            endConv=NULL;
            continue;
        }
        else if (cmd->operator==ZERO_IN_BRACKETS){
            cmdret=0;
            inconveyor=0;
            startConv=NULL;
            endConv=NULL;
        }
        else if (cmd->operator==ONE_IN_BRACKETS){
            cmdret=1;
            inconveyor=0;
            startConv=NULL;
            endConv=NULL;
        }
        else if (cmd->operator==OR || cmd->operator==AND){  }
        else if (cmd->operator==SEMICOLON){
            cmdret=-1;
        }
        else if (!inconveyor && cmd->operator==CONVEYOR){
            inconveyor=1;
            startConv=cur;
        }
        else if (inconveyor && cmd->operator==NOOPERATOR){
            endConv=cur;
            cmdret = run_conveyor(startConv, endConv);
            inconveyor=0;
            startConv=NULL;
            endConv=NULL;
        }
        else if (!inconveyor){
            cmdret = run_conveyor(cur, cur);
        }
// print_cmd(cmd);
// printf("\n%d\n", getpid());


        cur=cur->nextcmd;
    }
    onestr_returnstatus=cmdret;
    return cmdlist;
}






int run_bgmain(char* str, int start, int end){
    /* str[start, end-1] */
    pid_t chpid=fork();
    if (chpid==0){
        if (fork()==0){
            signal(SIGINT, SIG_IGN);
            int fdpipe[2];
            pipe(fdpipe);
            write(fdpipe[1], str+start, (end-start)*sizeof(char));
            close(fdpipe[1]);

            dup2(fdpipe[0], DEFAULT_INPUT);
            close(fdpipe[0]);
            
            execlp(progname, progname, "--one-str", NULL);
            exit(1);
        }
        exit(0);
    }
    int status;
    waitpid(chpid, &status, 0);
    if (WIFEXITED(status) && WEXITSTATUS(status)==0){
        return 0;
    }
    else{
        return 1;
    }
}
char* make_background(char* str){
    int istart=0, ibg=-1, i;
    for (i=0;i<strlen(str);i++){
        if (str[i]==';'){
            istart=i+1;
        }
        else if (str[i]=='&' && str[i+1]!='&' && str[i-1]!='&'){
            ibg=i;
            run_bgmain(str, istart, ibg);
        }
    }
    istart=0; ibg=-1;
    for (i=0;i<strlen(str);i++){
        if (str[i]=='&' && str[i+1]!='&' && str[i-1]!='&'){
            char* temp = (char*)calloc(MAXLEN, sizeof(char));
            temp=strcpy(temp, str+i+1);
            str[istart]='\0';
            str=strcat(str, temp);
            free(temp);
        }
        else if (str[i]==';'){
            istart=i;
        }
    }
    return str;
}


int run_bracketsmain(char* str, int start, int end){
    /* str[start, end-1] */
    pid_t chpid=fork();
    if (chpid==0){
        // signal(SIGINT, SIG_IGN);
        int fdpipe[2];
        pipe(fdpipe);
        write(fdpipe[1], str+start, (end-start)*sizeof(char));
        close(fdpipe[1]);

        dup2(fdpipe[0], DEFAULT_INPUT);
        close(fdpipe[0]);
            
        execlp(progname, progname, "--one-str", NULL);
        exit(1);
    }
    int status;
    waitpid(chpid, &status, 0);
    if (WIFEXITED(status) && WEXITSTATUS(status)==0){
        return 0;
    }
    else{
        return 1;
    }
}
char* make_brackets(char* str){
    int start=-1, opencount=0;
    for (int i=0;i<strlen(str);i++){
        if (str[i]=='('){
            opencount++;
            if (start==-1){
                start=i;
            }
        }
        else if (str[i]==')'){
            opencount--;
        }
        if (opencount==0 && start!=-1){
            /* inbrackets=[start+1:i-1] */
            int status = run_bracketsmain(str, start+1, i);
            char* temp = (char*)calloc(MAXLEN, sizeof(char));
            temp=strcpy(temp, str+i);
            if (status==0){
                str[start+1]='0';
            }
            else {
                str[start+1]='1';
            }
            str[start+2]='\0';
            str=strcat(str, temp);
            free(temp);
            i=start+2;
            start=-1;
        }
    }
    return str;
}



void onestr_mode(){
    char *inpstr=NULL;
    CommandList *list=NULL;
    inpstr = scanstring();
    if (inpstr==NULL){ EndOfInput=1; }
        
    if (inpstr!=NULL){
        list=commandparser(inpstr);
// print_cmdlist(list);
        list=run_cmdlist(list);
// print_cmdlist(list);
        free_cmdlist(list);
    }
    if (inpstr!=NULL) free(inpstr); 
    exit(onestr_returnstatus);
}

int main(int argc, char** argv){
    progname=argv[0];
    if (argc>1){
        if (strcmp(argv[1], "--one-str")==0){
            onestr_mode();
            exit(0);
        }
    }

    // signal(SIGINT, sig_handler);
    char *inpstr=NULL;
    CommandList *list=NULL;

    while (!EndOfShell){
        printf(">> ");
        inpstr = scanstring();
        if (inpstr==NULL){ EndOfInput=1; }
        if (EndOfInput){ EndOfShell=1; }
        
        if (inpstr!=NULL && !EndOfShell){
            inpstr=make_background(inpstr);
            inpstr=make_brackets(inpstr);
            list=commandparser(inpstr);
// print_cmdlist(list);
            list=run_cmdlist(list);
// print_cmdlist(list);
            free_cmdlist(list);
        }
        if (inpstr!=NULL) free(inpstr);
    }

 
}