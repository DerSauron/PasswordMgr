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

#include "PasswordListItem.h"

PasswordListItem::Data::Data() : password(), length(-1)
{
}

PasswordListItem::PasswordListItem(const QString& password, int length) :
    QListWidgetItem(password, 0, QListWidgetItem::UserType)
{
    setData(PasswordRole, password);
    setData(PasswordLengthRole, length);
}

PasswordListItem::PasswordListItem(const PasswordListItem& other) : QListWidgetItem(other)
{
}

PasswordListItem::~PasswordListItem()
{
}

void PasswordListItem::setData(int role, const QVariant& value)
{
    if (role == Qt::DisplayRole)
        return;

    if (role == Qt::EditRole)
        role = PasswordRole;

    QListWidgetItem::setData(role, value);
}

QVariant PasswordListItem::data(int role) const
{
    if (role == Qt::DisplayRole) {
        return QString("Len %1: %2").arg(QListWidgetItem::data(PasswordLengthRole).toInt(), 2)
            .arg(QListWidgetItem::data(PasswordRole).toString());
    }

    if (role == Qt::EditRole)
        role = PasswordRole;

    return QListWidgetItem::data(role);
}
