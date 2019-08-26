/***************************************************************************
 *   aux_functions.cpp  --  This file is part of hdlgi-cli.                *
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

#include "aux_functions.h"

#include <iostream>

std::wstring convUTF8FromBytes(const char* bytes)
{
    std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> converter;
    return converter.from_bytes(bytes);
}

std::string convStringFromWstring(const std::wstring& wstr)
{
    std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> converter;
    return converter.to_bytes(wstr);
}

size_t readFileIntoBuffer(const char* path, void** buffer)
{
    FILE *file;
    int result;
    size_t length = 0;

    if((file = fopen(path, "rb")))
    {
        fseek(file, 0, SEEK_END);
        length = (size_t)ftell(file);
        rewind(file);
        if((*buffer = malloc(length)))
        {
            if(fread(*buffer, 1, length, file) == length) {result = SUCCESS;}
            else
            {
                free(*buffer);
                result = EIO;
            }
        }
        else {result = ENOMEM;}
        fclose(file);
    }
    else {result = ENOMEM;}

    return(result != SUCCESS ? result : length);
}