#pragma once
#include "Sound.hpp"

struct ResourceSystemConfig
{
    bool m_bRemoveSoundPlaybackID = false;
};

class ResourceSubsystem
{
public:
    ResourceSubsystem() = delete;
    ResourceSubsystem(ResourceSystemConfig config);
    ~ResourceSubsystem();

    void BeginFrame();
    void Startup();
    void Shutdown();
    void Update();
    void EndFrame();

    /// Pre Registering Resource
    void RegisterSounds();

    /// Sound Resource management
    std::vector<SoundPlaybackID> StopSoundsBySoundID(SoundID soundID);
    void                         CachedSoundPlaybackID(SoundPlaybackID playback, SoundID soundID);
    int                          GetCachedSoundPlaybackIDs(SoundID soundID, std::vector<SoundPlaybackID>& playbacks);
    int                          ForceStopSoundAndRemoveSoundPlaybackID(SoundID soundID);

private:
    ResourceSystemConfig               m_config;
    std::map<SoundPlaybackID, SoundID> m_cachedSounds;
};
