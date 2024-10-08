#include "fat_filelib.h"
#include "kheap.h"
#include "logging.h"
#include "stdbool.h"
#include "file.h"
#include "stdio.h"
#include "io_ports.h"
#include "virtual/devices.h"
#include "string.h"
#include "stdlib.h"
#include "printf.h"
extern bool fs_active;
char *_cwd;
size_t _cwd_len;
char _fixed_cwd[FATFS_MAX_LONG_FILENAME];
bool use_cwd = true;
int stdin = STDIN_VALUE;
int stdout = STDOUT_VALUE;
int stderr = STDERR_VALUE;
drive_info drives[24];
uint32_t open_files[FATFS_MAX_OPEN_FILES];
uint32_t open_files_count = 0;
int init_fs()
{
    if(fs_active != true)
    {
        // printf("Ret -1\n");
        return -1;
    }
    _cwd = kmalloc(FATFS_MAX_LONG_FILENAME);
    if(_cwd == NULL)
    {
        printf("%d %d %s %s %s",3,__LINE__,__func__,__FILE__,"FS: Failed to allocate memory for _cwd. Defaulting to fixed size buffer\nUsing fixed size buffer");
        use_cwd = false;

        return -1;
    }
    else
    {
        memset(_cwd, 0, sizeof(FATFS_MAX_LONG_FILENAME));
        _cwd[0] = '/';
        _cwd_len = 1;
        use_cwd = true;
        return 1;
    }
    
}

char* remove_double_path_separators(char* path) {
    // Check if the path is empty or null
    if (path == NULL || strlen(path) == 0) {
        return path;
    }

    int len = strlen(path);
    char* result = (char*)malloc((len + 1) * sizeof(char)); // Allocate memory for the result string
    if (result == NULL) {
        return NULL; // Memory allocation failed
    }

    int i, j;
    for (i = 0, j = 0; i < len; i++) {
        // If current character is a path separator
        if (path[i] == '/' || path[i] == '\\') {
            // If the next character is also a path separator, skip it
            if (i + 1 < len && (path[i + 1] == '/' || path[i + 1] == '\\')) {
                continue;
            }
        }
        // Copy character to the result string
        result[j++] = path[i];
    }
    result[j] = '\0'; // Add null terminator to end the string

    return result;
}
int chdir(const char *path)
{
    // //printf("Path to chdir: %s\n", path);
    if(use_cwd == true)
    {
        if(strcmp(path, "..") == 0)
        {
            char *new = move_back_one_folder(_cwd);
            strcpy(_cwd, new);
            kfree(new);
            return 0;
        }
        if(path[0] == '/')
        {
            strcpy(_cwd, path);
            return 0;
        }
        char *tmp = kmalloc(_cwd_len+strlen(path)+3);
        // strcat(tmp,"");
        // //printf("cwd %s | tmp %s\n",_cwd,tmp);
        strcpy(tmp,_cwd);
        if(tmp[_cwd_len] != '/')
        {
            dbgprintf("error\n");
            strcat(tmp,"/");
        }
         //printf("cwd %s | tmp %s\n",_cwd,tmp);
        strcat(tmp,path);
        tmp = remove_double_path_separators(tmp);
         //printf("cwd %s | tmp %s\n",_cwd,tmp);
         //printf("TEMP = %s\n",tmp);
        if(fl_is_dir(tmp) == 1)
        {
            //printf("cwd [%s] tmp [%s]\n",_cwd,tmp);
            strcpy(_cwd,tmp);
            //printf("cwd: [%s]\n",_cwd);
            _cwd_len = _cwd_len+1+strlen(path)+2;
            kfree(tmp);
            return 0;
        }
        //printf("HEre\n");
        kfree(tmp);
        return -1;
    }
    else
    {
        dbgprintf("Fixed path\n");
        char tmp[FATFS_MAX_LONG_FILENAME];
        strcpy(tmp,_fixed_cwd);
        strcat(tmp,"/");
        strcat(tmp,path);
        
    }

}

char *getcwd()
{
    if(fs_active != true)
    {
        char *none = "/";
        return none;
    }
    if(use_cwd)
    {
        return _cwd;
    }
    else
    {
        return _fixed_cwd;
    }
}
int is_valid_char(char c) {
    return isalnum(c) || c == '_' || c == '-';
}
// Function to filter the current working directory
void filter_cwd(char* cwd) {
    char filtered_cwd[FATFS_MAX_LONG_FILENAME];
    int j = 0;

    for (int i = 0; cwd[i] != '\0'; i++) {
        if (is_valid_char(cwd[i])) {
            filtered_cwd[j++] = cwd[i];
        }
    }

    filtered_cwd[j] = '\0';
    // printf("len(filtered_cwd) == %d",strlen(filtered_cwd));
    if(strlen(filtered_cwd) == 0)
    {
        filtered_cwd[0] = ' ';
    filtered_cwd[1] = '\0';
    }
    
    strcpy(cwd, filtered_cwd);
}
void remove_leading_whitespace(char* path) {
    char* trimmed_path = path;

    // Find the first non-whitespace character
    while (isspace((unsigned char)*trimmed_path)) {
        trimmed_path++;
    }

    // Move the trimmed path to the beginning of the original string
    memmove(path, trimmed_path, strlen(trimmed_path) + 1);
}
char* process_path(const char* input_string) {
    // Check if the input string starts with '/'
    if (input_string[0] == '/' || input_string[0] == '.' ) {
        // Allocate memory and return the input string as is
        char* result = (char*)malloc(strlen(input_string) + 1);
        if (result == NULL) {
            perror("malloc");
            return NULL;
            // exit(EXIT_FAILURE);
        }
        strcpy(result, input_string);
        return result;
    } else {
        // Get the current working directory
        char cwd[FATFS_MAX_LONG_FILENAME];
        if (getcwd(cwd, sizeof(cwd)) == NULL) {
            perror("getcwd");
            return NULL;
        }

        // Filter the current working directory to remove invalid characters
        filter_cwd(cwd);

        // Allocate memory for the concatenated result
        size_t result_size = strlen(cwd) + strlen(input_string) + 2; // +2 for '/' and '\0'
        char* result = (char*)malloc(result_size);
        if (result == NULL) {
            perror("malloc");
                        return NULL;

        }

        // Construct the full path
        // printf("\nlen(filtered_cwd) == %d\n",strlen(cwd));

        snprintf(result, result_size, "%s/%s", cwd, input_string);
        remove_leading_whitespace(result);
        return result;
    }
}

void *fopen(const char *path,const char *modifiers)
{
    
    char *path_to_open = malloc(strlen(path) + 100);
    // printf("path_to_open = %p\n",path_to_open);
    // printf("path_fake = %p\n",path_fake);
    memset(path_to_open, 0, strlen(path) + 100);
    if(path_to_open == NULL)
    {
        printf("Couldn't allocate memory for path\n");
        return NULL;
    }
    if(is_virtual_device_path(path) == 1)
    {
        return handle_virtual_device_fopen(path,modifiers);
    }
    else if(0 == 0)//If fat32 or ext#, ATM only fat works
    {
        if(fs_active != true)
    {
        return -1;
    }
        if(path[0] != '/')
        {
            char *cwd_path = _cwd;
            strcat(path_to_open, cwd_path);
            strcat(path_to_open,"/");
            strcat(path_to_open,path);
        }
        // // printf("Path to open == %s\n",path_to_open);
        char* result = process_path(path_to_open);
        // printf("Opening file [%s]\n",result);
        void * ret = fl_fopen(result,modifiers);
        if(ret != NULL)
        {
            open_files[open_files_count] = ret;
            open_files_count++;
        }
        free(result);
        // free(path_tmp);makemake
        return ret;
    }
    else if (2 == 1)
    {
        //EXT not supported yet
    }
    
}
/**
 * Function Name: fread
 * Description: Reads data from the given file stream.
 *
 * Parameters:
 *   ptr (void*) - Pointer to the memory area where data will be read into.
 *   size (size_t) - Size of each element to be read.
 *   nmemb (size_t) - Number of elements to read.
 *   stream (FILE*) - Pointer to a FILE object that specifies the stream to read from.
 *
 * Return:
 *   size_t - Number of elements successfully read.
 */
size_t fread(void *ptr, size_t size, size_t nmemb, void *stream) {

    
    if(is_virtual_device(stream) == 1)
    {
        return device_read(ptr, size, nmemb, stream);
    }
    if((int)stream == stdin)
    {   
        char *buf = (char *)ptr;
        for (size_t i = 0; i < nmemb; i++)
        {
            char chr;
            while(chr = getchar() <= 0)
            {

            }
            buf[i] = chr;
        }
        ptr = buf;
        
    }
    else if(0 == 0)
    {
        if(fs_active != true)
    {
        return -1;
    }
        return fl_fread(ptr,size,nmemb,stream);
    }
    else if (0 == 2)
    {
        // EXT not supported
    }
    
    return 0;
}

/**
 * Function Name: fwrite
 * Description: Writes data to the given file stream.
 *
 * Parameters:
 *   ptr (const void*) - Pointer to the array containing the data to be written.
 *   size (size_t) - Size of each element to be written.
 *   nmemb (size_t) - Number of elements to write.
 *   stream (FILE*) - Pointer to a FILE object that specifies the stream to write to.
 *
 * Return:
 *   size_t - Number of elements successfully written.
 */
size_t fwrite(const void *ptr, size_t size, size_t nmemb, void *stream) {
    
    if(is_virtual_device(stream))
    {
        printf("is virtual device\n");
        return device_write(ptr, size, nmemb, stream);
    }
    if((int)stream == stdout)
    {
        char *buf = (char *)ptr;
        for (size_t i = 0; i < nmemb; i++)
        {
            printf("%c", buf[i]);
        }
        buf = NULL;
        return nmemb;
        

    }
    else if ((int)stream == stderr)
    {
        if(fs_active != true)
    {
        return -1;
    }
        char *buf = (char *)ptr;
        for (size_t i = 0; i < nmemb; i++)
        {
            dbgprintf("%c",buf[i]);
        }
        return nmemb;

        
        
    }
    else
    {
        if(0 == 0)
        {
            return fl_fwrite(ptr,size,nmemb,stream);
        }
        else
        {
            //EXT not supported
        }
    }
    return 0;
}

/**
 * Function Name: fputc
 * Description: Writes a character to the specified stream.
 *
 * Parameters:
 *   c (int) - The character to be written.
 *   stream (FILE*) - Pointer to a FILE object that specifies the stream to write to.
 *
 * Return:
 *   int - On success, returns the written character as an unsigned char cast to an int.
 *         On failure, returns EOF.
 */
int fputc(int c, void *stream) {
    
    if((int)stream == stdout)
    {
        _putchar(c);
        return c;
    }
    else if ((int)stream == stderr)
    {
        write_to_com1(c);
        return c;
    }
    else
    {
        if(fs_active != true)
    {
        return -1;
    }
        printf("%c",c);
        return fl_fputc(c,stream);
        
    }
    
    // Empty implementation
    return EOF;
}

/**
 * Function Name: fputs
 * Description: Writes a string to the specified stream.
 *
 * Parameters:
 *   str (const char*) - Pointer to the null-terminated string to be written.
 *   stream (FILE*) - Pointer to a FILE object that specifies the stream to write to.
 *
 * Return:
 *   int - On success, returns a non-negative value.
 *         On failure, returns EOF.
 */
int fputs(const char *str, void *stream) {
    
    if((int)stream == stdout)
    {
        for(int i = 0; i < strlen(str); i++)
        {
            fputc(str[i], stream);
        }
        // printf("%s", str);
        return 1;
    }
    else if ((int)stream == stderr)
    {
        printf("%s", str);
        dbgprintf("%s", str);
        printf_debug("%s", str);
        return 1;
    }
    else if ((int)stream != stdin)
    {
        if(fs_active != true)
    {
        return -1;
    }
        return fl_fputs(str,stream);
    }
    else
    {
        return NULL;
    }
    
    
    // Empty implementation
    return EOF;
}

/**
 * Function Name: fclose
 * Description: Closes the given file stream.
 *
 * Parameters:
 *   stream (FILE*) - Pointer to a FILE object to be closed.
 *
 * Return:
 *   int - On success, returns 0.
 *         On failure, returns EOF.
 */
int fclose(void *stream) {
    if(fs_active != true)
    {
        return -1;
    }
    for (size_t i = 0; i < open_files_count; i++)
    {
        FL_FILE *f = (FL_FILE *)stream;
        FL_FILE *tmp = (FL_FILE *)open_files[i];
        if(strcmp(f->path,tmp->path) == 0 && strcmp(tmp->filename,f->filename) == 0)
        {
            open_files[i] = NULL;
            for (size_t j = i; j < open_files_count - 1; j++) {
                open_files[j] = open_files[j + 1];
            }
            open_files[open_files_count - 1] = NULL; // Set the last element to NULL
            open_files_count--;
        }
    }
    
    fl_fclose(stream);
     return 1;
}

/**
 * Function Name: fseek
 * Description: Sets the file position indicator for the given file stream.
 *
 * Parameters:
 *   stream (FILE*) - Pointer to a FILE object that specifies the stream.
 *   offset (long) - Number of bytes to offset from the origin.
 *   whence (int) - Position from where offset is added (SEEK_SET, SEEK_CUR, SEEK_END).
 *
 * Return:
 *   int - On success, returns 0.
 *         On failure, returns non-zero value.
 */
int fseek(void *stream, long offset, int whence) {
    
    if((int)stream == stdin)
    {
        long new_position;
        switch(whence) {
        case SEEK_SET:
            new_position = offset;
            break;
        case SEEK_CUR:
            new_position = offset + ftell(stream); // Assuming ftell() gives the current position
            break;
        case SEEK_END:
            logging(3,__LINE__,__func__,__FILE__,"ATTEMPT TO READ OUT OF BUFFER");
            return -1;
            // new_position = buffer_size + offset;
            // break;
        default:
            return -1; // Invalid origin
        
        set_io_pos(new_position);
    }
    }
    if(fs_active != true)
    {
        return -1;
    }
    return fl_fseek(stream,offset,whence);
    // return -1;
}

/**
 * Function Name: fgetpos
 * Description: Gets the current file position.
 *
 * Parameters:
 *   stream (FILE*) - Pointer to a FILE object that specifies the stream.
 *   pos (fpos_t*) - Pointer to store the current file position.
 *
 * Return:
 *   int - On success, returns 0.
 *         On failure, returns a non-zero value.
 */
int fgetpos(void *stream, fpos_t *pos) {
    if(fs_active != true)
    {
        return -1;
    }
    return fl_fgetpos(stream, pos);
    // Empty implementation
    // return -1;
}

/**
 * Function Name: ftell
 * Description: Gets the current file position.
 *
 * Parameters:
 *   stream (FILE*) - Pointer to a FILE object that specifies the stream.
 *
 * Return:
 *   long - On success, returns the current offset in the stream.
 *          On failure, returns -1L.
 */
long ftell(void *stream) {
    
    if((int)stream == stdin)
    {
        return get_io_pos();
    }
    if(fs_active != true)
    {
        return -1;
    }
    return fl_ftell(stream);
    // Empty implementation
    // return -1L;
}

/**
 * Function Name: feof
 * Description: Checks if the end-of-file indicator associated with the stream is set.
 *
 * Parameters:
 *   stream (FILE*) - Pointer to a FILE object that specifies the stream.
 *
 * Return:
 *   int - Returns non-zero if end-of-file indicator is set, otherwise returns 0.
 */
int feof(void *stream) {
    if(fs_active != true)
    {
        return -1;
    }
    return fl_feof(stream);

}
char *fgets(char *s, int n, void *f) {
    if ((int)f == stdin) {
        char ch;
        int count = 0;
        while (((ch = getch_()) > 0 && ch != '\n' && ch != EOF) && count < n - 1) {
            if (ch == '\b') {
                if (count > 0) { // Ensure not underflowing the buffer
                    *s = '\0';
                    s--;
                    *s = '\0';

                    // s--;
                    count--;
                    // s[count] = ' ';
                    printf("\b");
                    dbgprintf(">>%s\n",s);
                }
            } else {
                printf("%c", ch);
                *s = ch;
                s++;
                count++;
            }
        }
        int point = strlen(s);
        // printf("Point: %d\n", point);
        s[count] = '\0'; // Null-terminate the string
    } else {
        if(fs_active != true)
    {
        return -1;
    }
        return fl_fgets(s, n, f); // Use the standard library fgets for other file streams
    }
    return s; // Return the pointer to the string
}
/**
 * Function Name: fgetc
 * Description: Implementation of fgetc to read a single character from standard input.
 *              Utilizes similar logic to custom fgets function.
 *
 * Parameters:
 *   f (void *) - Pointer to the file stream. In this case, it should be stdin.
 *
 * Return:
 *   int - The next character from the input stream, or EOF if the end of the file or an error occurs.
 */
int fgetc(void *f) {
    
    if ((int)f == stdin) {
        char ch;
        ch = getch_(); // Assuming getch_() is a function to get a single character from input
        return (ch == EOF) ? EOF : (unsigned char)ch; // Return EOF or the character read
    } else {
        if(fs_active != true)
    {
        return -1;
    }
        return fl_fgetc(f); // Use the standard library fgetc for other file streams
    }
}
/**
 * Function Name: mkdir
 * Description: Creates a new directory with the specified name.
 *
 * Parameters:
 *   path (const char*) - Pointer to a null-terminated string containing the directory name.
 *
 * Return:
 *   int - On success, returns 0.
 *         On failure, returns -1.
 */
int mkdir(const char *path) {
    if(fs_active != true)
    {
        return -1;
    }
    return fl_createdirectory(path);
    // return -1;
}

/**
 * Function Name: rmdir
 * Description: Removes the specified directory.
 *
 * Parameters:
 *   path (const char*) - Pointer to a null-terminated string containing the directory name.
 *
 * Return:
 *   int - On success, returns 0.
 *         On failure, returns -1.
 */
int rmdir(const char *path) {
    if(fs_active != true)
    {
        return -1;
    }
    // Empty implementation
    return -1;
}
/**
 * Function Name: fprintf
 * Description: Writes formatted data to a stream.
 *
 * Parameters:
 *   fp (void*) - Pointer to the FILE object where the data will be written.
 *   format (const char*) - Format string specifying how subsequent arguments are converted for output.
 *   ... - Additional arguments that will be formatted according to the format string.
 *
 * Return:
 *   Returns the number of characters written on success; a negative value is returned if an error occurs.
 */
int fprintf(void *fp,const char *format,...)
{
    
    
    char buffer[1024*3];
    va_list va;
    va_start(va, format);
    if(buffer == NULL)
    {
        printf("Buffer allocation failed\n");
        return -1;
        // buffer = backup_buffer;

    }
    const int ret = vsnprintf_(buffer, 1024*3, format, va);
    va_end(va);
    // printf("buffer = %s\nlen = %d\n", buffer,strlen(buffer));
    if((int)fp == stdout)
    {
        printf("%s",buffer);
    }
    else if ((int)fp == stderr)
    {
        printf_debug("%s",buffer);
        dbgprintf("%s",buffer);
    }
    else
    {
        if(fs_active != true)
    {
        return -1;
    }
        int write_ret = fwrite(buffer,sizeof(char),strlen(buffer),fp);
    }
    
    // printf("write_ret = %d\n",write_ret);
    memset(buffer,0,1024*3);
    // kfree(buffer);
}

int ungetc(int c,void *stream)
{
    if((int)stream == stdin)
    {
        push_io(c);
        return c; 
    }
    else
    {
        if(fs_active != true)
    {
        return -1;
    }
        return fputc(c,stream);
    }
}

int fflush(void *stream)
{
    if(fs_active != true)
    {
        return -1;
    }
    return fl_fflush(stream);
}


int fs_shutdown()
{
    if(fs_active != true)
    {
        return -1;
    }
    for (size_t i = 0; i < open_files_count; i++)
    {
        if(open_files[i] != NULL)
        {
            fclose(open_files[i]);
        }
    }
    
}