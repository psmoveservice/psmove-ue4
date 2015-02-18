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

Then refresh your code.

# Developer Notes

## Bare Minimum

We first create an 'interface' class `IPSMovePlugin` (Public/IPSMovePlugin.h) that inherits from [IModuleInterface](https://docs.unrealengine.com/latest/INT/API/Runtime/Core/Modules/IModuleInterface/index.html).
Technically, this is more of an 'abstract' class because it does have two implemented functions Get() and IsAvailable().
This abstract class defines one or more virtual functions (Leap plugin defines a virtual class).

This class is implemented by `FPSMovePlugin` (Private/FPSMovePlugin.h|cpp).
In the header, it is necessary to declare previously abstract-functions (`virtual TYPE FUNC() override;` vs normal declaration?).
Two functions that you definitely need to declare are `void StartupModule();` and `void ShutdownModule();`, previously defined in IModuleInterface, which are called on module load (e.g., opening editor) and shutdown, respectively.

In the definition (FPSMovePlugin.cpp), we have to call `IMPLEMENT_MODULE(FPSMovePlugin, PSMovePlugin), which is an alias to DLLEXPORT the initialization of this module.

That is the bare minimum for a plugin.

## Adding functionality

To Do.