#include <iostream>
#include <thread>
#include "queue.h"
#include <atomic>
#include <memory>
#include <thread>
#include <chrono>

std::mutex logMtx;

void log(const std::string& log) {
    std::lock_guard lock{logMtx};
    std::cout << log << '\n';
}

void pushData(Queue<int>& queue, int pushSize) {
    log("Push thread is started, desired push operation count : "+std::to_string(pushSize));
    for (int i=1; i <= pushSize; ++i) {
        if(queue.push(i) ) {
            log("Pushed data count "+std::to_string(i));
        }
    }
    log("Push thread is ended");
}

void popData(Queue<int>& queue,int popSize) {
    log("Pop thread is started, desired pop operation count : "+std::to_string(popSize));
    for (int i=1; i <= popSize; ++i) {

        auto data = queue.pop();

        if (data)
            log("Poped data count : "+std::to_string(i));

    }
    log("Pop thread is ended");
}

void pushPopApp(Queue<int>& queue) {
    log("----------------------- Push, Pop Test -----------------------");
    std::thread pop(popData,std::ref(queue),3);
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    std::thread push(pushData,std::ref(queue),5);

    pop.join();
    push.join();
    log("----------------------- Push, Pop Test End -----------------------\n\n");
}

void closeQueue(Queue<int>& queue) {
    log("----------------------- Close Queue Test -----------------------");
    queue.close();
    pushPopApp(queue);
    log("----------------------- Close Queue Test End -----------------------\n\n");
}

void getFromQueue(Queue<int>& queue,auto pred) {
    log("----------------------- Get From Queue Test -----------------------");
    if(auto data = queue.get(pred);data.has_value()) {
        log("Get from queue is sucessful : "+ std::to_string(data.value()));
    } else {
        log("Get from queue is failed, pred is not satisfied");
    }
    log("----------------------- Get From Queue Test End -----------------------\n\n");
}


int main() {
    Queue<int> queue{100};  

    //First start pop then get blocked, then start push
    //{1,2,3,4,5} elements will added to queue
    //{1,2,3} elements will removed from queue
    pushPopApp(queue);

    //Try to get 1 int data from queue
    getFromQueue(queue,[](int i){return i==1;});

    //Close queue and try push pop operations
    closeQueue(queue);

    //Try to get 4 int data from queue
    getFromQueue(queue,[](int i){return i==4;});

    //Try to get 4 int data from queue
    getFromQueue(queue,[](int i){return i==5;});

}