# psmove-ue4

Plugin for using PSMove as input into Unreal Engine 4.

# Notes

This plugin works in OS X and Windows 64-bit (I use 8.1 Pro).

Working features:

- Position and orientation of multiple controllers.
- Button presses trigger events.
- Trigger-button value (0-255)
- Set vibration (0-255)

Planned features:

- Use TThreadSafeSharedPtr
- Zeroing position and orientation
- Co-registration with HMD

Maybe someday features:

- Optional 320x240 @ 140 Hz positional tracking
- Hot-plugging
- Controlling LEDs
- Other camera types

# Setup

Plug your PS Eye camera into a USB port.

## OSX

In the `Binaries\Mac` folder you'll find some tools.
Plug your PSMove controller in to the computer via USB then run `psmovepair` to pair your PSMove controller to your computer.
Then disconnect the USB cable and press the PS button on the controller to connect to the computer.
Run `test_tracker` to test that the camera and controller are working properly.
Run `magnetometer_calibration` to calibrate the magnetometer.

In the future, you will only have to press the PS button to connect.

## Windows

### Camera

First, open Control Panel and uninstall any driver you may have associated with the camera (the audio device can be left alone).
Then, use [Zadig](http://zadig.akeo.ie/) to install the libusb-win32 (libusb0.sys) version of the driver for the camera.
(You may have to reboot at this point)
Double click on ps3eye.reg to add a registry entry for it (I hope to get rid of this in the future).

### Controlller

Getting the PSMove controller to pair and connect on Windows has traditionally been very difficult.
There has been a lot of progress recently in improving this process, but it is not yet streamlined.
In this repository's `Binaries\Win64` folder you will find some tools to help.

Plug the controller into the computer via USB.
First, run `psmovepair.exe` and follow the on-screen instructions. Chances are it will pair, but it won't connect.
Then, run `psmove-pair-win.exe` to pair yet again, only this time it'll add some additional settings that help with connecting.
If you're still having trouble, try reading [this link](https://github.com/cboulay/psmoveapi/blob/master/WINDOWS_EXTRA)

Once paired and connected, run `test_tracker` to verify that the camera and controller are working properly.
Then run `magnetometer_calibration` to calibrate the magnetometer.

# Install

Change to your project directory (must be a project with C++ code).

```
mkdir Plugins
cd Plugins
git clone https://github.com/cboulay/psmove-ue4.git
mv psmove-ue4 PSMovePlugin
```

In Windows command prompt, replace 'mv' with 'rename'.

Then refresh your code (in Windows, right click on .uproject; in Mac, use File>Refresh XCode Project from within editor) and build your project.

# Use

For example, from the default blank project:

* Open the editor.
* Move the player start to -100, 0, 0
* Move the floor to 0, 0, -100 
* Create a cube
    * Scale it to about 0.1, 0.1, 0.3
    * Move the cube to 0, 0, 0
    * Make the cube movable
* To the cube, add a PSMove component.
    * At this point, you can specify the PSMove ID (controller number), or you can do it from Blueprints later.
* When the Cube is selected, click on the Blueprint/Add Script button.
    * At this point, the camera will turn on and the controller will flash to set its brightness.
    * Make sure the controller is on and in view of the camera. I find it works best from about 2 m away.
* Edit the blueprint as in the image below
    * On the left of the blueprint editor, click on the PSMove component to select it.
    * On the right of the blueprint editor, click on the + next to the event you want to use, e.g., On Data Updated event.
    * Create a node for Set Actor Location and Rotation
    * Connect the event Location to the Actor Location, and the event Rotation to the Actor Rotation. Also wire up the execution points.
* Compile, save, Play!

![BP example](https://github.com/cboulay/psmove-ue4/blob/master/bp.png)