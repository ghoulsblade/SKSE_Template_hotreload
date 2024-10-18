> 📜 other templates available at https://github.com/SkyrimScripting/SKSE_Templates

# SKSE GHotReload

C++ SKSE plugin example for Skyrim
- it shows c++23 .ixx modules so you dont need to make a separate header file for every class
- example for "hot reload", see below
- also has some disabled example code how to load IBodyMorphInterface from skee/racemenu in the loader and pass it to the main

# What does it do?

After running Skyrim, once at the Main Menu, press the `~` key to open the game console.
- Pressing F7 should print something from this plugin, like "OnLoadPlugin bReload=true"

- "Hot reload": Tab out, change myprint("OnLoadPlugin..") in plugin_main and recompile, tab back to skyrim and press F7
- you should now see it being reloaded and printing the changed text. Without having to restart skyrim.
- Useful when working on a big modlist to avoid load times / quick turnaround.

- It also shows how to get a TESNPC pointer by the crosshair event and storing it as formid + looking it up when you press F1.

- Logging to file too, see youtube skyrimscripting logging

# hot reload

"hot reload" -> we can recompile + reload most of our code with a hotkey (F7) without restarting the game

- it split into two dlls :
- GHotReload_loader.dll = the "skse plugin", cannot be changed while the game is running
- sub/GHotReload.dll  = our main code, we can reload this with f7

- when you press f7, the GHotReload.dll is copied to GHotReload_tmp.dll and loaded.
- OnLoadPlugin is called at the start.
- OnUnLoadPlugin is called when reloading so the old version can clean itself up.

- copy_dll.bat is a utility script that to overwrite the GHotReload_loader.dll , but does not throw a build error when it cannot, - like when the game is currently running.
- to update GHotReload_loader.dll : close the game and rebuild (f7 in vscode), that will run copy_dll.bat

- some things like registering listeners can only be done in the loader at game start, so there are a few more functions that pass events like crosshair and hotkeys to the main.
- Also gregistry can be used to store some data between reloads.

# CommonLibSSE NG

Because this uses [CommonLibSSE NG](https://github.com/CharmedBaryon/CommonLibSSE-NG), it supports Skyrim SE, AE, GOG, and VR.

[CommonLibSSE NG](https://github.com/CharmedBaryon/CommonLibSSE-NG) is a fork of the popular [powerof3 fork](https://github.com/powerof3/CommonLibSSE) of the _original_ `CommonLibSSE` library created by [Ryan McKenzie](https://github.com/Ryan-rsm-McKenzie) in [2018](https://github.com/Ryan-rsm-McKenzie/CommonLibSSE/commit/224773c424bdb8e36c761810cdff0fcfefda5f4a).

# Requirements

- [Visual Studio 2022](https://visualstudio.microsoft.com/) (_the free Community edition_)
- [`vcpkg`](https://github.com/microsoft/vcpkg)
  - 1. Clone the repository using git OR [download it as a .zip](https://github.com/microsoft/vcpkg/archive/refs/heads/master.zip)
  - 2. Go into the `vcpkg` folder and double-click on `bootstrap-vcpkg.bat`
  - 3. Edit your system or user Environment Variables and add a new one:
    - Name: `VCPKG_ROOT`  
      Value: `C:\path\to\wherever\your\vcpkg\folder\is`

## Opening the project

Once you have Visual Studio 2022 installed, you can open this folder in basically any C++ editor, e.g. [VS Code](https://code.visualstudio.com/) 
- > _for VS Code, if you are not automatically prompted to install the [C++](https://marketplace.visualstudio.com/items?itemName=ms-vscode.cpptools) and [CMake Tools](https://marketplace.visualstudio.com/items?itemName=ms-vscode.cmake-tools) extensions, please install those and then close VS Code and then open this project as a folder in VS Code_

You may need to click `OK` on a few windows, but the project should automatically run CMake!

It will _automatically_ download [CommonLibSSE NG](https://github.com/CharmedBaryon/CommonLibSSE-NG) and everything you need to get started making your new plugin!

# Project setup

By default, when this project compiles it will output a `.dll` for your SKSE plugin into the `build/` folder.

If you want to configure this project to output your plugin files
into your "`mods`" folder:  
(_for Mod Organizer 2_)

- Set the `SKYRIM_MODS_FOLDER` environment variable to the path of your mods folder:  
  e.g. `C:\game\SkyrimStuff\MyModList\mods`  

Reboot your PC after changing environment variables.

# commonlibsse-ng update

vcpkg-configuration.json
Update the baseline to the latest commit from the above repo. 6309841a.. = 2023-05-13 = latest on colored glass as of 2024-10
see https://github.com/CharmedBaryon/CommonLibSSE-NG?tab=readme-ov-file#use

# tweaks

- to enable searching commonlib HEADERS (not source) but exclude most other build/ stuff :
- copy .vscode/settings.json.dist to .vscode/settings.json

# misc

- [MIT License](https://choosealicense.com/licenses/mit/), a permissive license which is used by many popular Skyrim mods
- project setup derived from mrowrpurr's "Hello World" template : https://github.com/SkyrimScripting/SKSE_Template_HelloWorld
