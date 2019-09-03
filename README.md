# hdlgi-cli
UNIX version of sp193's HDL Game Installer tool based on CLI
Original Project: https://sites.google.com/view/ysai187/home/projects/hdlgameinstaller

Compiles in GNU/Linux and macOS (maybe on some other BSDs too)

## --help
```
./cmake-build-debug/hdlgi-cli ACTION <action arguments> -H <IP address> | [-h --help | -v --version]
Connects to a PlayStation 2 running HDLGameInstaller to install games and more.

./cmake-build-debug/hdlgi-cli 1.0

Actions:
 install --title <"Game title"> --osd1 <"OSD Line 1"> --osd2 <"OSD Line 2">
         --type <CD|DVD> --compat="1,2,3,4,5,6,8"
         --icon-src <default|gamesave|external --icon-path <path-to-gamesave>>
         --use-mdma0 --image <path-to-iso>
    Installs the provided game ISO image into the console

 edit --newtitle <"Game title"> --osd1 <"OSD Line 1"> --osd2 <"OSD Line 2">
      --type <CD|DVD> --compat="1,2,3,4,5,6,8"
      --icon-src <default|gamesave|external --icon-path <path-to-gamesave>>
      --use-mdma0 (--title <Game title> | --discid <Disc ID>)
    Edits the specified game's settings

 download (--title <Game title> | --discid <Disc ID>) --output <output-image>
    Downloads the specified game from the console

 list
    List installed games and available space

 print (--title <Game title> | --discid <Disc ID>)
    Prints the specified game's settings

 remove (--title <Game title> | --discid <Disc ID>)
    Removes the specified game from the console

 shutdown
    Shuts the console down

Global flags:
 -H --host <Hostname | IP>
    Hostname or IP address of the console running HDLGameInstaller

 -h --help
    Print detailed help screen

 -v --version
    Prints version

Compat modes:
Mode 1 - ALTERNATE_EE_CORE
Mode 2 - ALTERNATE_READING_METHOD
Mode 3 - UNHOOK_SYSCALLS
Mode 4 - DISABLE_PSS_VIDEOS
Mode 5 - DISABLE_DVD9_SUPPORT
Mode 6 - DISABLE_IGR
Mode 8 - HIDE_DEV9_MODULE
```
