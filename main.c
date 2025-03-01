#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sqlite3.h>

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

/*
CREATE TABLE tasks (
    uid INTEGER PRIMARY KEY,
    desc TEXT NOT NULL,
    status INTEGER NOT NULL
);
*/

CLI_Argument cli_args[] = {
    {1,"add", "Add a new task"},
    {2,"remove", "Remove a task"},
    {3,"list", "List all tasks"},
    {4,"complete", "Mark a task as done"},
    {5,"help", "Show available commands"},
    {6,"search", "Search tasks using a keyword"}                 
};
unsigned long long generate_uid(){
    srand(time(0) ^ getpid());
    return ((unsigned long long)time(0) << 20) | (getpid() << 8) | (rand() & 0xFF); //returns a 64-bit unique id
}
static int callback(void* NotUsed, int argc, char* argv[], char* azColName[]){
    for (int i = 0; i < argc; i++)
    {
        printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
    }
    printf("\n");
    return 0;
    
}

// int is_json_empty(){
//     FILE* file = fopen("data.json", "r");
//     if (file == NULL)
//     {
//         return -1;
//     }
//     fseek(file, 0, SEEK_END);
//     long size = ftell(file);
//     fclose(file);
//     return (size == 0);
// }

// int add_task(char* task_content){
//     // printf("Added task: %s\n", task);
//     unsigned long long uid = generate_uid();
//     struct Task task = {uid,task_content,false};
//     FILE* file = fopen("data.json", "r+");
//     if (file == NULL) {
//         file = fopen("data.json", "w");
//         if (file == NULL) {
//             printf("Error creating file!\n");
//             return 1;
//         }
//         fprintf(file, "[{\n");
//         fprintf(file, "  \"task\": \"%s\",\n", *task.desc);
//         fprintf(file, "  \"uid\": %u,\n", task.uid);
//         fprintf(file, "  \"status\": %d\n", task.status);
//         fprintf(file, "}\n]");
//         fclose(file);
//     }
//     if (is_json_empty())
//     {
//         fprintf(file, "[{\n");
//         fprintf(file, "  \"task\": \"%s\",\n", *task.desc);
//         fprintf(file, "  \"uid\": %u,\n", task.uid);
//         fprintf(file, "  \"status\": %d\n", task.status);
//         fprintf(file, "}\n]");
//         fclose(file);
//         printf("Task JSON data written to data.json\n");
//     }
//     else{
//         fseek(file,-2,SEEK_END);
//         fprintf(file, ",\n {\n");
//         fprintf(file, "  \"task\": \"%s\",\n", *task.desc);
//         fprintf(file, "  \"uid\": %u,\n", task.uid);
//         fprintf(file, "  \"status\": %d\n", task.status);
//         fprintf(file, "}\n]");
//         fclose(file);
//         printf("Task JSON data written to data.json\n");
//     }
// }

int add_task(char* task){
    sqlite3* db;
    char* zErrMsg = 0;
    int rc = sqlite3_open("data.db", &db);

    if (rc)
    {
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return(1);
    }

    const char *sql = "INSERT INTO tasks (uid, desc, status) VALUES (?, ?, ?);";
    sqlite3_stmt *stmt;

    if (sqlite3_prepare_v2(db,sql,-1,&stmt,0))
    {
        printf("SQL error: %s\n", sqlite3_errmsg(db));
        return 1;
    }

    unsigned long long uid = generate_uid();
    struct Task generated_task = {uid, task, false};
    sqlite3_bind_int(stmt,1,generated_task.uid);
    sqlite3_bind_text(stmt,2,*generated_task.desc,-1,SQLITE_STATIC);
    sqlite3_bind_int(stmt,3,generated_task.status);

    if (sqlite3_step(stmt) != SQLITE_DONE)
    {
        printf("Failed to insert task: %s\n", sqlite3_errmsg(db));
    } else{
        printf("Task inserted successfully!\n");
    }

    sqlite3_finalize(stmt);

    if (rc = SQLITE_OK)
    {
        fprintf(stderr, "SQL error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
    }

    sqlite3_close(db);
}
int search_task(char *task_keyword){
    sqlite3* db;
    char* zErrMsg = 0;
    int rc = sqlite3_open("data.db", &db);

    if (rc)
    {
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return(1);
    }

    const char *sql = "SELECT * FROM tasks WHERE desc LIKE ?;";
    sqlite3_stmt *stmt;

    if (sqlite3_prepare_v2(db,sql,-1,&stmt,0))
    {
        printf("SQL error: %s\n", sqlite3_errmsg(db));
        return 1;
    }

    char search_pattern[110];
    snprintf(search_pattern, sizeof(search_pattern), "%%%s%%", task_keyword);
    sqlite3_bind_text(stmt,2,search_pattern,-1,SQLITE_STATIC);

    int found = 0;
    while (sqlite3_step(stmt) == SQLITE_ROW)
    {
        struct Task task;
        task.uid = sqlite3_column_int(stmt,0);
        snprintf(*task.desc,sizeof(task.desc),"%s", sqlite3_column_text(stmt,1));
        task.status = sqlite3_column_int(stmt,2);
        printf("Task Found:\n");
        printf("UID: %d\n", task.uid);
        printf("Description: %s\n", *task.desc);
        printf("Status: %d\n", task.status);
        found = 1;
    }
    if (!found) {
        printf("No task found with description containing: %s\n", task_keyword);
    }

    sqlite3_finalize(stmt);

    if (rc = SQLITE_OK)
    {
        fprintf(stderr, "SQL error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
    }

    sqlite3_close(db);
}
void delete_task(char* task){
}

void update_task(char* task){
    printf("Marking task: %s as complete\n", task);
}

int list_tasks() {
    sqlite3 *db;
    sqlite3_stmt *stmt;
    char *zErrMsg = 0;
    int rc;

    rc = sqlite3_open("data.db", &db);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
        return 1;
    }

    const char *sql = "SELECT * FROM tasks;";
    rc = sqlite3_prepare_v2(db, sql, -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return 1;
    }

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        struct Task task;
        task.uid = sqlite3_column_int(stmt, 0);
        snprintf(task.desc, sizeof(task.desc), "%s", (const char *)sqlite3_column_text(stmt, 1));
        task.status = sqlite3_column_int(stmt, 2);
        printf("Task Found:\n");
        printf("UID: %d\n", task.uid);
        printf("Description: %s\n", task.desc);
        printf("Status: %d\n", task.status);
    }

    sqlite3_finalize(stmt);
    sqlite3_close(db);
    return 0;
}
void help(){
    for (int i = 0; i < 5; i++)
    {
        printf("Argument: %s, Description: %s\n", cli_args[i].arg, cli_args[i].desc);
    }
    
}


void parse_args(char* arg, char* task){
    for (int i = 0; i < 6; i++)
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
            case 6:
                search_task(task);
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