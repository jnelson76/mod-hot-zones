#include "ScriptMgr.h"
#include "Config.h"
#include "Log.h"

extern void AddSC_HotZones();

void Addmod_hot_zonesScripts()
{
    if (!sConfigMgr->LoadModulesConfigs())
        LOG_ERROR("module", "HotZones: Failed to load hot_zones.conf");
    AddSC_HotZones();
}