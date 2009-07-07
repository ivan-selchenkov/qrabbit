#ifndef SEARCHITEM_H
#define SEARCHITEM_H

#include <QString>

struct SearchItem
{
    QString host;
    QString nick;
    QString data;
    int port;

    bool isLimit;
    bool isMore;
    long size;

    enum SEARCH_FILE_TYPE
    {
        ANY = 1,
        AUDIO = 2,
        ARCHIVE = 3,
        DOCUMENT = 4,
        BIN = 5,
        IMAGE = 6,
        VIDEO = 7,
        FOLDER = 8,
        TTH = 9,
        ISO = 10
    };

    SEARCH_FILE_TYPE type;
};

#endif // SEARCHITEM_H
