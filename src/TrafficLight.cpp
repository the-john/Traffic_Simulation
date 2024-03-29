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
    std::unique_lock<std::mutex> uLock(_mutex);
    _condition.wait(uLock, [this]() { return !_queue.empty(); });

    T t = std::move(_queue.back());
    _queue.pop_back();
    return t;
}

template <typename T>
void MessageQueue<T>::send(T &&msg)
{
    // FP.4a : The method send should use the mechanisms std::lock_guard<std::mutex> 
    // as well as _condition.notify_one() to add a new message to the queue and afterwards send a notification.
    std::lock_guard<std::mutex> uLock(_mutex);
    _queue.push_back(std::move(msg));
    _condition.notify_one();
}


/* Implementation of class "TrafficLight" */

TrafficLight::TrafficLight()
{
    _currentPhase = TrafficLightPhase::red;
}

TrafficLight::~TrafficLight() {}


void TrafficLight::waitForGreen()
{
    // FP.5b : add the implementation of the method waitForGreen, in which an infinite while-loop 
    // runs and repeatedly calls the receive function on the message queue. 
    // Once it receives TrafficLightPhase::green, the method returns.
    while (true) {
        TrafficLightPhase msg = _messages.receive();
        if (msg == TrafficLightPhase::green) {
            return;
        }
    }
}


TrafficLightPhase TrafficLight::getCurrentPhase()
{
    return _currentPhase;
}

void TrafficLight::setCurrentPhase(TrafficLightPhase phase)
{
    _currentPhase = phase;
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
    
    //srand(time(0));                                                                                     // seed the random number generator
                                                                                                          // https://www.geeksforgeeks.org/generating-random-number-range-c/
    //float duration = (rand()%(6 - 4 +1)) + 4;                                                           // generate a random number between 4 and 6
    //duration = duration * 1000;                                                                         // turn it into seconds
    
    // NOTE:  Thanks to feedback from my reviewer, there is a better way to generate random numbers (longer period and better statistical behavior) than rand()
    std::random_device seed;                                                                            // create a seed for the random number engine
    std::mt19937 gen(seed());                                                                           // generate random number with seed & Mersenne Twister
    std::uniform_real_distribution<double> dis(4.0, 6.0);                                               // set between 4 and 6
    double duration = dis(gen);                                                                         // define and set duration
    duration = duration * 1000;                                                                         // turn it into seconds
        
    std::chrono::time_point<std::chrono::system_clock> lastUpdate;                                      // get the current system time
    lastUpdate = std::chrono::system_clock::now();                                                      // make lastUpdate now

    while (true) {                                                                                      // infinite loop
        std::this_thread::sleep_for(std::chrono::milliseconds(1));                                      // wait between cycles as per Task list
        int timeDifference = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - lastUpdate).count();
        if (timeDifference >= duration) {

            if (getCurrentPhase() == TrafficLightPhase::red) {                                              // from red to green
                _currentPhase = TrafficLightPhase::green;
            }
            else
            {
                _currentPhase = TrafficLightPhase::red;                                                     // or green to red
            }
            lastUpdate = std::chrono::system_clock::now();                                                  // make lastUpdate now
            duration = (rand()%(6-4+1)) + 4;                                                                // generate a random number between 4 and 6
            duration = duration * 1000;                                                                     // turn it into seconds
            _messages.send(std::move(_currentPhase));                                                       // push message to queue
        }
    }

}

