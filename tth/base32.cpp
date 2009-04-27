#include "base32.h"

QString Base32::ToBase32String(const QByteArray inArray)
{

           if (inArray.isEmpty()) return QString();

           int len = inArray.size();
            // divide the input into 40-bit groups, so let's see,
            // how many groups of 5 bytes can we get out of it?
            int numberOfGroups = len / 5;
            // and how many remaining bytes are there?
            int numberOfRemainingBytes = len - 5 * numberOfGroups;

            // after this, we're gonna split it into eight 5 bit
            // values.
           // StringBuilder sb = new StringBuilder();
            //int resultLen = 4*((len + 2)/3);
            //StringBuffer result = new StringBuffer(resultLen);

            // Translate all full groups from byte array elements to Base64
            QString sb;
            unsigned char b0, b1, b2, b3, b4;

            int byteIndexer = 0;
            unsigned char index;
            for (int i = 0; i < numberOfGroups; i++)
            {
                b0 = inArray[byteIndexer++];
                b1 = inArray[byteIndexer++];
                b2 = inArray[byteIndexer++];
                b3 = inArray[byteIndexer++];
                b4 = inArray[byteIndexer++];

                // first 5 bits from byte 0
                index = b0 >> 3;
                sb.append(Base32Chars[index]);
                // the remaining 3, plus 2 from the next one
                index = (b0 << 2) & 0x1F | (b1 >> 6);
                sb.append(Base32Chars[index]);
                // get bit 3, 4, 5, 6, 7 from byte 1
                sb.append(Base32Chars[(b1 >> 1) & 0x1F]);
                // then 1 bit from byte 1, and 4 from byte 2
                sb.append(Base32Chars[(b1 << 4) & 0x1F | (b2 >> 4)]);
                // 4 bits from byte 2, 1 from byte3
                sb.append(Base32Chars[(b2 << 1) & 0x1F | (b3 >> 7)]);
                // get bit 2, 3, 4, 5, 6 from byte 3
                sb.append(Base32Chars[(b3 >> 2) & 0x1F]);
                // 2 last bits from byte 3, 3 from byte 4
                sb.append(Base32Chars[(b3 << 3) & 0x1F | (b4 >> 5)]);
                // the last 5 bits
                sb.append(Base32Chars[b4 & 0x1F]);
            }

            // Now, is there any remaining bytes?
            if (numberOfRemainingBytes > 0)
            {
                b0 = inArray[byteIndexer++];
                // as usual, get the first 5 bits
                sb.append(Base32Chars[b0 >> 3]);
                // now let's see, depending on the
                // number of remaining bytes, we do different
                // things
                switch (numberOfRemainingBytes)
                {
                    case 1:
                        // use the remaining 3 bits, padded with five 0 bits
                        sb.append(Base32Chars[(b0 << 2) & 0x1F]);
                        //						sb.Append("======");
                        break;
                    case 2:
                        b1 = inArray[byteIndexer++];
                        sb.append(Base32Chars[(b0 << 2) & 0x1F | (b1 >> 6)]);
                        sb.append(Base32Chars[(b1 >> 1) & 0x1F]);
                        sb.append(Base32Chars[(b1 << 4) & 0x1F]);
                        //						sb.Append("====");
                        break;
                    case 3:
                        b1 = inArray[byteIndexer++];
                        b2 = inArray[byteIndexer++];
                        sb.append(Base32Chars[(b0 << 2) & 0x1F | (b1 >> 6)]);
                        sb.append(Base32Chars[(b1 >> 1) & 0x1F]);
                        sb.append(Base32Chars[(b1 << 4) & 0x1F | (b2 >> 4)]);
                        sb.append(Base32Chars[(b2 << 1) & 0x1F]);
                        //						sb.Append("===");
                        break;
                    case 4:
                        b1 = inArray[byteIndexer++];
                        b2 = inArray[byteIndexer++];
                        b3 = inArray[byteIndexer++];
                        sb.append(Base32Chars[(b0 << 2) & 0x1F | (b1 >> 6)]);
                        sb.append(Base32Chars[(b1 >> 1) & 0x1F]);
                        sb.append(Base32Chars[(b1 << 4) & 0x1F | (b2 >> 4)]);
                        sb.append(Base32Chars[(b2 << 1) & 0x1F | (b3 >> 7)]);
                        sb.append(Base32Chars[(b3 >> 2) & 0x1F]);
                        sb.append(Base32Chars[(b3 << 3) & 0x1F]);
                        //						sb.Append("=");
                        break;
                }
            }
            return sb;
        }
