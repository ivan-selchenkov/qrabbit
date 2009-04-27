//////////////////////////////////////////////////////////////////////
// Base32.h
//////////////////////////////////////////////////////////////////////

#if !defined(_BASE32_H_)
#define _BASE32_H_
#include <QtCore>

const char Base32Chars[32] = {  'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H',
                                                                                                'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P',
                                                                                                'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X',
                                                                                                'Y', 'Z', '2', '3', '4', '5', '6', '7'};
class Base32
{
public:
    QString ToBase32String(const QByteArray inArray);
};

#endif
