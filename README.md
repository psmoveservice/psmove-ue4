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

# Setup Hardware

Read the [Wiki](https://github.com/cboulay/psmove-ue4/wiki).

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
    * Scale it to about 0.05, 0.2, 0.03
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

![BP example](https://github.com/cboulay/psmove-ue4/blob/master/wiki_pics/bp.png)