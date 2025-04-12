#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sqlite3.h>

struct Task {
    unsigned int uid;
    char desc[100];  //a regular character array (string)
    bool status;
};

typedef struct {
    int id;
    char *arg;
    char *desc;
} CLI_Argument;

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

static int callback(void *NotUsed, int argc, char *argv[], char *azColName[]){
    for (int i = 0; i < argc; i++) {
        printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
    }
    printf("\n");
    return 0;
}

int add_task(char *task){
    sqlite3* db;
    char* zErrMsg = 0;
    int rc = sqlite3_open("tasks.db", &db);

    if (rc != SQLITE_OK) {
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return 1;
    }

    const char *sql = "INSERT INTO tasks (uid, desc, status) VALUES (?, ?, ?);";
    sqlite3_stmt *stmt;

    rc = sqlite3_prepare_v2(db, sql, -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return 1;
    }

    unsigned long long uid = generate_uid();
    
    sqlite3_bind_int(stmt, 1, uid);
    sqlite3_bind_text(stmt, 2, task, -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 3, 0);  // false for status

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        fprintf(stderr, "Failed to insert task: %s\n", sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        sqlite3_close(db);
        return 1;
    } else {
        printf("Task inserted successfully!\n");
    }

    sqlite3_finalize(stmt);
    sqlite3_close(db);
    return 0;
}

int search_task(char *task_keyword){
    sqlite3* db;
    char* zErrMsg = 0;
    int rc = sqlite3_open("tasks.db", &db);

    if (rc != SQLITE_OK) {
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return 1;
    }

    const char *sql = "SELECT * FROM tasks WHERE desc LIKE ?;";
    sqlite3_stmt *stmt;

    rc = sqlite3_prepare_v2(db, sql, -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return 1;
    }

    char search_pattern[110];
    snprintf(search_pattern, sizeof(search_pattern), "%%%s%%", task_keyword);
    
    sqlite3_bind_text(stmt, 1, search_pattern, -1, SQLITE_STATIC);

    int found = 0;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        struct Task task;
        task.uid = sqlite3_column_int(stmt, 0);
        
        const char *desc_text = (const char *)sqlite3_column_text(stmt, 1);
        strncpy(task.desc, desc_text, sizeof(task.desc) - 1);
        task.desc[sizeof(task.desc) - 1] = '\0';  // Ensure null termination
        
        task.status = sqlite3_column_int(stmt, 2);
        
        printf("Task Found:\n");
        printf("UID: %d\n", task.uid);
        printf("Description: %s\n", task.desc);
        printf("Status: %s\n", task.status ? "Completed" : "Pending");
        found = 1;
    }
    
    if (!found) {
        printf("No task found with description containing: %s\n", task_keyword);
    }

    sqlite3_finalize(stmt);
    sqlite3_close(db);
    return 0;
}

int delete_task(char* task){
    sqlite3 *db;
    sqlite3_stmt *stmt;
    char *zErrMsg = 0;
    int rc;

    rc = sqlite3_open("tasks.db", &db);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
        return 1;
    }

    const char *sql = "DELETE FROM tasks WHERE desc = ?";

    rc = sqlite3_prepare_v2(db, sql, -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Delete prepare error: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return 1;
    }

    sqlite3_bind_text(stmt, 1, task, -1, SQLITE_STATIC);
    
    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        fprintf(stderr, "Delete execution error: %s\n", sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        sqlite3_close(db);
        return 1;
    }
    
    int changes = sqlite3_changes(db);
    if (changes > 0) {
        printf("Task deleted successfully. Tasks affected: %d\n", changes);
    } else {
        printf("No task found with description: %s\n", task);
    }

    sqlite3_finalize(stmt);
    sqlite3_close(db);
    return 0;
}

int update_task(char* task){
    sqlite3 *db;
    sqlite3_stmt *stmt;
    char *zErrMsg = 0;
    int rc;

    rc = sqlite3_open("tasks.db", &db);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
        return 1;
    }

    const char *sql = "UPDATE tasks SET status = 1 WHERE desc = ?";

    rc = sqlite3_prepare_v2(db, sql, -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Update prepare error: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return 1;
    }

    sqlite3_bind_text(stmt, 1, task, -1, SQLITE_STATIC);
    
    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        fprintf(stderr, "Update execution error: %s\n", sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        sqlite3_close(db);
        return 1;
    }
    
    int changes = sqlite3_changes(db);
    if (changes > 0) {
        printf("Task marked as complete. Tasks affected: %d\n", changes);
    } else {
        printf("No task found with description: %s\n", task);
    }

    sqlite3_finalize(stmt);
    sqlite3_close(db);
    return 0;
}

int list_tasks() {
    sqlite3 *db;
    sqlite3_stmt *stmt;
    char *zErrMsg = 0;
    int rc;

    rc = sqlite3_open("tasks.db", &db);
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

    printf("--- TASK LIST ---\n");
    int count = 0;
    
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        struct Task task;
        task.uid = sqlite3_column_int(stmt, 0);
        
        const char *desc_text = (const char *)sqlite3_column_text(stmt, 1);
        strncpy(task.desc, desc_text, sizeof(task.desc) - 1);
        task.desc[sizeof(task.desc) - 1] = '\0';  // Ensure null termination
        
        task.status = sqlite3_column_int(stmt, 2);
        
        printf("Task #%d:\n", ++count);
        printf("  UID: %d\n", task.uid);
        printf("  Description: %s\n", task.desc);
        printf("  Status: %s\n\n", task.status ? "Completed" : "Pending");
    }
    
    if (count == 0) {
        printf("No tasks found.\n");
    }

    sqlite3_finalize(stmt);
    sqlite3_close(db);
    return 0;
}

void help(){
    printf("--- TASK MANAGER COMMANDS ---\n");
    for (int i = 0; i < 6; i++) {
        printf("%s: %s\n", cli_args[i].arg, cli_args[i].desc);
    }
}

void parse_args(char *arg, char *task){
    for (int i = 0; i < 6; i++) {
        if (strcmp(arg, cli_args[i].arg) == 0) {
            switch (cli_args[i].id) {
            case 1: // add
                if (task == NULL) {
                    printf("Error: 'add' command requires a task description\n");
                    return;
                }
                add_task(task);
                break;
            case 2: // remove
                if (task == NULL) {
                    printf("Error: 'remove' command requires a task description\n");
                    return;
                }
                delete_task(task);
                break;
            case 3: // list
                list_tasks();
                break;
            case 4: // complete
                if (task == NULL) {
                    printf("Error: 'complete' command requires a task description\n");
                    return;
                }
                update_task(task);
                break;
            case 5: // help
                help();
                break;
            case 6: // search
                if (task == NULL) {
                    printf("Error: 'search' command requires a keyword\n");
                    return;
                }
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
    sqlite3 *db;
    char *errMsg = 0;
    int rc;

    if (argc < 2) {
        printf("Usage: codo <command> [task_description]\n");
        help();
        return 1;
    }

    rc = sqlite3_open("tasks.db", &db);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
        return 1;
    }

    rc = sqlite3_exec(db, "CREATE TABLE IF NOT EXISTS tasks (uid INTEGER PRIMARY KEY, desc TEXT NOT NULL, status INTEGER NOT NULL);", NULL, 0, &errMsg);

    if (rc != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\n", errMsg);
        sqlite3_free(errMsg);
        sqlite3_close(db);
        return 1;
    }
    
    sqlite3_close(db);
    
    char *task = (argc >= 3) ? argv[2] : NULL;
    parse_args(argv[1], task);
    
    return 0;
}