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

#ifndef PASSWORDLISTITEM_H
#define	PASSWORDLISTITEM_H

#include <QListWidgetItem>

class PasswordListItem : public QListWidgetItem {
public:
    struct Data {
        QString password;
        int length;

        Data();
        Data(const QString& password, int length);
    };

    enum Role {
        PasswordRole = Qt::UserRole,
        PasswordLengthRole,
    };

    PasswordListItem(const QString& password, int length);
    PasswordListItem(const PasswordListItem& other);
    virtual ~PasswordListItem();

    virtual void setData(int role, const QVariant& value);
    virtual QVariant data(int role) const;

private:

};

Q_DECLARE_METATYPE(PasswordListItem::Data);

#endif	/* PASSWORDLISTITEM_H */

