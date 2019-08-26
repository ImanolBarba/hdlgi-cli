/***************************************************************************
 *   ISO9660Image.cpp  --  This file is part of hdlgi-cli.                 *
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

#include "ISO9660Image.h"

void stripDirFilename(const char *path, char *target, unsigned int *externPtr)
{
    const char *pathPtr;
    size_t nameLen;

    pathPtr = strchr(path,'\\');
    if(pathPtr == nullptr)
    { /* A file's name */
        nameLen = strlen(path);
        strcpy(target,path);
        if(externPtr) {*externPtr+=nameLen;}
    }
    else
    { /* A folder's name */
        nameLen = (unsigned int)(pathPtr - path);
        strncpy(target, path, nameLen);
        if(externPtr) {*externPtr += nameLen;}
    }
}

ISO9660Image::ISO9660Image(std::string path) : path(path)
{
    currentLSN = 0;
    sectorCount = 0;
    DLDVDSectors = 0;

    discImg = fopen(path.c_str(), "rb");
    if(!discImg)
    {
        throw std::runtime_error("Error opening image: " + std::string(strerror(errno)));
    }

    unsigned char *sectorBuff;
    DLDVDSectors = 0;

    /* Allocate memory to copy a sample of the sectors on the disc. */
    sectorBuff = (unsigned char*)malloc(2352);
    fread(sectorBuff, 12, 1, discImg); /* Copy the sync bytes (1st 12 bytes) of the 1st sector as a sample. */

    if(!memcmp(SYNCBYTES, sectorBuff, 12))
    {
        fseek(discImg, 15, SEEK_SET); /* Seek to the "mode" byte of the first sector of the disc image file. */
        fread(&sectorType, 1, 1, discImg);
    }
    else {sectorType = 0xFF;} /* "Headerless" disc image */

    /* Check if the loaded disc/disc image has a ISO9660 format. */
    if(readSectors(sectorType, 16, 1, sectorBuff) != 1)
    { /* Read sector 16. */
        free(sectorBuff);
        throw std::runtime_error("Error determining ISO9660 format");
    }

    if((sectorBuff[0] != 0x01) || (memcmp(&sectorBuff[1], "CD001", 5) != 0))
    {
        free(sectorBuff);
        throw std::runtime_error("Image is not ISO9660");
    }

    memcpy(&(this->sectorCount), &sectorBuff[80], 4); /* Copy the recorded number of sectors in the ISO9660 filesystem. */
    memset(sectorBuff, 0, 2352); /* Flood-fill the temporary buffer with 0s to prevent false DVD9 format disc image detections. */

    if(readSectors(sectorType, (uint32_t)sectorCount, 1, sectorBuff) == 1)
    { /* Attempt to read the sector after the last sector after the last sector of layer 0. That sector is layer 1's sector #16. */
        if((sectorBuff[0] == 0x01) && (!memcmp(&sectorBuff[1], "CD001", 5)))
        { /* The last sector of layer 0 is sector 16 of layer 1. */
            memcpy(&(this->DLDVDSectors), &sectorBuff[80], 4);
        }
        else
        {
            rewind(discImg);
            fseek(discImg, 0, SEEK_END);
            if(sectorCount != (ftell(discImg) / ((sectorType == 0xFF) ? 2048 : 2352)))
            {
                std::cerr << "The number of sectors indicated within the ISO9660 filesystem does not match the size of the disc image" << std::endl;
                //There may be trailing data, so calculate the number of sectors based on the disc image file's size instead.
                sectorCount = (uint32_t)fseek(discImg, 0, SEEK_END) / ((sectorType == 0xFF) ? 2048 : 2352);
            }
            rewind(discImg);
        }
    }
    free(sectorBuff);
}

int ISO9660Image::parsePS2CNF(char *discID, char *startupFilename, unsigned char sectorType)
{
    uint32_t cnflsn;
    char data[22], *cnf;
    unsigned char sectorBuff[2048];
    unsigned int cnfSize, bytesToRead, bytesRemaining;

    if(!(cnflsn = getFileLSN("SYSTEM.CNF;1", sectorType, &cnfSize))) {return FAILURE;}

    cnf = (char*)calloc(cnfSize + 1,sizeof(char));
    bytesRemaining = cnfSize;
    while(bytesRemaining > 0)
    {
        bytesToRead = (bytesRemaining > 2048) ? 2048 : bytesRemaining;
        if(readSectors(sectorType, cnflsn++, 1, sectorBuff) != 1) {return FAILURE;}
        memcpy(&cnf[cnfSize - bytesRemaining], sectorBuff, bytesToRead);

        bytesRemaining -= bytesToRead;
    }

    if(strstr(cnf, "BOOT2") != nullptr)
    {
        strncpy(data, strstr(cnf, "cdrom0"), 22); /* Copy "cdrom0:\SLXX_XXX.XX;1" */
        data[21] = '\0';
        strncpy(startupFilename, &data[8], 11);
        startupFilename[11] = '\0';
    }
    else
    {
        free(cnf);
        return FAILURE;
    }

    free(cnf);

    /* Generate the disc ID string in this format: SXXX-XXXXX. */
    strncpy(discID, startupFilename, 4);
    discID[4] = '-';
    strncpy(&discID[5], &startupFilename[5], 3);
    strncpy(&discID[8], &startupFilename[9], 2);
    discID[10] = '\0';

    return SUCCESS;
}

uint32_t ISO9660Image::getFileLSN(const char *path, unsigned char sectorType, unsigned int *fileSize)
{
    char tgtfName[14];
    unsigned int pathPtr = 0;
    uint8_t *dataBuf; /* Data buffer for disc information extraction */
    uint32_t extentLSN, lsn;
    unsigned char fileEntryName[16];

    iso9660_pathTable *ptable;
    iso9660_dirRec *dirRec;

    unsigned char IDLen;
    unsigned char nameLen;		/* The length of the file entry (On the ISO9660 disc image), and the length of the name of the file/directory that's being searched for. */
    uint32_t ptableSize;
    uint32_t ptableLSN;
    uint32_t rootDirRecLSN;
    uint32_t recLSN;
    uint32_t recGrpOffset;
    uint32_t recGrpSize;   /* Size of an entire directory record group. */

    dataBuf = (unsigned char*)malloc(2048);
    ptable = (struct iso9660_pathTable *)dataBuf;

    /* 1. Seek to the path table.
    2. begin tracing.
    3. Return LSN of file when found. */
    readSectors(sectorType, 16, 1, dataBuf); /* Read sector 16. */
    ptableLSN = (uint32_t)*((unsigned long*)(&dataBuf[140])); /* Read path table LSN. */
    memcpy((unsigned char*)&ptableSize, &dataBuf[132], 4); /* Read the size of the type-L path table. */
    rootDirRecLSN = (uint32_t)(*(unsigned long*)(&dataBuf[158])); /* Read the root directory record LSN. */

    while(path[pathPtr] == '\\') {++pathPtr;} /* Skip any '\' characters present */
    extentLSN = rootDirRecLSN; /* Start in the root directory record */
    if(strchr(&path[pathPtr],'\\'))
    {
        /* Continue tracing at path table if the file is not @ the root folder */
        recLSN = ptableLSN;
        recGrpOffset = 0;
        readSectors(sectorType, recLSN, 1, dataBuf); /* Read the path table. */

        stripDirFilename(&path[pathPtr], tgtfName, &pathPtr); /* "Strip and search..." */

        recGrpSize = ptableSize;
        nameLen = (unsigned char)strlen(tgtfName);
        do{
            if(recGrpOffset > recGrpSize)
            { /* Prevent an overflow when file is not found */
                free(dataBuf);
                return 0;
            }

            IDLen = ptable->lenDI; /* Get LEN_DI(Length of directory identifier) */
            memcpy((unsigned char*)&extentLSN, &ptable->extLoc, 4); /* Copy the location of the extent. */
            memcpy(fileEntryName, &ptable->dirIdentifier, IDLen); /* name + extension must be at least 1 character long + ";" (e.g. "n.bin;1") */
            if(ptable->lenDI % 2) {ptable->lenDI = (uint8_t)(ptable->lenDI + 1);} /* There will be a padding field if the directory identifier is an odd number */

            if(IDLen)
            {
                recGrpOffset = recGrpOffset + 8 + (uint32_t)ptable->lenDI;
                ptable = (iso9660_pathTable *)((std::ptrdiff_t)ptable + 8 + (uint32_t)ptable->lenDI);
            }
            else
            { /* The last path table entry (It's a blank one) in this sector has been reached. Continue searching in the next sector. */
                recLSN++;
                readSectors(sectorType, recLSN, 1, dataBuf); /* Read the sector containing the next extent. */
                ptable = (iso9660_pathTable *)dataBuf; /* Reset sector pointer */
            }

            if(nameLen>IDLen) {IDLen = nameLen;} /* Compare both names, up to the length of the longer string. */

        }while(memcmp(fileEntryName, tgtfName, IDLen) != 0); /* Break out of searching loop if match is found */
    }

    while(strchr(&path[pathPtr],'\\'))
    {  /* While the current segment is not the file's name */
        recLSN = extentLSN;
        readSectors(sectorType, recLSN, 1, dataBuf); /* Read the sector containing the next extent. */
        dirRec = (iso9660_dirRec *)dataBuf; /* Reset sector pointer */
        recGrpOffset = 0;

        memcpy((unsigned char*)&recGrpSize, &dirRec->dataLen_LE, 4);
        stripDirFilename(&path[pathPtr], tgtfName, &pathPtr); /* "Strip and search..." */
        nameLen = (unsigned char)strlen(tgtfName);

        do
        {
            if(((std::ptrdiff_t)dirRec - (std::ptrdiff_t)dataBuf) > recGrpSize)
            { /* Prevent overflow when file is not found */
                free(dataBuf);
                return 0;
            }

            memcpy((unsigned char*)&extentLSN, &dirRec->extentLoc_LE , 4); /* Read extent LSN address */
            IDLen = dirRec->lenFI; /* Get file ID length */
            memcpy(fileEntryName, &dirRec->fileIdentifier, IDLen); /* name + extension must be at least 1 character long + ";" (e.g. "n.bin;1") */
            if(IDLen)
            {
                recGrpOffset = recGrpOffset + (uint32_t)dirRec->lenDR;
                dirRec = (iso9660_dirRec *)((std::ptrdiff_t)dirRec + (uint32_t)dirRec->lenDR);
            }
            else
            { /* The last directory record (It's a blank one) in this sector has been reached. Continue searching in the next sector. */
                ++recLSN;
                readSectors(sectorType, recLSN, 1, dataBuf); /* Read the sector containing the next extent. */
                dirRec = (iso9660_dirRec *)dataBuf; /* Reset sector pointer */
            }

            if((IDLen) && (nameLen > IDLen)) {IDLen = nameLen;} /* Compare both names, up to the length of the longer string. */
        }while((!IDLen) || memcmp(fileEntryName, tgtfName, IDLen) != 0); /* Break out of searching loop if match is found */
    }

    /* Sector was read into buffer earlier while tracing through directory structure */
    stripDirFilename(&path[pathPtr], tgtfName, &pathPtr); /* "Strip and search..." */
    nameLen = (unsigned char)strlen(tgtfName);

    dirRec = (iso9660_dirRec *)dataBuf;
    recGrpOffset = 0;
    lsn = extentLSN;

    readSectors(sectorType, extentLSN, 1, dataBuf); /* Read the sector containing the next extent. */

    memcpy((unsigned char*)&recGrpSize, &dirRec->dataLen_LE, 4);
    do
    {
        if(recGrpOffset >= recGrpSize)
        {
            free(dataBuf);
            return 0; /* Prevent an overflow when file is not found */
        }

        memcpy((unsigned char*)&lsn, &dirRec->extentLoc_LE, 4); /* Read the extent's LSN. */
        memcpy((unsigned char*)fileSize, &dirRec->dataLen_LE, 4); /* Copy the size of the extent. */
        IDLen = dirRec->lenFI; /* Get file ID length */
        memset(fileEntryName, 0, 16); /* 0-fill the file/folder name array */
        memcpy(fileEntryName, &dirRec->fileIdentifier, IDLen); /* name + extension must be at least 1 character long + ";" (e.g. "n.bin;1") */
        recGrpOffset = recGrpOffset + (uint32_t)dirRec->lenDR;
        dirRec = (iso9660_dirRec *)((std::ptrdiff_t)dirRec + (uint32_t)dirRec->lenDR); /* Seek to next record */

        if(!IDLen)
        {
            /* The last directory record (It's a blank one) in this sector has been reached. Continue searching in the next sector. */
            ++extentLSN;
            readSectors(sectorType, extentLSN, 1, dataBuf); /* Read the sector containing the next extent. */
            dirRec = (iso9660_dirRec *)dataBuf; /* Reset sector pointer */
        }

        if((IDLen) && (nameLen > IDLen)) {IDLen = nameLen;} /* Compare both names, up to the length of the longer string. */
    }while((!IDLen) || memcmp(fileEntryName, tgtfName, IDLen) != 0); /* Break out of searching loop if match is found. */

    free(dataBuf);

    return lsn;
}

/* Returns information about the disc (Number of sectors (For both layers, if present) and the type of disc). */
int ISO9660Image::getDiscInfo(size_t* nSectorsLayer0, size_t* nSectorsLayer1, unsigned char* type)
{
    *nSectorsLayer0 = sectorCount;
    *nSectorsLayer1 = DLDVDSectors;
    *type = sectorType;
    return SUCCESS;
}

int ISO9660Image::rawReadSectors(unsigned char sectorType, uint32_t lsn, unsigned int sectors, void *buffer)
{
    unsigned int sectorsToRead;
    unsigned char *bufferPtr;

    sectorsToRead = sectors;
    bufferPtr = (unsigned char*)buffer;

    /* ISO9660 MODE 1;		1 sector = 12 bytes "sync" + 4 bytes CRC + 2048 bytes data + 8 bytes reserved space + 280 bytes parity error checking codes. */
    /* Skip the MODE2/XA header;	1 sector = 12 bytes "sync" + 4 bytes CRC + 8 bytes subchannel + 2048 bytes data + 280 bytes parity error checking codes.  */
    /* "Headerless" disc image;	1 sector = 2048 bytes data. */

    if((sectorType == 1) || (sectorType == 2)) {fseek(discImg, (long)lsn*2352, SEEK_SET);}
    else {fseek(discImg, (long)lsn*2048, SEEK_SET);}

    if((sectorType == 1) || (sectorType == 2))
    {
        while(sectorsToRead-- > 0)
        {
            if(fread(bufferPtr,2352,1,discImg) != 2352) {break;}
            bufferPtr += 2352;
        }
    }
    else {sectorsToRead -= fread(bufferPtr, IO_BLOCK_SIZE, sectorsToRead, discImg);}
    return(sectors - sectorsToRead);
}

int ISO9660Image::readSectors(unsigned char sectorType, uint32_t lsn, unsigned int sectors, void *buffer)
{
    unsigned int sectorsToRead;
    unsigned char *bufferPtr, *sectorBuff;

    sectorsToRead = sectors;
    bufferPtr = (unsigned char*)buffer;

    if(sectorType == 0xFF)
    { /* "Headerless" disc image. */
        if(rawReadSectors(sectorType, lsn, sectors, buffer) != sectors) {return 0;}
        sectorsToRead = 0; /* Read all sectors in one shot. */
    }
    else
    { /* MODE 1/2048, or MODE 2/XA Form 1. */
        sectorBuff = (unsigned char*)malloc(2352);
        while(sectorsToRead > 0)
        {
            if(rawReadSectors(sectorType, lsn, 1, sectorBuff) != 1) {break;}

            if(sectorType == 1) {memcpy(bufferPtr, &sectorBuff[16], 2048); /* Skip sync and mode bytes. */}
            else {memcpy(bufferPtr, &sectorBuff[24], 2048); /* Skip the sync bytes and subchannel data for MODE2/XA FORM 1. */}

            --sectorsToRead;
            bufferPtr += 2048;
            ++lsn;
        }
        free(sectorBuff);
    }
    return(sectors - sectorsToRead);
}

int ISO9660Image::readNext(void* buffer, size_t bufferSize)
{
    size_t sectorsToRead = ((sectorCount + DLDVDSectors) - currentLSN) > bufferSize ? bufferSize : ((sectorCount + DLDVDSectors) - currentLSN);
    int sectorsRead = readSectors(sectorType, (uint32_t)currentLSN, (unsigned int)sectorsToRead, buffer);
    currentLSN += sectorsRead;
    return sectorsRead;
}

int ISO9660Image::close()
{
    return fclose(discImg);
}