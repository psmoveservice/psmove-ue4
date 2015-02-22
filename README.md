# psmove-ue4

Plugin for using PSMove as input into Unreal Engine 4.

# Notes

This plugin is in the very early stages of development. It works in OS X, and will probably work in Windows if the libraries can be linked properly.

Working features:

- Positional information in game.

Not-working features:

- Hot-plugging (camera must be plugged in and controller turned on when module loaded, i.e. when launching editor)
- Multiple controllers (currently only uses last controller)
- Detect button presses —> events
- Controlling LEDs
- Controlling vibration

# Install

Change to your project directory (must be a project with C++ code).

```
mkdir Plugins
cd Plugins
git clone https://github.com/cboulay/psmove-ue4.git
mv psmove-ue4 PSMovePlugin
```

Then refresh your code (in Windows, right click on .uproject; in Mac, do from within editor) and build your project.

Make sure your camera is plugged in and the psmove controller is connected and in view of the camera.

Open the editor. In your game, open the Actor you want to have PSMove input. Add the PSMove component to the actor. This actor component will automatically update its variables Position (FVector, in cm) and Orientation (FQuat) from the PSMove on each tick. You can access these variables off the PSMoveComponent in blueprints.

# Developer Notes

## Bare Minimum

This is a [UE4 Plugin](https://docs.unrealengine.com/latest/INT/Programming/Plugins/index.html). We are making a game plugin and we want the game to statically link against our PSMove module so the game can use (or inherit from) our provided classes.

If desired, [there is a tool](https://github.com/karolz/PluginCreator) that automates the creation of a minimal plugin. A minimal plugin is constructed as follows:

First, we define our plugin's module(s) and list them in the plugin descriptor: `PSMovePlugin.uplugin`. We only have one module: `PSMove`. Next, we create a Source/<ModuleName> folder for each of our modules.

In each module folder we have its <ModuleName>.Build.cs that describes how to build it. There we define the include directories and the libraries (and other modules) we will link. 

Somewhere in the module's code we need to call `IMPLEMENT_MODULE(<ModuleImplementation>, <ModuleName>), which is an alias to DLLEXPORT the initialization of this module. This is typically done in <ModuleName>/Private/F<ModuleName>.cpp, which is the definition for a class (declared in Private/F<ModuleName>.h) that implements the [module interface](https://docs.unrealengine.com/latest/INT/API/Runtime/Core/Modules/IModuleInterface/index.html) (declared in Public/I<ModuleName>.h).

That is the bare minimum for a UE4 plugin.

## Adding functionality

The desired end result is an in-game class that can use Blueprints to access the module's functions. I provide an example C++ class UPSMoveComponent, subclassing UActorComponent, that has this ability. You may use or inherit directly from this class, or use it as a template to build your own C++ or Blueprint class. (TODO: Make an abstract class that has all the PSMove-specific functionality).

The FPSMove (i.e., module singleton), upon startup, initializes the FPSMoveWorker passing pointers to the module's MyPosition and MyOrientation members.
The FPSMoveWorker's PSMoveWorkerInit function constructs a static FPSMoveWorker instance (relaying pointers to MyPosition and MyOrientation).
Upon construction of the FPSMoveWorker instance, MyPosition and MyOrientation pointers are copied to local member pointers WorkerPosition and WorkerOrientation. Then the worker Thread is created, the PSMove controllers are connected and the tracker is initialized.

The worker Thread automatically Init() and Run(). On each iteration of the thread loop

1. the tracker is updated (i.e., image pulled and orb(s) found), then for each controller
2. The orb position is updated and set to the WorkerPosition variable.
3. The controller is polled.
4. The controller orientation quaternion is obtained and set to WorkerOrientation.
5. The buttons are polled (but we don’t use them currently).

By copying the controller position and orientation into these variables, which are really just pointers to the module’s Position and Orientation, we have made the controller’s position and orientation available to the main game thread as members of the module singleton.

The PSMoveComponent, on each TickComponent, copies the module singleton ModulePosition and ModuleOrientation into its own member variables Position, Orientation. These variables can be used in game.