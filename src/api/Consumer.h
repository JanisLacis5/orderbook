#pragma once

#include <thread>

// TODO(1): event handling (another thread listening)
class Consumer
{
public:
    Consumer()
    {
    }

    // void send_event();  TODO(1)

private:
    std::thread thread_;
    void start();
};
