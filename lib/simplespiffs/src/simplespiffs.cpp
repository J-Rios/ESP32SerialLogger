/**************************************************************************************************/
// File: simplespiffs.cpp
// Description: HAL library to ease SPIFFS usage.
// Created on: 22 dec. 2018
// Last modified date: 25 dec. 2018
// Version: 0.0.1
/**************************************************************************************************/

/* Libraries */

#include "simplespiffs.h"

/**************************************************************************************************/

/* Resource Safe Access Mutex Macro */

#if !FREERTOS_MUTEX
    #define MUTEX_INIT() do { } while (0)
    #define SAFE(x) do { x; } while (0)
#else
    #define MUTEX_INIT() do { this_mutex = xSemaphoreCreateMutex(); } while (0)
    #define SAFE(x) do \
    { \
        if(xSemaphoreTake(this_mutex, (portTickType)10)==pdTRUE) \
        { \
            x; \
            xSemaphoreGive(this_mutex); \
        } \
    } while (0)
#endif

/**************************************************************************************************/

/* Constructor */

// SimpleSPIFFS constructor
SimpleSPIFFS::SimpleSPIFFS(void)
{
    // Set SPIFFS properties configuration
    spiffs_conf.base_path = "";
    spiffs_conf.partition_label = NULL;
    spiffs_conf.max_files = 5;
    spiffs_conf.format_if_mount_failed = true;

    // Initialize SPIFFS access Mutex
    MUTEX_INIT();
}

/**************************************************************************************************/

/* Public Methods */

// Mount SPIFFS
bool SimpleSPIFFS::mount(void)
{
    bool mount_ok = false;

    SAFE
    (
        esp_err_t ret = esp_vfs_spiffs_register(&spiffs_conf);
        if(ret != ESP_OK)
        {
            if (ret == ESP_FAIL)
                debug("SPIFFS - Failed to mount filesystem.\n");
            else if (ret == ESP_ERR_NOT_FOUND)
                debug("SPIFFS - Failed to find reserved FileSystem partition in Flash.\n");
            else
                debug("SPIFFS - Failed to mount SPIFFS: %s\n", esp_err_to_name(ret));

            mount_ok = false;
        }
        else
        {
            size_t total = 0;
            size_t used = 0;
            ret = esp_spiffs_info(NULL, &total, &used);
            if (ret != ESP_OK)
                debug("SPIFFS - Can't get FS partition information: %s\n", esp_err_to_name(ret));
            else
                debug("SPIFFS - Partition usage: %d bytes from %d bytes.\n", used, total);
            
            mount_ok = true;
        }
    );

    return mount_ok;
}

// Unmount SPIFFS
bool SimpleSPIFFS::unmount(void)
{
    bool unmount_ok = false;

    SAFE
    (
        esp_err_t ret = esp_vfs_spiffs_unregister(NULL);
        if(ret != ESP_OK)
            debug("SPIFFS - Can't unmount FS partition: %s\n", esp_err_to_name(ret));
        else
        {
            debug("SPIFFS - FileSystem successfully Unmounted.\n");
            unmount_ok = true;
        }
    );

    return unmount_ok;
}

// Check if a file exists
bool SimpleSPIFFS::file_exists(const char* path)
{
    bool exists = false;

    SAFE
    (
        static struct stat st;
        if(stat(path, &st) == 0)
            exists = true;
    );

    return exists;
}

// Count number of lines that a file contains
int32_t SimpleSPIFFS::file_count_lines(const char* path)
{
    uint16_t num_lines = -1;

    SAFE
    (
        FILE* f = fopen(path, "r");
        if(f != NULL)
        {
            char line[MAX_LENGHT_SPIFFS_LINE];
            num_lines = 0;
            while(fgets(line, MAX_LENGHT_SPIFFS_LINE, f) != NULL)
                num_lines = num_lines + 1;
            fclose(f);
        }
        else
            debug("SPIFFS - Error, can't open file for count number of lines.\n");
    );

    return num_lines;
}

// Get file size (in Bytes)
size_t SimpleSPIFFS::file_size(const char* path)
{
    long f_size = 0;

    SAFE
    (
        FILE* f = fopen(path, "rb");
        if(f != NULL)
        {
            fseek(f, 0, SEEK_END);
            f_size = ftell(f);

            fclose(f);
        }
    );

    return f_size;
}

// Read all file content
bool SimpleSPIFFS::file_read(const char* path, char* read, const size_t read_len)
{
    bool read_ok = false;
    long f_size = 0;

    SAFE
    (
        FILE* f = fopen(path, "r");
        if(f != NULL)
        {
            memset(read, '\0', read_len);

            // Check file size
            fseek(f, 0, SEEK_END);
            f_size = ftell(f);
            fseek(f, 0, SEEK_SET);

            // Read file content
            if(f_size > read_len-1)
            {
                fread(read, read_len-1, 1, f);
                read[read_len-1] = '\0';

                debug("SPIFFS - Warning, file too large to be read into read buffer.\n");
            }
            else
            {
                fread(read, f_size, 1, f);
                read[f_size] = '\0';
                read_ok = true;
            }

            fclose(f);
        }
        else
            debug("SPIFFS - Error, can't open file for reading.\n");
    );

    return read_ok;
}

// Read specified line of a file (line_num start at 0 and goes to 65535)
bool SimpleSPIFFS::file_read_line(const char* path, const uint16_t line_num, char* read_line, const size_t read_len)
{
    bool read_ok = false;

    SAFE
    (
        FILE* f = fopen(path, "r");
        if(f != NULL)
        {
            for(uint16_t i = 0; i <= line_num; i++)
            {
                memset(read_line, '\0', read_len);
                if(fgets(read_line, read_len, f) == NULL)
                {
                    // If error is caused by reach End-Of-File, exit the loop
                    if(feof(f))
                    {
                        read_ok = true;
                        break;
                    }
                }
                else
                {
                    // If there is no error when reading the expected line
                    if(i == line_num)
                        read_ok = true;
                }
            }
            fclose(f);

            // Remove newline character (if any)
            char* eol_position = strchr(read_line, '\n');
            if(eol_position != NULL)
                *eol_position = '\0';
        }
        else
            debug("SPIFFS - Error, can't open file for reading.\n");
    );

    return read_ok;
}

// Read all file content
bool SimpleSPIFFS::file_write(const char* path, const char* write)
{
    bool write_ok = false;

    // Write to file
    SAFE
    (
        FILE* f = fopen(path, "w");
        if (f != NULL)
        {
            write_ok = true;
            fprintf(f, "%s", write);
            fclose(f);
        }
        else
            debug("SPIFFS - Error, can't open file for writing.\n");
    );

    return write_ok;
}

// Write to file
bool SimpleSPIFFS::file_write_line(const char* path, char* text_line)
{
    bool write_ok = false;

    // If the input text string doesnt has an End-Of-Line, add it
    if(text_line[strlen(text_line)-1] != '\n')
    {
        if(strlen(text_line) < MAX_LENGHT_SPIFFS_LINE)
        {
            text_line[strlen(text_line)-1] = '\n';
            text_line[strlen(text_line)] = '\0';
        }
        else
        {
            text_line[strlen(text_line)-2] = '\n';
            text_line[strlen(text_line)-1] = '\0';
        }
    }

    // Write to file
    SAFE
    (
        FILE* f = fopen(path, "w");
        if (f != NULL)
        {
            write_ok = true;
            fprintf(f, text_line);
            fclose(f);
        }
        else
            debug("SPIFFS - Error, can't open file for writing.\n");
    );

    return write_ok;
}

// Append to file
bool SimpleSPIFFS::file_append_line(const char* path, char* text_line)
{
    bool append_ok = false;

    // If the input text string doesnt has an End-Of-Line, add it
    if(text_line[strlen(text_line)-1] != '\n')
    {
        if(strlen(text_line) < MAX_LENGHT_SPIFFS_LINE)
        {
            text_line[strlen(text_line)-1] = '\n';
            text_line[strlen(text_line)] = '\0';
        }
        else
        {
            text_line[strlen(text_line)-2] = '\n';
            text_line[strlen(text_line)-1] = '\0';
        }
    }

    // Append line
    SAFE
    (
        FILE* f = fopen(path, "a");
        if (f != NULL)
        {
            append_ok = true;
            fprintf(f, text_line);
            fclose(f);
        }
        else
            debug("SPIFFS - Error, can't open file for appending.\n");
    );

    return append_ok;
}

// Move/Rename a file
bool SimpleSPIFFS::file_move_rename(const char* path1, const char* path2)
{
    bool rename_ok = false;

    SAFE
    (
        if(rename(path1, path2) == 0)
            rename_ok = true;
        else
            debug("SPIFFS - Error, can't rename file %s.\n", path1);
    );

    return rename_ok;
}

// Remove a file
bool SimpleSPIFFS::file_remove(const char* path)
{
    bool remove_ok = false;

    SAFE
    (
        struct stat st;
        if(stat(path, &st) == 0)
        {
            if(unlink(path) != 0)
                debug("SPIFFS - Error, can't remove remove file %s.\n", path);
            else
                remove_ok = true;
        }
        else
            debug("SPIFFS - Error, can't remove file %s: File not found.\n", path);
    );

    return remove_ok;
}
