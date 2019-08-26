/***************************************************************************
 *   HDLGIConnection.cpp  --  This file is part of hdlgi-cli.              *
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

#include <netdb.h>
#include "HDLGIConnection.h"

int HDLGIConnection::connSend(int socket, const void* buffer, const uint32_t length)
{
    int status = SUCCESS;
    size_t remaining = length;
    ssize_t numBytes = 0;
    while(remaining)
    {
        if((numBytes = send(socket,(uint8_t*)buffer + (length-remaining), remaining, 0)) <= 0)
        {
            status = EEXTCONNLOST;
            break;
        }
        remaining -= numBytes;
    }
    return status;
}

int HDLGIConnection::connRecv(int socket, void* buffer, const uint32_t length) {
    int rc;
    int status = SUCCESS;
    size_t remaining = length;
    ssize_t numBytes = 0;

    struct pollfd pollFDs;
    pollFDs.fd = socket;
    pollFDs.events = POLLIN;

    while (remaining)
    {
        rc = poll(&pollFDs, 1, TIMEOUT_RECV * 1000);
        if (rc == 1 && (pollFDs.revents & POLLIN) == POLLIN)
        {
            if ((numBytes = recv(socket, (uint8_t*)buffer + (length - remaining), remaining, 0)) <= 0)
            {
                status = EEXTCONNLOST;
                break;
            }
            remaining -= numBytes;
        }
        else
        {
            status = EEXTCONNLOST;
            this->closeConnection();
            break;
        }
    }
    return status;
}

int HDLGIConnection::connectToInstance()
{
    int value;
    int version = 0;
    int result;

    struct in_addr *addr_ptr;
    const char* address;
    struct hostent *hostPtr = gethostbyname(ip.c_str());
    if(hostPtr == NULL)
    {
        return UNRESOLVED_HOST;
    }
    addr_ptr = (struct in_addr *)*hostPtr->h_addr_list;
    address = inet_ntoa(*addr_ptr);
    if(!strcmp(address,""))
    {
        return UNRESOLVED_HOST;
    }

    this->cmdSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    this->serverAddr.sin_family = AF_INET;
    this->serverAddr.sin_port = htons(SERVER_PORT_NUM);
    this->serverAddr.sin_addr.s_addr = inet_addr(address);
    value = IO_BLOCK_SIZE * IO_BUFFER_SIZE;
    setsockopt(this->cmdSocket, SOL_SOCKET, SO_RCVBUF, (const char*)&value, sizeof(value));

    if((result = connect(this->cmdSocket, (struct sockaddr*)&(this->serverAddr), sizeof(this->serverAddr))) == SUCCESS)
    {
        if((result = sendCmd(nullptr, HDLGMAN_SERVER_GET_VERSION, 0)) == SUCCESS)
        {
            if((result = getResponse(&version, sizeof(version))) == SUCCESS)
            {
                if (version != HDLGMAN_SERVER_VERSION) {result = INVALID_VERSION;}
                else
                {
                    version = HDLGMAN_CLIENT_VERSION;
                    sendCmd(&version, HDLGMAN_CLIENT_SEND_VERSION, sizeof(version));
                    result = getResponse(nullptr, 0);
                }
            }
        }
    }
    return result;
}

int HDLGIConnection::prepareDataConnection(int* s)
{
    struct sockaddr_in addr;
    int value;
    *s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    addr.sin_family = AF_INET;
    addr.sin_port = htons(SERVER_DATA_PORT_NUM);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);

    setsockopt(*s, SOL_SOCKET, SO_REUSEADDR, (const char*)&value, sizeof(value));

    if(bind(*s, (struct sockaddr*)&addr, sizeof(addr)) == SUCCESS)
    {
        return listen(*s, 1);
    }

    return INVALID_SOCKET;
}

int HDLGIConnection::acceptDataConnection(int server)
{
    int rc;
    int result;
    int peer = 0;

    struct pollfd pollFDs;
    pollFDs.fd = server;
    pollFDs.events = POLLIN;

    rc = poll(&pollFDs, 1, TIMEOUT_ACCEPT * 1000);

    if(rc == 1 && (pollFDs.revents & POLLIN)  == POLLIN)
    {
        peer = accept(server, nullptr, nullptr);
        result = SUCCESS;
    }
    else {result = EEXTCONNLOST;}

    if(result == SUCCESS)
    {
        this->dataSocket = peer;
    }
    return result;
}

int HDLGIConnection::sendCmd(const void *buffer, const uint8_t command, const uint32_t numBytes)
{
    int result;
    HDDToolsPacketHdr header;

    header.command = command;
    header.result = 0;
    header.payloadLength = numBytes;

    if((result = this->connSend(this->cmdSocket, (void*)&header, sizeof(header))) == SUCCESS)
    {
        if(numBytes) {result = this->connSend(this->cmdSocket, (void*)buffer, numBytes);}
    }
    return result;
}

int HDLGIConnection::sendData(const void *buffer, const uint32_t numBytes)
{
    return this->connSend(this->dataSocket, (void*)buffer, numBytes);
}

int HDLGIConnection::getResponse(void *buffer, const uint32_t numBytes)
{
    int result;
    HDDToolsPacketHdr header;

    if((result = this->connRecv(this->cmdSocket, (void*)&header, sizeof(header))) == SUCCESS)
    {
        result = header.result;
        if(numBytes > 0 && header.payloadLength > 0)
        {
            if((result = this->connRecv(this->cmdSocket, buffer, header.payloadLength > numBytes ? numBytes : header.payloadLength)) == SUCCESS)
            {
                result = header.result;
            }
        }
    }
    return result;
}

int HDLGIConnection::getPayload(void *buffer, const uint32_t numBytes)
{
    return this->connRecv(this->cmdSocket, buffer, numBytes);
}

int HDLGIConnection::recvData(void *buffer, const uint32_t numBytes)
{
    return this->connRecv(this->dataSocket, buffer, numBytes);
}


int HDLGIConnection::closeConnection()
{
    shutdown(this->cmdSocket, SHUT_RDWR);
    return close(this->cmdSocket);
}

void HDLGIConnection::closeDataConnection()
{
    char dummy;

    shutdown(this->dataSocket, SHUT_WR);
    while(recv(this->dataSocket, &dummy, sizeof(dummy), 0) > 0);
    close(this->dataSocket);
}

void HDLGIConnection::closePendingDataConnection(int s)
{
    shutdown(s, SHUT_WR);
    close(s);
}