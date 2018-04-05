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

/*!
    \page boblight.html
    \title Boblight

    \ingroup plugins
    \ingroup network

    This plugin allows to communicate with a \l{https://code.google.com/p/boblight/}{boblight} server
    running on localhost:19333. If a boblight server is running ,the configured light devices from the server will
    appear automatically in guh.

    \chapter Plugin properties
    Following JSON file contains the definition and the description of all available \l{DeviceClass}{DeviceClasses}
    and \l{Vendor}{Vendors} of this \l{DevicePlugin}.

    Each \l{DeviceClass} has a list of \l{ParamType}{paramTypes}, \l{ActionType}{actionTypes}, \l{StateType}{stateTypes}
    and \l{EventType}{eventTypes}. The \l{DeviceClass::CreateMethod}{createMethods} parameter describes how the \l{Device}
    will be created in the system. A device can have more than one \l{DeviceClass::CreateMethod}{CreateMethod}.
    The \l{DeviceClass::SetupMethod}{setupMethod} describes the setup method of the \l{Device}.
    The detailed implementation of each \l{DeviceClass} can be found in the source code.

    \note If a \l{StateType} has the parameter \tt{"writable": {...}}, an \l{ActionType} with the same uuid and \l{ParamType}{ParamTypes}
    will be created automatically.

    \quotefile plugins/deviceplugins/boblight/devicepluginboblight.json
*/

#include "devicepluginboblight.h"

#include "plugin/device.h"
#include "devicemanager.h"

#include "bobclient.h"
#include "plugininfo.h"
#include "plugintimer.h"

#include <QDebug>
#include <QStringList>
#include <QtMath>

DevicePluginBoblight::DevicePluginBoblight()
{
}

void DevicePluginBoblight::init()
{
    m_pluginTimer = hardwareManager()->pluginTimerManager()->registerTimer(15);
    connect(m_pluginTimer, &PluginTimer::timeout, this, &DevicePluginBoblight::guhTimer);
}

void DevicePluginBoblight::deviceRemoved(Device *device)
{
    if (device->deviceClassId() == boblightServerDeviceClassId) {
        BobClient *client = m_bobClients.take(device->id());
        client->deleteLater();
    }
}

void DevicePluginBoblight::startMonitoringAutoDevices()
{
    m_canCreateAutoDevices = true;
    qCDebug(dcBoblight()) << "Populating auto devices" << myDevices().count();
    QHash<DeviceId, int> parentDevices;
    QHash<DeviceId, Device*> deviceIds;
    foreach (Device *device, myDevices()) {
        deviceIds.insert(device->id(), device);
        if (device->deviceClassId() == boblightServerDeviceClassId) {
            qWarning() << "Device" << device->id() << "is bridge";
            if (!parentDevices.contains(device->id())) {
                parentDevices[device->id()] = 0;
            }
        } else if (device->deviceClassId() == boblightDeviceClassId) {
            qWarning() << "Device" << device->id() << "is child to bridge" << device->parentId();
            parentDevices[device->parentId()] += 1;
        }
    }
    qWarning() << "have bridge devices:" << parentDevices.count();
    QList<DeviceDescriptor> descriptors;
    foreach (const DeviceId &id, parentDevices.keys()) {
        if (parentDevices.value(id) < deviceIds.value(id)->paramValue(boblightServerChannelsParamTypeId).toInt()) {
            for (int i = parentDevices.value(id); i < deviceIds.value(id)->paramValue(boblightServerChannelsParamTypeId).toInt(); i++) {
                DeviceDescriptor descriptor(boblightDeviceClassId, deviceIds.value(id)->name() + " " + QString::number(i + 1), QString(), id);
                descriptor.setParams(ParamList() << Param(boblightChannelParamTypeId, i));
                qCDebug(dcBoblight()) << "Adding new boblight channel" << i + 1;
                descriptors.append(descriptor);
            }
        }
    }
    if (!descriptors.isEmpty()) {
        emit autoDevicesAppeared(boblightDeviceClassId, descriptors);
    }
}

void DevicePluginBoblight::guhTimer()
{
    foreach (BobClient *client, m_bobClients) {
        if (!client->connected()) {
            client->connectToBoblight();
        }
    }
}

void DevicePluginBoblight::onPowerChanged(int channel, bool power)
{
    qCDebug(dcBoblight()) << "power changed" << channel << power;
    BobClient *sndr = dynamic_cast<BobClient*>(sender());
    foreach (Device* device, myDevices()) {
        if (m_bobClients.value(device->parentId()) == sndr && device->paramValue(boblightChannelParamTypeId).toInt() == channel) {
            qCWarning(dcBoblight()) << "setting state power" << power;
            device->setStateValue(boblightPowerStateTypeId, power);
        }
    }
}

void DevicePluginBoblight::onBrightnessChanged(int channel, int brightness)
{
    BobClient *sndr = dynamic_cast<BobClient*>(sender());
    foreach (Device* device, myDevices()) {
        if (m_bobClients.value(device->parentId()) == sndr && device->paramValue(boblightChannelParamTypeId).toInt() == channel) {
            device->setStateValue(boblightBrightnessStateTypeId, brightness);
        }
    }
}

void DevicePluginBoblight::onColorChanged(int channel, const QColor &color)
{
    BobClient *sndr = dynamic_cast<BobClient*>(sender());
    foreach (Device* device, myDevices()) {
        if (m_bobClients.value(device->parentId()) == sndr && device->paramValue(boblightChannelParamTypeId).toInt() == channel) {
            device->setStateValue(boblightColorStateTypeId, color);
        }
    }
}

// mired color temp to rgb
// mired: m = 1000000/T(kelvin)
QColor DevicePluginBoblight::tempToRgb(int miredColorTemp)
{
    // hue: cold:      0: Param(Id: {d25423e7-b924-4b20-80b6-77eecc65d089}, Value:QVariant(QColor, QColor(AHSV 1, 0.622222, 0.176471, 1)))
    // hue: warm:      0: Param(Id: {d25423e7-b924-4b20-80b6-77eecc65d089}, Value:QVariant(QColor, QColor(AHSV 1, 0.111111, 0.839216, 1)))

    // this; cold:  I | Boblight: set channel 1 to color QColor(ARGB 1, 0.678431, 0.784314, 1)
    // this: warm:  I | Boblight: set channel 1 to color QColor(ARGB 1, 0.576471, 0.713725, 1)

    // convert mired color temp to kelvin
    int temp = 1000000/miredColorTemp;

//    temp = temp / 100;

    int red = temp - 60;
    red = 329.698727446 * qPow(red, -0.1332047592);

    int green = temp - 60;
    green = 99.4708025861 * qLn(green) - 161.1195681661;

    int blue = 255;
    qWarning() << "temp:" << temp << "rgb" << red << green << blue;
    return QColor(red, green, blue);
}

DeviceManager::DeviceSetupStatus DevicePluginBoblight::setupDevice(Device *device)
{
    if (device->deviceClassId() == boblightServerDeviceClassId) {

        BobClient *bobClient = new BobClient(device->paramValue(boblightServerHostAddressParamTypeId).toString(), device->paramValue(boblightServerPortParamTypeId).toInt(), this);
        if (!bobClient->connectToBoblight()) {
            qCWarning(dcBoblight()) << "Error connecting to boblight...";
            bobClient->deleteLater();
            return DeviceManager::DeviceSetupStatusFailure;
        }
        qCDebug(dcBoblight()) << "Connected to boblight";
        device->setStateValue(boblightServerConnectedStateTypeId, true);
        m_bobClients.insert(device->id(), bobClient);
        connect(bobClient, SIGNAL(connectionChanged()), this, SLOT(onConnectionChanged()));
        connect(bobClient, &BobClient::powerChanged, this, &DevicePluginBoblight::onPowerChanged);
        connect(bobClient, &BobClient::brightnessChanged, this, &DevicePluginBoblight::onBrightnessChanged);
        connect(bobClient, &BobClient::colorChanged, this, &DevicePluginBoblight::onColorChanged);

        return DeviceManager::DeviceSetupStatusSuccess;
    }

    return DeviceManager::DeviceSetupStatusSuccess;
}

void DevicePluginBoblight::postSetupDevice(Device *device)
{
    if (device->deviceClassId() == boblightServerDeviceClassId && m_canCreateAutoDevices) {
        startMonitoringAutoDevices();
    }
    if (device->deviceClassId() == boblightDeviceClassId) {
        BobClient *bobClient = m_bobClients.value(device->parentId());
        if (bobClient) {
            device->setStateValue(boblightConnectedStateTypeId, bobClient->connected());

            QColor color = device->stateValue(boblightColorStateTypeId).value<QColor>();
            int brightness = device->stateValue(boblightBrightnessStateTypeId).toInt();
            bool power = device->stateValue(boblightPowerStateTypeId).toBool();

            bobClient->setColor(device->paramValue(boblightChannelParamTypeId).toInt(), color);
            bobClient->setBrightness(device->paramValue(boblightChannelParamTypeId).toInt(), brightness);
            bobClient->setPower(device->paramValue(boblightChannelParamTypeId).toInt(), power);

        }
    }
}

DeviceManager::DeviceError DevicePluginBoblight::executeAction(Device *device, const Action &action)
{
//    if (!device->setupComplete()) {
//        return DeviceManager::DeviceErrorHardwareNotAvailable;
//    }
    qCDebug(dcBoblight()) << "Execute action for boblight" << action.params();
    if (device->deviceClassId() == boblightServerDeviceClassId) {
        BobClient *bobClient = m_bobClients.value(device->id());
        if (!bobClient || !bobClient->connected()) {
            qCWarning(dcBoblight()) << "Boblight on" << device->paramValue(boblightServerHostAddressParamTypeId).toString() << "not connected";
            return DeviceManager::DeviceErrorHardwareNotAvailable;
        }

        if (action.actionTypeId() == boblightServerPriorityActionTypeId) {
            bobClient->setPriority(action.param(boblightServerPriorityActionParamTypeId).value().toInt());
            return DeviceManager::DeviceErrorNoError;
        }
        qCWarning(dcBoblight()) << "Unhandled action" << action.actionTypeId() << "for BoblightServer device" << device;
        return DeviceManager::DeviceErrorActionTypeNotFound;
    }

    if (device->deviceClassId() == boblightDeviceClassId) {
        BobClient *bobClient = m_bobClients.value(device->parentId());
        if (action.actionTypeId() == boblightPowerActionTypeId) {
            bobClient->setPower(device->paramValue(boblightChannelParamTypeId).toInt(), action.param(boblightPowerActionParamTypeId).value().toBool());
            return DeviceManager::DeviceErrorNoError;
        }
        if (action.actionTypeId() == boblightColorActionTypeId) {
            bobClient->setColor(device->paramValue(boblightChannelParamTypeId).toInt(), action.param(boblightColorActionParamTypeId).value().value<QColor>());
            return DeviceManager::DeviceErrorNoError;
        }
        if (action.actionTypeId() == boblightBrightnessActionTypeId) {
            bobClient->setBrightness(device->paramValue(boblightChannelParamTypeId).toInt(), action.param(boblightBrightnessActionParamTypeId).value().toInt());
            return DeviceManager::DeviceErrorNoError;
        }
        if (action.actionTypeId() == boblightColorTemperatureActionTypeId) {
            bobClient->setColor(device->paramValue(boblightChannelParamTypeId).toInt(), tempToRgb(action.param(boblightColorTemperatureActionParamTypeId).value().toInt()));
            return DeviceManager::DeviceErrorNoError;
        }
        return DeviceManager::DeviceErrorActionTypeNotFound;
    }
    return DeviceManager::DeviceErrorDeviceClassNotFound;
}

void DevicePluginBoblight::onConnectionChanged()
{
    BobClient *bobClient = static_cast<BobClient *>(sender());
    foreach (const DeviceId &deviceId, m_bobClients.keys(bobClient)) {
        foreach (Device *device, myDevices()) {
            if (device->id() == deviceId || device->parentId() == deviceId) {
                device->setStateValue(boblightConnectedStateTypeId, bobClient->connected());
                break;
            }
        }
    }
}

