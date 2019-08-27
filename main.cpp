/***************************************************************************
 *   main.cpp  --  This file is part of hdlgi-cli.                         *
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

#include <iostream>
#include <map>
#include <getopt.h>
#include "HDLGIInstance.h"

#define VERSION "1.0"

enum paramIndex
{
    UNKNOWN = 0,
    NEWTITLE,
    OSD1,
    OSD2,
    TYPE,
    COMPAT,
    ICONSRC,
    ICONPATH,
    MDMA0,
    TITLE,
    IMAGE,
    OUTPUT,
    DISCID,
    OVERWRITE
};

static struct option longopts[] = {
    { "newtitle",  required_argument,            NULL,           NEWTITLE  },
    { "osd1",      required_argument,            NULL,           OSD1      },
    { "osd2",      required_argument,            NULL,           OSD2      },
    { "type",      required_argument,            NULL,           TYPE      },
    { "compat",    required_argument,            NULL,           COMPAT    },
    { "icon-src",  required_argument,            NULL,           ICONSRC   },
    { "icon-path", required_argument,            NULL,           ICONPATH  },
    { "use-mdma0", no_argument,                  NULL,           MDMA0     },
    { "title",     required_argument,            NULL,           TITLE     },
    { "image",     required_argument,            NULL,           IMAGE     },
    { "output",    required_argument,            NULL,           OUTPUT    },
    { "discid",    required_argument,            NULL,           DISCID    },
    { "host",      required_argument,            NULL,           'H'       },
    { "help",      no_argument,                  NULL,           'h'       },
    { "version",   no_argument,                  NULL,           'v'       },
    { "overwrite", no_argument,                  NULL,           OVERWRITE },
    { NULL,        0,                            NULL,           UNKNOWN   }
};

void printError(int code)
{
    switch(code)
    {
        case EEXTCONNLOST:
            std::cerr << "Lost connection with HDLGI instance" << std::endl;
            break;
        case EEXTABORT:
            std::cerr << "Operation aborted" << std::endl;
            break;
        case PART_ATTR_AREA_CORRUPTED:
            std::cerr << "Partition attribute area corrupted" << std::endl;
            break;
        case GAME_EXISTS:
            std::cerr << "Game is already installed" << std::endl;
            break;
    }
}

void printCompatFlags(uint8_t flags)
{
    std::cout << "Mode 1 - ALTERNATE_EE_CORE        (0x01): " << ((flags & ALTERNATE_EE_CORE)               ? "ENABLED" : "DISABLED") << std::endl;
    std::cout << "Mode 2 - ALTERNATE_READING_METHOD (0x02): " << (((flags & ALTERNATE_READING_METHOD) >> 1) ? "ENABLED" : "DISABLED") << std::endl;
    std::cout << "Mode 3 - UNHOOK_SYSCALLS          (0x04): " << (((flags & UNHOOK_SYSCALLS)          >> 2) ? "ENABLED" : "DISABLED") << std::endl;
    std::cout << "Mode 4 - DISABLE_PSS_VIDEOS       (0x08): " << (((flags & DISABLE_PSS_VIDEOS)       >> 3) ? "ENABLED" : "DISABLED") << std::endl;
    std::cout << "Mode 5 - DISABLE_DVD9_SUPPORT     (0x10): " << (((flags & DISABLE_DVD9_SUPPORT)     >> 4) ? "ENABLED" : "DISABLED") << std::endl;
    std::cout << "Mode 6 - DISABLE_IGR              (0x20): " << (((flags & DISABLE_IGR)              >> 5) ? "ENABLED" : "DISABLED") << std::endl;
    //std::cout << "Mode 7 - UNUSED_COMPAT            (0x40): " << (((flags & UNUSED_COMPAT)            >> 6) ? "ENABLED" : "DISABLED") << std::endl;
    std::cout << "Mode 8 - HIDE_DEV9_MODULE         (0x80): " << (((flags & HIDE_DEV9_MODULE)         >> 7) ? "ENABLED" : "DISABLED") << std::endl;
}

int shutdownConsole(HDLGIInstance& ps2)
{
    return ps2.poweroff();
}

int printGameList(HDLGIInstance& ps2)
{
    int freeSpace = ps2.getFreeSpace();
    std::cout << "Free space: " << std::setprecision(2) << std::fixed << (double)freeSpace/1024 << " GB" << std::endl << std::endl;
    std::vector<HDLGameEntry> entries = ps2.getGamesInstalled();
    std::cout << "Installed Games list:" << std::endl << std::endl;
    for(HDLGameEntry& entry : entries) {std::cout << "  * " << entry.title << " (" << entry.discID << ")" << std::endl;}
    std::cout << std::endl;
    return SUCCESS;
}

int printGameDetails(HDLGIInstance& ps2, const char* identifier)
{
    std::vector<HDLGameEntry> entries = ps2.getGamesInstalled();
    for(HDLGameEntry& entry : entries)
    {
        if(!(strncmp(entry.discID,identifier,DISCID_LENGTH) && strncmp(entry.title,identifier,GAME_TITLE_MAX_LEN_BYTES)))
        {
            GameParameters params = ps2.readGameParams(entry);
            std::cout << "Details for: " << entry.title << std::endl << std::endl;

            std::cout  <<  "Disc ID    : " << entry.discID << std::endl;
            std::cout  <<  "Partition  : " << entry.partition << std::endl;
            std::cout  <<  "Sectors    : " << entry.sectors << std::endl;
            std::cout  <<  "Disc Type  : " << (entry.discType == SCECdPS2DVD ? "DVD" : "CD") << std::endl;
            std::wcout << L"OSD Line 1 : " << params.osd1 << std::endl;
            std::wcout << L"OSD Line 2 : " << params.osd2 << std::endl;
            std::cout  <<  "MDMA0      : " << (params.useMDMA0 ? "ENABLED" : "DISABLED") << std::endl;
            std::cout << std::endl;
            printCompatFlags(entry.compatFlags);

            return SUCCESS;
        }
    }
    std::cerr << "No game was found matching title or discID: " << identifier << std::endl;
    return FAILURE;
}

int downloadGame(HDLGIInstance& ps2, const char* identifier, const char* file)
{
    FILE* f = fopen(file,"rb");
    if(f)
    {
        std::cerr << "File exists" << std::endl;
        fclose(f);
        return FAILURE;
    }
    f = fopen(file,"wb");
    if(!f)
    {
        std::cerr << "Invalid path specified" << std::endl;
        return FAILURE;
    }
    fclose(f);
    std::vector<HDLGameEntry> entries = ps2.getGamesInstalled();
    for(HDLGameEntry& entry : entries)
    {
        if(!(strncmp(entry.discID,identifier,DISCID_LENGTH) && strncmp(entry.title,identifier,GAME_TITLE_MAX_LEN_BYTES)))
        {
            return ps2.downloadGame(std::string(file),entry);
        }
    }
    std::cerr << "No game was found matching title or discID: " << identifier << std::endl;
    return FAILURE;
}

int installGame(HDLGIInstance& ps2, GameParameters& params, const char* file, bool overwrite)
{
    FILE* f = fopen(file,"rb");
    if(!f)
    {
        std::cerr << "Error opening file: " << strerror(errno) << std::endl;
        return FAILURE;
    }

    return ps2.installGame(file,params,overwrite);
}

int removeGame(HDLGIInstance& ps2, const char* identifier)
{
    std::vector<HDLGameEntry> entries = ps2.getGamesInstalled();
    for(HDLGameEntry& entry : entries)
    {
        if(!(strncmp(entry.discID,identifier,DISCID_LENGTH) && strncmp(entry.title,identifier,GAME_TITLE_MAX_LEN_BYTES)))
        {
            return ps2.deleteGame(entry);
        }
    }
    std::cerr << "No game was found matching title or discID: " << identifier << std::endl;
    return FAILURE;
}

int editGame(HDLGIInstance& ps2, GameParameters& params, bool updateIcon, const char* identifier)
{
    std::vector<HDLGameEntry> entries = ps2.getGamesInstalled();
    for(HDLGameEntry& entry : entries)
    {
        if(!(strncmp(entry.discID,identifier,DISCID_LENGTH) && strncmp(entry.title,identifier,GAME_TITLE_MAX_LEN_BYTES)))
        {
            return ps2.editGame(entry,params,updateIcon);
        }
    }
    std::cerr << "No game was found matching title or discID: " << identifier << std::endl;
    return FAILURE;
}

bool genParams(const char* action, std::map<const char*,const char*> arguments, GameParameters& params)
{
    std::string titleStr = "title";
    if(!strcmp(action,"edit")) {titleStr = "newtitle";}
    params.title = L"";
    params.disc_type = 0xFF;
    params.iconSrc = 0xFF;
    params.osd1 = L"";
    params.osd2 = L"";
    params.compatFlags = 0;
    params.useMDMA0 = false;
    for(const std::pair<const char*,const char*>& elem : arguments)
    {
        if(!strcmp(elem.first,titleStr.c_str())) {params.title = convUTF8FromBytes(elem.second);}
        else if(!strcmp(elem.first,"osd1")) {params.osd1 = convUTF8FromBytes(elem.second);}
        else if(!strcmp(elem.first,"osd2")) {params.osd2 = convUTF8FromBytes(elem.second);}
        else if(!strcmp(elem.first,"type"))
        {
            if(!strcmp(elem.second,"CD")) {params.disc_type = SCECdPS2CD;}
            else {params.disc_type = SCECdPS2DVD;}
        }
        else if(!strcmp(elem.first,"compat"))
        {
            std::stringstream sstream(elem.second);
            for(unsigned int val; sstream >> val;)
            {
                params.compatFlags |= (1 << (val-1));
                if (sstream.peek() == ',') {sstream.ignore();}
            }
        }
        else if(!strcmp(elem.first,"icon-src"))
        {
            if(!strcmp(elem.second,"gamesave")) {params.iconSrc = ICON_SRC_GAMESAVE;}
            else if(!strcmp(elem.second,"external")) {params.iconSrc = ICON_SRC_EXTERNAL;}
            else {params.iconSrc = ICON_SRC_DEFAULT;}
        }
        else if(!strcmp(elem.first,"icon-path")) {params.iconPath = elem.second;}
    }
    if(params.title == L"") {std::cerr << "--" << titleStr << " was not specified" << std::endl; return false;}
    if(params.disc_type == 0xFF) {std::cerr << "--type was not specified" << std::endl; return false;}
    if(params.iconSrc == 0xFF) {std::cerr << "--icon-src was not specified" << std::endl; return false;}
    return ((params.iconSrc == ICON_SRC_EXTERNAL && params.iconPath != "") || (params.iconSrc != ICON_SRC_EXTERNAL && params.iconPath == ""));
}

void printVersion(const char* argv)
{
    std::cout << argv << " " << VERSION << std::endl << std::endl;
}

void printHelp(const char* argv, bool longVersion = true)
{
    std::cout << argv << " ACTION <action arguments> -H <IP address> | [-h --help | -v --version]" << std::endl;
    std::cout << "Connects to a PlayStation 2 running HDLGameInstaller to install games and more." << std::endl << std::endl;

    if(longVersion)
    {
        printVersion(argv);
        std::cout << "Actions:" << std::endl;
        std::cout << " install --title <\"Game title\"> --osd1 <\"OSD Line 1\"> --osd2 <\"OSD Line 2\">" << std::endl;
        std::cout << "         --type <CD|DVD> --compat=\"1,2,3,4,5,6,8\" " << std::endl;
        std::cout << "         --icon-src <default|gamesave|external --icon-path <path-to-gamesave>>" << std::endl;
        std::cout << "         --use-mdma0 --overwrite --image <path-to-iso>" << std::endl;
        std::cout << "    Installs the provided game ISO image into the console" << std::endl << std::endl;

        std::cout << " edit --newtitle <\"Game title\"> --osd1 <\"OSD Line 1\"> --osd2 <\"OSD Line 2\">" << std::endl;
        std::cout << "      --type <CD|DVD> --compat=\"1,2,3,4,5,6,8\" " << std::endl;
        std::cout << "      --icon-src <default|gamesave|external --icon-path <path-to-gamesave>>" << std::endl;
        std::cout << "      --use-mdma0 (--title <Game title> | --discid <Disc ID>)" << std::endl;
        std::cout << "    Edits the specified game's settings" << std::endl << std::endl;

        std::cout << " download (--title <Game title> | --discid <Disc ID>) --output <output-image>" << std::endl;
        std::cout << "    Downloads the specified game from the console" << std::endl << std::endl;

        std::cout << " list" << std::endl;
        std::cout << "    List installed games and available space" << std::endl << std::endl;

        std::cout << " print (--title <Game title> | --discid <Disc ID>)" << std::endl;
        std::cout << "    Prints the specified game's settings" << std::endl << std::endl;

        std::cout << " remove (--title <Game title> | --discid <Disc ID>)" << std::endl;
        std::cout << "    Removes the specified game from the console" << std::endl << std::endl;

        std::cout << " shutdown" << std::endl;
        std::cout << "    Shuts the console down" << std::endl << std::endl;

        std::cout << "Global flags:" << std::endl;
        std::cout << " -H --host <Hostname | IP>" << std::endl;
        std::cout << "    Hostname or IP address of the console running HDLGameInstaller" << std::endl << std::endl;

        std::cout << " -h --help" << std::endl;
        std::cout << "    Print detailed help screen" << std::endl << std::endl;

        std::cout << " -v --version" << std::endl;
        std::cout << "    Prints version" << std::endl << std::endl;

        std::cout << "Compat modes:" << std::endl;
        std::cout << "Mode 1 - ALTERNATE_EE_CORE        " << std::endl;
        std::cout << "Mode 2 - ALTERNATE_READING_METHOD " << std::endl;
        std::cout << "Mode 3 - UNHOOK_SYSCALLS          " << std::endl;
        std::cout << "Mode 4 - DISABLE_PSS_VIDEOS       " << std::endl;
        std::cout << "Mode 5 - DISABLE_DVD9_SUPPORT     " << std::endl;
        std::cout << "Mode 6 - DISABLE_IGR              " << std::endl;
        //std::cout << "Mode 7 - UNUSED_COMPAT            " << std::endl;
        std::cout << "Mode 8 - HIDE_DEV9_MODULE         " << std::endl << std::endl;
    }
}

int main(int argc, char **argv)
{
    const char* arg = argv[0];
    const char* ip = nullptr;
    const char* file = nullptr;
    int result = SUCCESS;
    int longIndex = UNKNOWN;
    int c;

    if(argc < 2) {printHelp(argv[0],false); return 1;}

    const char* action = argv[1];
    std::map<const char*, const char*> arguments;

    char** argvIndex;
    int argNum;

    // Actionless options
    if(!strcmp(action,"-h")
       || !strcmp(action,"--help")
       || !strcmp(action,"-v")
       || !strcmp(action,"--version")) {argvIndex = argv;argNum = argc;}

    else if(!strcmp(action,"install")
       || !strcmp(action,"edit")
       || !strcmp(action,"download")
       || !strcmp(action,"list")
       || !strcmp(action,"print")
       || !strcmp(action,"remove")
       || !strcmp(action,"shutdown"))
    {
        if(argc < 3)
        {
            std::cerr << "Invalid number of arguments" << std::endl;
            printHelp(argv[0], false);
            return 1;
        }
        argvIndex = argv+1;
        argNum = argc-1;
    }
    else
    {
        std::cerr << "Unrecognised action" << std::endl;
        printHelp(argv[0],false);
        return 1;
    }

    bool updateIcon = false;
    bool mdma0 = false;
    bool overwrite = false;

    while ((c = getopt_long(argNum, argvIndex, "H:hv", longopts, &longIndex)) != -1)
    {
        if(c == 'H') {ip = optarg;}
        else if(c == 'v') {printVersion(argv[0]);return 0;}
        else if(c == 'h') {printHelp(argv[0],true);return 0;}
        else if(c == NEWTITLE) {arguments["newtitle"] = optarg;}
        else if(c == OSD1) {arguments["osd1"] = optarg;}
        else if(c == OSD2) {arguments["osd2"] = optarg;}
        else if(c == TYPE)
        {
            if (strcmp(optarg, "CD") && strcmp(optarg, "DVD"))
            {
                std::cerr << "Invalid option for --type" << std::endl;
                printHelp(argv[0], true);
                return 1;
            }
            arguments["type"] = optarg;
        }
        else if(c == COMPAT)
        {
            const std::regex compatRegex("([1234568]{1},)*[1234568]{1}$");

            if (!std::regex_match(optarg, compatRegex))
            {
                std::cerr << "Invalid option for --compat" << std::endl;
                printHelp(argv[0], true);
                return 1;
            }
            arguments["compat"] = optarg;
        }
        else if(c == ICONSRC)
        {
            if (strcmp(optarg, "default")
                && strcmp(optarg, "gamesave")
                && strcmp(optarg, "external"))
            {
                std::cerr << "Invalid option for --icon-src" << std::endl;
                printHelp(argv[0], true);
                return 1;
            }
            arguments["icon-src"] = optarg;
            updateIcon = true;
        }
        else if(c == ICONPATH) {arguments["icon-path"] = optarg;}
        else if(c == MDMA0) {mdma0 = true;}
        else if(c == TITLE) {arguments["title"] = optarg;}
        else if(c == IMAGE) {arguments["image"] = optarg;}
        else if(c == OUTPUT) {arguments["output"] = optarg;}
        else if(c == DISCID) {arguments["discid"] = optarg;}
        else if(c == OVERWRITE) {overwrite = true;}
        else
        {
            std::cerr << "Unrecognised argument: " << optarg << std::endl;
            printHelp(argv[0],false);
            return 1;
        }
    }
    if(optind != argNum)
    {
        std::cerr << "Unrecognised argument: " << argv[optind+1] << std::endl;
        printHelp(argv[0],false);
        return 1;
    }
    if(ip == nullptr)
    {
        std::cerr << "No host specified" << std::endl;
        printHelp(argv[0],false);
        return 1;
    }

    try
    {
        HDLGIInstance ps2(ip);
        size_t numParsed = arguments.size();

        if(!strcmp(action,"install"))
        {
            if(numParsed < 4)
            {
                std::cerr << "Insufficient number of arguments for action install" << std::endl;
                printHelp(argv[0],true);
                return 1;
            }
            if(arguments["image"] == nullptr)
            {
                std::cerr << "No image specified" << std::endl;
                printHelp(argv[0],true);
                return 1;
            }
            GameParameters params;
            if(!genParams(action,arguments,params))
            {
                std::cerr << "Invalid arguments specified for action edit" << std::endl;
                printHelp(argv[0],true);
                return 1;
            }
            params.useMDMA0 = mdma0;
            result = installGame(ps2,params,arguments["image"],overwrite);
            printError(result);
        }
        else if(!strcmp(action,"edit"))
        {
            if(numParsed < 3)
            {
                std::cerr << "Insufficient number of arguments for action edit" << std::endl;
                printHelp(argv[0],true);
                return 1;
            }
            char* identifierPtr = (char*)((ptrdiff_t)arguments["discid"] + (ptrdiff_t)arguments["title"]);
            if(!identifierPtr)
            {
                std::cerr << "No identifier specified" << std::endl;
                printHelp(argv[0],true);
                return 1;
            }
            std::string identifier = std::string(identifierPtr);

            GameParameters params;
            if(!genParams(action,arguments,params))
            {
                std::cerr << "Invalid arguments specified for action edit" << std::endl;
                printHelp(argv[0],true);
                return 1;
            }
            params.useMDMA0 = mdma0;
            result = editGame(ps2,params,updateIcon,identifier.c_str());
            printError(result);
        }
        else if(!strcmp(action,"download"))
        {
            if(numParsed != 2)
            {
                std::cerr << "Incorrect number of arguments for action download" << std::endl;
                printHelp(argv[0],true);
                return 1;
            }
            if(arguments["output"] == nullptr)
            {
                std::cerr << "No output file specified" << std::endl;
                printHelp(argv[0],true);
                return 1;
            }
            char* identifierPtr = (char*)((ptrdiff_t)arguments["discid"] + (ptrdiff_t)arguments["title"]);
            if(!identifierPtr)
            {
                std::cerr << "No identifier specified" << std::endl;
                printHelp(argv[0],true);
                return 1;
            }
            std::string identifier = std::string(identifierPtr);
            result = downloadGame(ps2,identifier.c_str(),arguments["output"]);
            printError(result);
        }
        else if(!strcmp(action,"list"))
        {
            if(numParsed != 0)
            {
                std::cerr << "Incorrect number of arguments for action list" << std::endl;
                printHelp(argv[0],true);
                return 1;
            }
            result = printGameList(ps2);
            printError(result);
        }
        else if(!strcmp(action,"print"))
        {
            if(numParsed != 1)
            {
                std::cerr << "Incorrect number of arguments for action print" << std::endl;
                printHelp(argv[0],true);
                return 1;
            }
            char* identifierPtr = (char*)((ptrdiff_t)arguments["discid"] + (ptrdiff_t)arguments["title"]);
            if(!identifierPtr)
            {
                std::cerr << "No identifier specified" << std::endl;
                printHelp(argv[0],true);
                return 1;
            }
            std::string identifier = std::string(identifierPtr);
            result = printGameDetails(ps2,identifier.c_str());
            printError(result);
        }
        else if(!strcmp(action,"remove"))
        {
            if(numParsed != 1)
            {
                std::cerr << "Incorrect number of arguments for action remove" << std::endl;
                printHelp(argv[0],true);
                return 1;
            }
            char* identifierPtr = (char*)((ptrdiff_t)arguments["discid"] + (ptrdiff_t)arguments["title"]);
            if(!identifierPtr)
            {
                std::cerr << "No identifier specified" << std::endl;
                printHelp(argv[0],true);
                return 1;
            }
            std::string identifier = std::string(identifierPtr);
            result = removeGame(ps2,identifier.c_str());
            printError(result);
        }
        else if(!strcmp(action,"shutdown"))
        {
            if(numParsed != 0)
            {
                std::cerr << "Incorrect number of arguments for action shutdown" << std::endl;
                printHelp(argv[0],true);
                return 1;
            }
            result = shutdownConsole(ps2);
            printError(result);
        }
    }
    catch(std::runtime_error& e)
    {
        std::cerr << e.what() << std::endl;
    }
    return result;
}
