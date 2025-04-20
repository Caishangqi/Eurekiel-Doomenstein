#include "ResourceSubsystem.hpp"

#include "Game/Game.hpp"
#include "Game/GameCommon.hpp"

ResourceSubsystem::ResourceSubsystem(ResourceSystemConfig config): m_config(config)
{
}

ResourceSubsystem::~ResourceSubsystem()
{
}

void ResourceSubsystem::BeginFrame()
{
}

void ResourceSubsystem::Startup()
{
    printf("ResourceSubsystem::Startup    Initialize Resource Subsystem\n");
    RegisterSounds();
}

void ResourceSubsystem::Shutdown()
{
}

void ResourceSubsystem::Update()
{
}

void ResourceSubsystem::EndFrame()
{
}

void ResourceSubsystem::RegisterSounds()
{
    printf("ResourceSubsystem::RegisterSounds       register sounds\n");
    g_theAudio->CreateOrGetSound(g_gameConfigBlackboard.GetValue("mainMenuMusic", ""));
    g_theAudio->CreateOrGetSound(g_gameConfigBlackboard.GetValue("gameMusic", ""),FMOD_2D);
    g_theAudio->CreateOrGetSound(g_gameConfigBlackboard.GetValue("buttonClickSound", ""));
}

std::vector<SoundPlaybackID> ResourceSubsystem::StopSoundsBySoundID(SoundID soundID)
{
    std::vector<SoundPlaybackID> sounds;
    sounds.reserve(m_cachedSounds.size());
    for (std::pair<SoundPlaybackID, SoundID> sound : m_cachedSounds)
    {
        if (sound.second == soundID)
        {
            g_theAudio->StopSound(sound.first);
            sounds.emplace_back(sound.second);
            m_cachedSounds.erase(sound.first); // Erase
        }
    }
    return sounds;
}

void ResourceSubsystem::CachedSoundPlaybackID(SoundPlaybackID playback, SoundID soundID)
{
    auto it = m_cachedSounds.find(playback);
    if (it == m_cachedSounds.end())
        m_cachedSounds.emplace(playback, soundID);
    else
        m_cachedSounds[playback] = soundID;
}

int ResourceSubsystem::GetCachedSoundPlaybackIDs(SoundID soundID, std::vector<SoundPlaybackID>& playbacks)
{
    for (std::pair<SoundPlaybackID, SoundID> sound : m_cachedSounds)
    {
        if (sound.second == soundID)
            playbacks.emplace_back(sound.first);
    }
    return static_cast<int>(playbacks.size());
}

int ResourceSubsystem::ForceStopSoundAndRemoveSoundPlaybackID(SoundID soundID)
{
    int numOfRemove = 0;
    // Use iterator to traverse manually, erase(it) directly when encountering the target, and continue traversing with the return value
    for (auto it = m_cachedSounds.begin(); it != m_cachedSounds.end();)
    {
        if (it->second == soundID)
        {
            g_theAudio->StopSound(it->first);
            it = m_cachedSounds.erase(it); // erase returns the next valid iterator
            numOfRemove++;
        }
        else
        {
            ++it; // proceed normally
        }
    }
    return numOfRemove;
}
