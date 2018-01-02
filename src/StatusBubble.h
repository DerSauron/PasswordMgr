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

#ifndef STATUSBUBBLE_H
#define	STATUSBUBBLE_H

#include <QLabel>
#include <QParallelAnimationGroup>
#include <QColor>
#include <QTimer>

class StatusBubble : public QLabel {
    Q_OBJECT

    Q_PROPERTY(QColor textColor READ textColor WRITE setTextColor)

public:
    enum Duration {
        Short = 1000,
        Medium = 3000,
        Long = 5000,
    };

    StatusBubble(QWidget* parent=0);
    virtual ~StatusBubble();

    QColor textColor() const;
    void setTextColor(const QColor& color);

    void showText(const QString& text, Duration duration=Medium);

private slots:
    void handleShowFinished();
    void startHiding();
    void handleHideFinished();

private:
    Duration duration;
    QParallelAnimationGroup inAnimation;
    QParallelAnimationGroup outAnimation;
    QTimer displayTimer;

    QFont origFont;
    QColor origColor;

    bool showing;


    void init();
    void resetProperties();
};

#endif	/* STATUSBUBBLE_H */
