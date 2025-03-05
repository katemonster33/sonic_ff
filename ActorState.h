#pragma once

enum class ActorState
{
    Invalid,
    Default,
    Idle,
    Running,
    Jumping,
    Attacking,
    LookingUp,
    Crouching,
    Hurt,
    Falling,
    Dead
};

enum ActorIntent
{
    NoIntent = 0,
    Run = 1,
    Jump = 2,
    Attack = 4,
    Crouch = 8,

};

enum MovementKeysDown
{
    MKeyNone = 0,
    MKeyDown = 1,
    MKeyUp = 2,
    MKeyLeft = 4,
    MKeyRight = 8
};