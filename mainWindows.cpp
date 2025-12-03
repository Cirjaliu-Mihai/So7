#include <windows.h>
#include <iostream>
#include <vector>

using namespace std;

CRITICAL_SECTION mtx;
CONDITION_VARIABLE cvWhite;
CONDITION_VARIABLE cvBlack;

int whiteActive = 0, blackActive = 0;
int waitingWhite = 0, waitingBlack = 0;

enum Turn { NONE, WHITE, BLACK };
Turn turn = NONE;

void beginWhite()
{
    EnterCriticalSection(&mtx);
    waitingWhite++;

    while (blackActive > 0 || turn == BLACK)
        SleepConditionVariableCS(&cvWhite, &mtx, INFINITE);

    waitingWhite--;
    whiteActive++;
    turn = WHITE;

    LeaveCriticalSection(&mtx);
}

void endWhite()
{
    EnterCriticalSection(&mtx);

    whiteActive--;

    if (whiteActive == 0)
    {
        if (waitingBlack > 0)
            turn = BLACK;
        else
            turn = NONE;

        WakeAllConditionVariable(&cvBlack);
    }

    LeaveCriticalSection(&mtx);
}

void beginBlack()
{
    EnterCriticalSection(&mtx);
    waitingBlack++;

    while (whiteActive > 0 || turn == WHITE)
        SleepConditionVariableCS(&cvBlack, &mtx, INFINITE);

    waitingBlack--;
    blackActive++;
    turn = BLACK;

    LeaveCriticalSection(&mtx);
}

void endBlack()
{
    EnterCriticalSection(&mtx);

    blackActive--;

    if (blackActive == 0)
    {
        if (waitingWhite > 0)
            turn = WHITE;
        else
            turn = NONE;

        WakeAllConditionVariable(&cvWhite);
    }

    LeaveCriticalSection(&mtx);
}

DWORD WINAPI whiteTask(LPVOID)
{
    beginWhite();
    cout << "White thread using resource\n";
    Sleep(500);
    endWhite();
    return 0;
}

DWORD WINAPI blackTask(LPVOID)
{
    beginBlack();
    cout << "Black thread using resource\n";
    Sleep(500);
    endBlack();
    return 0;
}

int main()
{
    InitializeCriticalSection(&mtx);
    InitializeConditionVariable(&cvWhite);
    InitializeConditionVariable(&cvBlack);

    vector<HANDLE> threads;

    for (int i = 0; i < 5; i++)
        threads.push_back(CreateThread(NULL, 0, whiteTask, NULL, 0, NULL));

    for (int i = 0; i < 5; i++)
        threads.push_back(CreateThread(NULL, 0, blackTask, NULL, 0, NULL));

    WaitForMultipleObjects(threads.size(), threads.data(), TRUE, INFINITE);

    for (auto h : threads) CloseHandle(h);
    DeleteCriticalSection(&mtx);

    return 0;
}
