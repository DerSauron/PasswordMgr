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

#include "MainFrame.h"
#include "PasswordGenerator.h"
#include "PasswordListItem.h"
#include "helper.h"
#include "StatusBubble.h"
#include "WalletDelegate.h"

#include <QClipboard>
#include <QSettings>
#include <QDialog>
#include <QFormLayout>
#include <QDialogButtonBox>
#include <QComboBox>
#include <QMessageBox>

#define FILL_ARRAY(arr, o0, o1, o2, o3) \
    arr[0] = ui->o0; arr[1] = ui->o1; arr[2] = ui->o2; arr[3] = ui->o3

#define STREAM_FAILED_RETURN() \
    if (stream.status() != QDataStream::Ok) { \
        failedState = true; \
        return; \
    }

QStringList getComboBoxItems(QComboBox* cb) {
    QStringList list;
    for (int i = 0; i < cb->model()->rowCount(QModelIndex()); ++i) {
        list << cb->itemText(i);
    }
    return list;
}

void setComboBoxItems(QComboBox* cb, QStringList& list) {
    QString currentText = cb->lineEdit()->text();
    for (int i = 0; i < list.count(); ++i) {
        QString entry = list[i];
        if (currentText != entry) {
            cb->addItem(entry);
        }
    }
}

static QDataStream& operator>>(QDataStream& stream, Qt::CheckState& checkState) {
    int i;
    stream >> i;
    checkState = (Qt::CheckState)i;
    return stream;
}

MainFrame::OptionSet::OptionSet() {
    failedState = true;
}

MainFrame::OptionSet::OptionSet(MainFrame* mainFrame, const QString& name) : mainFrame(mainFrame),
name(name) {
    readState();
}

MainFrame::OptionSet::OptionSet(MainFrame* mainFrame, const QString& name, QByteArray& data) :
mainFrame(mainFrame),
name(name) {
    QDataStream stream(&data, QIODevice::ReadOnly);
    stream.setVersion(QDataStream::Qt_4_8);
    stream >> lowerCaseCharsOn;
    STREAM_FAILED_RETURN();
    stream >> lowerCaseChars;
    STREAM_FAILED_RETURN();
    stream >> upperCaseCharsOn;
    STREAM_FAILED_RETURN();
    stream >> upperCaseChars;
    STREAM_FAILED_RETURN();
    stream >> numbersOn;
    STREAM_FAILED_RETURN();
    stream >> numbers;
    STREAM_FAILED_RETURN();
    stream >> specialCharsOn;
    STREAM_FAILED_RETURN();
    stream >> specialChars;
    STREAM_FAILED_RETURN();
    stream >> length;
    STREAM_FAILED_RETURN();
}

MainFrame::OptionSet::OptionSet(const MainFrame::OptionSet& other) :
    mainFrame(other.mainFrame),
    name(other.name),
    lowerCaseCharsOn(other.lowerCaseCharsOn),
    lowerCaseChars(other.lowerCaseChars),
    upperCaseCharsOn(other.upperCaseCharsOn),
    upperCaseChars(other.upperCaseChars),
    numbersOn(other.numbersOn),
    numbers(other.numbers),
    specialCharsOn(other.specialCharsOn),
    specialChars(other.specialChars),
    length(other.length),
    failedState(other.failedState)
{
}

MainFrame::OptionSet::~OptionSet() {

}

MainFrame::OptionSet& MainFrame::OptionSet::operator=(const MainFrame::OptionSet& other) {
    mainFrame = other.mainFrame;
    name = other.name;
    lowerCaseCharsOn = other.lowerCaseCharsOn;
    lowerCaseChars = other.lowerCaseChars;
    upperCaseCharsOn = other.upperCaseCharsOn;
    upperCaseChars = other.upperCaseChars;
    numbersOn = other.numbersOn;
    numbers = other.numbers;
    specialCharsOn = other.specialCharsOn;
    specialChars = other.specialChars;
    length = other.length;
    failedState = other.failedState;
    return *this;
}

void MainFrame::OptionSet::readState() {
    lowerCaseCharsOn = mainFrame->ui->useLowerCaseChars->checkState();
    lowerCaseChars = mainFrame->ui->lowerCaseChars->currentText();
    upperCaseCharsOn = mainFrame->ui->useUpperCaseChars->checkState();
    upperCaseChars = mainFrame->ui->upperCaseChars->currentText();
    numbersOn = mainFrame->ui->useNumbers->checkState();
    numbers = mainFrame->ui->numbers->currentText();
    specialCharsOn = mainFrame->ui->useSpecialChars->checkState();
    specialChars = mainFrame->ui->specialChars->currentText();
    length = mainFrame->ui->passwordLength->value();
    failedState = false;
}

void MainFrame::OptionSet::writeState() {
    if (failedState)
        return;

    mainFrame->ui->useLowerCaseChars->setCheckState(lowerCaseCharsOn);
    mainFrame->ui->lowerCaseChars->setEditText(lowerCaseChars);
    mainFrame->ui->useUpperCaseChars->setCheckState(upperCaseCharsOn);
    mainFrame->ui->upperCaseChars->setEditText(upperCaseChars);
    mainFrame->ui->useNumbers->setCheckState(numbersOn);
    mainFrame->ui->numbers->setEditText(numbers);
    mainFrame->ui->useSpecialChars->setCheckState(specialCharsOn);
    mainFrame->ui->specialChars->setEditText(specialChars);
    mainFrame->ui->passwordLength->setValue(length);
}

bool MainFrame::OptionSet::checkState() {
    return lowerCaseCharsOn == mainFrame->ui->useLowerCaseChars->checkState()
        && lowerCaseChars == mainFrame->ui->lowerCaseChars->currentText()
        && upperCaseCharsOn == mainFrame->ui->useUpperCaseChars->checkState()
        && upperCaseChars == mainFrame->ui->upperCaseChars->currentText()
        && numbersOn == mainFrame->ui->useNumbers->checkState()
        && numbers == mainFrame->ui->numbers->currentText()
        && specialCharsOn == mainFrame->ui->useSpecialChars->checkState()
        && specialChars == mainFrame->ui->specialChars->currentText()
        && length == mainFrame->ui->passwordLength->value();
}

QByteArray MainFrame::OptionSet::toByteArray() {
    QByteArray rawData;

    if (!failedState) {
        QDataStream stream(&rawData, QIODevice::WriteOnly);
        stream.setVersion(QDataStream::Qt_4_8);
        stream << lowerCaseCharsOn;
        stream << lowerCaseChars;
        stream << upperCaseCharsOn;
        stream << upperCaseChars;
        stream << numbersOn;
        stream << numbers;
        stream << specialCharsOn;
        stream << specialChars;
        stream << length;
    }

    return rawData;
}

MainFrame::MainFrame() :
    ui(new Ui::MainFrame()),
    generator_(new PasswordGenerator())
{
    ui->setupUi(this);
    init();
}

MainFrame::~MainFrame()
{
    delete ui;
}

void MainFrame::init()
{
    FILL_ARRAY(charForms, lowerCaseForm, upperCaseForm, numbersForm, specialsForm);
    FILL_ARRAY(charClassToggles, useLowerCaseChars, useUpperCaseChars, useNumbers, useSpecialChars);
    FILL_ARRAY(charClassMin, lowerCaseMin, upperCaseMin, numbersMin, specialsMin);
    FILL_ARRAY(charClass, lowerCaseChars, upperCaseChars, numbers, specialChars);

    loadConfig();

    handleCharsCheckboxChange(0);
    handleMinSpinnerChange(0);
    handlePasswordLengthChange(ui->passwordLength->value());

    wallet = createWallet();
    if (wallet)
    {
        ui->tabWidget->addTab(wallet->getWidget(), "Saved Passwords");
        ui->savePasswordButton->setVisible(true);
    }
    else
    {
        ui->savePasswordButton->setVisible(false);
    }

    ui->statusBar->addWidget(statusBubble = new StatusBubble());

    isSelectingPreset = false;
}

StatusBubble* MainFrame::getStatusBubble() const {
    return statusBubble;
}

void MainFrame::closeEvent(QCloseEvent* event) {
    saveConfig();
    QMainWindow::closeEvent(event);
}

void MainFrame::loadConfig() {
    QSettings s;

    restoreGeometry(s.value("window/geometry").toByteArray());
    restoreState(s.value("window/state").toByteArray(), 0);

    for (int i = 0; i < CHAR_CLASSES; ++i) {
        charClassToggles[i]->setChecked(
            s.value(QString("characters/%1").arg(charClassToggles[i]->objectName()),
            charClassToggles[i]->isChecked()).toBool());

        charClassMin[i]->setValue(
            s.value(QString("characters/%1").arg(charClassMin[i]->objectName()),
            charClassMin[i]->value()).toInt());

        QStringList list =
            s.value(QString("characters/%1-data").arg(charClass[i]->objectName())).toStringList();
        if (!list.isEmpty()) {
            setComboBoxItems(charClass[i], list);

            charClass[i]->setCurrentIndex(
                s.value(QString("characters/%1-index").arg(charClass[i]->objectName()),
                charClass[i]->currentIndex()).toInt());
        }
    }

    s.beginGroup("presets");
    QStringList presetNames = s.childKeys();
    QString selectedPresetName;
    for (int i = 0; i < presetNames.size(); ++i) {
        QByteArray bytes(s.value(presetNames[i]).toByteArray());
        OptionSet optionSet(this, presetNames[i], bytes);
        if (!optionSet.failedState) {
            addOptionSet(optionSet);
            if (selectedPresetName.isNull() && optionSet.checkState()) {
                selectedPresetName = presetNames[i];
            }
        } else {
            // clean incomplete presets
            s.remove(presetNames[i]);
        }
    }
    s.endGroup();
    selectPreset(selectedPresetName);
}

void MainFrame::saveConfig() {
    QSettings s;

    s.setValue("window/geometry", saveGeometry());
    s.setValue("window/state", saveState(0));

    for (int i = 0; i < CHAR_CLASSES; ++i) {
        s.setValue(QString("characters/%1").arg(charClassToggles[i]->objectName()),
            charClassToggles[i]->isChecked());

        s.setValue(QString("characters/%1").arg(charClassMin[i]->objectName()),
            charClassMin[i]->value());

        s.setValue(QString("characters/%1-data").arg(charClass[i]->objectName()),
            getComboBoxItems(charClass[i]));

        s.setValue(QString("characters/%1-index").arg(charClass[i]->objectName()),
            charClass[i]->currentIndex());
    }

    for (QMap<QString, OptionSet>::iterator it = presetList.begin(); it != presetList.end(); ++it) {
        s.setValue(QString("presets/%1").arg(it.key()), it.value().toByteArray());
    }
}

void MainFrame::handleGeneratePressed() {


    CharacterStock stock;
    for (int i = 0; i < CHAR_CLASSES; ++i) {
        if (charClassToggles[i]->isChecked()) {
            populateComboBox(charClass[i]);
            stock.add(charClass[i]->currentText(), charClassMin[i]->value());
        }
    }

    int length = ui->passwordLength->value();
    QString password = generator_->generate(stock, length);

    ui->output->setText(password);
    ui->historyList->insertItem(0, new PasswordListItem(password, length));

    statusBubble->showText(tr("New password with %1 chars generated").arg(length));
}

void MainFrame::handleCopyToClipboardPressed() {
    QString passwd = ui->output->text();

    if (passwd.isEmpty()) {
        return;
    }

    QApplication::clipboard()->setText(passwd);

    statusBubble->showText(tr("Password copied to clipboard"));
}

void MainFrame::handleSavePasswordPressed() {
    if (!wallet || ui->output->text().isEmpty())
        return;

    QDialog dialog(this);

    QFormLayout form(&dialog);
    form.addRow(new QLabel("Select an entry and specify an user name for the password"));

    QComboBox* entryName = new QComboBox(&dialog);
    entryName->insertItems(0, wallet->getEntryList());
    entryName->setEditable(true);
    entryName->setInsertPolicy(QComboBox::NoInsert);
    entryName->setSizePolicy(QSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed));
    entryName->lineEdit()->setPlaceholderText(tr("Select an entry or specify a new one"));
    entryName->setCurrentIndex(-1);
    form.addRow(QString("Entry"), entryName);

    QLineEdit* username = new QLineEdit(&dialog);
    username->setPlaceholderText(tr("Leave empty for no username"));
    form.addRow(QString("Username"), username);

    QDialogButtonBox buttonBox(QDialogButtonBox::Save | QDialogButtonBox::Cancel, Qt::Horizontal, &dialog);
    form.addRow(&buttonBox);
    connect(&buttonBox, &QDialogButtonBox::accepted, [&dialog, entryName, username, this] ()
    {
        if (!entryName->currentText().isEmpty())
        {
            ui->tabWidget->setCurrentWidget(wallet->getWidget());
            wallet->savePassword(entryName->currentText(), username->text(), ui->output->text());
            statusBubble->showText(tr("Password saved"));
            emit dialog.accept();
        }
        else
        {
            QMessageBox::critical(this, tr("Information missing"),
                tr("Select an entry or specify a new one"));
        }
    });
    connect(&buttonBox, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

    if (dialog.exec() == QDialog::Accepted) {
        ui->tabWidget->setCurrentWidget(wallet->getWidget());
    }
}

void MainFrame::handleHistoryDblClick(QListWidgetItem* item) {
    ui->output->setText(item->data(PasswordListItem::PasswordRole).toString());
}

void MainFrame::handlePasswordLengthChange(int value) {
    if (ui->passwordLength->value() < minimumPasswordLength)
        ui->passwordLength->setValue(minimumPasswordLength); // this causes the slot to be called again
    else
        ui->pwLengthDisplay->setText(QString::number(value));

    shouldResetPresetName();
}

void MainFrame::handleCharsCheckboxChange(int)
{
    QCheckBox* sndr = qobject_cast<QCheckBox*>(sender());
    for (int i = 0; i < CHAR_CLASSES; ++i)
    {
        if (sndr == 0 || sndr == charClassToggles[i])
        {
            bool checked = charClassToggles[i]->isChecked();
            QObjectList list = charForms[i]->children();
            for (int e = 0; e < list.count(); e++)
            {
                QWidget* w = qobject_cast<QWidget*>(list[e]);
                if (w != 0 && w != qobject_cast<QWidget*>(charClassToggles[i]))
                    w->setEnabled(checked);
            }
        }
    }

    handleMinSpinnerChange(0);

    shouldResetPresetName();
}

void MainFrame::handleComboboxChange(const QString& newText) {
    QComboBox* combo = qobject_cast<QComboBox*>(sender());

    int idx = combo->findText(newText);
    if (idx != -1) {
        combo->removeItem(idx);
    }
    combo->insertItem(0, newText);

    shouldResetPresetName();
}

void MainFrame::handleMinSpinnerChange(int) {
    minimumPasswordLength = 0;
    for (int i = 0; i < CHAR_CLASSES; ++i) {
        if (charClassToggles[i]->isChecked())
            minimumPasswordLength += charClassMin[i]->value();
    }
    if (ui->passwordLength->value() < minimumPasswordLength)
        ui->passwordLength->setValue(minimumPasswordLength);

    shouldResetPresetName();
}

void MainFrame::handlePresetNameChange(const QString& presetName) {
    if (!presetName.isEmpty() && presetList.contains(presetName)) {
        isSelectingPreset = true;
        presetList[presetName].writeState();
        isSelectingPreset = false;
    }
    ui->savePresetButton->setEnabled(!presetName.isEmpty());
    ui->deletePresetButton->setEnabled(!presetName.isEmpty());
}

void MainFrame::handlePresetNameTextChange(const QString& presetName) {
    ui->savePresetButton->setEnabled(!presetName.isEmpty());
    ui->deletePresetButton->setEnabled(ui->presetName->findText(presetName) >= 0);
}

void MainFrame::handleSavePresetPressed() {
    QString presetName = ui->presetName->lineEdit()->text();
    if (!presetName.isEmpty()) {
        addOptionSet(OptionSet(this, presetName));
        statusBubble->showText(tr("Preset %1 saved").arg(presetName));
    }
}

void MainFrame::handleDeletePresetPressed() {

}

void MainFrame::addOptionSet(const OptionSet& optionSet) {
    QString presetName(optionSet.name);

    if (!presetList.contains(presetName)) {
        ui->presetName->insertItem(0, presetName);
        ui->presetName->model()->sort(0);
    }
    presetList[presetName] = optionSet;
}

void MainFrame::selectPreset(const QString& presetName) {
    int idx = presetName.isEmpty() ? -1 : ui->presetName->findText(presetName);
    // this causes a reload even if presetName is already selected
    ui->presetName->setCurrentIndex(idx);
}

void MainFrame::shouldResetPresetName() {
    if (!isSelectingPreset && ui->presetName->currentIndex() >= 0) {
        QString presetName = ui->presetName->currentText();
        if (presetList.contains(presetName)) {
            if (!presetList[presetName].checkState()) {
                selectPreset(QString());
            }
        }
    }
}
