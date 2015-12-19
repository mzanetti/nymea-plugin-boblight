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

#ifndef BOBCHANNEL_H
#define BOBCHANNEL_H

#include <QColor>
#include <QObject>
#include <QPropertyAnimation>

class BobChannel : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QColor color READ color WRITE setColor NOTIFY colorChanged)

public:
    explicit BobChannel(const int &id, QObject *parent = 0);

    int id() const;

    QColor color() const;
    void setColor(const QColor &color);
    void animateToColor(const QColor &color);

private:
    QPropertyAnimation *m_animation;
    int m_id;
    QColor m_color;
    int m_brightness;

signals:
    void colorChanged();
    void brightnessChanged();

public slots:
};

#endif // BOBCHANNEL_H
