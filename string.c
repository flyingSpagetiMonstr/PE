#include <stdio.h>

// int compare(const char* a, const char* b);
// size_t length(const char* str);

int compare(const char *a, const char *b) 
{
    while (*a != '\0' && *a == *b) {
        a++;
        b++;
    }
    return (int)(unsigned char)(*a) - (int)(unsigned char)(*b);
}

size_t length(const char* str) 
{
    const char* s = str;
    while (*s) {
        s++;
    }
    return (size_t)(s - str);
}

#include <wchar.h>
#include <stdint.h>

// #define compare(a, b, size) {uint##size_t cmp_tmp_a = a, cmp_tmp_b = b; while (*(uint##size_t)a != 0 && *a == *b) {a++;b++;}}
// #define xcompare(a, b, type)

// int wcompare(const wchar_t *a, const wchar_t *b)
// {
//     while (*a != 0 && *a == *b) {
//         a++;
//         b++;
//     }
//     return (int)(*a) - (int)(*b);
// }

// Silly stuff
// #define cmp_prepare int cmp_i;
// #define compare(a, b) cmp_i = 0; while (*((a)+cmp_i) != 0 && *((a)+cmp_i) == *((b)+cmp_i)) {cmp_i++;}
// #define cmp_result(a, b)  ((int)(*((a)+cmp_i)) - (int)(*((b)+cmp_i)))