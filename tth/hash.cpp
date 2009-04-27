#include "hash.h"
#include "tiger.h"
void Tiger::hash(const char* array, int size, char* res)
{
    tiger_ctx s;
    tiger_init(&s);
    tiger_update(&s, (unsigned char*) array, size);
    tiger_final(&s, (unsigned char*) res);
}
void Tiger::Swap(char& ch1, char& ch2)
{
    char ch = ch1;
    ch1 = ch2;
    ch2 = ch;
}
