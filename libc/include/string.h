#ifndef __STRING__H
#define __STRING__H
#include "stddef.h"
#include "stdbool.h"
#include "stdint.h"
#include "types.h"
#define swap(a, b) (a ^= b ^= a ^= b)
void *memset(void *dst, char c, uint32 n);
char *strstr(char *s1, const char *s2);
void *memcpy(void *dst, const void *src, uint32 n);
void* memmove(void* dest, const void* src, size_t size);
void parse_string(char *parser, char *string, char c);
int memcmp(uint8 *s1, uint8 *s2, uint32 n);
uint32_t atoi(const char *str);
int strlen(const char *s);
int strncmp(const char *s1, const char *s2, size_t n);
char *strncpy(char *dst, const char *src, size_t n);
int strcmp(const char *s1, char *s2);

int strcpy(char *dst, const char *src);
char *ctos(char s[2], const char c);
void strcat(char *dest, const char *src);
// char *strncpy(char *dst, const char *src, size_t n);
int isspace(char c);
void strncat(char* dest, const char* src, size_t destSize);
int isalpha(char c);
char upper(char c);
char lower(char c);
bool backspace(char *buffer);
char* strrchr(const char* str, int ch);
void itoa(char *buf, int base, int d);
int strncmp(const char *s1, const char *s2, size_t n);
int string_length(char s[]);
void append(char s[], char n);
bool hex_string_to_uint16(const char* hexString, uint16_t* result);
unsigned long strtoul(const char* str, char** endptr, int base);
char* strtok(char* str, const char* delimiters);
char *strtok_r(char* str, const char* delim, char** saveptr);
unsigned long long strtoull(const char *str, char **endptr, int base);
int tolower(int c);
int isdigit(int c);
int isxdigit(int c);
long strtol(const char* str, char** endptr, int base);
bool isalnum(int c);
size_t strcspn(const char* str, const char* reject);
char* strdup(const char* str);
#endif