# psmove-ue4

Plugin for using PSMove as input into Unreal Engine 4.

# Notes

This plugin is in the very early stages of development. It works in OS X and Windows 64-bit (I use 8.1 Pro). Before you even bother trying to use this plugin you should make sure you can get the [psmoveapi](https://github.com/cboulay/psmoveapi)'s test_tracker application working for you.

Working features:

- Position and orientation of a single controller.

Planned features:

- Multiple controllers (currently only uses last controller)
- Detect button presses —> events
- Controlling vibration
- Zeroing position and orientation
- Co-registration with HMD

Maybe someday features:

- Hot-plugging (camera must be plugged in and controller turned on when module loaded, i.e. when launching editor)
- Controlling LEDs (psmove_tracker controls LED colours)
- Other camera types

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

Make sure your camera is plugged in and the psmove controller is connected and in view of the camera before you launch the editor. If you are using Windows then getting the controller connected and the camera working requires a little work. Be sure to read the [psmoveapi README.win64](https://github.com/cboulay/psmoveapi/blob/master/README.win64) notes 8 and 9.

Open the editor. In your game, open the Actor you want to have PSMove input. Add the PSMove component to the actor. This actor component will automatically update its variables Position (FVector, in cm) and Rotator (FRot) from the PSMove on each tick. You can access these variables off the PSMove component in blueprints and use them to Set Actor Location and Rotation.