# Supercell Coding Test
Hi! This is the base code structure for the Supercell coding test.
The code is basic and plays poorly, so we need YOU to make it better.

## Theme

This year's theme is "Endless Runner". The starter code gives you a basic, generic infinite runner game.

## Disclaimer - MANDATORY RULES

We strongly recommend that you:
- Read this README to the end.
- Keep to the theme.
- Place assets in the `/assets` folder and use the `ResourceManager` class to get file paths.
    - Example usage: `ResourceManager::getOrLoadFont("Lavigne.ttf")`
    - Look in `/src/gamestates/StateMenu.cpp` for examples of loading fonts and textures.
- DO NOT MODIFY the ResourceManager. We might run your game on a different platform than what you developed on.
- Avoid modifying the CMakeLists.txt file. We will use Cmake to build your project.
- (optional) Test on different platforms if you can, to assure your submission builds and functions the same.

We will build your project with the build steps outlined below. If your submission produces any build errors we will most likely end the evaluation there.

## Prerequisites

NOTE: Hive computers (should) have these dependencies installed already.

You must have [CMake](https://cmake.org/) version 3.22.1 or later installed to build this project.

If you are on Linux, make sure to install SFML's dependencies. For Debian-based distros:
```
sudo apt update
sudo apt install \
    libxrandr-dev \
    libxcursor-dev \
    libudev-dev \
    libfreetype-dev \
    libopenal-dev \
    libflac-dev \
    libvorbis-dev \
    libgl1-mesa-dev \
    libegl1-mesa-dev
```

## Building

Simply run `cmake -B build` and `cmake --build build`. This builds the executable in `./build/bin/`.

The exact location of the executable may differ depending on your system and compiler. Example locations are `./build/bin/shooter` and `./build/bin/Debug/shooter.exe`.

Alternatively, you can specify a generator suited for your toolchain.
- `cmake -B build -G"Unix Makefiles"`
- `cmake -B build -G"Visual Studio 17 2022"`
- `cmake -B build -GXcode`

Run `cmake -G` to list all available generators.

## Submission

Upload your work to Google Drive, Dropbox (or some other service), and complete the provided form with:
- A link to your submission (remember to check permissions)
- Your full name
- Your email address

[**SUBMIT**](https://supr.cl/hive2026submit)

## Your Game's Instructions

Game: "Runes of Fire"

You play as the Sorceress. Your goal is to defeat the final Boss.

Objective
- Defeat enemies to collect Orbs. Each defeated enemy drops an Orb.
- When you have 10 Orbs, you can invoke the Boss. Defeat the Boss to win.

Controls
- Move: Left / Right arrows or A / D
- Jump: Up arrow or W
- Charge Fireball: Hold Space
- Release Fireball: Release Space (damage and speed scale with charge)
- Invoke Boss: Press F when you have 10/10 Orbs

Gameplay notes
- Enemies move from right to left. Shoot them with charged Fireballs to kill them and collect Orbs.
- The Fireball's damage and speed increase with how long you hold Space. Release to fire.
- When you have 10 Orbs a prompt appears. Press F to start a short countdown and summon the Boss.

HUD and feedback
- The top-center HUD shows "Orbs: X/10". When it reaches 10/10 a prompt appears to invoke the Boss.
- On Boss defeat the game shows a large "YOU WIN!" message and returns to the menu after a short delay.

Tips
- Charge your shot for tougher enemies and the Boss — higher charge deals more damage and travels faster.
- Keep moving: enemies approach from the right and faster enemies will appear as you progress.
- Watch the Boss' hit area (it may be slightly smaller than the sprite); tune your shots accordingly.




## Troubleshooting

### MacOS
Your game may not be receiving keyboard inputs. If you see "Detected a keyboard without any keys" printed to the terminal. You will need to treat the *launcher* as the app that is "monitoring input"
- Open System Settings -> Privacy & Security -> Input Monitoring
    - If you run from Terminal or iTerm2: enable it for that app.
    - If you run from Xcode or Visual Studio Code: enable it for that app.
- Quit and reopen the launcher, then run your game again.
