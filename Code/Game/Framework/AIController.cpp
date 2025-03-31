#include "AIController.hpp"

#include "Game/Definition/ActorDefinition.hpp"
#include "Game/Gameplay/Map.hpp"

AIController::AIController(Map* map): Controller(map)
{
}

void AIController::Update(float deltaTime)
{
    Controller::Update(deltaTime);
    Actor* controlledActor = m_map->GetActorByHandle(m_actorHandle);
    Actor* target          = m_map->GetClosestVisibleEnemy(controlledActor);
    if (target && m_targetActorHandle.IsValid() && m_targetActorHandle != target->m_handle)
    {
        m_targetActorHandle = target->m_handle; // switch
        printf("AIController::Update    > Target actor changed to %s\n", target->m_definition->m_name.c_str());
    }
}

void AIController::Possess(ActorHandle& actorHandle)
{
    Controller::Possess(actorHandle);
}

Actor* AIController::GetActor()
{
    return Controller::GetActor();
}

void AIController::DamagedBy(ActorHandle& attacker)
{
    m_targetActorHandle = attacker;
    printf("AIController::Update    > Target actor changed to %s\n", m_map->GetActorByHandle(attacker)->m_definition->m_name.c_str());
}
