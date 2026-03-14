#pragma once

#include <thread>

// TODO(1): event handling (another thread listening)
class Producer
{
public:
    Producer()
        : thread_{&Producer::start, this}
    {
    }

    // void send_event();  TODO(1)

private:
    std::thread thread_{};
    void start();
};
