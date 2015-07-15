#include "StandAlonePrivatePCH.h"

#include "AnimNode_LegsFabrik.h"
#include "AnimGraphNode_LegsFabrik.h"

class LegsIKPlugin : public ModuleInterface
{
	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};

IMPLEMENT_MODULE(LegsIKPlugin, UObjectPlugin)

void LegsIKPlugin::StartupModule()
{
	// This code will execute after your module is loaded into memory (but after global variables are initialized, of course.)
}


void LegsIKPlugin::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
}
