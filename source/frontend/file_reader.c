#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "file_reader_api.h"


/**
 * @see file_reader_api.h
 */
file_reader_status_e file_reader_load(file_in_memory_t *file, const char *filename)
{
    FILE *fp;

    if ((NULL == filename) || (NULL == file))
    {
        return FILE_READER_INVALID_PARAM;
    }

    if ((fp = fopen(filename, "rb")) == NULL)
    {
        return FILE_READER_OPEN_ERROR;
    }

    long file_size_bytes_long;

    // Get file size in bytes
    fseek(fp, 0, SEEK_END);
    file_size_bytes_long = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    if (SIZE_MAX <= (size_t) file_size_bytes_long)
    {
        return FILE_READER_FILE_TOO_LARGE;
    }

    size_t file_size_bytes = (size_t) file_size_bytes_long;
    size_t filename_len = strlen(filename);

    // Allocate space to store filename + file data
    char *file_data;
    if ((file_data = malloc(file_size_bytes + filename_len + 2u)) == NULL)
    {
        return FILE_READER_MEMORY_ERROR;
    }

    // Copy filename to allocated block
    memcpy(file_data, filename, filename_len);
    file_data[filename_len] = '\0';
    file->filename = file_data;

    // Read file data into allocated block
    file_data += filename_len + 1u;
    size_t bytes_read = fread(file_data, 1u, file_size_bytes, fp);
    if (bytes_read != file_size_bytes)
    {
        return FILE_READER_READ_ERROR;
    }

    file_data[file_size_bytes] = '\0';
    file->data = file_data;
    file->size_bytes = bytes_read;

    fclose(fp);

    return FILE_READER_OK;
}


/**
 * @see file_reader_api.h
 */
file_reader_status_e file_reader_destroy(file_in_memory_t *file)
{
    if (NULL == file)
    {
        return FILE_READER_INVALID_PARAM;
    }

    if (NULL != file->filename)
    {
        free(file->filename);
        file->data = NULL;
        file->filename = NULL;
        file->size_bytes = 0u;
    }

    return FILE_READER_OK;
}
