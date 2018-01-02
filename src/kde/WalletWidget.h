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

#ifndef _WALLETWIDGET_H
#define	_WALLETWIDGET_H

#include "ui_WalletWidget.h"
#include "WalletContent.h"
#include "../WalletDelegate.h"

class WalletModel;
class QToolButton;

class WalletWidget : public QWidget {
    Q_OBJECT
public:
    enum {
        KeyRole = Qt::UserRole + 1,
    };

    WalletWidget();
    virtual ~WalletWidget();

    bool savePassword(const QString& entry, const QString& username, const QString& password);
    QStringList entryList();

private slots:
    void onListEntryChanged(const QItemSelection& selected, const QItemSelection& deselected);
    void onAddEntryBtnPressed();
    void onRemoveEntryBtnPressed();
    void onAddPasswordBtnPressed();
    void copyPassword(int pos=-1);
    void removePassword(int pos=-1);
    void onShowPasswordsPressed();
    void onEntryContentSubmitted();

private:
    struct Fields {
        QWidget* pane;
        QLineEdit* username;
        QLineEdit* password;

        Fields(QWidget* pane, int num);

        Fields(const Fields& o);

        Fields& operator=(const Fields& o);

        void fillData(const WalletContent& content);

        void copyDataFrom(const Fields& other);

        void clearData();
    };

    Ui::WalletWidget* ui;

    WalletModel* walletModel_;

    QList<Fields> fieldsList;
    int fieldsVisible;
    QString entryToSelect;

    void init();
    void loadContent(const QModelIndex& selectedIndex);
    Fields* fields(int pos);
    QWidget* createFieldsPane(QWidget* previous, int num);
    int getPanePos(QToolButton* button);
    void appendPasswordFields();
    void saveEntryContent();
};

#endif	/* _WALLETWIDGET_H */
