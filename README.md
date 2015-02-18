# psmove-ue4
Plugin for using PSMove as input into Unreal Engine 4.
DO NOT USE! IT DOES NOT WORK!

# User Notes

Change to your project directory (must be a project with C++ code).
`mkdir Plugins`
`cd Plugins`
`git clone https://github.com/cboulay/psmove-ue4.git`
`mv psmove-ue4 PSMovePlugin`
Then refresh your code.

# Developer Notes

As far as I can tell, there are [two ways](https://answers.unrealengine.com/questions/2649/how-exactly-do-game-plugins-work.html) to design Plugins for use in the game.

1. [Using UINTERFACE macro](https://docs.unrealengine.com/latest/INT/Programming/UnrealArchitecture/Reference/Interfaces/index.html) - but you cannot expose UObjects this way. TODO EXAMPLE
2. [API-style interface](http://adamitskiy.wix.com/daniel#!ue4-programming-api-fundamentals/c1f6q), where each individual exported function is tagged with PSMOVEPLUGIN_API. In this case the Plugin will be statically linked to your game (I think). TODO EXAMPLE

One convenient way to have your Plugin functions used by the game is through event delegation. Again, there seems to be two ways to do this:
1. Create a class that [implements the interface](https://wiki.unrealengine.com/Interfaces_in_C%2B%2B), and in its implementation it calls BlueprintImplementableEvents. Then you can use this class in the game. Some links suggest this style (i.e., interfaces) cannot be implemented in blueprints but [maybe that is wrong](https://wiki.unrealengine.com/Interfaces_And_Blueprints). TODO EXAMPLE
2. [UE4s event delegation system](https://docs.unrealengine.com/latest/INT/Programming/UnrealArchitecture/Delegates/Events/index.html). TODO EXAMPLE

## Bottom-Up

`PSMovePlugin.uplugin` defines a module named `PSMovePlugin`.

`PSMovePlugin.Build.cs` defines the Public and Private include paths and links the psmoveapi libraries.

(I think) The build tool searches the public and private include paths for `IMPLEMENT_MODULE( FPSMovePlugin, PSMovePlugin )`. With that identified, we know that the implementation of the `PSMovePlugin` module can be found in the `FPSMovePlugin` class.

This class (declared in `Private/FPSMovePlugin.h, inherits from Public/IPSMovePlugin.h`, necessarily inherits from `IModuleInterface`; implemented in `Private/FPSMovePlugin.cpp`), defines `SetDelegate(PSMoveDelegate*)`, `PSMoveTick` (poll for the latest data then calls…), and `DelegateTick`.



## Top-Down

[Interfaces](https://docs.unrealengine.com/latest/INT/Programming/UnrealArchitecture/Reference/Interfaces/index.html)

We need a PlayerController that we can use to control a pawn.
To do this, we create `APSMovePlayerController`, which inherits from `APlayerController` and others that inherit from `PSMoveDelegate`. The `APSMovePlayerController` defines virtual functions `BeginPlay()`, `EndPlay(const EEndPlayReason::Type EndPlayReason)`, and `Tick(DeltaTime)`. Each of these calls their `Super`, and then the PSMove-specific functions (`PSMoveStartup`, `PSMoveShutdown`, `PSMoveTick`) defined by `PSMoveDelegate`.

