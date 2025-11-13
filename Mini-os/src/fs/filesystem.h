#ifndef FILESYSTEM_H
#define FILESYSTEM_H

#define MAX_FILES 32
#define MAX_FILENAME 32
#define FILE_CONTENT_SIZE 1024

typedef struct {
    char name[MAX_FILENAME];
    char content[FILE_CONTENT_SIZE];
    int size;
    int used;
} file_t;

// Filesystem functions
void fs_init(void);                              // Initialize filesystem
int fs_create_file(const char *name);           // Create new file
int fs_write_file(const char *name, const char *content);  // Write to file
int fs_read_file(const char *name, char *buffer);         // Read from file
int fs_list_files(char *buffer);                // List all files
int fs_delete_file(const char *name);           // Delete file

#endif
