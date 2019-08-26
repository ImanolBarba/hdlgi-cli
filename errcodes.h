/***************************************************************************
 *   errcodes.h  --  This file is part of hdlgi-cli.                       *
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

#ifndef HDLGI_CLI_ERRCODES_H
#define HDLGI_CLI_ERRCODES_H

enum ErrorCodes {
    SUCCESS = 0,
    INSTALL_SUCCESS = 1,
    EEXTCONNLOST,
    EEXTABORT,
    INVALID_VERSION,
    LOAD_ICON_FAIL,
    PART_ATTR_AREA_CORRUPTED,
    GAME_EXISTS,
    INVALID_SOCKET,
    UNRESOLVED_HOST
};

#define FAILURE (-1)


#endif //HDLGI_CLI_ERRCODES_H
