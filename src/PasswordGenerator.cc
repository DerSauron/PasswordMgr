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

#include "PasswordGenerator.h"

#include <qglobal.h>
#include <QDebug>
#include <QDateTime>
#include <QCoreApplication>
#include <QStandardPaths>
#include <QDir>
#include <random>

namespace {

union split
{
    quint64 b[4];
    quint32 p[8];
};

std::mt19937_64* createRndEngine(quint64 id)
{
    split s = {{
                   static_cast<quint64>(QCoreApplication::applicationPid()),
                   static_cast<quint64>(QDateTime::currentMSecsSinceEpoch()),
                   reinterpret_cast<quint64>(&createRndEngine),
                   id
               }};
    std::seed_seq seq(s.p, s.p + 8);

    return new std::mt19937_64(seq);
}

quint64 gRndSequence_ = 0;

}

template<int L>
QDataStream& operator>>(QDataStream& in, PasswordGenerator::DataBlock<L>& b)
{
    in >> b.data;
    Q_ASSERT(b.data.size() == L);
    return in;
}

template<int L>
QDataStream& operator<<(QDataStream& out, const PasswordGenerator::DataBlock<L>& b)
{
    out << b.data;
    return out;
}

QDataStream& operator>>(QDataStream& in, PasswordGenerator::Counter& cntr)
{
    in >> cntr.b[0];
    in >> cntr.b[1];
    return in;
}

QDataStream& operator<<(QDataStream& out, const PasswordGenerator::Counter& cntr)
{
    out << cntr.b[0];
    out << cntr.b[1];
    return out;
}

template<int L>
PasswordGenerator::DataBlock<L>::DataBlock()
{
    data.reserve(L);
}

template<int L>
void PasswordGenerator::DataBlock<L>::initRandom()
{
    QScopedPointer<std::mt19937_64> rnd(createRndEngine(gRndSequence_++));

    int wc = sizeof(quint64);
    int wi = 0;
    quint64 v;

    for (int i = 0; i < L; ++i)
    {
        if (wi == 0)
             v = (*rnd)();

        data.append((v >> (wi * 8)) % 0xFF);

        ++wi;
        if (wi == wc)
            wi = 0;
    }
}

PasswordGenerator::Counter::Counter()
{
    b[0] = 0;
    b[1] = 0;
}

void PasswordGenerator::Counter::initRandom()
{
    QScopedPointer<std::mt19937_64> rnd(createRndEngine(gRndSequence_++));

    b[0] = (*rnd)();
    b[1] = (*rnd)();
}

PasswordGenerator::Counter& PasswordGenerator::Counter::operator++()
{
    if (b[0] == std::numeric_limits<quint64>::max())
    {
        b[0] = 0;
        ++b[1];
    }
    else
    {
        ++b[0];
    }
    return *this;
}

quint8 PasswordGenerator::Counter::operator[](unsigned int idx)
{
    Q_ASSERT(idx < sizeof(p));

    return p[idx];
}

PasswordGenerator::PasswordGenerator() :
    randomBytePos_(0)
{
}

PasswordGenerator::~PasswordGenerator()
{
}

void PasswordGenerator::initDataStore()
{
    QDir dataDir(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation));

    if (!dataDir.exists())
        dataDir.mkpath(".");

    stateFile_ =
            dataDir.absolutePath()
            + QDir::separator() + "state.bin";
}

void PasswordGenerator::initCipher()
{
    if (cipher.isInitialized())
        return;

    initDataStore();

    loadGeneratorState();

    cipher.setKey(cipherKey_.data);

    randomBytePos_ = AES256::BLOCK_LENGTH;
    generateNewBlock();
}

void PasswordGenerator::applyCounter()
{
    ++counter_;

    for (int i = 0; i < AES256::BLOCK_LENGTH; ++i)
    {
        randomBytes_.data[i] = randomBytes_.data[i] ^ counter_[i];
    }
}

void PasswordGenerator::generateNewBlock()
{
    applyCounter();

    randomBytes_.data = cipher.ecbEncrypt(randomBytes_.data);
    Q_ASSERT(randomBytes_.data.size() == AES256::BLOCK_LENGTH);

    saveGeneratorState();

    randomBytePos_ = 0;
}

quint8 PasswordGenerator::getNextRandomByte()
{
    if (randomBytePos_ >= AES256::BLOCK_LENGTH)
        generateNewBlock();

    return (quint8)randomBytes_.data.at(randomBytePos_++);
}

void PasswordGenerator::loadGeneratorState()
{
    bool loaded = false;
    QFile stateFile(stateFile_);
    if (stateFile.open(QFile::ReadOnly))
    {
        QDataStream in(&stateFile);
        in.setVersion(QDataStream::Qt_5_9);

        in.startTransaction();
        in >> cipherKey_;
        in >> counter_;
        in >> randomBytes_;
        if (in.commitTransaction())
            loaded = true;
    }

    if (!loaded)
    {
        cipherKey_.initRandom();
        counter_.initRandom();
        randomBytes_.initRandom();
        saveGeneratorState();
    }
}

void PasswordGenerator::saveGeneratorState()
{
    QFile stateFile(stateFile_);
    if (stateFile.open(QFile::WriteOnly))
    {
        QDataStream out(&stateFile);
        out.setVersion(QDataStream::Qt_5_9);

        out << cipherKey_;
        out << counter_;
        out << randomBytes_;
    }
}

QString PasswordGenerator::generate(const CharacterStock& characterStock, int length)
{
    initCipher();

    generateNewBlock();

    QString charPool;
    QString password;
    int currLength = 0;
    foreach (CharacterStock::Item item, characterStock.items)
    {
        charPool.append(item.chars);
        currLength += item.minLength;
        for (int i = 0; i < item.minLength; ++i)
        {
            quint8 byte = getNextRandomByte();
            password.append(item.chars.at(byte % item.chars.length()));
        }
    }
    for (int i = currLength; i < length; ++i)
    {
        quint8 byte = getNextRandomByte();
        password.append(charPool.at(byte % charPool.length()));
    }
    for (int i = 0; i < 128 + getNextRandomByte(); ++i)
    {
        quint8 from = getNextRandomByte() % password.length();
        quint8 to = getNextRandomByte() % password.length();
        if (from != to) {
            QChar t = password.at(from);
            password.replace(from, 1, password.at(to));
            password.replace(to, 1, t);
        }
    }
    return password;
}
