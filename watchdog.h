// Watchdog library for C++
// by Dheera Venkatraman <https://dheera.net/>
// License: 3-clause BSD

#ifndef _watchdog_h
#define _watchdog_h

#include <thread>
#include <atomic>
#include <chrono>
#include <condition_variable>
#include <mutex>
#include <iostream>
#include <functional>

namespace watchdog {

class Watchdog {
  public:
    Watchdog();
    Watchdog(std::function<void()> callback);
    ~Watchdog();
    void start(unsigned int _interval);
    void stop();
    void refresh();

  private:
    unsigned int mInterval;
    std::atomic<bool> mIsRunning;
    std::thread mThread;
    std::function<void()> mCallback;
    std::mutex mMutex;
    std::chrono::steady_clock::time_point mLastRefreshTime;
    std::condition_variable mStopCondition;
    void loop();
};

Watchdog::Watchdog() {
  mCallback = [] {
      std::cerr << "watchdog triggered\n";
      kill(getpid(), SIGTERM);
  };
  mIsRunning = false;
}

Watchdog::Watchdog(std::function<void()> callback) {
  mCallback = callback;
  mIsRunning = false;
}

Watchdog::~Watchdog() {
  stop();
}

void Watchdog::start(unsigned int interval) {
    std::unique_lock<std::mutex> lock(mMutex);
    if(mIsRunning) return;

    mLastRefreshTime = std::chrono::steady_clock::now();
    mInterval = interval;
    mIsRunning = true;
    mThread = std::thread(&Watchdog::loop, this);
}

void Watchdog::stop() {
    std::unique_lock<std::mutex> lock(mMutex);
    if(!mIsRunning) return;

    mIsRunning = false;
    mStopCondition.notify_all();
    lock.unlock();
    mThread.join();
}

void Watchdog::refresh() {
    std::unique_lock<std::mutex> lock(mMutex);
    mLastRefreshTime = std::chrono::steady_clock::now();
    mStopCondition.notify_all();
}

void Watchdog::loop() {
    std::unique_lock<std::mutex> lock(mMutex);
    while(mIsRunning) {
      if(mStopCondition.wait_for(lock, std::chrono::milliseconds(mInterval)) == std::cv_status::timeout) {
        if(mCallback != nullptr) {
          mIsRunning = false;
          mCallback();
        }
      }
    }
}  

}  

}

#endif /* _watchdog_h */

