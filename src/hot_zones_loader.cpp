#include "ScriptMgr.h"
#include "Config.h"

extern void AddSC_HotZones();

void Addmod_hot_zonesScripts()
{
    if (!sConfigMgr->LoadModulesConfigs())
        sLog->outError("HotZones", "Failed to load hot_zones.conf");
    AddSC_HotZones();
}