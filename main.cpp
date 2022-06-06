#include <iostream>

#include <queue>
#include <mutex>
#include <semaphore.h>
#include <string>
#include <random>
#include <unistd.h>

using namespace std;

//#include "BoundedQueue.h"
class BoundedQueue {

private:
    queue<std::string> q;
    mutex locker;
    sem_t empty;
    sem_t full;


public:
    BoundedQueue(int bound) {
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

    string pop() {
        string val = q.front();
        sem_wait(&full); //full--
        locker.lock();
        q.pop();
        locker.unlock();
        sem_post(&empty); // empty++

        return val;
    }

    string getFront() {
        return q.front();
    }
};


class UnboundedQueue {

private:
    queue<std::string> q;
    mutex locker;
    sem_t full;


public:
    UnboundedQueue() {
        sem_init(&full, 0, 0);

    }

    void push(string s) {
        locker.lock();
        q.push(s);
        locker.unlock();
        sem_post(&full); // full++
    }

    string getFront() {
        return q.front();
    }

    string pop() {
        string val = q.front();

        sem_wait(&full); //full--
        locker.lock();
        q.pop();
        locker.unlock();

        return val;
    }
};

class Producer {

private:
    BoundedQueue *bq;
    int articlesQuota;
    int id;
    int sportsCounter = 0;
    int newsCounter = 0;
    int weatherCounter = 0;

public:
    Producer(int newId, int newProductsQuota, BoundedQueue *newBq) {
        id = newId;
        bq = newBq;
        articlesQuota = newProductsQuota;

    }

public:
    string generateArticle() {
        int ranVal = rand() % 3;
        string type;
        int counter;
        string article;
        if (ranVal == 0) {
            type = "sports";
            sportsCounter++;
            counter = sportsCounter;
        }
        if (ranVal == 1) {
            type = "news";
            newsCounter++;
            counter = newsCounter;
        }
        if (ranVal == 2) {
            type = "weather";
            weatherCounter++;
            counter = weatherCounter;
        }
        article = "producer " + to_string(id) + ' ' + type + ' ' + to_string(counter);
        return article;
    }

public:
    void manufactureArticles() {
        int i;
        for (i = 0; i < articlesQuota; i++) {
            string newArticle = generateArticle();
            bq->push(newArticle);
        }
        bq->push("<DONE>");
    }
};


///////////////////////////////////////////////////////////////
vector<BoundedQueue *> blueQueues;
UnboundedQueue pinkSport;
UnboundedQueue pinkWeather;
UnboundedQueue pinkNews;

UnboundedQueue sharedQueue;


void dispatcher(int numOfProducers) {
    int dones = 0;
    while (dones < numOfProducers) {
        int i;
        ////////////////////////////////////////////////////////
        dones = 0;
        for (i = 0; i < numOfProducers; i++) {
            string w = blueQueues[i]->getFront();
            if (w == "<DONE>") {
                dones++;
                continue;
            }
            string article = blueQueues[i]->pop();
            //what type art thou?
            if(article.find("sports") != string::npos){
                pinkSport.push(article);
            }
            if(article.find("news") != string::npos){
                pinkNews.push(article);
            }
            if(article.find("weather") != string::npos){
                pinkWeather.push(article);
            }
        }



    }
    pinkSport.push("<DONE>");
    pinkNews.push("<DONE>");
    pinkWeather.push("<DONE>");
}

void coEditorProcedure(UnboundedQueue* pinkQueue){
    string article = "";
    while(article != "<DONE>"){
        article = pinkQueue->pop();
        usleep(100000);
        sharedQueue.push(article);
        }
    }
void screenManagerProcedure(){
    string article;
    int doneBreak = 3;
    while(doneBreak){
        article = sharedQueue.pop();
        if (article == "<DONE>"){
            doneBreak--;
            continue;
        }
        cout << article <<'\n';
    }
    cout << "<DONE>" <<'\n';
}


int main() {
    std::cout << "Hello, World!" << std::endl;
    BoundedQueue *b1 = new BoundedQueue(6);
    Producer producer1 = Producer(1, 5, b1);
    BoundedQueue *b2 = new BoundedQueue(4);
    Producer producer2 = Producer(2, 3, b2);
    BoundedQueue *b3 = new BoundedQueue(31);
    Producer producer3 = Producer(3, 30, b3);
    blueQueues.push_back(b1);
    blueQueues.push_back(b2);
    blueQueues.push_back(b3);
    producer1.manufactureArticles();
    producer2.manufactureArticles();
    producer3.manufactureArticles();
    dispatcher(3);
    coEditorProcedure(&pinkSport);
    coEditorProcedure(&pinkNews);
    coEditorProcedure(&pinkWeather);
    screenManagerProcedure();


    return 0;
}
