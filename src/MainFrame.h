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

#ifndef _MAINFRAME_H
#define	_MAINFRAME_H

#include "ui_MainFrame.h"

#include <QCloseEvent>

class StatusBubble;
class WalletDelegate;
class PasswordGenerator;

class MainFrame : public QMainWindow {
    Q_OBJECT
    const static int CHAR_CLASSES = 4;

public:
    MainFrame();
    virtual ~MainFrame();

    StatusBubble* getStatusBubble() const;

protected:
    virtual void closeEvent(QCloseEvent* /*event*/);

private slots:
    void handleGeneratePressed();
    void handleCopyToClipboardPressed();
    void handleSavePasswordPressed();
    void handleHistoryDblClick(QListWidgetItem* item);
    void handlePasswordLengthChange(int value);
    void handleMinSpinnerChange(int value);
    void handleComboboxChange(const QString& newText);
    void handleCharsCheckboxChange(int);
    void handlePresetNameChange(const QString& presetName);
    void handlePresetNameTextChange(const QString& presetName);
    void handleSavePresetPressed();
    void handleDeletePresetPressed();

private:
    class OptionSet {
    public:
        OptionSet();
        OptionSet(MainFrame* mainFrame, const QString& name);
        OptionSet(MainFrame* mainFrame, const QString& name, QByteArray& data);
        OptionSet(const OptionSet& other);
        ~OptionSet();

        OptionSet& operator=(const OptionSet& other);

        void readState();
        void writeState();
        bool checkState();

        QByteArray toByteArray();

        MainFrame* mainFrame;
        QString name;
        Qt::CheckState lowerCaseCharsOn;
        QString lowerCaseChars;
        Qt::CheckState upperCaseCharsOn;
        QString upperCaseChars;
        Qt::CheckState numbersOn;
        QString numbers;
        Qt::CheckState specialCharsOn;
        QString specialChars;
        int length;
        bool failedState;
    };

    Ui::MainFrame* ui;
    StatusBubble* statusBubble;
    WalletDelegate* wallet;

    QWidget* charForms[CHAR_CLASSES];
    QCheckBox* charClassToggles[CHAR_CLASSES];
    QSpinBox* charClassMin[CHAR_CLASSES];
    QComboBox* charClass[CHAR_CLASSES];

    QMap<QString, OptionSet> presetList;
    bool isSelectingPreset;

    int minimumPasswordLength;

    QScopedPointer<PasswordGenerator> generator_;

    void init();
    void saveConfig();
    void loadConfig();
    void addOptionSet(const OptionSet& optionSet);
    void selectPreset(const QString& presetName);
    void shouldResetPresetName();
};

#endif	/* _MAINFRAME_H */
