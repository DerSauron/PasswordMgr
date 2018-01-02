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

#include "WalletWidget.h"
#include "WalletModel.h"
#include "../main.h"
#include "../MainFrame.h"
#include "../StatusBubble.h"
#include "../helper.h"

#include <QInputDialog>
#include <QListWidget>
#include <QBuffer>
#include <QDataStream>
#include <QLabel>
#include <QPushButton>
#include <QMessageBox>
#include <QClipboard>
#include <QItemSelectionModel>
#include <QDebug>

class WalletWidgetDelegate : public WalletDelegate {
public:
    WalletWidgetDelegate(WalletWidget* widget) : widget(widget)
    {
    }

    virtual ~WalletWidgetDelegate()
    {
    }

    QWidget* getWidget()
    {
        return widget;
    }

    QStringList getEntryList()
    {
        return widget->entryList();
    }

    bool savePassword(const QString& entry, const QString& username, const QString& password)
    {
        return widget->savePassword(entry, username, password);
    }

private:
    WalletWidget* widget;
};

WalletDelegate* createWallet()
{
    return new WalletWidgetDelegate(new WalletWidget());
}

WalletWidget::Fields::Fields(QWidget* pane, int num) : pane(pane)
{
    username = pane->findChild<QLineEdit*>(QString("username_%1").arg(num));
    password = pane->findChild<QLineEdit*>(QString("password_%1").arg(num));
}

WalletWidget::Fields::Fields(const WalletWidget::Fields& o) : pane(o.pane), username(o.username),
    password(o.password)
{
}

WalletWidget::Fields& WalletWidget::Fields::operator=(const WalletWidget::Fields& o)
{
    pane = o.pane;
    username = o.username;
    password = o.password;
    return *this;
}

void WalletWidget::Fields::fillData(const WalletContent& content)
{
    pane->setVisible(true);
    username->setText(content.username());
    password->setText(content.password());
}

void WalletWidget::Fields::copyDataFrom(const Fields& other)
{
    username->setText(other.username->text());
    password->setText(other.password->text());
}

void WalletWidget::Fields::clearData()
{
    username->setText(QString());
    password->setText(QString());
    pane->setVisible(false);
}

WalletWidget::WalletWidget() :
    ui(new Ui::WalletWidget())
{
    ui->setupUi(this);
    init();
}

WalletWidget::~WalletWidget()
{
    delete ui;
}

void WalletWidget::init()
{
    walletModel_ = new WalletModel(this);

    QItemSelectionModel* selectionModel = ui->entryList->selectionModel();
    ui->entryList->setModel(walletModel_);
    delete selectionModel;

    ui->entryList->setSelectionMode(QAbstractItemView::SingleSelection);

    connect(ui->entryList->selectionModel(), &QItemSelectionModel::selectionChanged,
            this, &WalletWidget::onListEntryChanged);

    onListEntryChanged(QItemSelection(), QItemSelection());

    walletModel_->openWallet();
}

bool WalletWidget::savePassword(const QString& entry, const QString& username, const QString& password)
{
    QModelIndex pwIndex = walletModel_->addPassword(entry, username, password);
    ui->entryList->selectionModel()->select(pwIndex, QItemSelectionModel::ClearAndSelect);

    return true;
}

QStringList WalletWidget::entryList()
{
    return walletModel_->entryList();
}

void WalletWidget::onListEntryChanged(const QItemSelection& selected, const QItemSelection& deselected)
{
    if (selected.indexes().isEmpty()) {
        ui->contentPages->setCurrentWidget(ui->emptyPage);
        ui->removeEntryBtn->setEnabled(false);
        return;
    }

    ui->contentPages->setCurrentWidget(ui->contentWidget);
    ui->removeEntryBtn->setEnabled(true);

    loadContent(selected.first().topLeft());
}

void WalletWidget::loadContent(const QModelIndex& selectedIndex)
{
    if (!selectedIndex.isValid())
        return;

    WalletContentList contentList = walletModel_->data(
                selectedIndex,
                WalletModel::WalletContentRole)
            .value<WalletContentList>();

    int pos = 0;

    for (WalletContent& content : contentList)
    {
        fields(pos)->fillData(content);

        ++pos;
    }

    fieldsVisible = pos;
    for (int i = pos; i < fieldsList.length(); ++i) {
        fieldsList[i].clearData();
    }
}

void WalletWidget::saveEntryContent()
{
    QModelIndexList indexList = ui->entryList->selectionModel()->selectedIndexes();
    if (indexList.isEmpty())
        return;

    WalletContentList contentList;
    for (Fields& f : fieldsList)
    {
        if (!f.pane->isVisible())
            continue;

        const QString username = f.username->text();
        const QString password = f.password->text();
        if (!username.isEmpty() || !password.isEmpty()) {
            contentList << WalletContent(username, password);
        }
    }

    walletModel_->setData(indexList[0], QVariant::fromValue(contentList), WalletModel::WalletContentRole);
}

void WalletWidget::onAddEntryBtnPressed()
{
    bool ok;
    QString name = QInputDialog::getText(
                this,
                tr("New entry"), tr("Enter a name for the new entry"),
                QLineEdit::Normal, QString(), &ok);
    if (ok)
    {
        if (walletModel_->hasEntry(name))
        {
            QMessageBox::information(
                        this,
                        tr("Context exists"),
                        tr("A context with this name already exists"));
        } else {
            QModelIndex idx = walletModel_->addEntry(name);
            ui->entryList->selectionModel()->select(idx, QItemSelectionModel::ClearAndSelect);
        }
    }
}

void WalletWidget::onRemoveEntryBtnPressed()
{
    QModelIndexList indexList = ui->entryList->selectionModel()->selectedIndexes();
    if (!indexList.isEmpty())
    {
        QMessageBox::StandardButton btn =
            QMessageBox::question(
                    this,
                    tr("Remove entry"),
                    tr("Really remove the selected entry?"),
                    QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
        if (btn != QMessageBox::Yes)
            return;
        walletModel_->removeEntry(indexList[0]);
    }
}

void WalletWidget::removePassword(int pos)
{
    QToolButton* button = qobject_cast<QToolButton*>(sender());
    if (button != 0)
    {
        pos = getPanePos(button);
    }

    if (pos < 0)
        return;

    QString username = fieldsList[pos].username->text();
    if (!username.isEmpty() || !fieldsList[pos].password->text().isEmpty())
    {
        QMessageBox::StandardButton btn = QMessageBox::question(
                    this,
                    tr("Remove password"),
                    tr("Do you want to remove the password %1?").arg(username),
                    QMessageBox::Yes | QMessageBox::No,
                    QMessageBox::No);
        if (btn != QMessageBox::Yes)
            return;
    }

    for (; pos < fieldsList.length() - 1; ++pos)
    {
        fieldsList[pos].copyDataFrom(fieldsList[pos + 1]);
    }

    fieldsList[pos].clearData();
    saveEntryContent();
}

void WalletWidget::copyPassword(int pos)
{
    QToolButton* button = qobject_cast<QToolButton*>(sender());
    if (button != 0)
    {
        pos = getPanePos(button);
    }

    if (pos < 0)
        return;

    QString username = fieldsList[pos].username->text();
    QString passwd = fieldsList[pos].password->text();
    if (passwd.isEmpty())
    {
        return;
    }
    QApplication::clipboard()->setText(passwd);

    getMainFrame()->getStatusBubble()->showText(
                tr("Password for %1 copied to clipboard").arg(username));
}

void WalletWidget::onAddPasswordBtnPressed()
{
    appendPasswordFields();
}

void WalletWidget::onShowPasswordsPressed()
{
    QLineEdit::EchoMode echoMode = ui->showPasswordsCheck->isChecked() ?
                QLineEdit::Normal : QLineEdit::Password;
    for (int i = 0; i < fieldsList.length(); ++i)
    {
        fieldsList[i].password->setEchoMode(echoMode);
    }
}

void WalletWidget::onEntryContentSubmitted()
{
    saveEntryContent();
}

WalletWidget::Fields* WalletWidget::fields(int pos)
{
    if (pos >= fieldsList.length()) {
        QWidget* lastPane = findChild<QWidget*>(QString("entryPane_%1").arg(qMax(0, fieldsList.length() - 1)));

        for (int i = fieldsList.length(); i <= pos; ++i) {
            lastPane = createFieldsPane(lastPane, i);
            Fields fields(lastPane, i);

            fields.password->setEchoMode(ui->showPasswordsCheck->isChecked() ?
                                             QLineEdit::Normal : QLineEdit::Password);

            fieldsList << fields;
        }
    }
    return &fieldsList[pos];
}

QWidget* WalletWidget::createFieldsPane(QWidget* previous, int num)
{
    static QIcon removeIcon;
    if (removeIcon.isNull())
        removeIcon = loadIcon(QString::fromUtf8("list-remove"));
    static QIcon copyIcon;
    if (copyIcon.isNull())
        copyIcon = loadIcon(QString::fromUtf8("edit-copy"));

    QWidget* pane = new QWidget(ui->entryPaneContent);
    pane->setObjectName(QString("entryPane_%1").arg(num));
    QSizePolicy sizePolicy(QSizePolicy::Preferred, QSizePolicy::Maximum);
    sizePolicy.setHorizontalStretch(0);
    sizePolicy.setVerticalStretch(0);
    sizePolicy.setHeightForWidth(pane->sizePolicy().hasHeightForWidth());
    pane->setSizePolicy(sizePolicy);
    pane->setMaximumSize(QSize(16777215, 16777215));
    QHBoxLayout* layout = new QHBoxLayout(pane);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setObjectName(QString("paneLayout_%1").arg(num));

    QLabel* noLabel = new QLabel(pane);
    noLabel->setObjectName(QString("usrLabel_%1").arg(num));
    noLabel->setText(tr("#%1").arg(num));
    layout->addWidget(noLabel);

    QLineEdit* username = new QLineEdit(pane);
    username->setObjectName(QString("username_%1").arg(num));
    username->setPlaceholderText(tr("Username"));
    layout->addWidget(username);

    QLineEdit* password = new QLineEdit(pane);
    password->setObjectName(QString("password_%1").arg(num));
    password->setPlaceholderText(tr("Password"));
    password->setEchoMode(ui->showPasswordsCheck->isChecked() ? QLineEdit::Normal : QLineEdit::Password);
    layout->addWidget(password);

    QToolButton* copyPasswordBtn = new QToolButton(pane);
    copyPasswordBtn->setObjectName(QString("copyPassword_%1").arg(num));
    copyPasswordBtn->setIcon(copyIcon);
    layout->addWidget(copyPasswordBtn);

    QToolButton* removePasswordBtn = new QToolButton(pane);
    removePasswordBtn->setObjectName(QString("removePassword_%1").arg(num));
    removePasswordBtn->setIcon(removeIcon);
    layout->addWidget(removePasswordBtn);

    connect(username, &QLineEdit::editingFinished, this, &WalletWidget::onEntryContentSubmitted);
    connect(password, &QLineEdit::editingFinished, this, &WalletWidget::onEntryContentSubmitted);
    connect(copyPasswordBtn, &QToolButton::clicked, this, &WalletWidget::copyPassword);
    connect(removePasswordBtn, &QToolButton::clicked, this, &WalletWidget::removePassword);

    int pos = previous ? ui->entryPaneLayout->indexOf(previous) + 1 : 1;
    ui->entryPaneLayout->insertWidget(pos, pane);

    return pane;
}

int WalletWidget::getPanePos(QToolButton* button)
{
    int pos;
    QWidget* pane = button->parentWidget();
    for (pos = 0; pos < fieldsList.length(); ++pos) {
        if (pane == fieldsList[pos].pane)
            break;
    }
    if (pos == fieldsList.length())
        return -1;

    return pos;
}

void WalletWidget::appendPasswordFields()
{
    Fields* fields0 = fields(fieldsVisible);
    ++fieldsVisible;

    fields0->pane->setVisible(true);
}
