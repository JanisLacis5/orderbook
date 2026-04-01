#pragma once

#include <thread>

// TODO(1): event handling (another thread listening)
class Producer
{
public:
    Producer() {}

    // void send_event();  TODO(1)

private:
    std::thread thread_{};
    void start();
};
