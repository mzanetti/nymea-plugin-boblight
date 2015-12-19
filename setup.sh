#!/bin/bash

# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
#                                                                         #
#  Copyright (C) 2015 Simon Stuerz <simon.stuerz@guh.guru>                #
#                                                                         #
#  This file is part of guh.                                              #
#                                                                         #
#  guh is free software: you can redistribute it and/or modify            #
#  it under the terms of the GNU General Public License as published by   #
#  the Free Software Foundation, version 2 of the License.                #
#                                                                         #
#  guh is distributed in the hope that it will be useful,                 #
#  but WITHOUT ANY WARRANTY; without even the implied warranty of         #
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the           #
#  GNU General Public License for more details.                           #
#                                                                         #
#  You should have received a copy of the GNU General Public License      #
#  along with guh. If not, see <http://www.gnu.org/licenses/>.            #
#                                                                         #
# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #

set -e
#set -x

BUILD_DIR=$(pwd)/build
CPUS=$(nproc)

# get start time for process duration
startTime=$(date +%s)

########################################################
# bash colors
BASH_GREEN="\e[1;32m"
BASH_RED="\e[1;31m"
BASH_NORMAL="\e[0m"

printGreen() {
    echo -e "${BASH_GREEN}$1${BASH_NORMAL}"
}

printRed() {
    echo -e "${BASH_RED}$1${BASH_NORMAL}"
}

########################################################
# check build dir
if [ ! -d ${BUILD_DIR} ]; then
    mkdir -p ${BUILD_DIR}
fi

########################################################
# update and install build dependencies
printGreen 'Get updates...'
sudo apt-get update

printGreen 'Get upgrades...'
sudo apt-get -y upgrade

printGreen 'Install build tools...'
sudo apt-get -y install git subversion build-essential

printGreen 'Update guh-boblight repository...'
git pull origin master

########################################################
cd ${BUILD_DIR}

if [ ! -d ${BUILD_DIR}/boblight ]; then
    printGreen 'Download boblight source code...'
    svn checkout http://boblight.googlecode.com/svn/trunk/ boblight
else
    printGreen 'Update boblight source code...'
    svn update
fi

printGreen 'Install boblight dependencies...'
sudo apt-get -y install libusb-1.0-0-dev portaudio19-dev

cd boblight

printGreen 'Configure boblight...'
./configure --without-x11

printGreen 'Clean boblight...'
make clean

printGreen 'Build boblight...'
make -j${CPUS}

printGreen 'Install boblight...'
sudo make install

########################################################
printGreen 'Install guh-boblight dependencies...'
sudo apt-get -y install python qt5-default qtbase5-dev libguh1-dev

cd ${BUILD_DIR}
mkdir -p ${BUILD_DIR}/guh-boblight
cd ${BUILD_DIR}/guh-boblight

printGreen 'Configure guh-boblightplugin...'
qmake ../../guh-boblight/

printGreen 'Build guh-boblight-plugin...'
make -j${CPUS}

printGreen 'Install guh-boblight-plugin...'
sudo make install

########################################################
printGreen 'Clean up...'
sudo apt-get -y autoremove

printGreen 'Restart guhd...'
sudo systemctl restart guhd

########################################################
# calculate process time
endTime=$(date +%s)
dt=$((endTime - startTime))
ds=$((dt % 60))
dm=$(((dt / 60) % 60))
dh=$((dt / 3600))

echo -e "${BASH_GREEN}"
echo -e "-------------------------------------------------------"
printf '\tTotal time: %02d:%02d:%02d\n' ${dh} ${dm} ${ds}
echo -e "-------------------------------------------------------"
echo -e "Done! You should now have the boblight plugin available in your guhd environment."
echo -e "${BASH_NORMAL}"

