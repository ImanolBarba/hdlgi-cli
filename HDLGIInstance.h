/***************************************************************************
 *   HDLGIInstance.h  --  This file is part of hdlgi-cli.                  *
 *                                                                         *
 *   Copyright (C) 2019 Imanol-Mikel Barba Sabariego                       *
 *                                                                         *
 *   hdlgi-cli is free software: you can redistribute it and/or modify     *
 *   it under the terms of the GNU General Public License as published     *
 *   by the Free Software Foundation, either version 3 of the License,     *
 *   or (at your option) any later version.                                *
 *                                                                         *
 *   hdlgi-cli is distributed in the hope that it will be useful,          *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty           *
 *   of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.               *
 *   See the GNU General Public License for more details.                  *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program.  If not, see http://www.gnu.org/licenses/.   *
 *                                                                         *
 *   This project uses source code from HDLGameInstaller from Woon Yung    *
 *   (https://sites.google.com/view/ysai187/home/projects/hdlgameinstaller *
 *                                                                         *
 ***************************************************************************/

#ifndef HDLGI_CLI_HDLGIINSTANCE_H
#define HDLGI_CLI_HDLGIINSTANCE_H

#include "aux_functions.h"
#include "errcodes.h"
#include "HDLGI_commands.h"
#include "HDLGIConnection.h"
#include "ISO9660Image.h"
#include "OSD.h"
#include "ProgressBar.h"

#include <chrono>
#include <iostream>
#include <string>
#include <vector>

#include <csignal>
#include <cstring>

class HDLGIInstance
{
private:
    HDLGIConnection conn;
    int serverSocket;
    int initGameRead(const char *partition, const uint32_t sectors, const uint32_t offset);
    int initGameWrite(const char *partition, const uint32_t sectors, const uint32_t offset);
    int initOSDResources(const char *partition, const char *discID, const wchar_t *osd1, const wchar_t *osd2, const int useSaveData);
    int OSDResourceLoad(const int index, const void *buffer, const uint32_t length);
    int OSDResourceWrite();
    int installGameIcon(const char* partition, const char* discID, GameParameters& params, ConvertedMcIcon& icon);
    int installGameInstallationOSDResources(const char *partition, const char *discID, const GameParameters* params, const ConvertedMcIcon *icon);
    int updateGameInstallationOSDResources(const char* partition, const wchar_t* osd1, const wchar_t* osd2);

public:
    HDLGIInstance(std::string ip);
    int reconnect();
    uint32_t getFreeSpace();
    std::vector<HDLGameEntry> getGamesInstalled();
    GameParameters readGameParams(const HDLGameEntry& game);
    int editGame(HDLGameEntry& game, GameParameters& newParam, const bool updateIcon);
    int installGame(const std::string sourcePath, GameParameters& param, const bool overwrite);
    int downloadGame(const std::string path, const HDLGameEntry& game);
    int deleteGame(const HDLGameEntry& game);

    int poweroff();
    ~HDLGIInstance();
};


#endif //HDLGI_CLI_HDLGIINSTANCE_H
