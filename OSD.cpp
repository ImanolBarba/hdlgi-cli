/***************************************************************************
 *   OSD.cpp  --  This file is part of hdlgi-cli.                          *
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

#include "OSD.h"

int getXYZFVectorValueFromConfigLine(float *field,  const std::wstring& line)
{
    const std::wregex floatVecRegex(L"[a-zA-Z0-9]+\\s*=\\s*(-?[0-9]+\\.[0-9]+)\\s*,\\s*(-?[0-9]+\\.[0-9]+)\\s*,\\s*(-?[0-9]+\\.[0-9]+)");
    std::wsmatch matches;

    if(std::regex_search(line,matches,floatVecRegex))
    {
        if(matches.size()-1 == 3)
        {
            field[0] = wcstof(matches[1].str().c_str(), nullptr);
            field[1] = wcstof(matches[2].str().c_str(), nullptr);
            field[2] = wcstof(matches[3].str().c_str(), nullptr);
            return SUCCESS;
        }
    }
    return FAILURE;
}

int getXYZIVectorValueFromConfigLine(unsigned char *field, const std::wstring& line)
{
    const std::wregex intVecRegex(L"[a-zA-Z0-9]+\\s*=\\s*(-?[0-9]+)\\s*,\\s*(-?[0-9]+)\\s*,\\s*(-?[0-9]+)");
    std::wsmatch matches;

    if(std::regex_search(line,matches,intVecRegex))
    {
        if(matches.size()-1 == 3)
        {
            field[0] = (unsigned char)wcstoul(matches[1].str().c_str(), nullptr, 10);
            field[1] = (unsigned char)wcstoul(matches[2].str().c_str(), nullptr, 10);
            field[2] = (unsigned char)wcstoul(matches[3].str().c_str(), nullptr, 10);
            return SUCCESS;
        }
    }
    return FAILURE;
}

int getStringValueFromConfigLine(wchar_t* field, const std::wstring& line)
{
    const std::wregex stringRegex(L"[a-zA-Z0-9]+\\s*=\\s*(.*)$");
    std::wsmatch matches;

    if(std::regex_search(line,matches,stringRegex))
    {
        wcsncpy(field,matches[1].str().c_str(),matches[1].str().size());
        return SUCCESS;
    }
    return FAILURE;
}

int getIntValueFromConfigLine(unsigned char* field, const std::wstring& line)
{
    const std::wregex intRegex(L"[a-zA-Z0-9]+\\s*=\\s*(-?\\d+)");
    std::wsmatch matches;

    if(std::regex_search(line,matches,intRegex))
    {
        *field = (unsigned char)wcstoul(matches[1].str().c_str(), nullptr, 10);
        return SUCCESS;
    }
    return FAILURE;
}

int parseIconSysFile(const wchar_t *file, unsigned int length, IconSysData *data)
{
    int result = SUCCESS;
    std::wstring value;
    memset(data, 0, sizeof(IconSysData));
    std::wstring line;
    std::wstring unicodeBuffer(file,(unsigned long)length);

    std::wcout << unicodeBuffer << std::endl;

    std::wstringstream wsstream(unicodeBuffer);
    std::getline(wsstream,line);
    if(line == L"PS2X")
    {
        while(std::getline(wsstream,line))
        {
            if(line.rfind(L"title0",0) == 0)
            {
                result = getStringValueFromConfigLine(data->title0, line);
            }
            else if(line.rfind(L"title1",0) == 0)
            {
                result = getStringValueFromConfigLine(data->title1, line);
            }
            else if(line.rfind(L"bgcola",0) == 0)
            {
                result = getIntValueFromConfigLine(&(data->bgcola),line);
            }
            else if(line.rfind(L"bgcol0",0) == 0){
                result = getXYZIVectorValueFromConfigLine(data->bgcol0, line);
            }
            else if(line.rfind(L"bgcol1",0) == 0){
                result = getXYZIVectorValueFromConfigLine(data->bgcol1, line);
            }
            else if(line.rfind(L"bgcol2",0) == 0){
                result = getXYZIVectorValueFromConfigLine(data->bgcol2, line);
            }
            else if(line.rfind(L"bgcol3",0) == 0){
                result = getXYZIVectorValueFromConfigLine(data->bgcol3, line);
            }
            else if(line.rfind(L"lightdir0",0) == 0){
                result = getXYZFVectorValueFromConfigLine(data->lightdir0, line);
            }
            else if(line.rfind(L"lightdir1",0) == 0){
                result = getXYZFVectorValueFromConfigLine(data->lightdir1, line);
            }
            else if(line.rfind(L"lightdir2",0) == 0){
                result = getXYZFVectorValueFromConfigLine(data->lightdir2, line);
            }
            else if(line.rfind(L"lightcolamb",0) == 0){
                result = getXYZIVectorValueFromConfigLine(data->lightcolamb, line);
            }
            else if(line.rfind(L"lightcol0",0) == 0){
                result = getXYZIVectorValueFromConfigLine(data->lightcol0, line);
            }
            else if(line.rfind(L"lightcol1",0) == 0){
                result = getXYZIVectorValueFromConfigLine(data->lightcol1, line);
            }
            else if(line.rfind(L"lightcol2",0) == 0){
                result = getXYZIVectorValueFromConfigLine(data->lightcol2, line);
            }
            else if(line.rfind(L"uninstallmes0",0) == 0){
                result = getStringValueFromConfigLine(data->uninstallmes0, line);
            }
            else if(line.rfind(L"uninstallmes1",0) == 0){
                result = getStringValueFromConfigLine(data->uninstallmes1, line);
            }
            else if(line.rfind(L"uninstallmes2",0) == 0){
                result = getStringValueFromConfigLine(data->uninstallmes2, line);
            }
            else {result = FAILURE;}

            if(result != SUCCESS) {break;}
        }
    }
    else {result = EINVAL;}

    return result;
}

int generateHDDIconSysFile(const struct IconSysData *data, char *HDDIconSys, unsigned int outBuffLen)
{
    std::wstringstream wsstream;
    wsstream.precision(4);

    wsstream << L"PS2X" << std::endl;
    wsstream << L"title0 = " << data->title0 << std::endl; //Title line 1 is mandatory.
    wsstream << L"title1 = " << data->title1 << std::endl;
    wsstream << L"bgcola = " << data->bgcola << std::endl;
    wsstream << L"bgcol0 = " << data->bgcol0[0] << "," << data->bgcol0[1] << "," << data->bgcol0[2] << std::endl;
    wsstream << L"bgcol1 = " << data->bgcol1[0] << "," << data->bgcol1[1] << "," << data->bgcol1[2] << std::endl;
    wsstream << L"bgcol2 = " << data->bgcol2[0] << "," << data->bgcol2[1] << "," << data->bgcol2[2] << std::endl;
    wsstream << L"bgcol3 = " << data->bgcol3[0] << "," << data->bgcol3[1] << "," << data->bgcol3[2] << std::endl;
    wsstream << std::fixed << L"lightdir0 = " << data->lightdir0[0] << "," << data->lightdir0[1] << "," << data->lightdir0[2] << std::endl;
    wsstream << std::fixed << L"lightdir1 = " << data->lightdir1[0] << "," << data->lightdir1[1] << "," << data->lightdir1[2] << std::endl;
    wsstream << std::fixed << L"lightdir2 = " << data->lightdir2[0] << "," << data->lightdir2[1] << "," << data->lightdir2[2] << std::endl;
    wsstream << L"lightcolamb = " << data->lightcolamb[0] << "," << data->lightcolamb[1] << "," << data->lightcolamb[2] << std::endl;
    wsstream << L"lightcol0 = " << data->lightcol0[0] << "," << data->lightcol0[1] << "," << data->lightcol0[2] << std::endl;
    wsstream << L"lightcol1 = " << data->lightcol1[0] << "," << data->lightcol1[1] << "," << data->lightcol1[2] << std::endl;
    wsstream << L"lightcol2 = " << data->lightcol2[0] << "," << data->lightcol2[1] << "," << data->lightcol2[2] << std::endl;
    wsstream << L"uninstallmes0 = " << data->uninstallmes0 << std::endl;
    wsstream << L"uninstallmes1 = " << data->uninstallmes1 << std::endl;
    wsstream << L"uninstallmes2 = " << data->uninstallmes2 << std::endl;

    return (int)std::wcstombs(HDDIconSys, wsstream.str().c_str(), outBuffLen);
}

int generateHDDIconSysFileFromMCSave(const mcIcon* icon, char *HDDIconSys, unsigned int outBuffLen, const wchar_t *title1, const wchar_t *title2)
{
    IconSysData iconSysData;
    wcsncpy(iconSysData.title0, title1, sizeof(iconSysData.title0)-1);
    iconSysData.title0[OSD_TITLE_MAX_LEN] = '\0';

    //Line 2 is optional.
    if(title2)
    {
        wcsncpy(iconSysData.title1, title2, sizeof(iconSysData.title1)-1);
        iconSysData.title1[OSD_TITLE_MAX_LEN] = '\0';
    }
    else {iconSysData.title1[0] = '\0';}

    iconSysData.bgcola = (unsigned char)icon->trans;
    iconSysData.bgcol0[0] = (unsigned char)(icon->bgCol[0][0]/2);
    iconSysData.bgcol0[1] = (unsigned char)(icon->bgCol[0][1]/2);
    iconSysData.bgcol0[2] = (unsigned char)(icon->bgCol[0][2]/2);
    iconSysData.bgcol1[0] = (unsigned char)(icon->bgCol[1][0]/2);
    iconSysData.bgcol1[1] = (unsigned char)(icon->bgCol[1][1]/2);
    iconSysData.bgcol1[2] = (unsigned char)(icon->bgCol[1][2]/2);
    iconSysData.bgcol2[0] = (unsigned char)(icon->bgCol[2][0]/2);
    iconSysData.bgcol2[1] = (unsigned char)(icon->bgCol[2][1]/2);
    iconSysData.bgcol2[2] = (unsigned char)(icon->bgCol[2][2]/2);
    iconSysData.bgcol3[0] = (unsigned char)(icon->bgCol[3][0]/2);
    iconSysData.bgcol3[1] = (unsigned char)(icon->bgCol[3][1]/2);
    iconSysData.bgcol3[2] = (unsigned char)(icon->bgCol[3][2]/2);
    iconSysData.lightdir0[0] = icon->lightDir[0][0];
    iconSysData.lightdir0[1] = icon->lightDir[0][1];
    iconSysData.lightdir0[2] = icon->lightDir[0][2];
    iconSysData.lightdir1[0] = icon->lightDir[1][0];
    iconSysData.lightdir1[1] = icon->lightDir[1][1];
    iconSysData.lightdir1[2] = icon->lightDir[1][2];
    iconSysData.lightdir2[0] = icon->lightDir[2][0];
    iconSysData.lightdir2[1] = icon->lightDir[2][1];
    iconSysData.lightdir2[2] = icon->lightDir[2][2];
    iconSysData.lightcolamb[0] = (unsigned char)(icon->lightAmbient[0]*128);
    iconSysData.lightcolamb[1] = (unsigned char)(icon->lightAmbient[1]*128);
    iconSysData.lightcolamb[2] = (unsigned char)(icon->lightAmbient[2]*128);
    iconSysData.lightcol0[0] = (unsigned char)(icon->lightCol[0][0]*128);
    iconSysData.lightcol0[1] = (unsigned char)(icon->lightCol[0][1]*128);
    iconSysData.lightcol0[2] = (unsigned char)(icon->lightCol[0][2]*128);
    iconSysData.lightcol1[0] = (unsigned char)(icon->lightCol[1][0]*128);
    iconSysData.lightcol1[1] = (unsigned char)(icon->lightCol[1][1]*128);
    iconSysData.lightcol1[2] = (unsigned char)(icon->lightCol[1][2]*128);
    iconSysData.lightcol2[0] = (unsigned char)(icon->lightCol[2][0]*128);
    iconSysData.lightcol2[1] = (unsigned char)(icon->lightCol[2][1]*128);
    iconSysData.lightcol2[2] = (unsigned char)(icon->lightCol[2][2]*128);
    wcscpy(iconSysData.uninstallmes0, L"This will delete the game");
    iconSysData.uninstallmes1[0] = '\0';
    iconSysData.uninstallmes2[0] = '\0';

    return generateHDDIconSysFile(&iconSysData, HDDIconSys, outBuffLen);
}

int verifyMcSave(const std::string saveFolderPath)
{
    FILE *file;
    int result;
    mcIcon McIconSys;

    if((file = fopen((saveFolderPath + "/icon.sys").c_str(), "rb")) != nullptr)
    {
        result = fread(&McIconSys, sizeof(McIconSys), 1, file) == 1 ? SUCCESS : EIO;
        fclose(file);

        if(result == SUCCESS)
        {
            std::string iconView = convStringFromWstring(convUTF8FromBytes((const char*)McIconSys.view));
            if(access((saveFolderPath + "/" + iconView).c_str(),F_OK) == SUCCESS)
            {
                if(McIconSys.del[0] != '\0')
                {
                    std::string iconDel = convStringFromWstring(convUTF8FromBytes((const char*)McIconSys.del));
                    result = (access((saveFolderPath + "/" + iconDel).c_str(),F_OK) == SUCCESS) ? SUCCESS : ENOENT;
                }
                else {result = SUCCESS;}
            }
            else {result = ENOENT;}
        }
    }
    else {result = ENOENT;}

    return result;
}

int convertMcSave(const std::string saveFolderPath, ConvertedMcIcon *icon, const wchar_t *OSDTitleLine1, const wchar_t *OSDTitleLine2)
{
    FILE *file;
    int result;
    mcIcon McIconSys;

    memset(icon, 0, sizeof(ConvertedMcIcon));


    if((file = fopen((saveFolderPath + "/icon.sys").c_str(), "rb")) != nullptr)
    {
        result = fread(&McIconSys, sizeof(McIconSys), 1, file) == 1 ? SUCCESS : EIO;
        fclose(file);

        if(result == SUCCESS)
        {
            unsigned int fileSize = 0;
            std::string iconView = convStringFromWstring(convUTF8FromBytes((const char*)McIconSys.view));
            if((fileSize = (unsigned int)readFileIntoBuffer((saveFolderPath + "/" + iconView).c_str(), &icon->listViewIcon)) > 0)
            {
                icon->listViewIconSize = fileSize;
                result = SUCCESS;
                if(McIconSys.del[0] != '\0' && strcmp((const char*)McIconSys.del, (const char*)McIconSys.view) != 0)
                {
                    std::string iconDel = convStringFromWstring(convUTF8FromBytes((const char*)McIconSys.del));
                    if((fileSize = (unsigned int)readFileIntoBuffer((saveFolderPath + "/" + iconDel).c_str(), &icon->deleteIcon)) > 0)
                    {
                        icon->deleteIconSize = fileSize;
                    }
                    else {result = EIO;}
                }

                if(result == SUCCESS)
                {
                    if((icon->HDDIconSys = (char*)malloc(640)) != nullptr)
                    {
                        if((result = generateHDDIconSysFileFromMCSave(&McIconSys, icon->HDDIconSys, 640, OSDTitleLine1, OSDTitleLine2)) > 0)
                        {
                            icon->HDDIconSysSize = (unsigned int)result;
                            if((icon->HDDIconSys = (char*)realloc(icon->HDDIconSys, icon->HDDIconSysSize)) != nullptr) {result = SUCCESS;}
                            else {result = ENOMEM;}
                        }
                    }
                    else {result = ENOMEM;}
                }
            }
        }
    }
    else {result = ENOENT;}

    return result;
}

