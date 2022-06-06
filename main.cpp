#include <iostream>

#include <queue>
#include <mutex>
#include <semaphore.h>
#include <string>

using namespace std;

//#include "boundedQueue.h"
class boundedQueue {

private:
    queue<std::string> q;
    mutex locker;
    sem_t empty;
    sem_t full;



public: boundedQueue(int bound) {
        sem_init(&empty, 0, bound);
        sem_init(&full, 0, 0);
    }

    void push(string s) {
        sem_wait(&empty); // empty--
        locker.lock();
        q.push(s);
        locker.unlock();
        sem_post(&full); // full++
    }

    void pop() {
        sem_wait(&full); //full--
        locker.lock();
        q.pop();
        locker.unlock();
        sem_post(&empty); // empty++
    }
};


class unboundedQueue {

private:
    queue<std::string> q;
    mutex locker;
    sem_t full;



    public: unboundedQueue() {
        sem_init(&full, 0, 0);

    }

    void push(string s) {
        locker.lock();
        q.push(s);
        locker.unlock();
        sem_post(&full); // full++
    }

    void pop() {
        sem_wait(&full); //full--
        locker.lock();
        q.pop();
        locker.unlock();
    }
};

int main() {
    std::cout << "Hello, World!" << std::endl;
    boundedQueue* q = new boundedQueue(3);
    while(1){
        q->push("1");

    }
    return 0;
}
