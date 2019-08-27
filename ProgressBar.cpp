/***************************************************************************
 *   ProgressBar.cpp  --  This file is part of hdlgi-cli.                  *
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

#include "ProgressBar.h"

#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>

static bool windowResized = false;
static size_t terminalWidth;
void windowHandler(int signal) {windowResized = true;}

void updateTerminalWidth()
{
    #ifdef TIOCGWINSZ
        struct winsize wsz;
        int fd = fileno (stderr);
        if (ioctl(fd, TIOCGWINSZ, &wsz) < 0) {terminalWidth = DEFAULT_TERMINAL_WIDTH;} /* most likely ENOTTY */
        else
        {
            terminalWidth = wsz.ws_col;
            if(!terminalWidth) {terminalWidth = DEFAULT_TERMINAL_WIDTH;}
        }
    #else
        terminalWidth = DEFAULT_TERMINAL_WIDTH;
    #endif
}

std::string genProgressBar(double percentage)
{
    std::string bar = "[";
    size_t numEquals;
    size_t barWidth;

    if(windowResized)
    {
        updateTerminalWidth();
        windowResized = false;
    }
    barWidth = (terminalWidth > MIN_WIDTH) ? (terminalWidth - (MIN_WIDTH - MIN_BAR_WIDTH)) : MIN_BAR_WIDTH;
    numEquals = (size_t)((barWidth-2)*percentage/100);
    for(int i = 0; i < numEquals; ++i)
    {
        bar += "=";
    }
    if(numEquals != (barWidth-2))
    {
        bar += ">";
        ++numEquals;
    }
    for(size_t i = numEquals; i < (barWidth-2); ++i) {bar += " ";}
    bar += "]";
    return bar;
}

void ProgressBar::printStatus(double currPercentage)
{
    unsigned int timeRemaining = (unsigned int)((remainingBytes/1024) / this->avgSpeed);
    unsigned int hours = timeRemaining / 3600;
    unsigned int minutes = (timeRemaining - (hours*60)) / 60;
    unsigned int seconds = (timeRemaining - (hours*3600) - (minutes * 60));

    std::cout << "Current Speed: " << std::fixed << std::setprecision(2) << currSpeed << " KiB/s" << std::endl;
    std::cout << "Average Speed: " << std::fixed << std::setprecision(2) << avgSpeed << " KiB/s" << std::endl;
    std::cout << std::endl;
    std::cout << "Remaining Time: ";
    if(hours) {std::cout << hours << " h " << minutes << " min " << seconds << " sec" << std::endl;}
    else if(minutes) {std::cout << minutes << " min " << seconds << " sec" << std::endl;}
    else {std::cout << seconds << " sec" << std::endl;}
    std::cout << std::endl;
    std::cout << "Data transferred: " << std::fixed << std::setprecision(2) << ((double)(currentGameBytes - remainingBytes)/1048576) << " / " << ((double)currentGameBytes/1048576) << "MiB" << std::endl;
    std::cout << std::endl;
    std::wcout << L"Current Game: " << gameName << std::endl;
    std::cout << std::endl;
    std::cout << "Game Progress: " << genProgressBar(currPercentage) << " " << std::setprecision(2) << currPercentage << "%" << std::endl;

    /*
     * Current Speed: 1234 KiB/s
     * Average Speed: 1234 KiB/s
     *
     * Time remaining: 1 h 5 min 13 sec
     *
     * Data transferred: 1678 / 2700 MiB
     *
     * Current Game: $gameName
     *
     * Game Progress: [======================>                            ] 47.13%
     */
}

void clearLines(unsigned int numLines)
{
    char* spaces = (char*)calloc(terminalWidth+1,sizeof(char));
    memset(spaces,' ',terminalWidth);
    for(int i = 0; i < numLines; ++i)
    {
        std::cout << "\x1b[A";
        std::cout << spaces;
        std::cout << "\r";
    }
    free(spaces);
}

ProgressBar::ProgressBar(std::wstring& gameName, size_t totalBytes) :
        gameName(gameName), remainingBytes(totalBytes), currentGameBytes(totalBytes),
        avgSpeed(0), avgCounter(1), cumSpeed(0), stopDisplay(false), updateRequested(false)
{
    signal(SIGWINCH,windowHandler);
}

void ProgressBar::start()
{
    updateTerminalWidth();
    this->thread = std::thread(&ProgressBar::displayThread, this);
}

void ProgressBar::finish()
{
    std::unique_lock<std::mutex> lock(mutex, std::defer_lock);
    lock.lock();
    updateRequested = true;
    stopDisplay = true;
    lock.unlock();
    cond.notify_one();
    thread.join();
    signal(SIGWINCH,SIG_DFL);
}

void ProgressBar::update(size_t numBytes, long long int ms)
{
    currSpeed = ((double)numBytes/1024)/((double)ms/1000);
    cumSpeed += currSpeed;
    avgSpeed = (cumSpeed)/(avgCounter++);

    remainingBytes -= numBytes;

    std::unique_lock<std::mutex> lock(mutex, std::defer_lock);
    if(lock.try_lock()) // Don't wait for UI to finish drawing, we'll update next time
    {
        updateRequested = true;
        lock.unlock();
        cond.notify_one();
    }
    if(!remainingBytes) // Unless we're done
    {
        lock.lock();
        updateRequested = true;
        lock.unlock();
        cond.notify_one();
        finish();
    }
}

void ProgressBar::displayThread()
{
    for(int i = 0; i < DISPLAY_VERTICAL_LINES+1; ++i) {std::cout << std::endl;}
    std::unique_lock<std::mutex> lock(mutex);
    for(;;)
    {
        clearLines(DISPLAY_VERTICAL_LINES);
        printStatus((double)(currentGameBytes - remainingBytes)*100/currentGameBytes);
        if (stopDisplay) {break;}
        cond.wait(lock, [this]{return updateRequested;});
        updateRequested = false;
    }
    lock.unlock();
}