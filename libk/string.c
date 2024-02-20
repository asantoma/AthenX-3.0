#include <stddef.h>
#include "string.h"
#include "errno.h"
#include "limits.h"
/**
 * Copies the string pointed to by src, including the null terminator, 
 * to the buffer pointed to by dest.
 *
 * Parameters:
 *   dest (char*) - Pointer to the destination buffer where the string is to be copied.
 *   src (const char*) - Pointer to the source string to be copied.
 *
 * Return:
 *   (char*) - Pointer to the destination buffer (same as dest).
 */
char* strcpy(char* dest, const char* src) {
    char* original_dest = dest;
    while ((*dest++ = *src++) != '\0');
    return original_dest;
}

/**
 * Copies at most n characters from the string pointed to by src to the buffer 
 * pointed to by dest. If the length of src is less than n, the remainder of 
 * dest will be padded with null bytes.
 *
 * Parameters:
 *   dest (char*) - Pointer to the destination buffer where the string is to be copied.
 *   src (const char*) - Pointer to the source string to be copied.
 *   n (size_t) - Maximum number of characters to copy.
 *
 * Return:
 *   (char*) - Pointer to the destination buffer (same as dest).
 */
char* strncpy(char* dest, const char* src, size_t n) {
    char* original_dest = dest;
    while (n-- > 0 && (*dest++ = *src++) != '\0');
    while (n-- > 0) *dest++ = '\0'; // Padding with null bytes if necessary
    return original_dest;
}

/**
 * Concatenates the string pointed to by src to the end of the string 
 * pointed to by dest.
 *
 * Parameters:
 *   dest (char*) - Pointer to the destination string.
 *   src (const char*) - Pointer to the source string to be appended.
 *
 * Return:
 *   (char*) - Pointer to the destination string (same as dest).
 */
char* strcat(char* dest, const char* src) {
    char* original_dest = dest;
    while (*dest != '\0') dest++; // Move to the end of dest
    while ((*dest++ = *src++) != '\0'); // Copy src to the end of dest
    return original_dest;
}

/**
 * Concatenates at most n characters from the string pointed to by src to the end 
 * of the string pointed to by dest. A null terminator is always appended to dest.
 *
 * Parameters:
 *   dest (char*) - Pointer to the destination string.
 *   src (const char*) - Pointer to the source string to be appended.
 *   n (size_t) - Maximum number of characters to concatenate from src.
 *
 * Return:
 *   (char*) - Pointer to the destination string (same as dest).
 */
char* strncat(char* dest, const char* src, size_t n) {
    char* original_dest = dest;
    while (*dest != '\0') dest++; // Move to the end of dest
    while (n-- > 0 && (*dest++ = *src++) != '\0'); // Copy up to n characters from src to dest
    *dest = '\0'; // Ensure dest is null-terminated
    return original_dest;
}

/**
 * Compares two strings.
 *
 * Parameters:
 *   str1 (const char*) - Pointer to the first string.
 *   str2 (const char*) - Pointer to the second string.
 *
 * Return:
 *   (int) - Negative value if str1 < str2, 0 if str1 == str2, positive value if str1 > str2.
 */
int strcmp(const char* str1, const char* str2) {
    while (*str1 != '\0' && *str2 != '\0' && *str1 == *str2) {
        str1++;
        str2++;
    }
    return (*str1 - *str2);
}

/**
 * Compares up to n characters of two strings.
 *
 * Parameters:
 *   str1 (const char*) - Pointer to the first string.
 *   str2 (const char*) - Pointer to the second string.
 *   n (size_t) - Maximum number of characters to compare.
 *
 * Return:
 *   (int) - Negative value if str1 < str2, 0 if str1 == str2, positive value if str1 > str2.
 */
int strncmp(const char *s1, const char *s2, size_t n)
{
    while (n--)
        if (*s1++ != *s2++)
            return *(unsigned char *)(s1 - 1) - *(unsigned char *)(s2 - 1);
    return 0;
}

/**
 * Returns the length of the string pointed to by str, 
 * excluding the null terminator.
 *
 * Parameters:
 *   str (const char*) - Pointer to the string.
 *
 * Return:
 *   (size_t) - Length of the string.
 */
size_t strlen(const char* str) {
    size_t length = 0;
    while (*str++ != '\0') length++;
    return length;
}

/**
 * Finds the first occurrence of the character c in the string pointed to by str.
 *
 * Parameters:
 *   str (const char*) - Pointer to the string.
 *   c (int) - Character to search for.
 *
 * Return:
 *   (char*) - Pointer to the first occurrence of c in str, or NULL if not found.
 */
char* strchr(const char* str, int c) {
    while (*str != '\0') {
        if (*str == (char)c) return (char*)str;
        str++;
    }
    return NULL;
}

/**
 * Finds the last occurrence of the character c in the string pointed to by str.
 *
 * Parameters:
 *   str (const char*) - Pointer to the string.
 *   c (int) - Character to search for.
 *
 * Return:
 *   (char*) - Pointer to the last occurrence of c in str, or NULL if not found.
 */
char* strrchr(const char* str, int c) {
    char* last_occurrence = NULL;
    while (*str != '\0') {
        if (*str == (char)c) last_occurrence = (char*)str;
        str++;
    }
    return last_occurrence;
}

/**
 * Finds the first occurrence of the substring needle in the string haystack.
 *
 * Parameters:
 *   haystack (const char*) - Pointer to the string to search in.
 *   needle (const char*) - Pointer to the substring to search for.
 *
 * Return:
 *   (char*) - Pointer to the first occurrence of needle in haystack, or NULL if not found.
 */
char* strstr(const char* haystack, const char* needle) {
    if (*needle == '\0') return (char*)haystack; // Empty needle matches at the beginning
    while (*haystack != '\0') {
        const char* h = haystack;
        const char* n = needle;
        while (*n != '\0' && *h == *n) {
            h++;
            n++;
        }
        if (*n == '\0') return (char*)haystack; // Found a match
        haystack++;
    }
    return NULL; // Not found
}

/**
 * Splits the string str into tokens, using the delimiter delim.
 * This function maintains internal state between calls.
 *
 * Parameters:
 *   str (char*) - Pointer to the string to split. It is modified during the call.
 *   delim (const char*) - Pointer to the null-terminated delimiter string.
 *
 * Return:
 *   (char*) - Pointer to the next token found, or NULL if no more tokens are found.
 */
char* strtok(char* str, const char* delim) {
    static char* next_token = NULL; // Internal state to maintain between calls
    if (str == NULL && next_token == NULL) return NULL; // No more tokens left
    if (str != NULL) next_token = str;
    // Skip leading delimiters
    next_token += strspn(next_token, delim);
    if (*next_token == '\0') return NULL; // No more tokens left
    // Find end of token
    char* token_start = next_token;
    next_token += strcspn(next_token, delim);
    if (*next_token != '\0') {
        *next_token = '\0';
        next_token++;
    }
    return token_start;
}

/**
 * Calculates the length of the initial segment of the string str 
 * consisting of only the characters found in the string accept.
 *
 * Parameters:
 *   str (const char*) - Pointer to the string.
 *   accept (const char*) - Pointer to the null-terminated string containing the characters to accept.
 *
 * Return:
 *   (size_t) - Length of the initial segment of str consisting of only the characters found in accept.
 */
size_t strspn(const char* str, const char* accept) {
    size_t length = 0;
    while (*str != '\0' && strchr(accept, *str) != NULL) {
        length++;
        str++;
    }
    return length;
}

/**
 * Calculates the length of the initial segment of the string str 
 * consisting of only the characters not found in the string reject.
 *
 * Parameters:
 *   str (const char*) - Pointer to the string.
 *   reject (const char*) - Pointer to the null-terminated string containing the characters to reject.
 *
 * Return:
 *   (size_t) - Length of the initial segment of str consisting of only the characters not found in reject.
 */
size_t strcspn(const char* str, const char* reject) {
    size_t length = 0;
    while (*str != '\0' && strchr(reject, *str) == NULL) {
        length++;
        str++;
    }
    return length;
}

/**
 * Sets the first n bytes of the block of memory pointed by ptr 
 * to the specified value (interpreted as an unsigned char).
 *
 * Parameters:
 *   ptr (void*) - Pointer to the block of memory to fill.
 *   value (int) - Value to be set. The value is passed as an int, 
 *                 but the function fills the block of memory using 
 *                 the unsigned char conversion of this value.
 *   n (size_t) - Number of bytes to fill.
 *
 * Return:
 *   (void*) - Pointer to the memory block ptr.
 */
void* memset(void* ptr, int value, size_t n) {
    unsigned char* p = (unsigned char*)ptr;
    while (n-- > 0) *p++ = (unsigned char)value;
    return ptr;
}

/**
 * Copies n bytes from memory area src to memory area dest. 
 * The memory areas must not overlap.
 *
 * Parameters:
 *   dest (void*) - Pointer to the destination memory area.
 *   src (const void*) - Pointer to the source memory area.
 *   n (size_t) - Number of bytes to copy.
 *
 * Return:
 *   (void*) - Pointer to the destination memory area (same as dest).
 */
void* memcpy(void* dest, const void* src, size_t n) {
    char* d = (char*)dest;
    const char* s = (const char*)src;
    while (n-- > 0) *d++ = *s++;
    return dest;
}

/**
 * Compares the first n bytes of the memory areas pointed by ptr1 and ptr2.
 *
 * Parameters:
 *   ptr1 (const void*) - Pointer to the first memory area.
 *   ptr2 (const void*) - Pointer to the second memory area.
 *   n (size_t) - Number of bytes to compare.
 *
 * Return:
 *   (int) - Negative value if ptr1 < ptr2, 0 if ptr1 == ptr2, positive value if ptr1 > ptr2.
 */
int memcmp(const void* ptr1, const void* ptr2, size_t n) {
    const unsigned char* p1 = (const unsigned char*)ptr1;
    const unsigned char* p2 = (const unsigned char*)ptr2;
    while (n-- > 0) {
        if (*p1 != *p2) return (*p1 - *p2);
        p1++;
        p2++;
    }
    return 0;
}
/**
 * Function Name: memmove
 * Description: Copies a block of memory, handling overlapping regions.
 *
 * Parameters:
 *   dest (void*) - Pointer to the destination buffer where the content is to be copied
 *   src (const void*) - Pointer to the source of data to be copied
 *   n (size_t) - Number of bytes to copy
 *
 * Return:
 *   (void*) - Returns a pointer to the destination buffer (dest)
 */
void* memmove(void* dest, const void* src, size_t n) {
    unsigned char* d = (unsigned char*)dest;
    const unsigned char* s = (const unsigned char*)src;

    if (s < d) {
        for (size_t i = n; i > 0; --i) {
            d[i - 1] = s[i - 1];
        }
    } else {
        for (size_t i = 0; i < n; ++i) {
            d[i] = s[i];
        }
    }

    return dest;
}
int isspace(char ch) {
    // Check if the character is a space, tab, carriage return, newline, vertical tab, or form feed
    return (ch == ' ' || ch == '\t' || ch == '\r' || ch == '\n' || ch == '\v' || ch == '\f');
}

unsigned long strtoul(const char* str, char** endptr, int base) {
    unsigned long result = 0;
    int sign = 1;
    const char* p = str;

    // Skip leading white-space characters
    while (isspace(*p))
        p++;

    // Handle optional sign
    if (*p == '+' || *p == '-') {
        sign = (*p == '-') ? -1 : 1;
        p++;
    }

    // Handle base prefix (0x or 0X for hexadecimal, 0 for octal)
    if (base == 0) {
        if (*p == '0') {
            base = 8;
            p++;
            if (*p == 'x' || *p == 'X') {
                base = 16;
                p++;
            }
        } else {
            base = 10;
        }
    }

    // Convert the string to an unsigned long integer
    while (isdigit(*p)) {
        int digit = *p - '0';
        if (digit >= base) break; // Invalid digit for the specified base
        result = result * base + digit;
        p++;
    }

    if (endptr != NULL) *endptr = (char*)p;
    return sign > 0 ? result : -result;
}
int isdigit(char ch) {
    // Check if the character is between '0' and '9'
    return (ch >= '0' && ch <= '9');
}
long strtol(const char *str, char **endptr, int base) {
    // Skip leading white space
    while (*str == ' ' || (*str >= '\t' && *str <= '\r')) {
        str++;
    }
    
    // Handle optional '+' or '-' sign
    int sign = 1;
    if (*str == '-') {
        sign = -1;
        str++;
    } else if (*str == '+') {
        str++;
    }
    
    // Determine the base if not provided
    if (base == 0) {
        if (*str == '0') {
            if (*(str + 1) == 'x' || *(str + 1) == 'X') {
                base = 16;
                str += 2;
            } else {
                base = 8;
                str++;
            }
        } else {
            base = 10;
        }
    } else if (base == 16 && *str == '0' && (*(str + 1) == 'x' || *(str + 1) == 'X')) {
        str += 2;
    }
    
    // Convert the string to a long integer
    long result = 0;
    while (*str != '\0') {
        int digit;
        if (*str >= '0' && *str <= '9') {
            digit = *str - '0';
        } else if (*str >= 'a' && *str <= 'z') {
            digit = *str - 'a' + 10;
        } else if (*str >= 'A' && *str <= 'Z') {
            digit = *str - 'A' + 10;
        } else {
            break; // Invalid character
        }
        
        if (digit >= base) {
            break; // Invalid digit for base
        }
        
        // Check for overflow
        if (result > (LONG_MAX - digit) / base) {
            errno = ERANGE;
            result = sign == 1 ? LONG_MAX : LONG_MIN;
            break;
        }
        
        result = result * base + digit;
        str++;
    }
    
    // Set endptr to the next character after the last character used in the conversion
    if (endptr != NULL) {
        *endptr = (char *)str;
    }
    
    return result * sign;
}
int tolower(int c) {
    if (c >= 'A' && c <= 'Z') {
        return c + ('a' - 'A');
    }
    return c;
}
int toupper(int c) {
    if (islower(c)) {
        // If the character is a lowercase letter, convert it to uppercase
        return c - ('a' - 'A');
    } else {
        // If the character is not a lowercase letter, return it unchanged
        return c;
    }
}
int islower(int c) {
    return (c >= 'a' && c <= 'z');
}
int isupper(char ch) {
    return (ch >= 'A' && ch <= 'Z');
}

int exit(int v )
{
    errno = v;
}
void *realloc(void *ptr, size_t size) {
    if (!ptr) {
        // If ptr is NULL, behave like kmalloc
        return kmalloc(size);
    } else if (size == 0) {
        // If size is 0, behave like kfree
        kfree(ptr);
        return NULL;
    } else {
        // Allocate new memory block
        void *new_ptr = kmalloc(size);
        if (!new_ptr) {
            // Allocation failed
            return NULL;
        }
        // Copy contents of old block to new block
        memcpy(new_ptr, ptr, size);
        // Deallocate old block
        kfree(ptr);
        return new_ptr;
    }
}