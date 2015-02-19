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

Then refresh your code and build your project.

In your game, open the Actor you want to have PSMove input from. Add the PSMoveComponent to the actor.
Then... (in progress)

# Developer Notes

These notes are my notes taken during plugin development. They will help you understand how I developed it, which should help you understand how to modify/extend it.

## Bare Minimum

See here for a definition of a [UE4 Plugin](https://docs.unrealengine.com/latest/INT/Programming/Plugins/index.html). We are making a game plugin and we want the game to statically link against our PSMove module so the game can use (or inherit from) our provided classes.

The steps to create a minimal plugin are a bit tedious. See [here for a tool](https://github.com/karolz/PluginCreator) that automates this process.

First, we define our plugin's module(s) and list them in the plugin descriptor: `<PluginName>.uplugin`. We only have one module: `<ModuleName>`. Next, we create a Source/<ModuleName> folder for each of our modules.

In each module folder we have its <ModuleName>.Build.cs that describes how to build it. There we define the include directories and the libraries (and other modules) we will link. 

Somewhere in the module's code we need to call `IMPLEMENT_MODULE(<ModuleImplementation>, <ModuleName>), which is an alias to DLLEXPORT the initialization of this module. This is typically done in <ModuleName>/Private/F<ModuleName>.cpp, which is the definition for a class (declared in Private/F<ModuleName>.h) that implements the [module interface](https://docs.unrealengine.com/latest/INT/API/Runtime/Core/Modules/IModuleInterface/index.html) (declared in Public/I<ModuleName>.h).

That is the bare minimum for a plugin.

## Adding functionality

I provide an example C++ class UPSMoveComponent, subclassing UActorComponent. You may use or inherit directly from this class. Alternatively, you may create your own C++ or blueprint class in your game based on this example.

First, we need a way for the game to communicate with the module. Specifically, we need to startup the device and then to tick the device.

When this component does OnRegister, it will tell the module to startup.
When this component does OnUnregister, it will tell the module to unset.
When this component does Tick, it will tell the module to Tick.

Our component will instruct the module instance directly (e.g., with `IModule::Get().ModuleTick`), through inheritance of an abstract class called IPSMoveAbstract. This class makes subclassing easier and also provides something like an interface described next.