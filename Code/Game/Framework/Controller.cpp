#include "Controller.hpp"

#include "Game/Gameplay/Map.hpp"


Controller::Controller(Map* map): m_map(map)
{
}

Controller::~Controller()
{
}

void Controller::Update(float deltaTime)
{
}

void Controller::Possess(ActorHandle& actorHandle)
{
    Actor* currentPossessActor = m_map->GetActorByHandle(m_actorHandle);
    if (currentPossessActor && currentPossessActor->m_handle.IsValid())
        currentPossessActor->OnUnpossessed();
    Actor* newPossessActor = m_map->GetActorByHandle(actorHandle);
    if (newPossessActor && newPossessActor->m_handle.IsValid())
        newPossessActor->OnPossessed(this);
    m_actorHandle = actorHandle;
}

Actor* Controller::GetActor()
{
    return m_map->GetActorByHandle(m_actorHandle);
}
