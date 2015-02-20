# psmove-ue4
Plugin for using PSMove as input into Unreal Engine 4.
DO NOT USE! IT DOES NOT WORK!

# User Notes

Change to your project directory (must be a project with C++ code).

```
mkdir Plugins
cd Plugins
git clone https://github.com/cboulay/psmove-ue4.git
mv psmove-ue4 PSMovePlugin
```

Then refresh your code (in winows, right click on .uproject; in Mac, do from within editor) and build your project.

In your game, open the Actor you want to have PSMove input from. Add the PSMove component to the actor.
Then... (in progress)

# Developer Notes

## Bare Minimum

See here for a definition of a [UE4 Plugin](https://docs.unrealengine.com/latest/INT/Programming/Plugins/index.html). We are making a game plugin and we want the game to statically link against our PSMove module so the game can use (or inherit from) our provided classes.

The steps to create a minimal plugin are a bit tedious. See [here for a tool](https://github.com/karolz/PluginCreator) that automates this process.

First, we define our plugin's module(s) and list them in the plugin descriptor: `PSMovePlugin.uplugin`. We only have one module: `PSMove`. Next, we create a Source/<ModuleName> folder for each of our modules.

In each module folder we have its <ModuleName>.Build.cs that describes how to build it. There we define the include directories and the libraries (and other modules) we will link. 

Somewhere in the module's code we need to call `IMPLEMENT_MODULE(<ModuleImplementation>, <ModuleName>), which is an alias to DLLEXPORT the initialization of this module. This is typically done in <ModuleName>/Private/F<ModuleName>.cpp, which is the definition for a class (declared in Private/F<ModuleName>.h) that implements the [module interface](https://docs.unrealengine.com/latest/INT/API/Runtime/Core/Modules/IModuleInterface/index.html) (declared in Public/I<ModuleName>.h).

That is the bare minimum for a plugin.

## Adding functionality

The desired end result is an in-game class that can use Blueprints to access the module's functions. I provide an example C++ class UPSMoveComponent, subclassing UActorComponent, that has this ability. You may use or inherit directly from this class, or use it as a template to build your own C++ or Blueprint class.

The FPSMove (i.e., module singleton), upon startup, initializes the FPSMoveWorker passing pointers to the module's MyPosition and MyOrientation members.
The FPSMoveWorker's PSMoveWorkerInit function constructs a static FPSMoveWorker instance (relaying pointers to MyPosition and MyOrientation).
Upon construction of the FPSMoveWorker instance, MyPosition and MyOrientation pointers are copied to local member pointers WorkerPosition and WorkerOrientation. Then the worker Thread is created, the PSMove controllers are connected and the tracker is initialized. The trail ends here. We need a way to Init() and Run() the worker.





Right now this component, when it ticks, tells the device to update its data and log it. This instruction is enabled through inheritance of an abstract class called IPSMoveAbstract that communicates with the module via its singleton (e.g., `IModule::Get().ModuleTick`);  TODO: Rename this IPSMoveWrapper, because it wraps the module in an interface.

The better solution is to have the module updating its data in its own thread as fast as possible. Then our component, on each tick, will ask the device for its freshest data. If this can be done via the module singleton then any arbitrary object that can access the singleton (via implementing the module interface wrapper) can get the data.