// string_functions.c
#include "string.h"
#include <stddef.h>
int strlen(const char* str) {
    int length = 0;
    while (str[length] != '\0') {
        length++;
    }
    return length;
}

char* strstr(const char* haystack, const char* needle) {
    while (*haystack != '\0') {
        const char* h = haystack;
        const char* n = needle;

        while (*haystack != '\0' && *n != '\0' && *haystack == *n) {
            haystack++;
            n++;
        }

        if (*n == '\0') {
            return (char*)h;
        }

        haystack = h + 1;
    }

    return NULL;
}

char* strcpy(char* dest, const char* src) {
    char* original_dest = dest;

    while (*src != '\0') {
        *dest = *src;
        dest++;
        src++;
    }

    *dest = '\0'; // Add null terminator to the destination string
    return original_dest;
}
void* memcpy(void* dest, const void* src, size_t size) {
    char* dest_ptr = (char*)dest;
    const char* src_ptr = (const char*)src;

    while (size--) {
        *dest_ptr++ = *src_ptr++;
    }

    return dest;
}

void* memset(void* ptr, int value, size_t size) {
    unsigned char* char_ptr = (unsigned char*)ptr;
    while (size--) {
        *char_ptr++ = (unsigned char)value;
    }

    return ptr;
}
int strcmp(const char* str1, const char* str2) {
    while (*str1 && *str2 && *str1 == *str2) {
        str1++;
        str2++;
    }
    return *str1 - *str2;
}

char* strncpy(char* dest, const char* src, size_t n) {
    char* ret = dest;
    while (n > 0 && *src != '\0') {
        *dest++ = *src++;
        n--;
    }
    while (n > 0) {
        *dest++ = '\0';
        n--;
    }
    return ret;
}
int strncmp(const char* str1, const char* str2, size_t n) {
    while (n > 0 && *str1 != '\0' && *str2 != '\0' && *str1 == *str2) {
        str1++;
        str2++;
        n--;
    }
    if (n == 0) return 0; // Reached end of comparison
    return *(unsigned char*)str1 - *(unsigned char*)str2;
}