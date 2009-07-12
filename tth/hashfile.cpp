#include "hash.h"
#include "hashfile.h"
#include <iostream>
#include <fstream>
using namespace std;

void HashFile::Go(QString filename)
{
    char* buffer = new char[BLOCK_SIZE];
    char* result = new char[24];

  //ifstream file((char*) filename.constData(), ios::in|ios::binary|ios::ate);
    QFile qfile;
    qfile.setFileName(filename);
    qfile.open(QIODevice::ReadOnly);

    //qDebug() << filename;

    if(!qfile.isOpen()) return; // QString();

    qint64 size_to_read = qfile.size();

    int blocks = (int) (size_to_read / BLOCK_SIZE);
    if(blocks * BLOCK_SIZE < size_to_read) blocks++;

    char* interleaves = new char[blocks * 24];

    int il_pos = 0;

  int read_size;

  read_size = (size_to_read > BLOCK_SIZE) ? BLOCK_SIZE : size_to_read;

    while(read_size > 0)
    {
        qfile.read(buffer, read_size);
        size_to_read -= read_size;
        HashBlock(buffer, read_size, &interleaves[il_pos]);
        il_pos += 24;
        read_size = (size_to_read > BLOCK_SIZE) ? BLOCK_SIZE : size_to_read;
    }
    inter.clear();
    if(il_pos > 24) {
        inter.append(interleaves, blocks * 24);
        HashInternals(interleaves, blocks * 24, result);
    }
    else
    {
        inter.append(interleaves, 24);
        memcpy(result, interleaves, 24);
    }
    delete[] buffer;
    hashdata = result;
    //hash_size = 24;
    qfile.close();
    tth = GetTTHString();
    delete[] hashdata;
//    return out;
}
void HashFile::HashBlock(char* buffer, int size, char* result)
{ 
    int LeavesCount = (int) (size / LEAVE_SIZE);
    if(LeavesCount * LEAVE_SIZE < size) LeavesCount++;

    int leaves_size = (int) (LeavesCount / 2);  // each 2 leaves give 1 unit
    if(leaves_size * 2 < LeavesCount) leaves_size++; // if last leaves alone
    leaves_size = leaves_size * 24; // 1 unit = 24 bytes

    char* result_leaves = new char[leaves_size];

    HashLeaves(buffer, size, result_leaves);
    if(leaves_size > 24) HashInternals(result_leaves, leaves_size, result);
    else memcpy(result, result_leaves, 24);

    delete[] result_leaves;
}
void HashFile::HashInternals(char* internals, int size, char* result)
{
    int hash_pos = 0, temp_pos = 0;
    int hash_size = size;

    int temp_size =(int) (hash_size / (24 * 2));
    if(temp_size * 24 * 2 < hash_size) temp_size++;
    temp_size = temp_size * 24;

    char* hash = new char[size];
    memcpy(hash, internals, size);

    char* temp = new char[temp_size];
    char* ih = new char[49];

    while(hash_size > 24) // until more then one hash
    {
        temp_size =(int) (hash_size / (24 * 2));
        if(temp_size * 24 * 2 < hash_size) temp_size++;
        temp_size = temp_size * 24;

        temp_pos = 0;

        while(hash_pos < hash_size) // calculating new level of internal hashes
        {
            if(hash_pos + 48 <= hash_size) { // if both hashes present
                ih[0] = 0x01;
                memcpy(&ih[1], &hash[hash_pos], 48); // 0x01 + hash1 + hash2
                hash_pos += 48;
                Tiger::hash(ih, 49, &temp[temp_pos]);
                temp_pos += 24;
            }
            else if(hash_pos + 24 <= hash_size) { // if alone hash
                memcpy(&temp[temp_pos], &hash[hash_pos], 24);
                hash_pos += 24;
                temp_pos += 24;
            }
            else break;
        }
        hash_pos = 0;
        hash_size = temp_size;
        memcpy(hash, temp, temp_size);
    }
    memcpy(result, temp, 24);

    delete[] hash;
    delete[] temp;
    delete[] ih;
}

void HashFile::HashLeaves(char* hash, int size, char* result)
{
    char* leave1,* leave2,* internal;
    long pos, result_pos, l1_size, l2_size;
    char* lh1,* lh2,* ih;

    leave1 = new char[LEAVE_SIZE+1]; // 0x00 + leave size
    leave2 = new char[LEAVE_SIZE+1]; // 0x00 + leave size
    internal = new char[49]; // 0x01 + 24 bytes lh1 + 24 bytes lh2
    lh1 = new char[24];
    lh2 = new char[24];
    ih = new char[24];

    pos = 0;
    result_pos = 0;

    while(true)
    {
//        pos = file.tellg();
        if( (size - pos) < LEAVE_SIZE) l1_size = size - pos;
        else l1_size = LEAVE_SIZE;

        if(l1_size > 0) memcpy(&leave1[1], &hash[pos], l1_size);
        pos += l1_size;

        if( (size - pos) < LEAVE_SIZE) l2_size = size - pos;
        else l2_size = LEAVE_SIZE;

        if(l2_size > 0) memcpy(&leave2[1], &hash[pos], l2_size);
        pos += l2_size;

        if(l2_size > 0 && l1_size > 0) // calculating internal hash from leaves hash
        {
            internal[0] = 0x01;
            leave1[0] = 0x00;
            leave2[0] = 0x00;
            Tiger::hash(leave1, l1_size+1, &internal[1]); // hash for leave1
            Tiger::hash(leave2, l2_size+1, &internal[25]); // hash for leave2
            Tiger::hash(internal, 49, ih); // internal hash
        }
        else if(l1_size > 0) // hash for leave1 as internal hash
        {
            leave1[0] = 0x00;
            Tiger::hash(leave1, l1_size+1, ih);
            /*Tiger::hash(leave1, l1_size+1, &internal[1]);
            internal[0] = 0x01;
            Tiger::hash(internal, 25, ih);
            */
        }
        else break; // nothing to read

        memcpy(&result[result_pos], ih, 24); // copying to result buffer
        result_pos += 24;
    }
    if(leave1) { delete[] leave1; leave1 = 0; }
    if(leave2) { delete[] leave2; leave2 = 0; }
    if(internal) { delete[] internal; internal = 0; }
    if(lh1) { delete[] lh1; lh1 = 0; }
    if(lh2) { delete[] lh2; lh2 = 0; }
    if(ih) { delete[] ih;  ih = 0; }
}

QString HashFile::GetTTH()
{
    return tth;
}
QString HashFile::GetTTHString()
{
    QString res;
    Base32 b;
    return b.ToBase32String(QByteArray(hashdata, 24));
}
QByteArray HashFile::getInterleaves()
{
    return inter;
}
