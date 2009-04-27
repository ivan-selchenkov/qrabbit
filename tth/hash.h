#ifndef HASH_H
#define HASH_H
#include <QtCore>

class Tiger
{
public:
    static void hash(const char*, int size, char* res);
private:
    static void Swap(char&, char&);
};
#endif // HASH_H
