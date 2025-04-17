#pragma once
#include <vector>

struct PlayerSaveData
{
    int m_playerID    = -1;
    int m_numOfDeaths = 0;
    int m_numOfKilled = 0;
};

class PlayerSaveSubsystem
{
private:
    static std::vector<PlayerSaveData> s_playerSavedData;

public:
    static void            ClearSaves();
    static PlayerSaveData* GetPlayerSaveData(int id);
    static bool CreatePlayerSaveData(PlayerSaveData newSaveData);
    static bool            DoesPlayerSaveDataExist(int id);
};
