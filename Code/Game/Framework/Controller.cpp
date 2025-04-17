#include "Controller.hpp"

#include "Engine/Renderer/Camera.hpp"
#include "Game/Game.hpp"
#include "Game/GameCommon.hpp"
#include "Game/Gameplay/Map.hpp"


Controller::Controller(Map* map): m_map(map)
{
}

Controller::~Controller()
{
    m_map = nullptr;
}

void Controller::Update(float deltaTime)
{
    UNUSED(deltaTime)
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
    printf("Controller::Possess     Possessing Actor at (%f, %f, %f)\n", newPossessActor->m_position.x, newPossessActor->m_position.y, newPossessActor->m_position.z);
}

Actor* Controller::GetActor()
{
    return m_map->GetActorByHandle(m_actorHandle);
}

void Controller::SetControllerIndex(int index)
{
    m_index = index;
}

AABB2 Controller::SetViewport(AABB2 viewPort)
{
    m_viewport = viewPort;
    m_worldCamera->SetNormalizedViewport(viewPort);
    m_viewCamera->SetNormalizedViewport(viewPort);
    m_screenViewport = m_viewCamera->GetViewPortUnnormalized(Vec2(g_theGame->m_screenSpace.GetDimensions().x, g_theGame->m_screenSpace.GetDimensions().y));
    return viewPort;
}


int Controller::GetControllerIndex()
{
    return m_index;
}
