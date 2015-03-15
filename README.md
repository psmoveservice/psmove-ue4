# psmove-ue4

Plugin for using PSMove as input into Unreal Engine 4.

# Notes

This plugin is in the very early stages of development. It works in OS X and Windows 64-bit (I use 8.1 Pro).
Before you even bother trying to use this plugin you should make sure you can get the [psmoveapi](https://github.com/cboulay/psmoveapi)'s test_tracker application working for you.
The binaries for psmovepair, magnetometer_calibration, and test_tracker for OSX are provided.
I will supply the Windows binaries for those soon.

Working features:

- Position and orientation of multiple controllers.

Planned features:

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

Make sure your camera is plugged in and the psmove controller is connected and in view of the camera before you launch the editor.
If you are using Windows then getting the controller connected and the camera working requires a little work.
Be sure to read the [psmoveapi WINDOWS_EXTRA](https://github.com/cboulay/psmoveapi/blob/master/WINDOWS_EXTRA) notes.

# Use

Open the editor.
Create a cube and scale it to about 0.1, 0.1, 0.3. Make the cube movable.
Add to the cube a PSMove component.
Create + Edit the object's blueprint.
In Blueprints, add the OnDataUpdated event.
Next to that, drag the PSMove component into the graph.
Whenever the OnDataUpdated event fires, get Position and get Rotation from the PSMove component.
Then tie those into the Set Actor Location and Rotation node.
![BP example](https://github.com/cboulay/psmove-ue4/master/bp.png)