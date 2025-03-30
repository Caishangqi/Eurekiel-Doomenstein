#pragma once
#include "Controller.hpp"

class AIController : public Controller
{
public:
    AIController(Map* map);
    /// If we don't have a current target, try to find the nearest closest visible enemy and target them. If we have a target, move towards the target at maximum speed and turn towards them as maximum turn rate. If we have an equipped melee weapon, attack every frame we are in range of our melee weapon.
    /// @param deltaTime 
    void   Update(float deltaTime) override;
    void   Possess(ActorHandle& actorHandle) override;
    Actor* GetActor() override;

    void DamagedBy(ActorHandle& attacker); // Notification that the AI actor was damaged so this AI can target them.

private:
    ActorHandle m_targetActorHandle; // Handle for our current target actor, if any.
};
