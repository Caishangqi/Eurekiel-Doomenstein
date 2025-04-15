#pragma once
#include "ActorHandle.hpp"

class Actor;
class Map;

class Controller
{
public:
    Controller(Map* map);
    virtual ~Controller();

public:
    ActorHandle  m_actorHandle; // Handle of our currently possessed actor or INVALID if no actor is possessed.
    Map*         m_map   = nullptr; // Reference to the current map for purposes of dereferencing actor handles.
    std::string  m_state = "None";  // Track for state
    virtual void Update(float deltaTime);
    /// Unpossess any currently possessed actor and possess a new one. Notify each actor so it can check for
    /// restoring AI controllers or handling other changes of possession logic.
    /// @param actorHandle 
    virtual void Possess(ActorHandle& actorHandle);
    /// Returns the currently possessed actor or null if no actor is possessed.
    virtual Actor* GetActor();
};
