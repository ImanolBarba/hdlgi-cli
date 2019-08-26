/***************************************************************************
 *   OSD.h  --  This file is part of hdlgi-cli.                            *
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

#ifndef HDLGI_CLI_OSD_H
#define HDLGI_CLI_OSD_H

#include "aux_functions.h"
#include "errcodes.h"

#include <iomanip>
#include <iostream>
#include <regex>
#include <sstream>
#include <string>

#include <cstring>

#include <unistd.h>


#define OSD_TITLE_MAX_LEN_BYTES		(OSD_TITLE_MAX_LEN*4)			//In bytes, when in UTF-8 characters.
#define OSD_TITLE_MAX_LEN			16								//In characters

typedef int iconIVECTOR[4];
typedef float iconFVECTOR[4];

enum OSD_RESOURCE_FILE_TYPES{
    OSD_SYSTEM_CNF_INDEX	= 0,
    OSD_ICON_SYS_INDEX,
    OSD_VIEW_ICON_INDEX,
    OSD_DEL_ICON_INDEX,
    OSD_BOOT_KELF_INDEX,
    NUM_OSD_FILES_ENTS
};

typedef struct mcIcon
{
    unsigned char  head[4];     // header = "PS2D"
    unsigned short type;        // filetype, used to be "unknown1" (see MCICON_TYPE_* above)
    unsigned short nlOffset;    // new line pos within title name
    unsigned unknown2;          // unknown
    unsigned trans;             // transparency
    iconIVECTOR bgCol[4];       // background color for each of the four points
    iconFVECTOR lightDir[3];    // directions of three light sources
    iconFVECTOR lightCol[3];    // colors of each of these sources
    iconFVECTOR lightAmbient;   // ambient light
    unsigned short title[34];   // application title - NOTE: stored in sjis, NOT normal ascii
    unsigned char view[64];     // list icon filename
    unsigned char copy[64];     // copy icon filename
    unsigned char del[64];      // delete icon filename
    unsigned char unknown3[512];// unknown
} mcIcon;

typedef struct IconSysData{
    wchar_t title0[OSD_TITLE_MAX_LEN+1];
    wchar_t title1[OSD_TITLE_MAX_LEN+1];
    unsigned char bgcola;
    unsigned char bgcol0[3];
    unsigned char bgcol1[3];
    unsigned char bgcol2[3];
    unsigned char bgcol3[3];
    float lightdir0[3];
    float lightdir1[3];
    float lightdir2[3];
    unsigned char lightcolamb[3];
    unsigned char lightcol0[3];
    unsigned char lightcol1[3];
    unsigned char lightcol2[3];
    wchar_t uninstallmes0[61];
    wchar_t uninstallmes1[61];
    wchar_t uninstallmes2[61];
} IconSysData;

typedef struct ConvertedMcIcon{
    unsigned int HDDIconSysSize;
    char *HDDIconSys;
    unsigned int listViewIconSize;
    void *listViewIcon;
    unsigned int deleteIconSize;
    void *deleteIcon;
} ConvertedMcIcon;

typedef struct OSDResourceInitReq{
    int useSaveData;
    char osd1[OSD_TITLE_MAX_LEN_BYTES+1];	// 16 UTF-8 characters maximum.
    char osd2[OSD_TITLE_MAX_LEN_BYTES+1];	// 16 UTF-8 characters maximum.
    char discID[11];
    char partition[33];
} __attribute__((packed)) OSDResourceInitReq;

typedef struct OSDResourceWriteReq{
    int index;
    uint32_t length;
} __attribute__((packed)) OSDResourceWriteReq;

typedef struct OSDResourceStat{
    uint32_t lengths[NUM_OSD_FILES_ENTS];
} __attribute__((packed)) OSDResourceStat;

typedef struct OSDResourceStatReq{
    char partition[33];
} __attribute__((packed)) OSDResourceStatReq;

typedef struct OSDResourceReadReq{
    char partition[33];
    int index;
} __attribute__((packed)) OSDResourceReadReq;

int parseIconSysFile(const wchar_t *file, unsigned int length, IconSysData *data);
int generateHDDIconSysFile(const IconSysData *data, char *HDDIconSys, unsigned int outBuffLen);
int generateHDDIconSysFileFromMCSave(const mcIcon* icon, char *HDDIconSys, unsigned int outBuffLen, const wchar_t *title1, const wchar_t *title2);
int verifyMcSave(const std::string SaveFolderPath);
int convertMcSave(const std::string SaveFolderPath, ConvertedMcIcon *icon, const wchar_t *OSDTitleLine1, const wchar_t *OSDTitleLine2);

#endif //HDLGI_CLI_OSD_H
