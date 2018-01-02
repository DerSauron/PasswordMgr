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

#ifndef WALLETCONTENT_H
#define WALLETCONTENT_H

#include <QString>
#include <QMetaType>
#include <QList>

class WalletContent
{
public:
    WalletContent();
    WalletContent(const QString& username, const QString& password);

    QString username() const { return username_; }
    QString password() const { return password_; }

    operator QVariant() const { return QVariant(*this); }

private:
    QString username_;
    QString password_;

    friend QDataStream& operator<<(QDataStream&, const WalletContent&);
    friend QDataStream& operator>>(QDataStream&, WalletContent&);
};

QDataStream& operator<<(QDataStream& stream, const WalletContent& content);
QDataStream& operator>>(QDataStream& stream, WalletContent& content);

typedef QList<WalletContent> WalletContentList;

Q_DECLARE_METATYPE(WalletContent);
Q_DECLARE_METATYPE(WalletContentList);

#endif // WALLETCONTENT_H
