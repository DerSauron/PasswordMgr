/*  
*   Byte-oriented AES-256 implementation.
*   All lookup tables replaced with 'on the fly' calculations. 
*
*   Copyright (c) 2007-2009 Ilya O. Levin, http://www.literatecode.com
*   Other contributors: Hal Finney
*
*   Permission to use, copy, modify, and distribute this software for any
*   purpose with or without fee is hereby granted, provided that the above
*   copyright notice and this permission notice appear in all copies.
*
*   THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
*   WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
*   MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
*   ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
*   WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
*   ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
*   OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
*/

#ifndef _AES256_H
#define _AES256_H

#include <QByteArray>

#ifndef uint8_t
#define uint8_t  unsigned char
#endif

class AES256 {
public:
    static constexpr int KEY_LENGTH = 32;
    static constexpr int BLOCK_LENGTH = 16;

    typedef struct {
        uint8_t key[KEY_LENGTH];
        uint8_t enckey[KEY_LENGTH];
        uint8_t deckey[KEY_LENGTH];
    } Context;

    AES256();
    virtual ~AES256();

    void setKey(const QByteArray& key);
    bool isInitialized() const;
    QByteArray ecbEncrypt(const QByteArray& plainText);
    QByteArray ecbDecrypt(const QByteArray& cipherText);

private:
    Context context;
    bool initialized;
};

#endif // _AES256_H
