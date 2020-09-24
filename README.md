# Environment Setup

## Prerequisites:

1. Git installed on your computer: 

    [Git - Installing Git](https://git-scm.com/book/en/v2/Getting-Started-Installing-Git)

2. VS Code (if using vscode—feel free to use your preferred IDE)

    [Visual Studio Code - Code Editing. Redefined](https://code.visualstudio.com/)

3. Mbed CLI (and its dependencies): 

    [Overview - Build tools | Mbed OS 6 Documentation](https://os.mbed.com/docs/mbed-os/v6.3/build-tools/index.html)

4. GNU Arm Embedded Toolchain (this is a dependency of mbed cli, but putting the download page here for convenience). Make sure to either put this on your path, or point mbed to the directory.

    [GNU Toolchain | GNU Arm Embedded Toolchain Downloads - Arm Developer](https://developer.arm.com/tools-and-software/open-source-software/developer-tools/gnu-toolchain/gnu-rm/downloads)

## Mbed Project Setup

Here we get up and running with the base project. This ensures we're all able to build the code, flash, and debug our development kits.

1. Clone this repo to a folder on your computer:

    ```bash
    git clone https://gitlab.com/ka-moamoa/smart-ppe/embedded-firmware.git
    ```

2. Shift terminal focus to the `embedded-firmware` directory:

    ```bash
    cd embedded-firmware/
    ```

3. Initialize the mbed repo. Mbed projects store the version of mbed they're using in the `mbed-os.lib` folder (actually just a url containing a commit hash). This is so that you don't have to upload/download the entire mbed library whenever you pull from/push to the repo. The following command finds that url and clones the repository to a new folder `mbed-os`. 

    ```bash
    mbed deploy
    ```

4. I'm not sure if this step is required for Windows computer (or other Mac users, for that matter), but the first time I set this up I had to cd into the `mbed-os` directory and run `pip install -r requirements.txt --user`. I think the `--user` flag should only apply to Macs. `cd ..` back out into the root of the project if you do this.
5. Now you should be able to run `mbed compile -t GCC_ARM -m NRF52_DK` and watch as your computer compiles the code!

## Visual Studio Code Setup

1. IF you're using VS Code, first open up the folder `embedded firmware` in VS Code. On Windows you can do this by right clicking in the folder and selecting "Open in VS Code" (if you enabled that option) and on Mac you can do it by dragging the folder to the icon. There are other ways to do this too (like typing `code .` in your terminal program), but you may have to set them up. Read the docs if interested.
2. Then hit `F1` to bring up the command palette. Start typing `configure default build task`, then select it from the list. It will give you some default options, hit `other`. Paste this into the `tasks.json` file that pops up:

    ```bash
    {
        // See https://go.microsoft.com/fwlink/?LinkId=733558
        // for the documentation about the tasks.json format
        "version": "2.0.0",
        "tasks": [
            {
                "label": "build",
                "type": "shell",
                "command": "mbed compile -t GCC_ARM -m NRF52_DK",
                "problemMatcher": "$gcc",
                "group": {
                    "kind": "build",
                    "isDefault": true
                }
            }
        ]
    }
    ```

3. Now you can build right in vscode. You can kick it off by pressing `F1` then `Tasks: Run Build Task`, or you can kick it off with a key command (I think the default it cmd-shift-B on mac and ctrl-shift-b on windows, but you can change this). Key commands are the way to go.

## Flashing The Board

Work in progress 🚧... For now you can flash the board by dragging `BUILD/embedded-firmware.elf` onto the drive associated with the board that shows up in your finder/explorer.

## Debugging The Board

Work in progress 🚧...

# Mbed Instructions (for posterity):

# BLE Gatt Server example

This application demonstrates detailed uses of the GattServer APIs.

It starts by advertising to its environment with the device name "GattServer". Once you connect to the device with a BLE scanner on your phone, the scanner shows a service with three characteristics - each representing the hour, minute and second of a clock.

To see the clock values updating subscribe to the service using the "Enable CCCDs" (or similar) option provided by the scanner. Now the values get updated once a second.

# Running the application

## Requirements

You may use any BLE scanner, for example:

- [nRF Master Control Panel](https://play.google.com/store/apps/details?id=no.nordicsemi.android.mcp) for Android.

- [LightBlue](https://itunes.apple.com/gb/app/lightblue-bluetooth-low-energy/id557428110?mt=8) for iPhone.

Hardware requirements are in the [main readme](https://github.com/ARMmbed/mbed-os-example-ble/blob/master/README.md).

## Building instructions

Building instructions for all samples are in the [main readme](https://github.com/ARMmbed/mbed-os-example-ble/blob/master/README.md).

