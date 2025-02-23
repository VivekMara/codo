#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <uuid/uuid.h>
#include <string.h>

#define MAX_ARGS 5;

struct Task{
    unsigned int uuid;
    char desc[20];
    bool status;
};

typedef struct {
    int id;
    char* arg;
    char* desc;
} CLI_Argument;

CLI_Argument cli_args[] = {
    {1,"add", "Add a new task"},
    {2,"remove", "Remove a task"},
    {3,"list", "List all tasks"},
    {4,"done", "Mark a task as done"},
    {5,"help", "Show available commands"}                 
};

void generate_uuid(char* buffer){
    uuid_t uuid;
    uuid_generate(uuid);
    uuid_unparse(uuid, buffer);
}

void add_task(){
    // char uuid_str[37];
    // generate_uuid(uuid_str);
    // printf("Generated UUID: %s\n", uuid_str);
    printf("Adding task\n");
}

void delete_task(){
    printf("deleting task\n");
}

void update_task(){
    printf("updating task\n");
}

void list_tasks(){
    printf("listing task\n");
}

void help(){
    for (int i = 0; i < 5; i++)
    {
        printf("Argument: %s, Description: %s\n", cli_args[i].arg, cli_args[i].desc);
    }
    
}


void parse_args(char* arg){
    for (int i = 0; i < 5; i++)
    {
        if (strcmp(arg,cli_args[i].arg) == 0)
        {
            // printf("Command: %s - %s with id: %d\n", cli_args[i].arg, cli_args[i].desc, cli_args[i].id);
            switch (cli_args[i].id)
            {
            case 1:
                add_task();
                break;
            case 2:
                delete_task();
                break;
            case 3:
                list_tasks();
                break;
            case 4:
                update_task();
                break;
            case 5:
                help();
                break;
            default:
                break;
            }
            return;
        }
        
    }
    printf("Invalid argument: %s\n", arg);
}
int main(int argc, char* argv[]){
    
    if (argc < 2) {
        printf("Usage: todo <command>\n");
        return 1;
    }

    parse_args(argv[1]);
    return 0;
}