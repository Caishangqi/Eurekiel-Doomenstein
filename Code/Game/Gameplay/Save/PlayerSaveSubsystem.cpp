#include "PlayerSaveSubsystem.hpp"
std::vector<PlayerSaveData> PlayerSaveSubsystem::s_playerSavedData = {};

void PlayerSaveSubsystem::ClearSaves()
{
    printf("PlayerSaveSubsystem::ClearSaves       Clear All player Data\n");
    PlayerSaveSubsystem::s_playerSavedData.clear();
}

PlayerSaveData* PlayerSaveSubsystem::GetPlayerSaveData(int id)
{
    for (PlayerSaveData& data : s_playerSavedData)
    {
        if (data.m_playerID == id)
        {
            return &data;
        }
    }
    return nullptr;
}

bool PlayerSaveSubsystem::CreatePlayerSaveData(PlayerSaveData newSaveData)
{
    if (!DoesPlayerSaveDataExist(newSaveData.m_playerID))
    {
        s_playerSavedData.push_back(newSaveData);
        printf("PlayerSaveSubsystem::CreatePlayerSaveData       Create player data for player id: %d\n", newSaveData.m_playerID);
        return true;
    }
    return false;
}

bool PlayerSaveSubsystem::DoesPlayerSaveDataExist(int id)
{
    for (PlayerSaveData& data : s_playerSavedData)
    {
        if (data.m_playerID == id)
        {
            return true;
        }
    }
    return false;
}
