#ifndef HASHFILE_H
#define HASHFILE_H
#include <QtCore>
#include <iostream>
#include <fstream>
#include "base32.h"
#define LEAVE_SIZE 1024
#define BLOCK_SIZE 65536
using namespace std;

class HashFile
{
private:
    //! Calculating internal hash (24 bytes) from block 64k
    void HashBlock(char* buffer, int size, char* result);
    //! Calculating internal hash (24 bytes) from internal hashes
    void HashInternals(char* hash, int size, char* result);
    //! Calculating hashes of leaves from raw file
    void HashLeaves(char* hash, int size, char* result);
    //! 24 bytes hash data
    char* hashdata;
    //char* interleaves;
    QByteArray inter;
    QString tth;
    QString GetTTHString();
public:
    //! Calculating tth for file
    void Go(QString filename);
    //! Getting QByteArray tth
    QString GetTTH();
    //! Getting base32 QString tth
    QByteArray getInterleaves();
};
#endif // HASHFILE_H
