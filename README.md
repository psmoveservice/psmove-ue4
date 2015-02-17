# psmove-ue4
Plugin for using PSMove as input into Unreal Engine 4.

# User Notes

Change to your project directory (must be a project with C++ code).
`mkdir Plugins`
`cd Plugins`
`git clone https://github.com/cboulay/psmove-ue4.git`
`mv psmove-ue4 PSMovePlugin`
Then refresh your code.

# Developer Notes

## Bottom-Up

`PSMovePlugin.uplugin` defines a module named `PSMovePlugin`.

`PSMovePlugin.Build.cs` defines the Public and Private include paths and links the psmoveapi libraries.

(I think) The build tool searches the public and private include paths for `IMPLEMENT_MODULE( FPSMovePlugin, PSMovePlugin )`. With that identified, we know that the implementation of the `PSMovePlugin` module can be found in the `FPSMovePlugin` class.

This class (declared in `Private/FPSMovePlugin.h < Public/IPSMovePlugin.h`; defined in `Private/FPSMovePlugin.cpp`), defines `SetDelegate(PSMoveDelegate*)`, `PSMoveTick` (poll for the latest data then calls…), and `DelegateTick`.



## Top-Down

We need a PlayerController that we can use to control a pawn.
To do this, we create `APSMovePlayerController`, which inherits from `APlayerController` and others that inherit from `PSMoveDelegate`. The `APSMovePlayerController` defines virtual functions `BeginPlay()`, `EndPlay(const EEndPlayReason::Type EndPlayReason)`, and `Tick(DeltaTime)`. Each of these calls their `Super`, and then the PSMove-specific functions (`PSMoveStartup`, `PSMoveShutdown`, `PSMoveTick`) defined by `PSMoveDelegate`.

