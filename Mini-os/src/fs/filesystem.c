#include "filesystem.h"
#include <stdint.h>

// Simple string functions for filesystem
static void fs_strcpy(char *dest, const char *src) {
    while (*src) {
        *dest++ = *src++;
    }
    *dest = '\0';
}

static int fs_strcmp(const char *s1, const char *s2) {
    while (*s1 && (*s1 == *s2)) {
        s1++;
        s2++;
    }
    return *(unsigned char *)s1 - *(unsigned char *)s2;
}

static int fs_strlen(const char *str) {
    int len = 0;
    while (*str++) len++;
    return len;
}

static file_t files[MAX_FILES];

// Initialize filesystem with default files
void fs_init(void) {
    for(int i = 0; i < MAX_FILES; i++) {
        files[i].used = 0;
        files[i].size = 0;
        files[i].name[0] = '\0';
    }
    
    // Create some default files
    fs_create_file("readme.txt");
    fs_write_file("readme.txt", "Welcome to MyOS!\nType 'help' for commands.");
}

// Create new file with given name
int fs_create_file(const char *name) {
    for(int i = 0; i < MAX_FILES; i++) {
        if(!files[i].used) {
            fs_strcpy(files[i].name, name);
            files[i].used = 1;
            files[i].size = 0;
            files[i].content[0] = '\0';
            return i;
        }
    }
    return -1; // No space
}

// Write content to existing file
int fs_write_file(const char *name, const char *content) {
    for(int i = 0; i < MAX_FILES; i++) {
        if(files[i].used && fs_strcmp(files[i].name, name) == 0) {
            fs_strcpy(files[i].content, content);
            files[i].size = fs_strlen(content);
            return i;
        }
    }
    return -1; // File not found
}

// Read content from file into buffer
int fs_read_file(const char *name, char *buffer) {
    for(int i = 0; i < MAX_FILES; i++) {
        if(files[i].used && fs_strcmp(files[i].name, name) == 0) {
            fs_strcpy(buffer, files[i].content);
            return files[i].size;
        }
    }
    return -1; // File not found
}

// List all files with their sizes
int fs_list_files(char *buffer) {
    int pos = 0;
    
    // Add header
    const char *header = "Files:\n";
    while (*header) {
        buffer[pos++] = *header++;
    }
    
    // List each file with size
    for(int i = 0; i < MAX_FILES; i++) {
        if(files[i].used) {
            buffer[pos++] = '-';
            buffer[pos++] = ' ';
            
            const char *name = files[i].name;
            while (*name) {
                buffer[pos++] = *name++;
            }
            
            buffer[pos++] = ' ';
            buffer[pos++] = '(';
            
            // Convert size to string
            int size = files[i].size;
            char size_str[16];
            int size_len = 0;
            int temp = size;
            
            if (temp == 0) {
                size_str[size_len++] = '0';
            } else {
                while (temp > 0) {
                    size_len++;
                    temp /= 10;
                }
                temp = size;
                for (int j = size_len - 1; j >= 0; j--) {
                    size_str[j] = '0' + (temp % 10);
                    temp /= 10;
                }
            }
            size_str[size_len] = '\0';
            
            for (int j = 0; j < size_len; j++) {
                buffer[pos++] = size_str[j];
            }
            
            const char *footer = " bytes)\n";
            while (*footer) {
                buffer[pos++] = *footer++;
            }
        }
    }
    buffer[pos] = '\0';
    return pos;
}

// Delete file by name
int fs_delete_file(const char *name) {
    for(int i = 0; i < MAX_FILES; i++) {
        if(files[i].used && fs_strcmp(files[i].name, name) == 0) {
            files[i].used = 0;
            return 0;
        }
    }
    return -1; // File not found
}