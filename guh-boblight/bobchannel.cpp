/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2015 Simon Stuerz <simon.stuerz@guh.guru>                *
 *                                                                         *
 *  This file is part of guh.                                              *
 *                                                                         *
 *  Guh is free software: you can redistribute it and/or modify            *
 *  it under the terms of the GNU General Public License as published by   *
 *  the Free Software Foundation, version 2 of the License.                *
 *                                                                         *
 *  Guh is distributed in the hope that it will be useful,                 *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          *
 *  GNU General Public License for more details.                           *
 *                                                                         *
 *  You should have received a copy of the GNU General Public License      *
 *  along with guh. If not, see <http://www.gnu.org/licenses/>.            *
 *                                                                         *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include "bobchannel.h"

BobChannel::BobChannel(const int &id, QObject *parent) :
    QObject(parent),
    m_id(id)
{
    m_animation = new QPropertyAnimation(this, "color", this);
    m_animation->setDuration(1500);
    m_animation->setEasingCurve(QEasingCurve::Linear);
}

int BobChannel::id() const
{
    return m_id;
}

QColor BobChannel::color() const
{
    return m_color;
}

void BobChannel::setColor(const QColor &color)
{
    m_color = color;
    emit colorChanged();
}

void BobChannel::animateToColor(const QColor &color)
{
    if (m_animation->state() == QPropertyAnimation::Running)
        m_animation->stop();

    m_animation->setStartValue(m_color);
    m_animation->setEndValue(color);
    m_animation->start();
}

