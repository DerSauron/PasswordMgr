/*
 * Password Manager 1.0
 * Copyright (C) 2017 "Daniel Volk" <mail@volkarts.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef PASSWORDGENERATOR_H
#define	PASSWORDGENERATOR_H

#include "AES256.h"

#include <QString>
#include <QByteArray>
#include <QList>

struct CharacterStock
{
    struct Item
    {
        QString chars;
        int minLength;

        Item(QString chars, int minLength) : chars(chars), minLength(minLength)
        {
        }

        Item(const Item& other) : chars(other.chars), minLength(other.minLength)
        {
        }
    };

    QList<Item> items;

    void add(const QString& chars, int minLength)
    {
        items << Item(chars, minLength);
    }
};

class PasswordGenerator {
public:
    PasswordGenerator();
    virtual ~PasswordGenerator();

    QString generate(const CharacterStock& characterStock, int length);

private:
    template<int L>
    struct DataBlock
    {
        QByteArray data;

        DataBlock();

        void initRandom();
    };

    struct Counter
    {
        union
        {
            quint64 b[2];
            quint8 p[16];
        };

        Counter();

        void initRandom();

        Counter& operator++();
        quint8 operator[](unsigned int idx);
    };

    QString stateFile_;
    AES256 cipher;
    DataBlock<AES256::KEY_LENGTH> cipherKey_;
    Counter counter_;
    DataBlock<AES256::BLOCK_LENGTH> randomBytes_;
    int randomBytePos_;

    void initDataStore();
    void initCipher();
    void applyCounter();
    void generateNewBlock();
    quint8 getNextRandomByte();

    void loadGeneratorState();
    void saveGeneratorState();

    Q_DISABLE_COPY(PasswordGenerator);

    template<int L>
    friend QDataStream& operator>>(QDataStream&, DataBlock<L>&);
    template<int L>
    friend QDataStream& operator<<(QDataStream&, const DataBlock<L>&);

    friend QDataStream& operator>>(QDataStream&, Counter&);
    friend QDataStream& operator<<(QDataStream&, const Counter&);
};

#endif	/* PASSWORDGENERATOR_H */
