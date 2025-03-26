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
}

Actor* Controller::GetActor()
{
    return m_map->GetActorByHandle(m_actorHandle);
}
