#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <vector>
#include <chrono>

using namespace std;
mutex mtx;
condition_variable cvWhite, cvBlack;

int whiteActive = 0, blackActive = 0;
int waitingWhite = 0, waitingBlack = 0;

enum Turn
{
    NONE,
    WHITE,
    BLACK
};
Turn turn = NONE;

void beginWhite()
{
    unique_lock<mutex> lock(mtx);

    waitingWhite++;

    while (blackActive > 0 || turn == BLACK)
    {
        cvWhite.wait(lock);
    }

    waitingWhite--;
    whiteActive++;
    turn = WHITE;
}

void endWhite()
{
    unique_lock<mutex> lock(mtx);

    whiteActive--;

    if (whiteActive == 0)
    {
        if (waitingBlack > 0)
            turn = BLACK;
        else
            turn = NONE;

        cvBlack.notify_all();
    }
}

void beginBlack()
{
    unique_lock<mutex> lock(mtx);

    waitingBlack++;

    while (whiteActive > 0 || turn == WHITE)
    {
        cvBlack.wait(lock);
    }

    waitingBlack--;
    blackActive++;
    turn = BLACK;
}

void endBlack()
{
    unique_lock<mutex> lock(mtx);

    blackActive--;

    if (blackActive == 0)
    {
        if (waitingWhite > 0)
            turn = WHITE;
        else
            turn = NONE;

        cvWhite.notify_all();
    }
}

void whiteTask()
{
    beginWhite();
    cout << "White thread using resource\n";
    this_thread::sleep_for(chrono::milliseconds(500));
    endWhite();
}

void blackTask()
{
    beginBlack();
    cout << "Black thread using resource\n";
    this_thread::sleep_for(chrono::milliseconds(500));
    endBlack();
}

int main()
{
    vector<thread> threads;

    for (int i = 0; i < 5; i++)
    {
        threads.emplace_back(whiteTask);
    }
    for (int i = 0; i < 5; i++)
    {
        threads.emplace_back(blackTask);
    }

    for (auto &t : threads)
        t.join();

    return 0;
}
