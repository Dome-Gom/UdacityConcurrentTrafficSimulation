#include <iostream>
#include <random>
#include <chrono>
#include "TrafficLight.h"

/* Implementation of class "MessageQueue" */

template <typename T>
T MessageQueue<T>::receive()
{
    // FP.5a : The method receive should use std::unique_lock<std::mutex> and _condition.wait() 
    // to wait for and receive new messages and pull them from the queue using move semantics. 
    // The received object should then be returned by the receive function. 

    std::unique_lock<std::mutex> lock(_mtx);
    _condition.wait(lock, [this] {return !_messages.empty();});
    T msg = std::move(_messages.back());
    _messages.pop_back();

    return msg;
}

template <typename T>
void MessageQueue<T>::send(T &&msg)
{
    // FP.4a : The method send should use the mechanisms std::lock_guard<std::mutex> 
    // as well as _condition.notify_one() to add a new message to the queue and afterwards send a notification.
    
    std::lock_guard<std::mutex> lock(_mtx);
    _messages.push_back(std::move(msg));
    _condition.notify_one();
}


/* Implementation of class "TrafficLight" */

TrafficLight::TrafficLight()
{
    _currentPhase = TrafficLightPhase::red;
}

void TrafficLight::waitForGreen()
{
    // FP.5b : add the implementation of the method waitForGreen, in which an infinite while-loop 
    // runs and repeatedly calls the receive function on the message queue. 
    // Once it receives TrafficLightPhase::green, the method returns.
    TrafficLightPhase receivedPhase;
    bool phaseGreenReached = false;

    while(!phaseGreenReached)
    {
        //std::this_thread::sleep_for(std::chrono::milliseconds(1));
        receivedPhase = _phaseQueue.receive();
        if (receivedPhase == TrafficLightPhase::green && _currentPhase == TrafficLightPhase::green)
            phaseGreenReached = true;
    }
    return;
}

TrafficLightPhase TrafficLight::getCurrentPhase()
{
    return _currentPhase;
}

void TrafficLight::simulate()
{
    // FP.2b : Finally, the private method „cycleThroughPhases“ should be started in a thread when the public method „simulate“ is called. To do this, use the thread queue in the base class. 
    threads.emplace_back(std::thread(&TrafficLight::cycleThroughPhases, this));
}

// virtual function which is executed in a thread
void TrafficLight::cycleThroughPhases()
{
    // FP.2a : Implement the function with an infinite loop that measures the time between two loop cycles 
    // and toggles the current phase of the traffic light between red and green and sends an update method 
    // to the message queue using move semantics. The cycle duration should be a random value between 4 and 6 seconds. 
    // Also, the while-loop should use std::this_thread::sleep_for to wait 1ms between two cycles. 

    std::chrono::_V2::steady_clock::time_point startTime;
    std::chrono::_V2::steady_clock::time_point currentTime;
    int64_t elapsedTime;
    int64_t randomCycleTime;
    bool randomTimeReached;
    srand((unsigned int)time(NULL));

    while(true){
        randomCycleTime = static_cast<int64_t>(((static_cast<double>(rand())/RAND_MAX)*2000)+4000);
        randomTimeReached = false;
        startTime = std::chrono::steady_clock::now();

        do{
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
            currentTime = std::chrono::steady_clock::now();
            elapsedTime = std::chrono::duration_cast<std::chrono::milliseconds> (currentTime-startTime).count();
            if(elapsedTime >= randomCycleTime) randomTimeReached = true;
        }
        while(!randomTimeReached);

        if(_currentPhase == TrafficLightPhase::green)
            _currentPhase = TrafficLightPhase::red;
        else
            _currentPhase = TrafficLightPhase::green;

        TrafficLightPhase messagePhase = _currentPhase;
        _phaseQueue.send(std::move(messagePhase));
    }
}