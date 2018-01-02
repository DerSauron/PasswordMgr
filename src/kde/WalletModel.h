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

#ifndef WALLETMODEL_H
#define WALLETMODEL_H

#include "WalletContent.h"
#include <QAbstractListModel>

#if defined(QT_NO_DEBUG)
#define PASSWORD_MANAGER_FOLDER "Password Manager"
#else
#define PASSWORD_MANAGER_FOLDER "Password Manager (Debug)"
#endif

namespace KWallet {
class Wallet;
}

class QWidget;

class WalletModel : public QAbstractListModel
{
    Q_OBJECT

public:
    enum Role
    {
        WalletContentRole = Qt::UserRole + 1
    };

    WalletModel(QWidget* parent = nullptr);
    virtual ~WalletModel();

    void openWallet();

    QModelIndex addEntry(const QString& entry);
    QModelIndex addPassword(const QString& entry, const QString& username, const QString& password);

    QStringList entryList() const;
    bool hasEntry(const QString& entry) const;

    void removeEntry(const QModelIndex& index);
    void removeEntry(const QString& entry);

    Qt::ItemFlags flags(const QModelIndex& index) const override;
    int rowCount(const QModelIndex& parent) const override;
    QVariant data(const QModelIndex& index, int role) const override;
    bool setData(const QModelIndex& index, const QVariant& value, int role) override;

private slots:
    void onWalletOpened(bool ok);
    void onWalletClosed();
    void onFolderUpdated(const QString& folder);

private:
    void ensureOpenWallet();
    void load();
    void save();
    QModelIndex find(const QString& entry) const;
    QModelIndex findNext(const QString& entry) const;
    QModelIndex insert(const QString& entry, const QModelIndex& insertPos = QModelIndex());
    void walletInsert(const QString& entry);
    void remove(const QModelIndex& index);
    bool walletRemove(const QString& entry);
    void rename(const QModelIndex& index, const QString& newValue);
    bool walletRename(const QString& oldValue, const QString& newValue);
    WalletContentList loadEntryContent(const QString& entry) const;
    void saveEntryContent(const QString& entry, const WalletContentList& content);

private:
    KWallet::Wallet* wallet_;
    QStringList folderEntries_;
    //QMap<QString, WalletContentList> walletContents_;
};

#endif // WALLETMODEL_H
