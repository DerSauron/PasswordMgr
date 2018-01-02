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

#include "WalletModel.h"
#include "../main.h"
#include "../MainFrame.h"
#include "../StatusBubble.h"
#include <kwallet.h>
#include <QWidget>
#include <QDebug>
#include <algorithm>

namespace {

int compare(const QString& a, const QString&b)
{
    return a.compare(b);
}

}

WalletModel::WalletModel(QWidget* parent) :
    QAbstractListModel(parent),
    wallet_(nullptr)
{
}

WalletModel::~WalletModel()
{
    if (wallet_)
        delete wallet_;
}

void WalletModel::openWallet()
{
    ensureOpenWallet();
}

QModelIndex WalletModel::addEntry(const QString& entry)
{
    QModelIndex idx = find(entry);
    if (!idx.isValid())
    {
        QModelIndex nextIdx = findNext(entry);
        idx = insert(entry, nextIdx);
        walletInsert(entry);
    }
    return idx;
}

QModelIndex WalletModel::addPassword(const QString& entry, const QString& username, const QString& password)
{
    QModelIndex idx = addEntry(entry);

    WalletContentList content = loadEntryContent(entry);
    content << WalletContent(username, password);
    saveEntryContent(entry, content);

    return idx;
}

QStringList WalletModel::entryList() const
{
    // return copy
    return folderEntries_;
}

bool WalletModel::hasEntry(const QString& entry) const
{
    QModelIndex idx = find(entry);
    return idx.isValid();
}

void WalletModel::removeEntry(const QModelIndex& index)
{
    if (!index.isValid())
        return;

    const QString entry = folderEntries_[index.row()];
    if (walletRemove(entry))
        remove(index);
}

void WalletModel::removeEntry(const QString& entry)
{
    QModelIndex idx = find(entry);
    if (!idx.isValid())
        return;

    if (walletRemove(entry))
        remove(idx);
}

Qt::ItemFlags WalletModel::flags(const QModelIndex& index) const
{
    return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable;
}

int WalletModel::rowCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent);
    return folderEntries_.size();
}

QVariant WalletModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if (role == Qt::DisplayRole || role == Qt::EditRole)
        return folderEntries_[index.row()];

    if (role == WalletContentRole)
    {
        const QString& entry = folderEntries_[index.row()];
        return QVariant::fromValue(loadEntryContent(entry));
    }

    return QVariant();
}

bool WalletModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
    static auto entryRoles = QVector<int>({Qt::DisplayRole, Qt::EditRole});
    static auto contentRoles = QVector<int>({WalletContentRole});

    if (!index.isValid())
        return false;

    if (role == Qt::DisplayRole || role == Qt::EditRole)
    {
        QString& oldValue = folderEntries_[index.row()];
        QString newValue = value.toString();
        if (!newValue.isEmpty() && newValue != oldValue)
        {
            if (walletRename(oldValue, newValue))
            {
                rename(index, newValue);
                emit dataChanged(index, index, entryRoles);
                return true;
            }
        }
    }

    if (role == WalletContentRole)
    {
        const QString& entry = folderEntries_[index.row()];
        saveEntryContent(entry, value.value<WalletContentList>());
        emit dataChanged(index, index, contentRoles);
        return true;
    }

    return false;
}

void WalletModel::onWalletOpened(bool ok)
{
    if (ok &&
        (wallet_->hasFolder(PASSWORD_MANAGER_FOLDER) ||
        wallet_->createFolder(PASSWORD_MANAGER_FOLDER)) &&
        wallet_->setFolder(PASSWORD_MANAGER_FOLDER))
    {
        load();
    } else {
        getMainFrame()->getStatusBubble()->showText(tr("Wallet could not be opened"), StatusBubble::Long);
    }
}

void WalletModel::onWalletClosed()
{
    delete wallet_;
    wallet_ = nullptr;
}

void WalletModel::onFolderUpdated(const QString& folder)
{
    qDebug() << "sender():    " << sender();
    qDebug() << "WalletModel: " << this;

    if (folder == PASSWORD_MANAGER_FOLDER)
        load();
}

void WalletModel::ensureOpenWallet()
{
    if (wallet_)
    {
        if (wallet_->isOpen())
            return;

        delete wallet_;
    }

    wallet_ = KWallet::Wallet::openWallet(KWallet::Wallet::NetworkWallet(), static_cast<QWidget*>(parent())->winId(),
        KWallet::Wallet::Asynchronous);
    connect(wallet_, &KWallet::Wallet::walletOpened, this, &WalletModel::onWalletOpened);
    connect(wallet_, &KWallet::Wallet::walletClosed, this, &WalletModel::onWalletClosed);
    connect(wallet_, &KWallet::Wallet::folderUpdated, this, &WalletModel::onFolderUpdated);
}

void WalletModel::load()
{
    QStringList walletEntries = wallet_->entryList();
    std::sort(walletEntries.begin(), walletEntries.end(),
              [](const QString& a, const QString& b) -> bool {
        return compare(a, b) < 0;
    });

    int i = 0;

    while (i < walletEntries.size() && i < folderEntries_.size())
    {
        int cmp = compare(walletEntries[i], folderEntries_[i]);

        if (cmp < 0)
        {
            insert(walletEntries[i], createIndex(i, 0));
            ++i;
        }
        else if (cmp > 0)
        {
            remove(createIndex(i, 0));
        }
        else
        {
            ++i;
        }
    }

    while (i < walletEntries.size())
    {
        insert(walletEntries[i]);
        ++i;
    }
    while (i < folderEntries_.size())
        remove(createIndex(i, 0));
}

void WalletModel::save()
{

}

QModelIndex WalletModel::find(const QString& entry) const
{
    int row = folderEntries_.indexOf(entry);
    if (row >= 0)
        return createIndex(row, 0);

    return QModelIndex();
}

QModelIndex WalletModel::findNext(const QString& entry) const
{
    int row = 0;
    for (const QString& e : folderEntries_)
    {
        if (compare(e, entry) > 1)
            break;
        ++row;
    }
    return createIndex(row, 0);
}

QModelIndex WalletModel::insert(const QString& entry, const QModelIndex& insertPos)
{
    int newRow;

    if (insertPos.isValid())
        newRow = insertPos.row();
    else
        newRow = folderEntries_.size();

    beginInsertRows(QModelIndex(), newRow, newRow);

    folderEntries_.insert(newRow, entry);

    endInsertRows();

    return createIndex(newRow, 0);
}

void WalletModel::walletInsert(const QString& entry)
{
    saveEntryContent(entry, WalletContentList()); // this will create an empty entry
}

void WalletModel::remove(const QModelIndex& index)
{
    beginRemoveRows(QModelIndex(), index.row(), index.row());

    const QString entry = folderEntries_[index.row()];
    folderEntries_.removeAt(index.row());

    endRemoveRows();
}

bool WalletModel::walletRemove(const QString& entry)
{
    return wallet_->removeEntry(entry) == 0;
}

void WalletModel::rename(const QModelIndex& index, const QString& newValue)
{
    QString oldValue = folderEntries_[index.row()];
    folderEntries_[index.row()] = newValue;
    //walletContents_[newValue] = walletContents_[oldValue];
    //walletContents_.remove(oldValue);
}

bool WalletModel::walletRename(const QString& oldValue, const QString& newValue)
{
    return wallet_->renameEntry(oldValue, newValue) == 0;
}

WalletContentList WalletModel::loadEntryContent(const QString& entry) const
{
    QByteArray rawData;
    if (wallet_->readEntry(entry, rawData)) {
        getMainFrame()->getStatusBubble()
                ->showText(tr("Passwords could not be loaded from wallet"), StatusBubble::Long);
    }

    // deserialize
    QDataStream stream(&rawData, QIODevice::ReadOnly);
    stream.setVersion(QDataStream::Qt_5_9);

    WalletContentList content;
    stream >> content;
    return content;
}

void WalletModel::saveEntryContent(const QString& entry, const WalletContentList& content)
{
    QByteArray rawData;
    QDataStream stream(&rawData, QIODevice::WriteOnly);
    stream.setVersion(QDataStream::Qt_5_9);

    stream << content;

    wallet_->writeEntry(entry, rawData, KWallet::Wallet::Stream);
}
