# SouthParkFrameWork

![South Park Banner](https://etgeekera.com/wp-content/uploads/2013/05/south-park-the-stick-of-truth-banner.jpg)

SouthParkFrameWork is a lightweight modding, research, and experimentation framework for South Park: The Stick of Truth.

The goal of this project is to provide a clean and expandable foundation for:

* DLL modding
* Runtime hooking
* Overlay systems
* Debug logging
* File patching
* Research tooling
* Reverse engineering
* Experimental gameplay modifications

Built for developers, reverse engineers, preservation enthusiasts, and modders who want to learn more about how the game works internally while creating new tools and gameplay experiences.

---

# Features

## Core Framework

* Proxy `steam_api.dll` loader
* Expandable framework architecture
* Runtime initialization system
* Dynamic module loading
* MinHook integration
* x86-compatible design

## Overlay System

* DirectX9 overlay renderer
* Draggable in-game menu
* F1 toggle menu
* Mouse interaction support
* Notification system
* Real-time status information
* Configurable overlay settings

## Modding Support

* DLL mod loader
* Runtime DLL loading/unloading
* Per-mod enable/disable system
* Config-driven mod states
* Reload mods without restarting framework

## File Patch System

* Virtual file replacement system
* Non-destructive patch loading
* Patch enable/disable support
* Automatic patch detection
* Runtime patch refresh support

Example:

```txt
SouthParkFrameWork/
├─ Mods/
│  └─ ExampleMod.dll
└─ Patches/
   └─ SkipIntro/
      └─ data/movies/
```

No original game files are replaced or modified.

## Debugging / Research

* Debug logging system
* Runtime patch manager
* D3D9 hook system
* Experimental rendering tools
* Internal game research support
* Overlay diagnostics
* Hook initialization tracking

---

# Current Systems

* DirectX9 renderer hooks
* In-game overlay framework
* Runtime notification system
* Config system (`config.ini`)
* DLL mod loader
* Virtual file system
* Patch manager
* Drag-and-drop overlay window
* Mouse-enabled UI interaction
* Borderless window support
* Runtime toggle system

---

# Planned Features

* ImGui-style UI framework
* Dockable overlay windows
* In-game developer console
* Pattern scanner
* Signature database
* Entity viewer
* Free camera
* Texture viewer
* Shader tools
* Script research tools
* Runtime memory viewer
* Scaleform research/debugging
* Live patch editor
* Hot-reload systems
* Multiplayer experimentation
* Lua/Python scripting support
* Universal DX9 framework support

---

# Requirements

* Windows 10 / 11
* Visual Studio
* DirectX 9 SDK (June 2010)
* x86 Build Configuration
* Legally owned copy of the game

---

# Disclaimer

Yes, AI was used to assist with portions of this project. If that bothers you, you are free not to use it.

This framework uses a proxy DLL system. In simple terms, it loads through a custom `steam_api.dll` which forwards to the original game DLL. Because of this:

A legitimate copy of South Park: The Stick of Truth is required.

THIS WILL NEVER CHANGE.

This project exists strictly for:

* Research
* Learning
* Preservation
* Modding
* Reverse engineering experimentation

No copyrighted game assets, proprietary code, or original game content are included or distributed with this project.

WE DO NOT CONDONE PIRACY.

We deeply respect the original developers, publishers, artists, writers, engineers, and everyone involved in creating South Park: The Stick of Truth and the South Park franchise as a whole.

This framework exists because of appreciation for the game and the technology behind it.

If any rights holder or intellectual property owner wishes to contact us regarding content concerns, removal requests, or takedown requests, we will respectfully comply.

---

# Project Structure

```txt
SouthParkFrameWork/
├─ Framework.dll
├─ Logger.dll
├─ SteamOnline.dll
├─ steam_api.dll
├─ config.ini
├─ Mods/
│  └─ *.dll
├─ Patches/
│  └─ PatchName/
│     └─ game files
└─ Logs/
```

---

# Status

Work in progress.

SouthParkFrameWork is actively evolving into a full modding and research toolkit for South Park: The Stick of Truth.
