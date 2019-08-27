/***************************************************************************
 *   HDLGIInstance.cpp  --  This file is part of hdlgi-cli.                *
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

#include "HDLGIInstance.h"

static bool abortRequested = false;
void intHandler(int signal) {abortRequested = true;}


HDLGIInstance::HDLGIInstance(std::string ip) : conn(ip)
{
    int result;

    if((result = conn.connectToInstance()) != SUCCESS)
    {
        if(result == INVALID_VERSION) {throw std::runtime_error("Error connecting to HDLGI. Server returned invalid version");}
        else if(result == EEXTCONNLOST) {throw std::runtime_error("Error connecting to HDLGI. Connection was lost");}
        throw std::runtime_error("Error connecting to HDLGI. Connect() failed " + std::to_string(result));
    }

    if((result = conn.prepareDataConnection(&(this->serverSocket))) != SUCCESS)
    {
        if(result == INVALID_SOCKET) {throw std::runtime_error("Error connecting to HDLGI. bind() failed");}
        throw std::runtime_error("Error preparing data connection. listen() returned: " + std::to_string(result));
    }
}

int HDLGIInstance::initGameRead(const char *partition, const uint32_t sectors, const uint32_t offset)
{
    int result;
    IOInitReq req;

    strcpy(req.partition, partition);
    req.sectors = sectors;
    req.offset = offset;
    if((result = conn.sendCmd((uint8_t*)&req, HDLGMAN_SERVER_INIT_GAME_READ, sizeof(req))) == SUCCESS)
    {
        result = conn.getResponse(nullptr, 0);
    }
    if(result >= 0) {result = conn.acceptDataConnection(serverSocket);}
    else {conn.closePendingDataConnection(serverSocket);}


    return result;
}

int HDLGIInstance::initGameWrite(const char *partition, const uint32_t sectors, const uint32_t offset)
{
    int result;
    IOInitReq req;

    strcpy(req.partition, partition);
    req.sectors = sectors;
    req.offset = offset;
    if((result = conn.sendCmd(&req, HDLGMAN_SERVER_INIT_GAME_WRITE, sizeof(req))) == SUCCESS)
    {
        result = conn.getResponse(nullptr, 0);
    }

    if(result >= 0) {result = conn.acceptDataConnection(serverSocket);}
    else {conn.closePendingDataConnection(serverSocket);}

    return result;
}

int HDLGIInstance::initOSDResources(const char *partition, const char *discID, const wchar_t *osd1, const wchar_t *osd2, const int useSaveData)
{
    OSDResourceInitReq initReq;
    int result;

    strcpy(initReq.partition, partition);
    strcpy(initReq.discID, discID);
    wcstombs(initReq.osd1, osd1, sizeof(initReq.osd1));
    initReq.osd1[OSD_TITLE_MAX_LEN_BYTES] = '\0';
    wcstombs(initReq.osd2, osd2, sizeof(initReq.osd2));
    initReq.osd2[OSD_TITLE_MAX_LEN_BYTES] = '\0';
    initReq.useSaveData = useSaveData;
    if((result = conn.sendCmd((uint8_t*)&initReq, HDLGMAN_SERVER_INIT_OSD_RESOURCES, sizeof(OSDResourceInitReq))) == SUCCESS)
    {
        result = conn.getResponse(nullptr, 0);
    }
    return result;
}

int HDLGIInstance::OSDResourceLoad(const int index, const void *buffer, const uint32_t length)
{
    int result;
    OSDResourceWriteReq writeReq;

    writeReq.index = index;
    writeReq.length = length;
    if((result = conn.sendCmd(&writeReq, HDLGMAN_SERVER_OSD_RES_LOAD_INIT, sizeof(OSDResourceWriteReq))) == SUCCESS)
    {
        if((result = conn.getResponse(nullptr, 0)) == SUCCESS)
        {
            if((result = conn.sendCmd(buffer, HDLGMAN_SERVER_OSD_RES_LOAD, length)) == SUCCESS)
            {
                result = conn.getResponse(nullptr, 0);
            }
        }
    }

    return result;
}

int HDLGIInstance::OSDResourceWrite()
{
    int result;

    if((result = conn.sendCmd(nullptr, HDLGMAN_SERVER_WRITE_OSD_RESOURCES, 0)) == SUCCESS)
    {
        result = conn.getResponse(nullptr, 0);
    }

    return result;
}

int HDLGIInstance::installGameIcon(const char* partition, const char* discID, GameParameters& params, ConvertedMcIcon& icon)
{
    int result;
    if(params.iconSrc == 2)
    {
        if(verifyMcSave(params.iconPath) != SUCCESS || convertMcSave(params.iconPath, &icon, params.osd1.c_str(), params.osd2.c_str()) != SUCCESS)
        {
            std::cerr << "Failed to load icon from savedata. The default icon will be used" << std::endl;
            params.iconSrc = 0;
            memset(&icon, 0, sizeof(icon));
        }
    }
    else {memset(&icon, 0, sizeof(icon));}
    RetryIconInstall:

    result = installGameInstallationOSDResources(partition, discID, &params, &icon);
    if(result != INSTALL_SUCCESS && (params.iconSrc == 1 || params.iconSrc == 2))
    {
        std::cerr << "Failed to load icon from savedata. The default icon will be used" << std::endl;
        params.iconSrc = 0;
        goto RetryIconInstall;
    }
    if(result == INSTALL_SUCCESS) {result = SUCCESS;}
    if(icon.HDDIconSys) {free(icon.HDDIconSys);}
    if(icon.listViewIcon) {free(icon.listViewIcon);}
    if(icon.deleteIcon) {free(icon.deleteIcon);}

    return result;
}

int HDLGIInstance::installGameInstallationOSDResources(const char *partition, const char *discID, const GameParameters* params, const ConvertedMcIcon *icon)
{
    int result;

    if(params->iconSrc != ICON_SRC_GAMESAVE)
    {
        if((result = initOSDResources(partition, discID, params->osd1.c_str(), params->osd2.c_str(), 0)) >= 0)
        {
            if(params->iconSrc == ICON_SRC_EXTERNAL)
            {
                if((result = OSDResourceLoad(OSD_ICON_SYS_INDEX, icon->HDDIconSys, icon->HDDIconSysSize)) == SUCCESS)
                {
                    if((result = OSDResourceLoad(OSD_VIEW_ICON_INDEX, icon->listViewIcon, icon->listViewIconSize)) == SUCCESS)
                    {
                        if(icon->deleteIconSize > 0){
                            result = OSDResourceLoad(OSD_VIEW_ICON_INDEX, icon->deleteIcon, icon->deleteIconSize);
                        }
                    }
                }
                if(result == SUCCESS)
                {
                    if(OSDResourceWrite() == SUCCESS) {result = INSTALL_SUCCESS;}
                }
            }
            else {result = OSDResourceWrite();}
        }
    }
    else
    {
        if(initOSDResources(partition, discID, params->osd1.c_str(), params->osd2.c_str(), 1) == INSTALL_SUCCESS)
        {
            if((result = OSDResourceWrite()) == SUCCESS) {result = INSTALL_SUCCESS;}
        }
        else
        {
            if(conn.sendCmd(nullptr, HDLGMAN_SERVER_OSD_RES_WRITE_CANCEL, 0) == SUCCESS)
            {
                conn.getResponse(nullptr, 0);
            }
            result = LOAD_ICON_FAIL;
        }
    }

    return result;
}

int HDLGIInstance::updateGameInstallationOSDResources(const char* partition, const wchar_t* osd1, const wchar_t* osd2)
{
    void *fileBuffers[NUM_OSD_FILES_ENTS];
    OSDResourceStat resourceStats;
    int result;
    IconSysData iconSys;
    unsigned char *HDDIconSys;

    OSDResourceStatReq statReq;

    strcpy(statReq.partition, partition);
    if((result = conn.sendCmd(&statReq, HDLGMAN_SERVER_GET_OSD_RES_STAT, sizeof(OSDResourceStatReq))) == SUCCESS)
    {
        result = conn.getResponse(&resourceStats, sizeof(struct OSDResourceStat));
    }
    if(result == SUCCESS)
    {
        memset(fileBuffers, 0, sizeof(fileBuffers));

        for(int i = 0; i < NUM_OSD_FILES_ENTS; ++i)
        {
            if(resourceStats.lengths[i])
            {
                if((fileBuffers[i] = malloc(resourceStats.lengths[i])) != nullptr)
                {
                    OSDResourceReadReq readReq;
                    strcpy(readReq.partition, partition);
                    readReq.index = i;
                    if((result = conn.sendCmd(&readReq, HDLGMAN_SERVER_OSD_RES_READ, sizeof(OSDResourceReadReq))) == SUCCESS)
                    {
                        result = conn.getResponse(fileBuffers[i], resourceStats.lengths[i]);
                    }
                    if(result != SUCCESS) {break;}
                }
                else
                {
                    result = ENOMEM;
                    break;
                }
            }
        }

        if(result == SUCCESS && fileBuffers[OSD_ICON_SYS_INDEX] != nullptr)
        {
            if((result = parseIconSysFile(convUTF8FromBytes((const char*)(uint8_t*)fileBuffers[OSD_ICON_SYS_INDEX]).c_str(), (unsigned int)resourceStats.lengths[OSD_ICON_SYS_INDEX], &iconSys))==0){
                //Update icon sys file.
                wcscpy(iconSys.title0, osd1);
                wcscpy(iconSys.title1, osd2);

                free(fileBuffers[OSD_ICON_SYS_INDEX]);

                // Convert the icon file back into the HDD OSD format.
                HDDIconSys = (unsigned char*)malloc(640);	// Allocate sufficient memory to accommodate the longest possible title and the standard icon.sys file fields as in the template.
                resourceStats.lengths[OSD_ICON_SYS_INDEX] = (uint32_t)generateHDDIconSysFile(&iconSys, (char*)HDDIconSys, 640)-1;
                fileBuffers[OSD_ICON_SYS_INDEX] = malloc(resourceStats.lengths[OSD_ICON_SYS_INDEX]);
                memcpy(fileBuffers[OSD_ICON_SYS_INDEX], HDDIconSys, resourceStats.lengths[OSD_ICON_SYS_INDEX]);
                free(HDDIconSys);

                if((result = conn.sendCmd(partition, HDLGMAN_SERVER_INIT_DEFAULT_OSD_RESOURCES, (const uint32_t)strlen(partition)+1)) == SUCCESS)
                {
                    result = conn.getResponse(nullptr, 0);
                }
                if(result == SUCCESS)
                {
                    for(int i = 0; i < NUM_OSD_FILES_ENTS; ++i)
                    {
                        if(fileBuffers[i] != nullptr)
                        {
                            OSDResourceLoad(i, fileBuffers[i], resourceStats.lengths[i]);
                        }
                    }

                    //Write updated file back (Rebuild the partition attribute area).
                    result = OSDResourceWrite();
                }
            }
            else
            {
                //Can't parse the icon.sys file.
                for(int i = 0; i < NUM_OSD_FILES_ENTS; ++i)
                {
                    if(fileBuffers[i] != nullptr) {free(fileBuffers[i]);}
                }
            }
        }
        else
        {
            result = PART_ATTR_AREA_CORRUPTED;
            for(int i = 0; i < NUM_OSD_FILES_ENTS; ++i)
            {
                if(fileBuffers[i]) {free(fileBuffers[i]);}
            }
        }
    }

    return result;
}

int HDLGIInstance::reconnect()
{
    int result = FAILURE;

    for (int reconnects = 0; reconnects < RECONNECT_COUNT; ++reconnects)
    {
        if((result = conn.connectToInstance()) == SUCCESS) {break;}
    }
    if(result != SUCCESS) {result = EEXTCONNLOST;}

    return result;
}

uint32_t HDLGIInstance::getFreeSpace()
{
    int result;
    uint32_t space = 0;

    if(conn.sendCmd(nullptr, HDLGMAN_SERVER_GET_FREE_SPACE, 0) == SUCCESS)
    {
        result = conn.getResponse(&space, sizeof(uint32_t));
        if(result != SUCCESS)
        {
            return 0;
        }
    }

    return space/2048; //MiB
}

std::vector<HDLGameEntry> HDLGIInstance::getGamesInstalled()
{
    int numGamesInList = 0;
    int i;
    HDLGameEntry GameEntry;
    std::vector<HDLGameEntry> gameList;

    if(conn.sendCmd(nullptr, HDLGMAN_SERVER_LOAD_GAME_LIST, 0) == SUCCESS)
    {
        if((numGamesInList = conn.getResponse(nullptr, 0)) > 0)
        {
            if(conn.sendCmd(nullptr, HDLGMAN_SERVER_READ_GAME_LIST, 0) == SUCCESS)
            {
                if(conn.getResponse(nullptr, 0) >= 0)
                {
                    for (i = 0; i < numGamesInList; i++)
                    {
                        if (conn.getPayload(&GameEntry, sizeof(struct HDLGameEntry)) != SUCCESS) { break; }
                        gameList.push_back(GameEntry);
                    }
                }
            }
        }
    }
    return gameList;
}

GameParameters HDLGIInstance::readGameParams(const HDLGameEntry& game)
{
    GameParameters params;
    params.title = convUTF8FromBytes(game.title);
    params.disc_type = game.discType;
    params.compatFlags = game.compatFlags;
    params.useMDMA0 = false;
    params.iconSrc = 0;
    if(game.TRType == ATA_XFER_MODE_MDMA && game.TRMode == 0) {params.useMDMA0 = true;}

    OSDTitles OSDTitles;

    if(conn.sendCmd(game.partition, HDLGMAN_SERVER_OSD_RES_READ_TITLES, (const uint32_t)strlen(game.partition)+1) == SUCCESS)
    {
        if(conn.getResponse(&OSDTitles, sizeof(OSDTitles)) == SUCCESS)
        {
            params.osd1 = convUTF8FromBytes(OSDTitles.title1);
            params.osd2 = convUTF8FromBytes(OSDTitles.title2);
        }
    }
    return params;
}

int HDLGIInstance::editGame(HDLGameEntry& game, GameParameters& newParam, const bool updateIcon)
{
    int result;
    ConvertedMcIcon HDDIcon;

    std::wcstombs(game.title, newParam.title.c_str(), GAME_TITLE_MAX_LEN_BYTES);
    game.compatFlags = newParam.compatFlags;
    if(newParam.useMDMA0)
    {
        game.TRType=ATA_XFER_MODE_MDMA;
        game.TRMode=0;
    }
    else
    {
        game.TRType=ATA_XFER_MODE_UDMA;
        game.TRMode=4;
    }
    game.discType = newParam.disc_type;

    if((result = conn.sendCmd(&game, HDLGMAN_SERVER_UPD_GAME_ENTRY, sizeof(struct HDLGameEntry))) == SUCCESS)
    {
        result = conn.getResponse(nullptr,0);
    }
    if(result < 0)
    {
        return result;
    }

    if(!updateIcon)
    {
        result = updateGameInstallationOSDResources(game.partition, newParam.osd1.c_str(), newParam.osd2.c_str());
    }
    else
    {
        result = installGameIcon(game.partition, game.discID, newParam, HDDIcon);
    }
    return result;
}

int HDLGIInstance::downloadGame(const std::string path, const HDLGameEntry& game)
{
    int result;
    FILE* discImg;
    uint32_t lsn;
    uint32_t sectorsToRead;
    int installResult;
    int retries;
    int reconnects;
    uint8_t wrBuffer[IO_BUFFER_SIZE * IO_BLOCK_SIZE];

    std::chrono::steady_clock::time_point downloadStart = std::chrono::steady_clock::now();
    std::chrono::steady_clock::time_point downloadEnd;

    std::wstring gameName = convUTF8FromBytes(game.title);
    std::chrono::steady_clock::time_point timeStart;
    std::chrono::steady_clock::time_point timeEnd;
    ProgressBar bar(gameName, game.sectors * IO_BLOCK_SIZE);
    bar.start();

    signal(SIGINT, intHandler);

    if((result = initGameRead(game.partition, game.sectors, 0)) == SUCCESS)
    {
        //Begin game install process.
        discImg = fopen(path.c_str(), "wb");

        if(discImg)
        {
            for(lsn = 0; lsn < game.sectors; lsn += sectorsToRead)
            {
                if(abortRequested)
                {
                    abortRequested = false;
                    std::cerr << "Game copying aborted" << std::endl;
                    bar.finish();
                    result = EEXTABORT;
                    break;
                }

                sectorsToRead = IO_BUFFER_SIZE;
                if(game.sectors - lsn < IO_BUFFER_SIZE) {sectorsToRead = game.sectors - lsn;}

                for(retries = 0; retries < RETRY_COUNT; retries++)
                {
                    timeStart = std::chrono::steady_clock::now();
                    result = conn.recvData(wrBuffer, IO_BLOCK_SIZE * sectorsToRead);
                    if(result == SUCCESS) {break;}
                    else
                    {
                        std::cerr << "recvData failed with code: " << result << " in position lsn: " << lsn << std::endl;
                        //Terminate and reconnect (data connection was either lost or closed by remote peer).
                        conn.closeDataConnection();

                        if((result = conn.sendCmd(nullptr, HDLGMAN_SERVER_IO_STATUS, 0)) == SUCCESS)
                        {
                            result = conn.getResponse(nullptr, 0);
                        }

                        if(result != SUCCESS)
                        {
                            if (result == EEXTCONNLOST)
                            {
                                std::cerr << "Command socket disconnected. Attempt to reconnect." << std::endl;
                                if((result = reconnect()) != SUCCESS) {break;}
                            }
                            else
                            {
                                result = EIO;
                                break;
                            }
                        }

                        if(result == SUCCESS)
                        {
                            std::cerr << "Command connection is okay. Attempt to reconnect for data connection." << std::endl;
                            for (reconnects = 0; reconnects < RECONNECT_COUNT; reconnects++)
                            {
                                if ((result = initGameRead(game.partition, game.sectors - lsn, lsn)) == SUCCESS) {break;}
                            }

                            if(result != SUCCESS)
                            {
                                result = EEXTCONNLOST;
                                break;
                            }
                            else {std::cerr << "Connection reestablished. Resuming download." << std::endl;}
                        }
                    }
                }

                if(result != SUCCESS) {break;}

                fwrite(wrBuffer,IO_BLOCK_SIZE,sectorsToRead,discImg);
                timeEnd = std::chrono::steady_clock::now();
                bar.update(IO_BLOCK_SIZE * sectorsToRead, std::chrono::duration_cast<std::chrono::milliseconds>(timeEnd - timeStart).count());
                if((result = ferror(discImg))) {break;}

            } /* End of main loop */

            installResult = result;
            fclose(discImg);
            conn.closeDataConnection();	//Regardless of how it went, close the data connection.

            //Do not attempt to close with a closed socket.
            if(result != EEXTCONNLOST)
            {
                for(retries = 0; retries < RETRY_COUNT; retries++)
                {
                    if((result = conn.sendCmd(nullptr, HDLGMAN_SERVER_CLOSE_GAME, 0)) == SUCCESS)
                    {
                        result = conn.getResponse(nullptr, 0);
                    }

                    if(result == SUCCESS) {break;}
                    else if (result == EEXTCONNLOST)
                    {	//Command socket disconnected. Attempt to reconnect.
                        if(reconnect() != SUCCESS) {break;}
                    }
                    else
                    {
                        std::cerr << "Error closing game: " << result << std::endl;
                        break;
                    }
                }

                //Restore the copying result (the reason for failure).
                if(result < 0) {installResult = result;}
            }

            //If copying did not complete successfully, delete the partially-copied image file.
            if(installResult != SUCCESS) {unlink(path.c_str());}
        }
        else {result = EIO;}
    }
    signal(SIGINT,SIG_DFL);
    downloadEnd = std::chrono::steady_clock::now();
    std::cout << std::endl << "Download took " << std::chrono::duration_cast<std::chrono::seconds>(downloadEnd - downloadStart).count() << " seconds" << std::endl;
    return result;
}

int HDLGIInstance::installGame(const std::string sourcePath, GameParameters& param, const bool overwrite)
{
    char discID[11];
    char startupFilename[32];
    char partition[33];
    void* buffer;
    size_t DLDVDSectors;
    size_t sectorCount;
    uint8_t sectorType;
    int result;
    int installResult;
    int retries;
    int reconnects;
    uint8_t XFerMode;
    uint8_t XFerType;
    ConvertedMcIcon convertedIcon;

    std::chrono::steady_clock::time_point installStart = std::chrono::steady_clock::now();
    std::chrono::steady_clock::time_point installEnd;

    std::chrono::steady_clock::time_point timeStart;
    std::chrono::steady_clock::time_point timeEnd;

    signal(SIGINT, intHandler);

    try
    {
        //Begin game install process.
        ISO9660Image discImg = ISO9660Image(sourcePath);
        discImg.getDiscInfo(&sectorCount, &DLDVDSectors, &sectorType);
        ProgressBar bar(param.title, (sectorCount + DLDVDSectors) * IO_BLOCK_SIZE);
        if(discImg.parsePS2CNF(discID, startupFilename, sectorType) == FAILURE)
        {
            discImg.close();
            signal(SIGINT,SIG_DFL);
            return EINVAL;
        }
        if((result = conn.sendCmd(discID, HDLGMAN_SERVER_GET_GAME_PART_NAME, (const uint32_t)strlen(discID)+1)) == SUCCESS)
        {
            result = conn.getResponse(partition, 33);
        }

        if(result == SUCCESS)
        {
            if(!overwrite)
            {
                signal(SIGINT,SIG_DFL);
                return GAME_EXISTS;
            }
            if(conn.sendCmd(partition, HDLGMAN_SERVER_DEL_GAME_ENTRY, (const uint32_t)strlen(partition) + 1) == SUCCESS)
            {
                conn.getResponse(nullptr, 0);
            }
        }

        XFerType = ATA_XFER_MODE_UDMA;
        XFerMode = 4;

        if(param.useMDMA0)
        {
            XFerType=ATA_XFER_MODE_MDMA;
            XFerMode=0;
        }

        HDLGameInfo info;
        memset(info.discID, 0, sizeof(info.discID));
        memset(info.title, 0, sizeof(info.title));
        memset(info.startupFilename, 0, sizeof(info.startupFilename));
        wcstombs(info.title,param.title.c_str(),param.title.size());
        info.title[sizeof(info.title)-1]='\0';
        strncpy(info.discID, discID, sizeof(info.discID)-1);
        info.discID[sizeof(info.discID)-1]='\0';
        strncpy(info.startupFilename, startupFilename, sizeof(info.startupFilename)-1);
        info.startupFilename[sizeof(info.startupFilename)-1]='\0';
        info.discType = param.disc_type;
        info.sectorsInDiscLayer0 = (uint32_t)sectorCount;
        info.sectorsInDiscLayer1 = (uint32_t)DLDVDSectors;
        info.compatFlags = param.compatFlags;
        info.TRType = XFerType;
        info.TRMode = XFerMode;
        if((result = conn.sendCmd(&info, HDLGMAN_SERVER_PREP_GAME_INST, sizeof(struct HDLGameInfo))) == SUCCESS)
        {
            result = conn.getResponse(nullptr, 0);
        }
        if(result >= 0) {result = conn.acceptDataConnection(serverSocket);}
        else {conn.closePendingDataConnection(serverSocket);}

        if(result != SUCCESS)
        {
            discImg.close();
            signal(SIGINT,SIG_DFL);
            return result;
        }
        bar.start();
        buffer = calloc(IO_BUFFER_SIZE,IO_BLOCK_SIZE);
        size_t numSectors = sectorCount + DLDVDSectors;
        size_t sectorsRemaining = numSectors;
        while(sectorsRemaining)
        {
            if(abortRequested)
            {
                abortRequested = false;
                std::cerr << "Game copying aborted" << std::endl;
                bar.finish();
                result = EEXTABORT;
                break;
            }
            timeStart = std::chrono::steady_clock::now();
            int sectorsRead = discImg.readNext(buffer, IO_BUFFER_SIZE);
            sectorsRemaining -= sectorsRead;

            for(retries = 0; retries < RETRY_COUNT; ++retries)
            {
                result = conn.sendData(buffer, (const uint32_t)(IO_BLOCK_SIZE * sectorsRead));

                if(result == SUCCESS) {break;}
                else
                {
                    std::cerr << "sendData failed with code: " << result << " in position lsn: " << discImg.currentLSN << std::endl;
                    //Terminate and reconnect (data connection was either lost or closed by remote peer).
                    conn.closeDataConnection();

                    if((result = conn.sendCmd(nullptr, HDLGMAN_SERVER_IO_STATUS, 0)) == SUCCESS)
                    {
                        result = conn.getResponse(nullptr, 0);
                    }

                    if(result != SUCCESS)
                    {
                        if (result == EEXTCONNLOST)
                        {
                            std::cerr << "Command socket disconnected. Attempt to reconnect." << std::endl;
                            result = reconnect();

                            if (result != SUCCESS) {break;}
                        }
                        else
                        {
                            result = EIO;
                            break;
                        }
                    }
                    std::cerr << "Command connection is okay. Attempt to reconnect for data connection." << std::endl;
                    for (reconnects = 0; reconnects < RECONNECT_COUNT; reconnects++)
                    {
                        if((result = conn.sendCmd(discID, HDLGMAN_SERVER_GET_GAME_PART_NAME, (const uint32_t)strlen(discID)+1)) == SUCCESS)
                        {
                            result = conn.getResponse(partition, 33);
                        }

                        if(result == SUCCESS)
                        {
                            if ((result = initGameWrite(partition, (const uint32_t)sectorsRemaining, (const uint32_t)(numSectors - sectorsRemaining))) >= 0)
                            {
                                result = SUCCESS;
                                std::cerr << "Connection reestablished. Resuming download." << std::endl;
                            }
                        }
                        if (result == SUCCESS) {break;}
                    }

                    if(result != SUCCESS)
                    {
                        result = EEXTCONNLOST;
                        break;
                    }
                }
            }

            if(result != SUCCESS) {break;}
            timeEnd = std::chrono::steady_clock::now();
            bar.update((size_t)IO_BLOCK_SIZE * sectorsRead, std::chrono::duration_cast<std::chrono::milliseconds>(timeEnd - timeStart).count());
        } // End of main loop

        installResult = result;

        discImg.close();
        free(buffer);
        conn.closeDataConnection();	//Regardless of how it went, close the data connection.

        //Do not attempt to close with a closed socket.
        if(result != EEXTCONNLOST)
        {
            for(retries = 0; retries < RETRY_COUNT; retries++)
            {
                if((result = conn.sendCmd(nullptr, HDLGMAN_SERVER_CLOSE_GAME, 0)) == SUCCESS)
                {
                    result = conn.getResponse(nullptr, 0);
                }

                if(result == SUCCESS) {break;} 	//All OK.
                else if (result == EEXTCONNLOST)
                {	//Command socket disconnected. Attempt to reconnect.
                    result = reconnect();

                    if(result != 0) {break;}
                }
                else {break;} //Some other error, like disk I/O error (non-recoverable).
            }

            if(result != SUCCESS) {installResult = result;}
        }

        if(installResult == SUCCESS)
        {
            if((result = conn.sendCmd(discID, HDLGMAN_SERVER_GET_GAME_PART_NAME, (const uint32_t)strlen(discID)+1)) == SUCCESS)
            {
                result = conn.getResponse(partition, 33);
            }

            if(result != SUCCESS)
            {
                signal(SIGINT,SIG_DFL);
                return ENOENT;
            }
            result = installGameIcon(partition,discID,param,convertedIcon);

            if(result != SUCCESS)
            {
                if((result = conn.sendCmd(partition, HDLGMAN_SERVER_DEL_GAME_ENTRY, (const uint32_t)strlen(partition)+1)) == SUCCESS)
                {
                    conn.getResponse(nullptr, 0);
                }
            }
        }
        else
        {
            if((result = conn.sendCmd(discID, HDLGMAN_SERVER_GET_GAME_PART_NAME, (const uint32_t)strlen(discID)+1)) == SUCCESS)
            {
                result = conn.getResponse(partition, 33);
            }

            if(result == SUCCESS)
            {
                if(conn.sendCmd(partition, HDLGMAN_SERVER_DEL_GAME_ENTRY, (const uint32_t)strlen(partition)+1) == SUCCESS)
                {
                    conn.getResponse(nullptr, 0);
                }
            }
            result = installResult; //Restore the installation result (the reason for failure).
        }
    }
    catch(std::runtime_error& e)
    {
        result = ENOENT;
    }
    signal(SIGINT,SIG_DFL);
    installEnd = std::chrono::steady_clock::now();
    std::cout << std::endl << "Installation took " << std::chrono::duration_cast<std::chrono::seconds>(installEnd - installStart).count() << " seconds" << std::endl;
    return result;
}

int HDLGIInstance::deleteGame(const HDLGameEntry& game)
{
    int result;

    if((result = conn.sendCmd(game.partition, HDLGMAN_SERVER_DEL_GAME_ENTRY, (const uint32_t)strlen(game.partition)+1)) == SUCCESS)
    {
        result = conn.getResponse(nullptr, 0);
    }
    return result;
}



int HDLGIInstance::poweroff()
{
    return conn.sendCmd(nullptr, HDLGMAN_SERVER_SHUTDOWN, 0);
}

HDLGIInstance::~HDLGIInstance()
{
    conn.closeConnection();
    shutdown(serverSocket, SHUT_RDWR);
    close(serverSocket);
}