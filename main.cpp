//Yonadav Leibowitz 207424490

#include <iostream>

#include <queue>
#include <mutex>
#include <semaphore.h>
#include <string>
#include <random>
#include <unistd.h>
#include <thread>
#include <fstream>

using namespace std;


//#include "BoundedQueue.h"
class BoundedQueue {

private:
    queue<std::string> q;
    mutex locker;
    sem_t empty;
    sem_t full;


public:
    BoundedQueue() {

    }

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
        sem_wait(&full); //full--
        locker.lock();
        string val = q.front();
        q.pop();
        locker.unlock();
        sem_post(&empty); // empty++

        return val;
    }

    string getFront() {
        sem_wait(&full); //full--
        string val;
        locker.lock();
        val = q.front();
        locker.unlock();
        sem_post(&empty);
        return val;
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

        sem_wait(&full); //full--
        locker.lock();
        string val = q.front();
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
    std::thread spawnManufactureArticles() {
        return std::thread([this] { manufactureArticles(); });
    }

};


///////////////////////////////////////////////////////////////
vector<BoundedQueue *> blueQueues;
vector<Producer *> producers;

UnboundedQueue pinkSport;
UnboundedQueue pinkWeather;
UnboundedQueue pinkNews;

BoundedQueue* sharedQueue;

vector<std::thread> threadPool;


void dispatcher(int numOfProducers) {
    int dones = 0;
    while (dones < numOfProducers) {
        int i;
        ////////////////////////////////////////////////////////
        dones = 0;
        for (i = 0; i < numOfProducers; i++) {
            string article = blueQueues[i]->pop();
            if (article == "<DONE>") {
                dones++;
                blueQueues[i]->push(article);
                continue;
            }
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
        sharedQueue->push(article);
        }
    }
void screenManagerProcedure(){
    string article;
    int doneBreak = 3;
    while(doneBreak){
        article = sharedQueue->pop();
        if (article == "<DONE>"){
            doneBreak--;
            continue;
        }
        cout << article << '\n';
    }
    cout << "<DONE>" <<'\n';
}
void* producerManufactureInThread(void* arg) {
    Producer* producer = (Producer*)arg;
    producer->manufactureArticles();
    return NULL;
}

int main(int argc, char** args) {
    std::cout << "Hello, World!" << std::endl;
    ifstream configTxt;
    configTxt.open(args[1]);
    string line,id,quota,bound;

    if (!configTxt.is_open()) {
        cerr << "Error: file could not be opened" << endl;
        exit(1);
    }

    while (configTxt>>id) {
        if(configTxt >> quota) {
            configTxt >> bound;
            BoundedQueue* newBlueQueue = new BoundedQueue(stoi(bound));
            blueQueues.push_back(newBlueQueue);
            Producer* newProducer = new Producer(stoi(id), stoi(quota), newBlueQueue);
            producers.push_back(newProducer);
            std::thread t = newProducer->spawnManufactureArticles();
            threadPool.push_back(move(t));
        }
        else{
            sharedQueue = new BoundedQueue(stoi(id));
        }

    }
    configTxt.close();

    /*BoundedQueue *b1 = new BoundedQueue(6);
    Producer producer1 = Producer(1, 5, b1);
    BoundedQueue *b2 = new BoundedQueue(4);
    Producer producer2 = Producer(2, 3, b2);
    BoundedQueue *b3 = new BoundedQueue(31);
    Producer producer3 = Producer(3, 30, b3);


    blueQueues.push_back(b1);
    blueQueues.push_back(b2);
    blueQueues.push_back(b3);

    std::thread t1 = producer1.spawnManufactureArticles();
    threadPool.push_back(move(t1));
    std::thread t2 = producer2.spawnManufactureArticles();
    threadPool.push_back(move(t2));
    std::thread t3 = producer3.spawnManufactureArticles();
    threadPool.push_back(move(t3));

*/

    std::thread dispatcherThread = thread(dispatcher, producers.size());
    threadPool.push_back(move(dispatcherThread));

    std::thread coEditorProcedureThread1 = thread(coEditorProcedure, &pinkSport);
    threadPool.push_back(move(coEditorProcedureThread1));
    std::thread coEditorProcedureThread2 = thread(coEditorProcedure, &pinkNews);
    threadPool.push_back(move(coEditorProcedureThread2));
    std::thread coEditorProcedureThread3 = thread(coEditorProcedure, &pinkWeather);
    threadPool.push_back(move(coEditorProcedureThread3));

    std::thread screenManagerProcedureThread = thread(screenManagerProcedure);
    threadPool.push_back(move(screenManagerProcedureThread));







    for(int i = 0; i<threadPool.size(); i++){
        threadPool[i].join();
    }


    return 0;
}
