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

#include <QApplication>

static MainFrame* mainFrame;

MainFrame* getMainFrame()
{
    return mainFrame;
}

int main(int argc, char** argv)
{
    QCoreApplication::setOrganizationName("volkarts.com");
    QCoreApplication::setOrganizationDomain("volkarts.com");
    QCoreApplication::setApplicationName("Password Manager");

    QApplication app(argc, argv);

    MainFrame mf;
    mainFrame = &mf;
    mf.setVisible(true);

    return app.exec();
}

