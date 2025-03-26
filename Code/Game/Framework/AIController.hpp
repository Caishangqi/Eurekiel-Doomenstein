#pragma once
#include "Controller.hpp"

class AIController : public Controller
{
public:
    AIController(Map* map);

    void   Update(float deltaTime) override;
    void   Possess(ActorHandle& actorHandle) override;
    Actor* GetActor() override;
};
