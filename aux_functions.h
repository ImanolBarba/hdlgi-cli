/***************************************************************************
 *   aux_functions.h  --  This file is part of hdlgi-cli.                  *
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

#ifndef HDLGI_CLI_AUX_FUNCTIONS_H
#define HDLGI_CLI_AUX_FUNCTIONS_H

#include "errcodes.h"
#include <codecvt>
#include <locale>

std::wstring convUTF8FromBytes(const char* bytes);
std::string convStringFromWstring(const std::wstring& wstr);
size_t readFileIntoBuffer(const char* path, void** buffer);

#endif //HDLGI_CLI_AUX_FUNCTIONS_H
