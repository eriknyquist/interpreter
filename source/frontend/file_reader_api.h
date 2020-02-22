#ifndef FILE_READER_API_H
#define FILE_READER_API_H

typedef struct
{
    char *data;
    size_t size_bytes;
} file_in_memory_t;


typedef enum
{
    FILE_READER_OK,
    FILE_READER_INVALID_PARAM,
    FILE_READER_MEMORY_ERROR,
    FILE_READER_OPEN_ERROR,
    FILE_READER_READ_ERROR,
    FILE_READER_FILE_TOO_LARGE,
    FILE_READER_ERROR
} file_reader_status_e;


/**
 * Open the filename given and load the file data into the given file_in_memory_t
 * instance
 *
 * @param   file      Pointer to file_in_memory_t instance to populate
 * @param   filename  Pointer to name of file to load
 *
 * @return  FILE_READER_OK if file was loaded successfully
 */
file_reader_status_e file_reader_load(file_in_memory_t *file, const char *filename);


/**
 * Destroy file_in_memory_t instance. Frees any memory allocated for file data.
 *
 * @param   file      Pointer to file_in_memory_t instance to destroy
 *
 * @return  FILE_READER_OK if file was destroyed successfully
 */
file_reader_status_e file_reader_destroy(file_in_memory_t *file);


#endif /* FILE_READER_API_H */
