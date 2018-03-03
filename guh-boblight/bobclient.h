/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2015 Simon Stuerz <simon.stuerz@guh.guru>                *
 *  Copyright (C) 2014 Michael Zanetti <michael_zanetti@gmx.net>           *
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

#ifndef BOBCLIENT_H
#define BOBCLIENT_H

#include <QObject>
#include <QTimer>
#include <QMap>
#include <QColor>
#include <QTime>

#include <bobchannel.h>

class BobClient : public QObject
{
    Q_OBJECT
public:
    explicit BobClient(const QString &host = "127.0.0.1", const int &port = 19333, QObject *parent = 0);

    bool connectToBoblight();
    bool connected();

    void setDefaultColor(const QColor &color);

    int lightsCount();
    QColor currentColor(const int &channel);

    void setPriority(const int &priority);
    void setPower(int channel, bool power);

private:
    void *m_boblight;

    QTimer *m_syncTimer;
    QString m_host;
    int m_port;
    bool m_connected;
    QColor m_defaultColor;


    QMap<int, QColor> m_colors;
    QList<BobChannel *> m_channels;

    BobChannel *getChannel(const int &id);

public slots:
    void setColor(int channel, QColor color);
    void setBrightness(int brightness);

private slots:
    void sync();
    void setConnected(bool connected);

signals:
    void connectionChanged();
};

#endif // BOBCLIENT_H
