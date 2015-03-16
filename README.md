# psmove-ue4

Plugin for using PSMove as input into Unreal Engine 4.

# Notes

This plugin is in the very early stages of development. It works in OS X and Windows 64-bit (I use 8.1 Pro).
Before you even bother trying to use this plugin you should make sure you can get the [psmoveapi](https://github.com/cboulay/psmoveapi)'s test_tracker application working for you.
The binaries for psmovepair, magnetometer_calibration, and test_tracker for OSX are provided.
I will supply the Windows binaries for those soon, or you can build them yourself from the psmoveapi.

Working features:

- Position and orientation of multiple controllers.

Planned features:

- Detect button presses —> events
- Controlling vibration
- Zeroing position and orientation
- Co-registration with HMD

Maybe someday features:

- Optional 320x240 @ 140 Hz positional tracking
- Hot-plugging
- Controlling LEDs
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

In Windows, copy psmoveapi.dll and psmoveapi_tracker.dll from `PSMovePlugin\ThirdParty\psmoveapi\lib\Win64` into the project's Binaries folder. e.g., `MyGame\Binaries\Win64`

Then refresh your code (in Windows, right click on .uproject; in Mac, use File>Refresh XCode Project from within editor) and build your project.

If you are using Windows then getting the controller connected and the camera working requires a little work.
Be sure to read the [psmoveapi WINDOWS_EXTRA](https://github.com/cboulay/psmoveapi/blob/master/WINDOWS_EXTRA) notes.

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
    * Click on the PSMove component on the left then on the right click on the + next to On Data Updated event.
    * Next to that, drag the PSMove component into the graph.
    * From the PSMove component, create nodes for get Position and get Rotation
    * Create a node for Set Actor Location and Rotation
* Compile, save, Play!

![BP example](https://github.com/cboulay/psmove-ue4/blob/master/bp.png)