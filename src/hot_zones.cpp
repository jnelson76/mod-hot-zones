#include "ScriptMgr.h"
#include "ScriptObject.h"
#include "Config.h"
#include "Creature.h"
#include "Player.h"
#include "Map.h"
#include "GameObject.h"
#include <set>
#include <vector>

class HotZones : public PlayerScript
{
public:
    HotZones() : PlayerScript("HotZones") {}

    void OnPlayerCreatureKill(Player* player, Creature* creature) override
    {
        if (!player || !creature || !IsHotZone(creature->GetAreaId()))
            return;

        uint32 areaId = creature->GetAreaId();

        printf("Killed mob in Area %u: active=%d, killed=%u, currentWaveSize=%u\n", 
               areaId, waveTracker[areaId].active, waveTracker[areaId].killed, waveTracker[areaId].currentWaveSize);

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
            printf("Incremented killed: %u/%u in Area %u\n", waveTracker[areaId].killed, waveTracker[areaId].currentWaveSize, areaId);
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
        hotZones.insert(33); // Stranglethorn Vale
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
        waveTracker[areaId].killed = 1;
        printf("Starting waves in Area %u: wave=%u, size=%u, killed=%u\n", 
               areaId, waveTracker[areaId].currentWave, waveTracker[areaId].currentWaveSize, waveTracker[areaId].killed);
        SpawnWave(player, areaId, waveTracker[areaId].currentWaveSize);
    }

    void SpawnNextWave(Player* player, uint32 areaId)
    {
        waveTracker[areaId].currentWave++;
        if (waveTracker[areaId].currentWave > maxWaves)
        {
            waveTracker[areaId].active = false;
            printf("Waves completed in Area %u\n", areaId);
            return;
        }

        waveTracker[areaId].currentWaveSize = baseMobCount * waveTracker[areaId].currentWave;
        waveTracker[areaId].killed = 0;
        printf("Next wave in Area %u: wave=%u, size=%u, killed reset to %u\n", 
               areaId, waveTracker[areaId].currentWave, waveTracker[areaId].currentWaveSize, waveTracker[areaId].killed);
        SpawnWave(player, areaId, waveTracker[areaId].currentWaveSize);
    }

    void SpawnWave(Player* player, uint32 areaId, uint32 count)
    {
        printf("Spawning wave %u with %u mobs at player position in Area %u\n", waveTracker[areaId].currentWave, count, areaId);
        for (uint32 i = 0; i < count; ++i)
        {
            float x = player->GetPositionX();
            float y = player->GetPositionY();
            float z = player->GetPositionZ();
            printf("Attempting to spawn wolf %u/%u at X: %.2f, Y: %.2f, Z: %.2f in Area %u\n", i + 1, count, x, y, z, areaId);
            Creature* mob = player->SummonCreature(69, x, y, z, 0, TEMPSUMMON_TIMED_DESPAWN, 600000); // Use TEMPSUMMON_TIMED_DESPAWN for testing
            if (mob)
            {
                printf("Spawned wolf %u/%u at X: %.2f, Y: %.2f, Z: %.2f in Area %u\n", i + 1, count, x, y, z, areaId);
                mob->SetLevel(player->GetLevel() + 10);
                mob->SetMaxHealth(5000); // Fixed health to test
                printf("Wolf %u/%u spawned with health %u in Area %u\n", i + 1, count, mob->GetMaxHealth(), areaId);
            }
            else
            {
                printf("Failed to spawn wolf %u/%u in Area %u\n", i + 1, count, areaId);
            }
        }
    }
};

void AddSC_HotZones()
{
    new HotZones();
}