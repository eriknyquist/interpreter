#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

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

    fseek(fp, 0, SEEK_END);
    file_size_bytes_long = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    if (SIZE_MAX <= (size_t) file_size_bytes_long)
    {
        return FILE_READER_FILE_TOO_LARGE;
    }

    size_t file_size_bytes = (size_t) file_size_bytes_long;

    if ((file->data = malloc(file_size_bytes + 1u)) == NULL)
    {
        return FILE_READER_MEMORY_ERROR;
    }

    size_t bytes_read = fread(file->data, 1u, file_size_bytes, fp);
    if (bytes_read != file_size_bytes)
    {
        return FILE_READER_READ_ERROR;
    }

    fclose(fp);

    file->size_bytes = bytes_read;
    file->data[file_size_bytes] = '\0';

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

    if (NULL != file->data)
    {
        free(file->data);
        file->data = NULL;
        file->size_bytes = 0u;
    }

    return FILE_READER_OK;
}
