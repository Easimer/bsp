#pragma once

enum eInputAction {
    eInputInvalid = 0,
    eInputForward = 1,
    eInputBackward = 2,
    eInputStrafeLeft = 3,
    eInputStrafeRight = 4,
    eInputTurnLeft = 5,
    eInputTurnRight = 6,
    eInputQuitGame = 7,
    eInputLast
};

class IInputHandler {
public:
    virtual void Initialize() = 0;
    virtual void Shutdown() = 0;

    virtual int GetNextInputAction(eInputAction* pInputAction, int* bRelease) = 0;
    virtual int IsPressed(eInputAction eAction) = 0;
};

IInputHandler* Input();
