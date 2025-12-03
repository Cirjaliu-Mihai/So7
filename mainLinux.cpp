#include <pthread.h>
#include <unistd.h>
#include <iostream>
#include <vector>

using namespace std;

pthread_mutex_t mtx = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cvWhite = PTHREAD_COND_INITIALIZER;
pthread_cond_t cvBlack = PTHREAD_COND_INITIALIZER;

int whiteActive = 0, blackActive = 0;
int waitingWhite = 0, waitingBlack = 0;

enum Turn { NONE, WHITE, BLACK };
Turn turn = NONE;

void beginWhite()
{
    pthread_mutex_lock(&mtx);
    waitingWhite++;

    while (blackActive > 0 || turn == BLACK)
        pthread_cond_wait(&cvWhite, &mtx);

    waitingWhite--;
    whiteActive++;
    turn = WHITE;

    pthread_mutex_unlock(&mtx);
}

void endWhite()
{
    pthread_mutex_lock(&mtx);

    whiteActive--;

    if (whiteActive == 0)
    {
        if (waitingBlack > 0)
            turn = BLACK;
        else
            turn = NONE;

        pthread_cond_broadcast(&cvBlack);
    }

    pthread_mutex_unlock(&mtx);
}

void beginBlack()
{
    pthread_mutex_lock(&mtx);
    waitingBlack++;

    while (whiteActive > 0 || turn == WHITE)
        pthread_cond_wait(&cvBlack, &mtx);

    waitingBlack--;
    blackActive++;
    turn = BLACK;

    pthread_mutex_unlock(&mtx);
}

void endBlack()
{
    pthread_mutex_lock(&mtx);

    blackActive--;

    if (blackActive == 0)
    {
        if (waitingWhite > 0)
            turn = WHITE;
        else
            turn = NONE;

        pthread_cond_broadcast(&cvWhite);
    }

    pthread_mutex_unlock(&mtx);
}

void* whiteTask(void*)
{
    beginWhite();
    cout << "White thread using resource\n";
    usleep(500000);
    endWhite();
    return nullptr;
}

void* blackTask(void*)
{
    beginBlack();
    cout << "Black thread using resource\n";
    usleep(500000);
    endBlack();
    return nullptr;
}

int main()
{
    vector<pthread_t> threads(10);

    for (int i = 0; i < 5; i++) pthread_create(&threads[i], nullptr, whiteTask, nullptr);
    for (int i = 0; i < 5; i++) pthread_create(&threads[5+i], nullptr, blackTask, nullptr);

    for (auto& t : threads) pthread_join(t, nullptr);

    return 0;
}
