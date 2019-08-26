/***************************************************************************
 *   HDLGIConnection.h  --  This file is part of hdlgi-cli.                *
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

#ifndef HDLGI_HDLGICONNECTION_H
#define HDLGI_HDLGICONNECTION_H

#include "errcodes.h"
#include "HDLGI_commands.h"

#include <string>

#include <arpa/inet.h>
#include <netinet/tcp.h>
#include <netinet/in.h>
#include <poll.h>
#include <sys/socket.h>
#include <sys/types.h>


#define TIMEOUT_RECV 30
#define TIMEOUT_ACCEPT 80

#define IO_BLOCK_SIZE       2048

#define HDLGMAN_SERVER_VERSION		0x0C
#define HDLGMAN_CLIENT_VERSION		0x0C
#define SERVER_PORT_NUM	45061
#define SERVER_DATA_PORT_NUM 45062

typedef struct HDDToolsPacketHdr{
    uint32_t command;
    uint32_t payloadLength;
    int result;
} __attribute__((packed)) HDDToolsPacketHdr;


class HDLGIConnection
{
private:
    const std::string ip;
    int cmdSocket;
    int dataSocket;
    struct sockaddr_in serverAddr;
    int connSend(int socket, const void* buffer, const uint32_t length);
    int connRecv(int socket, void* buffer, const uint32_t length);

public:
    HDLGIConnection(const std::string& ip): ip(ip) {}
    int connectToInstance();
    int prepareDataConnection(int* s);
    void closeDataConnection();
    void closePendingDataConnection(int s);
    int acceptDataConnection(int s);
    int sendCmd(const void *buffer, const uint8_t command, const uint32_t numBytes);
    int sendData(const void *buffer, const uint32_t numBytes);
    int getResponse(void *buffer, const uint32_t numBytes);
    int getPayload(void *buffer, const uint32_t numBytes);
    int recvData(void *buffer, const uint32_t numBytes);
    int closeConnection();
};


#endif //HDLGI_HDLGICONNECTION_H
