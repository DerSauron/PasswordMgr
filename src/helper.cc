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

#include <QComboBox>
#include <QLineEdit>

#include "helper.h"

void populateComboBox(QComboBox* cb)
{
    if (cb->insertPolicy() == QComboBox::NoInsert)
        return;

    if (cb->lineEdit() && !cb->lineEdit()->text().isEmpty()) {
        cb->lineEdit()->deselect();
        cb->lineEdit()->end(false);
        QString text = cb->lineEdit()->text();
        // check for duplicates (if not enabled) and quit
        int index = -1;
        if (!cb->duplicatesEnabled()) {
            index = cb->findText(text, Qt::MatchFixedString | Qt::MatchCaseSensitive);
            if (index != -1) {
                cb->setCurrentIndex(index);
                return;
            }
        }
        switch (cb->insertPolicy()) {
        case QComboBox::InsertAtTop:
            index = 0;
            break;
        case QComboBox::InsertAtBottom:
            index = cb->model()->rowCount(QModelIndex());
            break;
        case QComboBox::InsertAtCurrent:
        case QComboBox::InsertAfterCurrent:
        case QComboBox::InsertBeforeCurrent:
            if (!cb->model()->rowCount(QModelIndex()))
                index = 0;
            else if (cb->insertPolicy() == QComboBox::InsertAtCurrent)
                cb->setItemText(cb->currentIndex(), text);
            else if (cb->insertPolicy() == QComboBox::InsertAfterCurrent)
                index = cb->currentIndex() + 1;
            else if (cb->insertPolicy() == QComboBox::InsertBeforeCurrent)
                index = cb->currentIndex();
            break;
        case QComboBox::InsertAlphabetically:
            index = 0;
            for (int i = 0; i < cb->model()->rowCount(QModelIndex()); i++, index++) {
                if (text.toLower() < cb->itemText(i).toLower())
                    break;
            }
            break;
        default:
            break;
        }
        if (index >= 0) {
            cb->insertItem(index, text);
            cb->setCurrentIndex(index);
        }
    }
}

QIcon loadIcon(const QString& name)
{
    QIcon icon;
    if (QIcon::hasThemeIcon(name)) {
        icon = QIcon::fromTheme(name);
    } else {
        // TOTO load the icon from file
        icon.addFile(QString::fromUtf8(""), QSize(), QIcon::Normal, QIcon::Off);
    }
    return icon;
}
