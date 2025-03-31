#include "AIController.hpp"

#include "Engine/Math/MathUtils.hpp"
#include "Game/Definition/ActorDefinition.hpp"
#include "Game/Gameplay/Map.hpp"
#include "Game/Gameplay/Weapon.hpp"

AIController::AIController(Map* map): Controller(map)
{
}

void AIController::Update(float deltaTime)
{
    Controller::Update(deltaTime);
    Actor* controlledActor = m_map->GetActorByHandle(m_actorHandle);
    if (!controlledActor || controlledActor->m_bIsDead)
    {
        return;
    }
    Actor* target = m_map->GetClosestVisibleEnemy(controlledActor);
    if (target && m_targetActorHandle.IsValid() && m_targetActorHandle != target->m_handle)
    {
        m_targetActorHandle = target->m_handle;
        printf("AIController::Update > Target actor changed to %s\n", target->m_definition->m_name.c_str());
    }
    Actor* targetActor = m_map->GetActorByHandle(m_targetActorHandle);
    if (!targetActor || targetActor->m_bIsDead)
    {
        m_targetActorHandle = ActorHandle::INVALID;
        return;
    }

    float turnSpeedDegPerSec      = controlledActor->m_definition->m_turnSpeed;
    float maxTurnDegreesThisFrame = turnSpeedDegPerSec * deltaTime;
    Vec3  toTarget3D              = targetActor->m_position - controlledActor->m_position;
    toTarget3D.z                  = 0.f;

    float desiredYaw = Atan2Degrees(toTarget3D.y, toTarget3D.x);
    float currentYaw = controlledActor->m_orientation.m_yawDegrees;
    float newYaw     = GetTurnedTowardDegrees(currentYaw, desiredYaw, maxTurnDegreesThisFrame);

    Vec3 newDirectionTurningTo = Vec3(newYaw, controlledActor->m_orientation.m_pitchDegrees, controlledActor->m_orientation.m_rollDegrees);
    controlledActor->TurnInDirection(newDirectionTurningTo);

    float distanceToTarget = toTarget3D.GetLength();
    float combinedRadius   = controlledActor->m_physicalRadius + targetActor->m_physicalRadius;
    if (distanceToTarget > combinedRadius + 0.1f)
    {
        float moveSpeed = controlledActor->m_definition->m_runSpeed;
        Vec3  forward, left, up;
        controlledActor->m_orientation.GetAsVectors_IFwd_JLeft_KUp(forward, left, up);
        controlledActor->MoveInDirection(forward, moveSpeed);
    }
    else
    {
        /// TODO: Consider find the ai current equipped weapon, if finded determine whether is melee weapon
        /// and using melee weapon range to determined whether or not fire the weapon
        controlledActor->m_currentWeapon->Fire();
        // controlledActor->MoveInDirection(dirToTarget, moveSpeed * 0.2f);
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
