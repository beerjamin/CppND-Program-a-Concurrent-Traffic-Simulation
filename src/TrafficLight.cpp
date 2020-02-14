#include <iostream>
#include <random>
#include "TrafficLight.h"

/* Implementation of class "MessageQueue" */


template <typename T>
T MessageQueue<T>::receive()
{
    // FP.5a : The method receive should use std::unique_lock<std::mutex> and _condition.wait() 
    // to wait for and receive new messages and pull them from the queue using move semantics. 
    // The received object should then be returned by the receive function.
    std::unique_lock<std::mutex> locker(_mutex);
    _condition.wait(locker, [this]{
               return !_queue.empty();
            });
    T message = std::move(_queue.back());
    _queue.pop_back();
    return message;
}

template <typename T>
void MessageQueue<T>::send(T &&msg)
{
    // FP.4a : The method send should use the mechanisms std::lock_guard<std::mutex> 
    // as well as _condition.notify_one() to add a new message to the queue and afterwards send a notification.
    std::lock_guard<std::mutex> locker(_mutex);
    _queue.push_back(std::move(msg));
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

    while (true){
        std::this_thread::sleep_for(std::chrono::microseconds(1));
        auto trafficLight = _messageQueue.receive();
        if (trafficLight == TrafficLightPhase::green){
            return;
        }
    }
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
    // source: https://www.fluentcpp.com/2019/05/24/how-to-fill-a-cpp-collection-with-random-values/
    std::random_device random_device;
    std::mt19937 random_engine(random_device());
    std::uniform_int_distribution<int> distribution_4_6(4,6);

    auto time_start = std::chrono::system_clock::now();
    auto cycle = distribution_4_6(random_engine);

    while (true){
        std::this_thread::sleep_for(std::chrono::microseconds(1));
        auto time_frame = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now() - time_start);

        if (time_frame.count() >= cycle){
            if (_currentPhase == TrafficLightPhase::green)
            {
                _currentPhase = TrafficLightPhase::red;
            }
            else
            {
                _currentPhase = TrafficLightPhase::green;
            }
            _messageQueue.send(std::move(_currentPhase));
            cycle = distribution_4_6(random_engine);
            time_start = std::chrono::system_clock::now();
        }
    }
}

