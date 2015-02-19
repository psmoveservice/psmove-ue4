#pragma once

class PSMoveAbstract
{
    public:

    virtual void MoveSetup(); // 
    virtual void MoveUnset(); // 
    virtual void MoveTick(float DeltaTime); // 
};