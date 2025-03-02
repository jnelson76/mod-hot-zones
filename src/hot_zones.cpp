#include "ScriptMgr.h"
#include "ScriptObject.h"
#include "Config.h"
#include "Creature.h"
#include "Player.h"
#include "Map.h"
#include "GameObject.h"
#include <set>
#include <vector>

class HotZones : public CreatureScript
{
public:
    HotZones() : CreatureScript("HotZones") {}

    void OnCreatureCreate(Creature* creature) override
    {
        if (IsHotZone(creature->GetAreaId()))
        {
            // Make all mobs elite
            creature->SetElite(true);
            creature->SetLevel(creature->GetLevel() + 5); // Boost level
            creature->SetBaseHealth(creature->GetBaseHealth() * 2); // Double HP
            creature->SetBaseDamage(creature->GetBaseDamage() * 1.5f); // 50% more damage
            printf("Elite mob spawned in hot zone: %u (Area: %u)\n", creature->GetEntry(), creature->GetAreaId());
        }
    }

    void OnCreatureDeath(Creature* creature, Unit* killer) override
    {
        if (!killer || !killer->IsPlayer() || !IsHotZone(creature->GetAreaId()))
            return;

        Player* player = killer->ToPlayer();
        uint32 areaId = creature->GetAreaId();

        // Check if this kill should trigger a wave
        if (waveTracker[areaId].active && waveTracker[areaId].killed >= waveTracker[areaId].currentWaveSize)
        {
            SpawnNextWave(player, areaId);
        }
        else if (!waveTracker[areaId].active)
        {
            StartWaves(player, areaId);
        }
        else
        {
            waveTracker[areaId].killed++;
            printf("Mob killed in wave: %u/%u (Area: %u)\n", waveTracker[areaId].killed, waveTracker[areaId].currentWaveSize, areaId);
        }
    }

private:
    std::set<uint32> hotZones;
    struct WaveInfo
    {
        bool active = false;
        uint32 waveCount = 0;
        uint32 currentWave = 0;
        uint32 currentWaveSize = 0;
        uint32 killed = 0;
    };
    std::map<uint32, WaveInfo> waveTracker;

    void LoadConfig()
    {
        std::string zoneList = sConfigMgr->GetOption<std::string>("HotZones.Zones", "17");
        std::stringstream ss(zoneList);
        std::string zone;
        while (std::getline(ss, zone, ','))
            hotZones.insert(std::stoul(zone));

        maxWaves = sConfigMgr->GetOption<int>("HotZones.WaveCount", 3);
        baseMobCount = sConfigMgr->GetOption<int>("HotZones.BaseMobCount", 5);
    }

    bool IsHotZone(uint32 areaId)
    {
        if (hotZones.empty())
            LoadConfig();
        return hotZones.count(areaId) > 0;
    }

    uint32 maxWaves = 3;
    uint32 baseMobCount = 5;

    void StartWaves(Player* player, uint32 areaId)
    {
        waveTracker[areaId].active = true;
        waveTracker[areaId].currentWave = 1;
        waveTracker[areaId].currentWaveSize = baseMobCount;
        waveTracker[areaId].killed = 1; // Count the first kill
        SpawnWave(player, areaId, waveTracker[areaId].currentWaveSize);
    }

    void SpawnNextWave(Player* player, uint32 areaId)
    {
        waveTracker[areaId].currentWave++;
        if (waveTracker[areaId].currentWave > maxWaves)
        {
            waveTracker[areaId].active = false;
            printf("Waves completed in Area: %u\n", areaId);
            return;
        }

        waveTracker[areaId].currentWaveSize = baseMobCount * waveTracker[areaId].currentWave;
        waveTracker[areaId].killed = 0;
        SpawnWave(player, areaId, waveTracker[areaId].currentWaveSize);
    }

    void SpawnWave(Player* player, uint32 areaId, uint32 count)
    {
        printf("Spawning wave %u with %u mobs in Area: %u\n", waveTracker[areaId].currentWave, count, areaId);
        for (uint32 i = 0; i < count; ++i)
        {
            // Example: Spawn a random elite (e.g., wolf entry 69)
            Creature* mob = player->SummonCreature(69, player->GetPositionX() + rand32() % 10 - 5, 
                                                  player->GetPositionY() + rand32() % 10 - 5, 
                                                  player->GetPositionZ(), 0, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 300000);
            if (mob)
            {
                mob->SetElite(true);
                mob->SetLevel(player->GetLevel()); // Match player level
                mob->SetBaseHealth(mob->GetBaseHealth() * 2);
                mob->SetBaseDamage(mob->GetBaseDamage() * 1.5f);
                // Add rare/epic loot
                mob->loot.clear();
                mob->loot.AddItem(LootStoreItem(5404, 0, 50, 0, LOOT_MODE_DEFAULT, 0, 1, 1)); // Serpent's Shoulders (rare)
                mob->loot.AddItem(LootStoreItem(13084, 0, 20, 0, LOOT_MODE_DEFAULT, 0, 1, 1)); // Kaleidoscope Chain (epic)
            }
        }
    }
};

void AddSC_HotZones()
{
    new HotZones();
}