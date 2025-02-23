#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

struct Task{
    unsigned int uid;
    char* desc[100];
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
    {4,"complete", "Mark a task as done"},
    {5,"help", "Show available commands"}                 
};
unsigned long long generate_uid(){
    srand(time(0) ^ getpid());
    return ((unsigned long long)time(0) << 20) | (getpid() << 8) | (rand() & 0xFF); //returns a 64-bit unique id
}
int is_json_empty(){
    FILE* file = fopen("data.json", "r");
    if (file == NULL)
    {
        return -1;
    }
    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    fclose(file);
    return (size == 0);
}
int add_task(char* task_content){
    // printf("Added task: %s\n", task);
    unsigned long long uid = generate_uid();
    struct Task task = {uid,task_content,false};
    FILE* file = fopen("data.json", "r+");
    if (file == NULL) {
        file = fopen("data.json", "w");
        if (file == NULL) {
            printf("Error creating file!\n");
            return 1;
        }
        fprintf(file, "[{\n");
        fprintf(file, "  \"task\": \"%s\",\n", *task.desc);
        fprintf(file, "  \"uid\": %u,\n", task.uid);
        fprintf(file, "  \"status\": %d\n", task.status);
        fprintf(file, "}\n]");
        fclose(file);
    }
    if (is_json_empty())
    {
        fprintf(file, "[{\n");
        fprintf(file, "  \"task\": \"%s\",\n", *task.desc);
        fprintf(file, "  \"uid\": %u,\n", task.uid);
        fprintf(file, "  \"status\": %d\n", task.status);
        fprintf(file, "}\n]");
        fclose(file);
        printf("Task JSON data written to data.json\n");
    }
    else{
        fseek(file,-2,SEEK_END);
        fprintf(file, ",\n {\n");
        fprintf(file, "  \"task\": \"%s\",\n", *task.desc);
        fprintf(file, "  \"uid\": %u,\n", task.uid);
        fprintf(file, "  \"status\": %d\n", task.status);
        fprintf(file, "}\n]");
        fclose(file);
        printf("Task JSON data written to data.json\n");
    }
}

void delete_task(char* task){
    printf("deleting task: %s\n", task);
}

void update_task(char* task){
    printf("Marking task: %s as complete\n", task);
}

void list_tasks(){
    printf("listing tasks\n");
}

void help(){
    for (int i = 0; i < 5; i++)
    {
        printf("Argument: %s, Description: %s\n", cli_args[i].arg, cli_args[i].desc);
    }
    
}


void parse_args(char* arg, char* task){
    for (int i = 0; i < 5; i++)
    {
        if (strcmp(arg,cli_args[i].arg) == 0)
        {
            // printf("Command: %s - %s with id: %d\n", cli_args[i].arg, cli_args[i].desc, cli_args[i].id);
            switch (cli_args[i].id)
            {
            case 1:
                add_task(task);
                break;
            case 2:
                delete_task(task);
                break;
            case 3:
                list_tasks(task);
                break;
            case 4:
                update_task(task);
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
    parse_args(argv[1],argv[2]);
    return 0;
}