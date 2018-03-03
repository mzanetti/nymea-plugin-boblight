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

#include "bobclient.h"
#include "extern-plugininfo.h"

#include "libboblight/boblight.h"

#include <QDebug>

BobClient::BobClient(const QString &host, const int &port, QObject *parent) :
    QObject(parent),
    m_host(host),
    m_port(port),
    m_connected(false)
{
    m_syncTimer = new QTimer(this);
    m_syncTimer->setSingleShot(false);
    m_syncTimer->setInterval(50);

    connect(m_syncTimer, SIGNAL(timeout()), this, SLOT(sync()));
}

bool BobClient::connectToBoblight()
{
    if (connected()) {
        return true;
    }
    m_boblight = boblight_init();

    //try to connect, if we can't then bitch to stderr and destroy boblight
    if (!boblight_connect(m_boblight, m_host.toLatin1().data(), m_port, 5000000) || !boblight_setpriority(m_boblight, 1)) {
        qCWarning(dcBoblight) << "Failed to connect:" << boblight_geterror(m_boblight);
        boblight_destroy(m_boblight);
        setConnected(false);
        return false;
    }

    qCDebug(dcBoblight) << "Connected to boblightd successfully.";
    for (int i = 0; i < lightsCount(); ++i) {
        BobChannel *channel = new BobChannel(i, this);
        channel->setColor(QColor(255,255,255,0));
        connect(channel, SIGNAL(colorChanged()), this, SLOT(sync()));
        m_channels.append(channel);
    }

    setColor(-1, m_defaultColor);
    setConnected(true);
    return true;
}

bool BobClient::connected()
{
    return m_connected;
}

void BobClient::setDefaultColor(const QColor &color)
{
    m_defaultColor = color;
}

void BobClient::setPriority(const int &priority)
{
    qCDebug(dcBoblight) << "setting priority to" << priority << boblight_setpriority(m_boblight, priority);
}

void BobClient::setPower(int channel, bool power)
{
    QColor color = currentColor(channel);
    if (power) {
        color.setAlpha(255);
    } else {
        color.setAlpha(0);
    }
    setColor(channel, color);
}

BobChannel *BobClient::getChannel(const int &id)
{
    foreach (BobChannel *channel, m_channels) {
        if (channel->id() == id)
            return channel;
    }
    return 0;
}

void BobClient::setColor(int channel, QColor color)
{    
    if (channel == -1) {
        for (int i = 0; i < lightsCount(); ++i) {
            setColor(i, color);
        }
    } else {
        BobChannel *c = getChannel(channel);
        if (c)
            c->animateToColor(color);
            qCDebug(dcBoblight) << "set channel" << channel << "to color" << color;
    }
}

void BobClient::setBrightness(int brightness)
{
    for (int i = 0; i < lightsCount(); ++i) {
        QColor color = currentColor(i);
        color.setAlpha(qRound(brightness * 255.0 / 100));
        setColor(i, color);
    }
}

void BobClient::sync()
{
    if (!m_connected)
        return;

    foreach (BobChannel *channel, m_channels) {
        int rgb[3];
        rgb[0] = channel->color().red() * channel->color().alphaF();
        rgb[1] = channel->color().green() * channel->color().alphaF();
        rgb[2] = channel->color().blue() * channel->color().alphaF();
        boblight_addpixel(m_boblight, channel->id(), rgb);
    }

    if (!boblight_sendrgb(m_boblight, 1, NULL)) {
        qCWarning(dcBoblight) << "Boblight connection error:" << boblight_geterror(m_boblight);
        boblight_destroy(m_boblight);
        setConnected(false);
    }
}

void BobClient::setConnected(bool connected)
{
    m_connected = connected;
    emit connectionChanged();

    // if disconnected, delete all channels
    if (!connected) {
        m_syncTimer->stop();
        qDeleteAll(m_channels);
    } else {
        m_syncTimer->start();
    }
}

int BobClient::lightsCount()
{
    return boblight_getnrlights(m_boblight);
}

QColor BobClient::currentColor(const int &channel)
{
    return getChannel(channel)->color();
}
