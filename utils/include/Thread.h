#ifndef _SDL2W_UTIL_THREAD_H
#define _SDL2W_UTIL_THREAD_H

#include <pthread.h>
#include <sched.h>
#include <string>
#include <cstdint>
#include <atomic>

namespace utils {
class Thread;
// Thread* CreateThread(const char* name, ThreadDelegate* delegate);

/**
 * A class that wraps a thread. Extend this class to create threads. The default
 * implementation uses pthreads, you must customize this to work on your platform.
 */
class Thread {
 public:
     /**
      *   Default Constructor for thread
      */
    Thread(void);

    /**
      *   virtual destructor
      */
    virtual ~Thread(void);

    // Thread* CreateThread(const char* name);

        /**
     * @brief This function sets the thread name.
     * @param[in] name name of thread
     */
    virtual int setThreadName(const std::string& name);

    /**
     * @brief This function sets the scheduling mode.
     * @param[in] name name of thread
     */
    virtual int setThreadSched(const int policy, const int priority);

    /**
      *   Thread functionality Pure virtual function  , it will be re implemented in derived classes
      */
    virtual void run(void) = 0;

    /**
     *   Function to start thread.
     */
    virtual int startThread(void);
    virtual int stopThread(void);

 private:
    /**
     *   Call back Function Passing to pthread create API
     */
    static void* threadFunc(void* pTr);

    void workerThread();

    /**
     *   Internal pthread ID.
     */
    pthread_t mId;
    std::string mName;
    bool mThreadCreated;
    std::atomic<bool>   mShutdown;
};

}
#endif // _SDL2W_UTIL_THREAD_H