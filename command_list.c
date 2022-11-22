// #include "types.c"
// #include "directives.c"


Command* init_cmd(char* name, char** args){
    Command* ans=malloc(sizeof(Command));
    ans->name=name;
    ans->args=args;
    ans->operator=NOOPERATOR;
    ans->input=DEFAULT_INPUT;
    ans->output=DEFAULT_OUTPUT;
    ans->mode=DEFAULT_MODE;
    return ans;
}

Command* init_empty_cmd(){
    return init_cmd(NULL, NULL);
}

void set_cmdoperator(Command *cmd, int operator){
    if (cmd==NULL){
        perror("cmd is NULL pointer");
        return;
    }
    cmd->operator=operator;
}

void set_cmdmode(Command *cmd, int mode){
    if (cmd==NULL){
        perror("cmd is NULL pointer");
        return;
    }
    cmd->mode=mode;
}

void set_cmdinput(Command *cmd, int input){
    if (cmd==NULL){
        perror("cmd is NULL pointer");
        return;
    }
    cmd->input=input;
}

void set_cmdoutput(Command *cmd, int output){
    if (cmd==NULL){
        perror("cmd is NULL pointer");
        return;
    }
    cmd->output=output;
}

CommandList* add_cmd(CommandList *list, Command *cmd){
    if (list==NULL){
        list=(CommandList*) malloc(sizeof(CommandList));
        list->cmd=cmd;
        list->nextcmd=NULL;
        return list;
    }
    else{
        CommandList *cur=list;
        while (cur->nextcmd!=NULL){
            cur=cur->nextcmd;
        }
        cur->nextcmd=(CommandList*) malloc(sizeof(CommandList));
        cur=cur->nextcmd;
        cur->cmd=cmd;
        cur->nextcmd=NULL;

        
        return list;
    }
}

CommandList* pop_cmd(CommandList *list, Command *cmd){
    if (list==NULL){
        perror("list is NULL pointer");
        return NULL;
    }
    if (cmd==NULL){
        perror("cmd is NULL pointer");
        return list;
    }
    cmd->args=list->cmd->args;
    cmd->name=list->cmd->name;
    cmd->operator=list->cmd->operator;
    cmd->input=list->cmd->input;
    cmd->output=list->cmd->output;
    CommandList* temp=list;
    list=list->nextcmd;
    if (temp!=NULL) free(temp);
    return list;
}

void print_cmd(Command *cmd){
    if (cmd==NULL) return;
    if (cmd->name!=NULL) printf("%s~", cmd->name);
    for (int i=0; cmd->args!=NULL && cmd->args[i]!=NULL;i++){
        printf("%s,", cmd->args[i]);
    }
    printf("~");
    printf("%d~", cmd->operator); 
    printf("%d~", cmd->input); 
    printf("%d~", cmd->output); 
    printf("%d", cmd->mode);
}

void print_cmdlist(CommandList *list){
    printf("name ~ args ~ operator ~ input ~ output ~ mode\n");
    while (list!=NULL){
        print_cmd(list->cmd);
        list=list->nextcmd;
        if (list!=NULL) {printf("--->");}
        printf("\n");
    }
}

void free_cmd(Command* cmd){
    if (cmd==NULL) return;
    if (cmd->name!=NULL){ free(cmd->name); }
    char** args=cmd->args, **temp;
    int i;

    if (args!=NULL){
        for (i=0;args[i]!=NULL;i++){
            free(args[i]);
        }
        free(args);
    }

    free(cmd);    
}

void free_cmdlist(CommandList *list){
    CommandList *temp;
    while (list!=NULL){
        free_cmd(list->cmd);
        temp=list;
        list=list->nextcmd;
        if (temp!=NULL) free(temp);
    }
}

CommandList* del_cmds_with_operator(CommandList* list, int op){
    CommandList *cur=list, *prev=NULL, *head=list;
    while (cur!=NULL){
        if (cur->cmd!=NULL && cur->cmd->operator==op){
            if (prev==NULL){
                head=cur->nextcmd;
                free_cmd(cur->cmd);
                free(cur);
                cur=head;
            }
            else{
                prev->nextcmd=cur->nextcmd;
                free_cmd(cur->cmd);
                free(cur);
                cur=prev->nextcmd;
            }
        }
        else{
            prev=cur;
            cur=cur->nextcmd;
        }
    }
    return head;
}

CommandList* del_cmds_from_to(CommandList* list, CommandList* start, CommandList* end){
    if (list==NULL || start==NULL || end==NULL){
        return NULL;
    }
   
    CommandList *cur=list, *prev=NULL, *head=list;
    while (cur!=start && cur!=NULL){
        prev=cur;
        cur=cur->nextcmd;
    }
    if (prev==NULL){
        head=end->nextcmd;
    }
    else {
        prev->nextcmd=end->nextcmd;
    }
    end->nextcmd=NULL;
    free_cmdlist(start);
    return head;
}