# Loadiine GX2
[![Build Status](https://travis-ci.org/dimok789/loadiine_gx2.svg?branch=master)](https://travis-ci.org/dimok789/loadiine_gx2)
---
[Current Stable Release](https://github.com/dimok789/loadiine_gx2/releases/tag/v0.2) | [Nightly builds](https://github.com/dimok789/loadiine_gx2/releases) | [Issue Tracker](https://github.com/dimok789/loadiine_gx2/issues) | [Support Thread](https://gbatemp.net/threads/loadiine-gx2.413823/)

### What is Loadiine GX2? ###
Loadiine is a WiiU homebrew. It launches WiiU game backups from SD card.
Its Graphical User Interface is based on the WiiU GX2 graphics engine.

### How do I use this? ###
All information about how this works and what is required to do for it to work is written on the following [support thread](https://gbatemp.net/threads/loadiine-gx2.413823/).

### How to build? ###
#### Main Application ####
To build the main application devkitPPC is required as well as some additionally libraries. Download the libogc and portlibs packages from the release pages and put them in your devkitPro path. Replace any existing files. If not yet done export the path of devkitPPC and devkitPRO to the evironment variables DEVKITPRO and DEVKITPPC.
All remaining is to enter the main application path and enter "make". You should get a loadiine_gx2.elf and a loadiine_gx2_dbg.elf in the main path.

#### Installer Application ####
To compile the installer application enter the "installer" path on the source code and type "make".

### Credits ###
* Dimok
* Cyan
* Maschell
* n1ghty
* dibas
* The anonymous graphics dude (he knows who is ment)
* and several more contributers


---