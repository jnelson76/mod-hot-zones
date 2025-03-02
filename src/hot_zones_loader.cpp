#include "ScriptMgr.h"
#include "Config.h"

extern void AddSC_HotZones();

class HotZonesLoader : public ModuleScript
{
public:
    HotZonesLoader() : ModuleScript("HotZonesLoader") {}

    void OnLoad() override
    {
        if (!sConfigMgr->LoadModuleConfig("hot_zones.conf"))
            sLog->outError("HotZones", "Failed to load hot_zones.conf");
    }
};

void Addmod_hot_zonesScripts()
{
    new HotZonesLoader();
    AddSC_HotZones();
}