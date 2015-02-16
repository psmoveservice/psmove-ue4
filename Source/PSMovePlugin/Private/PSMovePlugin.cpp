#include "PSMovePluginPrivatePCH.h"

#include "FPSMovePlugin.h"
//#include "PSMoveDelegate.h"

IMPLEMENT_MODULE( FPSMovePlugin, PSMovePlugin )

#define LOCTEXT_NAMESPACE "PSMovePlugin"

class DataCollector
{
public:
    DataCollector()
    {

    }

    ~DataCollector()
    {

    }

    void ConvertAllData()
    {

    }
};

void FPSMovePlugin::StartupModule()
{
    // This code will execute after your module is loaded into memory (but after global variables are initialized, of course.)

    //UE_LOG(PSMovePluginLog, Log, TEXT("Loaded PSMove Plugin"));

    collector = new DataCollector;
}


void FPSMovePlugin::ShutdownModule()
{
    // This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
    // we call this function before unloading the module.
}

/* Public API Implementation */
/*
PSMoveData* FPSMovePlugin::LatestData(int deviceId)
{

}
*/


void FPSMovePlugin::SetDelegate(PSMoveDelegate* newDelegate){
    //UE_LOG(PSMovePluginLog, Log, TEXT("PSMove Delegate Set (should only be called once per begin play)."));
    psmoveDelegate = newDelegate;
}

void FPSMovePlugin::RemoveDelegate()
{

}

void FPSMovePlugin::PSMoveTick(float DeltaTime)
{
    // Get the freshest data.

    // Convert and pass the data to the delegate
    collector->ConvertAllData();

    // Call teh delegate
    if (psmoveDelegate != NULL)
    {
        DelegateTick(DeltaTime);
    }
}

void FPSMovePlugin::DelegateTick(float DeltaTime)
{
    // Update data history

    // Trigger any delegate events

}

#undef LOCTEXT_NAMESPACE