# psmove-ue4
Plugin for using PSMove as input into Unreal Engine 4.

Clone into <My_Project>/Plugins/ and rename folder to PSMovePlugin.

# User Notes

# Developer Notes

I am very new to UE4 programming. Much of what I write below may be wrong, but the process helps me learn.

`PSMovePlugin.uplugin` defines a module named `PSMovePlugin`.

`PSMovePlugin.Build.cs` defines the Public and Private include paths.

I can only imagine what happens next. The build tool must search the public and private include paths for `IMPLEMENT_MODULE( FPSMovePlugin, PSMovePlugin )`.

With that identified, we know that the implementation of the PSMovePlugin module can be found in the FPSMovePlugin class.

`FPSMovePlugin` is declared in two stages: `Public/IPSMovePlugin.h` and `Private/FPSMovePlugin.h`
`FPSMovePlugin` implementation is in `Private/FPSMovePlugin.cpp`

`Public/IPSMovePlugin.h` declares the virtual functions `SetDelegate()` and `PSMoveTick(float DeltaTime)`.
`Private/FPSMovePlugin.h` declares class `DataCollector`, functions `StartupModule()`, and `ShutdownModule()`,
the previously virtual functions `SetDelegate(PSMoveDelegate* newDelegate)` and `PSMoveTick(float DeltaTime)`,
private member variables `collector` and `psmoveDelegate`,
and private functions `DelegateTick(float DeltaTime)` and `DelegateUpdateAllData`.

The implementation, `Private/FPSMovePlugin.cpp`, is where all the work happens.
The above classes and functions are all defined here.