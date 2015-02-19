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

There are a few things we must do to create a [UE4 Plugin](https://docs.unrealengine.com/latest/INT/Programming/Plugins/index.html). This is a game plugin and we want the game to statically link against our PSMove module so the game can use (or inherit from) our provided classes.

First, we define our plugin's module(s) and list them in `<PluginName>.uplugin`. We only have one module: `<ModuleName>`. Next, we create a Source/<ModuleName> folder for each of our modules.

In each module folder we have its <ModuleName>.Build.cs that describes how to build it. There we define the include directories and the libraries (and other modules) we will link. 

Somewhere in the module's code we need to call `IMPLEMENT_MODULE(<ModuleImplementation>, <ModuleName>), which is an alias to DLLEXPORT the initialization of this module. This is typically done in <ModuleName>/Private/F<ModuleName>.cpp, which is the definition for a class (declared in Private/F<ModuleName>.h) that implements the [module interface](https://docs.unrealengine.com/latest/INT/API/Runtime/Core/Modules/IModuleInterface/index.html) (declared in Public/I<ModuleName>.h).

That is the bare minimum for a plugin.

## Adding functionality

