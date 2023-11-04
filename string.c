#include <wchar.h>
// must be included at end of payload.c

// these string oprations don't handle NULL pointers 

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
    const char *s = str;
    while (*s) {
        s++;
    }
    return (size_t)(s - str);
}

int wcompare(const wchar_t *a, const wchar_t *b)
{
    while (*a != 0 && *a == *b) {
        a++;
        b++;
    }
    return (int)(*a) - (int)(*b);
}

int match_suffix(char *str, char* suffix)
{
    int suffix_l = length(suffix);
    int str_l = length(str);
    if (str_l < suffix_l)
    {
        return 0;
    }
    str += str_l - suffix_l;
    return !compare(str, suffix);
}

// You can use msvcrt.dll now and don't need to write it yourself anymore.
// char* strcopy(char* dst, const char* src) 
// {
//     char* ptr = dst;
//     while (*src != '\0') {
//         *ptr = *src;
//         src++;
//         ptr++;
//     }
//     *ptr = '\0';
//     return dst;
// }


// Silly stuff: 
// #define cmp_prepare int cmp_i;
// #define compare(a, b) cmp_i = 0; while (*((a)+cmp_i) != 0 && *((a)+cmp_i) == *((b)+cmp_i)) {cmp_i++;}
// #define cmp_result(a, b)  ((int)(*((a)+cmp_i)) - (int)(*((b)+cmp_i)))