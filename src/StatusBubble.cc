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

#include "StatusBubble.h"

#include <QPropertyAnimation>

#include <QDebug>

QVariant fontSizeInterpolator(const QFont& start, const QFont& end, qreal progress)
{
    QFont currentValue(start);

    currentValue.setPointSizeF(
        start.pointSizeF() + (end.pointSizeF() - start.pointSizeF()) * progress);

    return currentValue;
}

StatusBubble::StatusBubble(QWidget* parent) : QLabel(parent)
{
    init();
}

StatusBubble::~StatusBubble()
{
}

void StatusBubble::init()
{
    origColor = textColor();
    QColor hiddenCol(origColor);
    hiddenCol.setAlpha(0);

    origFont = font();
    QFont startFont(origFont);
    startFont.setPointSizeF(0.1);

    static int inAnimDuration = 300;
    static int outAnimDuration = 1000;

    QPropertyAnimation* colorAnim = new QPropertyAnimation(this, "textColor");
    colorAnim->setDuration(inAnimDuration);
    colorAnim->setStartValue(hiddenCol);
    colorAnim->setEndValue(origColor);
    inAnimation.addAnimation(colorAnim);
    QPropertyAnimation* fontAnim = new QPropertyAnimation(this, "font");
    fontAnim->setDuration(inAnimDuration / 3);
    fontAnim->setStartValue(startFont);
    fontAnim->setEndValue(origFont);
    inAnimation.addAnimation(fontAnim);
    connect(&inAnimation, SIGNAL(finished()), this, SLOT(handleShowFinished()));

    QPropertyAnimation* ocolorAnim = new QPropertyAnimation(this, "textColor");
    ocolorAnim->setDuration(outAnimDuration);
    ocolorAnim->setStartValue(origColor);
    ocolorAnim->setEndValue(hiddenCol);
    outAnimation.addAnimation(ocolorAnim);
    connect(&outAnimation, SIGNAL(finished()), this, SLOT(handleHideFinished()));

    displayTimer.setSingleShot(true);
    connect(&displayTimer, SIGNAL(timeout()), this, SLOT(startHiding()));

    showing = false;
}

void StatusBubble::resetProperties()
{
    setTextColor(origColor);
    setFont(origFont);
}

QColor StatusBubble::textColor() const
{
    return palette().color(QPalette::WindowText);
}

void StatusBubble::setTextColor(const QColor& color)
{
    QPalette palette(QLabel::palette());
    palette.setColor(QPalette::WindowText, color);
    setPalette(palette);
}

void StatusBubble::showText(const QString& text, Duration duration)
{
    this->duration = duration;

    setText(text);

    if (!showing) {
        inAnimation.start(QAbstractAnimation::KeepWhenStopped);
    } else {
        if (inAnimation.state() != QAbstractAnimation::Running) {
            handleShowFinished();   // restart timer
        }
        if (outAnimation.state() == QAbstractAnimation::Running) {
            outAnimation.stop();
            resetProperties();
        }
    }

    showing = true;
}

void StatusBubble::handleShowFinished()
{
    displayTimer.start(duration);
}

void StatusBubble::startHiding()
{
    outAnimation.start(QAbstractAnimation::KeepWhenStopped);
}

void StatusBubble::handleHideFinished()
{
    showing = false;
}

// interpolator registration
static int registerInterpolators()
{
    qRegisterAnimationInterpolator<QFont>(fontSizeInterpolator);
    return 1;
}
Q_CONSTRUCTOR_FUNCTION(registerInterpolators)

static void unregisterInterpolators()
{
    qRegisterAnimationInterpolator<QFont>(0);
}
Q_DESTRUCTOR_FUNCTION(unregisterInterpolators)
