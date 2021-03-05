# Environment Setup

## Prerequisites:

1. Git installed on your computer: 

    [Git - Installing Git](https://git-scm.com/book/en/v2/Getting-Started-Installing-Git)

2. VS Code (if using vscode—feel free to use your preferred IDE)

    [Visual Studio Code - Code Editing. Redefined](https://code.visualstudio.com/)

3. Mbed CLI (and its dependencies): 

    [Overview - Build tools | Mbed OS 6 Documentation](https://os.mbed.com/docs/mbed-os/v6.3/build-tools/index.html)

4. GNU Arm Embedded Toolchain (this is a dependency of mbed cli, but putting the download page here for convenience). Make sure to either put this on your path, or point mbed to the directory. You'll need this path later on as well.

    [GNU Toolchain | GNU Arm Embedded Toolchain Downloads - Arm Developer](https://developer.arm.com/tools-and-software/open-source-software/developer-tools/gnu-toolchain/gnu-rm/downloads)

5. [JLink Software and Documentation Pack](https://www.segger.com/downloads/jlink/#J-LinkSoftwareAndDocumentationPack). Keep track of where this is installed, you'll need the path later on.

6. Required Hardware:
    - Smart PPE board
    - JLINK debugger (e.g. [J-Link EDU Mini](https://www.adafruit.com/product/3571?gclid=EAIaIQobChMIqeXO9fvS7QIVEI3ICh3I5g0TEAQYAiABEgJDwvD_BwE), which we have a lot of in the lab). Note that you can also use a Nordic (or I think even ST) development kit in a pinch, but the dedicated J-Link will be faster.
    - Micro USB Cable
    - 7.9mm (W) x 5.4mm (H) Silver Oxide Battery (or dedicated power supply)

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

5. Go to the [bluetooth features snippets page](https://gitlab.com/ka-moamoa/smart-ppe/embedded-firmware/-/snippets/2069498), and copy-paste the contents over the contents of the file at this location: `embedded-firmware/mbed-os/connectivity/FEATURE_BLE/mbed_lib.json`. Eventually we may want to just make our own fork of the mbed repo, but for now we're doing it this way :)

6. Now you should be able to run `mbed compile -t GCC_ARM -m SMARTPPE` and watch as your computer compiles the code!

## Visual Studio Code Setup

1. If you're using VS Code, first open up the folder `embedded firmware` in VS Code. On Windows you can do this by right clicking in the folder and selecting "Open in VS Code" (if you enabled that option) and on Mac you can do it by dragging the folder to the icon. There are other ways to do this too (like typing `code .` in your terminal program), but you may have to set them up. Read the docs if interested.
2. Then hit `F1` to bring up the command palette. Start typing `configure default build task`, then select it from the list. It will give you some default options, hit `other`. Paste this into the `tasks.json` file that pops up:

```json
{
    // See https://go.microsoft.com/fwlink/?LinkId=733558
    // for the documentation about the tasks.json format
    "version": "2.0.0",
    "tasks": 
    [
        {
            "label": "build-debug",
            "type": "shell",
            "command": "mbed compile -t GCC_ARM -m SMARTPPE --profile debug",
            "problemMatcher": "$gcc",
            "group": {
                "kind": "build",
                "isDefault": true
            }
        },
        {
            "label": "build-develop",
            "type": "shell",
            "command": "mbed compile -t GCC_ARM -m SMARTPPE --profile develop",
            "problemMatcher": "$gcc",
            "group": "build"
        }
    ]
}
```

3. Now you can build right in vscode. You can kick it off by pressing `F1` then `Tasks: Run Build Task`, or you can kick it off with a key command (I think the default is cmd-shift-B on mac and ctrl-shift-b on windows, but you can change this). Key commands are the way to go IMO.

## Debugging The Board (VSCode)

Hit the "Run" button on your sidebar  
![](.readme-images/2020-09-25-15-29-00.png)  
and select "create a launch.json file". Choose any of the options (you'll delete the default configuration it adds).

Copy/paste this over the launch.json file that is generated:

```json
{
    "version": "0.2.0",
    "configurations": 
    [
        {
            "name": "debug",
            "type": "cortex-debug",
            "request": "launch",
            "servertype": "jlink",
            "serverpath": "/Applications/SEGGER/JLink_V692/JLinkGDBServerCLExe",
            "interface": "swd",
            "cwd": "${workspaceRoot}",
            "runToMain": true,
            "executable": "${workspaceRoot}/BUILD/SMARTPPE/GCC_ARM-DEBUG/${workspaceRootFolderName}.elf",
            "device": "nrf52",
            "svdFile": "${workspaceRoot}/nrf52.svd",
            "armToolchainPath": "/Applications/ARM/bin/",
            "swoConfig": {
                "enabled": true,
                "cpuFrequency": 64000000,
                "swoFrequency": 40000000,
                "source": "probe",
                "decoders": [
                    {
                        "type": "console",
                        "label": "SMART-PPE",
                        "port": 0
                    }
                ]
            }
        },
        {
            "name": "debug-develop",
            "type": "cortex-debug",
            "request": "launch",
            "servertype": "jlink",
            "serverpath": "/Applications/SEGGER/JLink_V692/JLinkGDBServerCLExe",
            "interface": "swd",
            "cwd": "${workspaceRoot}",
            "runToMain": true,
            "executable": "${workspaceRoot}/BUILD/SMARTPPE/GCC_ARM-DEVELOP/${workspaceRootFolderName}.elf",
            "device": "nrf52",
            "svdFile": "${workspaceRoot}/nrf52.svd",
            "armToolchainPath": "/Applications/ARM/bin/",
            "swoConfig": {
                "enabled": true,
                "cpuFrequency": 64000000,
                "swoFrequency": 40000000,
                "source": "probe",
                "decoders": [
                    {
                        "type": "console",
                        "label": "SMART-PPE",
                        "port": 0
                    }
                ]
            }
        }
    ]
}
```

Again, you may have to adjust some file paths. Particularly `serverpath`, `executable`, and `armToolchainPath`. 

Save, and now you should be able to debug your board from within VS Code by hitting the "run" button, and clicking the green arrow at the top of the panel. (Or, as with all things, you can set up a keyboard shortcut to do this, by default it's F5 on mac).

To get SWO output, after VSCode has downloaded the code to your board and is waiting for you to hit "start", go to `Output` in the bottom pane, and where it says `Tasks` in the upper right, switch to `SWO: SMART-PPE`
![](.readme-images/SelectSWO.png) 

## Flashing and Debugging The Board with Ozone

Sometimes VSCode's debugging is buggy or doesn't work for something specific. When that happens I usually code and build in VSCode, and then switch to Ozone when it's time to debug. Thankfully, Ozone makes this process relatively painless. Here's how to set it up:

1. Plug in the J-Link (or dev kit) to your computer via USB, connect the ribbon cable to the smart ppe board's debug header. The red wire on the ribbon should be on the side of the J-Link marked with a (1). ![](.readme-images/j-link.jpeg)
2. Download [Ozone](https://www.segger.com/products/development-tools/ozone-j-link-debugger/#download-installation)
3. Open it up, select `Create New Project`
4. Use the following Target Device settings: 
    - Device: `nRF52832_xxAA`
    - Register Set: `Cortex-M4 (with FPU)`
    - Peripherals: I placed a file `nrf52.svd` in the repository. Put in your system's pathname to this file. (e.g. `/Users/alex/google-drive/hester-lab/Projects/smart ppe/embedded-firmware/nrf52.svd`)
5. Hit next, then use these Connection Settings:
    - Target Interface: `SWD`
    - Target Interface Speed: `8 MHz`
    - Host Interface: `USB`
    - Serial No: (this should auto-populate with your connected j-link device's serial number, leave it as is)
6. Hit next, and enter your systems's pathname to the .elf file. From the base directory of the repo, the path is: `embedded-firmware/BUILD/SMARTPPE/GCC_ARM-DEBUG/embedded-firmware.elf` Note that this only exists if you have already successfully built the software from VSCode!
7. Hit Finish, you should see `main.cpp` up on the screen. 
8. Hit the green power button in the upper left corner of the screen. If your smart PPE board is powered, Ozone should connect and download the program to it. 
9. One more step before you run the program. In the menu bar, select Tools->Trace Settings. Under Trace Source, select `SWO`. Hit `Save to Project`. This enables SWO printf, which is what we're using to log messages from the board. Any SWO messages will show up in the "Terminal" window. I think it shows by default, but if not you can see it by selecting View->Terminal from the menu bar.
10. You should save this debug configuration to your project folder by hitting File->Save Project As. The .gitignore should ignore any ozone files so feel free to name it whatever you want. 
11. Now you can debug as usual. Whenever you rebuild the code, Ozone will automatically detect that the .elf file has changed and, when you switch focus back to Ozone, will ask if you want to reload it. I usually select the `Do not show again` checkbox and hit `Yes`. This way, Ozone will automatically flash and reset the board when it detects a change to the .elf file.
12. Random tips:
    - You can find a source file by clicking into the `Source Files` window and begin typing the name of the source file. It will search for and show the file. Handy if you want to set breakpoints in a given file.
    - Same goes for the `Registers` window. Expand the bank you're interested in (likely Peripherals with 1708 registers, which we get from the nrf52.svd file), and then begin typing the name of the register you're interested in. 

