/***************************************************************************
 *   HDLGI_commands.cpp  --  This file is part of hdlgi-cli.               *
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

#ifndef HDLGI_HDLGI_COMMANDS_H
#define HDLGI_HDLGI_COMMANDS_H

#include "OSD.h"

enum HDLGMAN_ClientServerCommands{
    //Server commands
    HDLGMAN_SERVER_RESPONSE	= 0,
    HDLGMAN_SERVER_GET_VERSION,

    //Client commands
    HDLGMAN_CLIENT_SEND_VERSION,
    HDLGMAN_CLIENT_VERSION_ERR,             //Server rejects connection to client (version mismatch).

    //Game installation commands
    HDLGMAN_SERVER_PREP_GAME_INST = 0x10,   //Creates a new partition and will also automatically invoke HDLGMAN_SERVER_INIT_GAME_WRITE.
    HDLGMAN_SERVER_INIT_GAME_WRITE,         //Opens an existing partition for writing.
    HDLGMAN_SERVER_INIT_GAME_READ,          //Opens an existing partition for reading.
    HDLGMAN_SERVER_IO_STATUS,
    HDLGMAN_SERVER_CLOSE_GAME,
    HDLGMAN_SERVER_LOAD_GAME_LIST,
    HDLGMAN_SERVER_READ_GAME_LIST,
    HDLGMAN_SERVER_READ_GAME_LIST_ENTRY,
    HDLGMAN_SERVER_READ_GAME_ENTRY,
    HDLGMAN_SERVER_UPD_GAME_ENTRY,
    HDLGMAN_SERVER_DEL_GAME_ENTRY,
    HDLGMAN_SERVER_GET_GAME_PART_NAME,
    HDLGMAN_SERVER_GET_FREE_SPACE,

    //OSD-resource management
    HDLGMAN_SERVER_INIT_OSD_RESOURCES = 0x20,
    HDLGMAN_SERVER_OSD_RES_LOAD_INIT,
    HDLGMAN_SERVER_OSD_RES_LOAD,
    HDLGMAN_SERVER_WRITE_OSD_RESOURCES,
    HDLGMAN_SERVER_OSD_RES_WRITE_CANCEL,
    HDLGMAN_SERVER_GET_OSD_RES_STAT,
    HDLGMAN_SERVER_OSD_RES_READ,
    HDLGMAN_SERVER_OSD_RES_READ_TITLES,
    HDLGMAN_SERVER_INIT_DEFAULT_OSD_RESOURCES,	//Same as HDLGMAN_SERVER_INIT_OSD_RESOURCES, but doesn't require the OSD titles and is designed for subsequent pre-built OSD resource file uploading (with HDLGMAN_SERVER_OSD_RES_LOAD) from the client.
    HDLGMAN_SERVER_OSD_MC_SAVE_CHECK = 0x40,
    HDLGMAN_SERVER_OSD_MC_GET_RES_STAT,
    HDLGMAN_SERVER_OSD_MC_RES_READ,

    HDLGMAN_SERVER_SHUTDOWN = 0xFF,
};

#define ALTERNATE_EE_CORE           0x01   //COMPAT_MODE_1
#define ALTERNATE_READING_METHOD    0x02   //COMPAT_MODE_2
#define UNHOOK_SYSCALLS             0x04   //COMPAT_MODE_3
#define DISABLE_PSS_VIDEOS          0x08   //COMPAT_MODE_4
#define DISABLE_DVD9_SUPPORT        0x10   //COMPAT_MODE_5
#define DISABLE_IGR                 0x20   //COMPAT_MODE_6
#define UNUSED_COMPAT               0x40   //COMPAT_MODE_7
#define HIDE_DEV9_MODULE            0x80   //COMPAT_MODE_8

#define ICON_SRC_DEFAULT    0
#define ICON_SRC_GAMESAVE   1
#define ICON_SRC_EXTERNAL   2

#define ATA_XFER_MODE_PIO	0x08
#define ATA_XFER_MODE_MDMA	0x20
#define ATA_XFER_MODE_UDMA	0x40

#define SCECdPSCD	0x10
#define SCECdPSCDDA	0x11
#define SCECdPS2CD	0x12
#define SCECdPS2CDDA	0x13
#define SCECdPS2DVD	0x14

#define GAME_TITLE_MAX_LEN_BYTES	160								//In bytes, when in UTF-8 characters.
#define GAME_TITLE_MAX_LEN			(GAME_TITLE_MAX_LEN_BYTES/2)	/*
                                                                     * In characters. Note: the original format for HDLoader just has a 160-character space.
                                                                     * But UTF-8 characters may have 1 or more bytes each. Hence this is an approximation.
                                                                     */

#define RETRY_COUNT		3	//Maximum number of attempts to make, for failures to read/write to the server.
#define RECONNECT_COUNT	5	/*	Maximum number of reconnection attempts to make, for every failed attempt to read/write.
								Note that the maximum number of connection attempts would be equal to RETRY_COUNT*RECONNECT_COUNT. */

#define IO_BUFFER_SIZE 2048

#define PARTITION_LENGTH 32
#define DISCID_LENGTH 10
#define FILENAME_LENGTH 13

typedef struct GameParameters
{
    std::wstring title;
    std::wstring osd1;
    std::wstring osd2;
    uint8_t compatFlags;
    uint8_t disc_type;
    uint8_t iconSrc;
    std::string iconPath;
    bool useMDMA0;
} GameParameters;

typedef struct HDLGameInfo{
    char title[GAME_TITLE_MAX_LEN_BYTES+1];
    char discID[DISCID_LENGTH+1];	/* E.g. SXXX-99999 */
    char startupFilename[FILENAME_LENGTH+1];	/* E.g. "SXXX-999.99;1" */
    uint8_t discType;
    uint32_t sectorsInDiscLayer0;
    uint32_t sectorsInDiscLayer1;
    uint8_t compatFlags;
    uint8_t TRType;
    uint8_t TRMode;
} __attribute__((packed)) HDLGameInfo;

typedef struct HDLGameEntry{
    char partition[PARTITION_LENGTH+1];
    char title[GAME_TITLE_MAX_LEN_BYTES+1];
    char discID[DISCID_LENGTH+1];
    uint8_t compatFlags;
    uint8_t TRType;
    uint8_t TRMode;
    uint8_t discType;
    uint32_t sectors;	//In 2048-byte units
} __attribute__((packed)) HDLGameEntry;

typedef struct OSDTitles{
    char title1[OSD_TITLE_MAX_LEN_BYTES+1];
    char title2[OSD_TITLE_MAX_LEN_BYTES+1];
} __attribute__((packed)) OSDTitles;

typedef struct IOInitReq{
    uint32_t sectors;
    uint32_t offset;
    char partition[PARTITION_LENGTH+1];
} __attribute__((packed)) IOInitReq;

#endif //HDLGI_HDLGI_COMMANDS_H
