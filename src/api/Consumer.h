#pragma once

#include <thread>

// TODO(1): event handling (another thread listening)
class Consumer
{
public:
    Consumer()
        : thread_{&Consumer::start, this}
    {
    }

    // void send_event();  TODO(1)

private:
    std::thread thread_{};
    void start();
};
