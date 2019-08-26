/***************************************************************************
 *   ProgressBar.h  --  This file is part of hdlgi-cli.                    *
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

#ifndef HDLGI_CLI_PROGRESSBAR_H
#define HDLGI_CLI_PROGRESSBAR_H

#include <condition_variable>
#include <iomanip>
#include <iostream>
#include <mutex>
#include <thread>

#include <cstdlib>
#include <cstring>
#include <signal.h>

#define DISPLAY_VERTICAL_LINES 10
#define MIN_WIDTH 34
#define MIN_BAR_WIDTH 10
#define DEFAULT_TERMINAL_WIDTH 80

class ProgressBar
{
    private:
        std::wstring gameName;

        size_t remainingBytes;
        size_t currentGameBytes;

        double currSpeed;
        double cumSpeed;
        double avgSpeed;
        size_t avgCounter;

        std::condition_variable cond;
        std::mutex mutex;
        std::thread thread;
        bool stopDisplay;
        bool updateRequested;
        void displayThread();
        void printStatus(double currPercentage);

    public:
        ProgressBar(std::wstring& gameName, size_t totalBytes);
        void finish();
        void update(size_t numBytes, long long int ms);
        void start();

};


#endif //HDLGI_CLI_PROGRESSBAR_H
