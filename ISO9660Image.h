/***************************************************************************
 *   ISO9660Image.h  --  This file is part of hdlgi-cli.                   *
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

#ifndef HDLGI_CLI_ISO9660_H
#define HDLGI_CLI_ISO9660_H

#include "errcodes.h"

#include <iostream>
#include <string>

#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <cstdlib>

#define SYNCBYTES "\x00\xFF\0xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\x00"
#define IO_BLOCK_SIZE       2048

typedef struct iso9660_dirRec{
    uint8_t lenDR;
    uint8_t lenExAttr;

    uint32_t extentLoc_LE;
    uint32_t extentLoc_BE;

    uint32_t dataLen_LE;
    uint32_t dataLen_BE;

    uint8_t recDateTime[7];

    uint8_t fileFlags;
    uint8_t fileUnitSz;
    uint8_t interleaveGapSz;

    uint16_t volumeSeqNum_LE;
    uint16_t volumeSeqNum_BE;

    uint8_t lenFI;
    uint8_t fileIdentifier; /* The actual length is specified by lenFI */

    /* There are more fields after this. */
    /* Padding field; Only present if the length of the file identifier is an even number. */
    /* Field for system use; up to lenDR */
} __attribute__((packed)) iso9660_dirRec;

/* ISO9660 filesystem file path table */
typedef struct iso9660_pathTable
{
    uint8_t lenDI;
    uint8_t lenExRec;
    uint32_t extLoc;
    uint16_t parentDirNum;
    uint8_t dirIdentifier; /* The actual length is specified by lenDI. */
    /* There are more fields after this. */
    /* Padding field; Only present if the length of the directory identifier is an even number. */
} __attribute__((packed)) iso9660_pathTable;

class ISO9660Image
{
private:
    std::string path;
    FILE* discImg;
    size_t sectorCount;
    size_t DLDVDSectors;
    unsigned char sectorType;
    int rawReadSectors(unsigned char sectorType, uint32_t lsn, unsigned int sectors, void *buffer);
    int readSectors(unsigned char sectorType, uint32_t lsn, unsigned int sectors, void *buffer);

public:
    size_t currentLSN;
    ISO9660Image(std::string path);
    int parsePS2CNF(char *discID, char *startupFilename, unsigned char sectorType);
    uint32_t getFileLSN(const char *path, unsigned char sectorType, unsigned int *fileSize);
    int getDiscInfo(size_t* nSectorsLayer0, size_t* nSectorsLayer1, unsigned char* type);
    int readNext(void* buffer, size_t bufferSize);
    int close();
};




#endif //HDLGI_CLI_ISO9660_H
