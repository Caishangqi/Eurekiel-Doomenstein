#include "AIController.hpp"

AIController::AIController(Map* map): Controller(map)
{
}

void AIController::Update(float deltaTime)
{
    Controller::Update(deltaTime);
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
}
