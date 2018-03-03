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

#include <QDebug>
#include <QStringList>

DevicePluginBoblight::DevicePluginBoblight()
{
}

void DevicePluginBoblight::deviceRemoved(Device *device)
{
    BobClient *client = m_bobClients.key(device);
    m_bobClients.remove(client);
    client->deleteLater();
}

//void DevicePluginBoblight::guhTimer()
//{
//    foreach (BobClient *client, m_bobClients.keys()) {
//        if (!client->connected()) {
//            client->connectToBoblight();
//        }
//    }
//}

DeviceManager::DeviceSetupStatus DevicePluginBoblight::setupDevice(Device *device)
{
    BobClient *bobClient = new BobClient(device->paramValue(boblightHostAddressParamTypeId).toString(), device->paramValue(boblightPortParamTypeId).toInt(), this);
    //bobClient->setDefaultColor(configValue("default color").value<QColor>());
    bobClient->setDefaultColor(QColor("#ffed2b"));

    if (!bobClient->connectToBoblight()) {
        bobClient->deleteLater();
        return DeviceManager::DeviceSetupStatusFailure;
    }

    device->setStateValue(boblightConnectedStateTypeId, true);
    m_bobClients.insert(bobClient, device);
    connect(bobClient, SIGNAL(connectionChanged()), this, SLOT(onConnectionChanged()));

    return DeviceManager::DeviceSetupStatusSuccess;
}

DeviceManager::DeviceError DevicePluginBoblight::executeAction(Device *device, const Action &action)
{
    BobClient *bobClient = m_bobClients.key(device);
    if (!bobClient)
        return DeviceManager::DeviceErrorHardwareNotAvailable;

    if (!bobClient->connected())
        return DeviceManager::DeviceErrorHardwareNotAvailable;

    if (device->deviceClassId() == boblightDeviceClassId) {
        if (action.actionTypeId() == boblightPowerActionTypeId) {
            bobClient->setAllPower(action.param(boblightPowerParamTypeId).value().toBool());
            return DeviceManager::DeviceErrorNoError;
        } else if (action.actionTypeId() == boblightPriorityActionTypeId) {
            bobClient->setPriority(action.param(boblightPriorityParamTypeId).value().toInt());
            return DeviceManager::DeviceErrorNoError;
        } else if (action.actionTypeId() == boblightColorActionTypeId) {
            bobClient->setColor(-1, action.param(boblightColorParamTypeId).value().value<QColor>());
            return DeviceManager::DeviceErrorNoError;
        }  else if (action.actionTypeId() == boblightBrightnessActionTypeId) {
            bobClient->setBrightness(action.param(boblightBrightnessParamTypeId).value().toInt());
            return DeviceManager::DeviceErrorNoError;
        }
    }

    return DeviceManager::DeviceErrorDeviceClassNotFound;
}

void DevicePluginBoblight::onConnectionChanged()
{
    BobClient *bobClient = static_cast<BobClient *>(sender());
    Device *device = m_bobClients.value(bobClient);
    device->setStateValue(boblightConnectedStateTypeId, bobClient->connected());
}

