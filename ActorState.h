#pragma once

enum class ActorState
{
    Invalid,
    Default,
    Idle,
    Running,
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
    MoveLeft = 1,
    MoveRight = 2,
    MoveForward = 4,
    MoveBack = 8,
    Jump = 16,
    Attack = 32,
    Crouch = 64,

};