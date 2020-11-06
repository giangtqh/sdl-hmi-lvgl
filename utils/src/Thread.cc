
#include "Log.h"
#include "Thread.h"
#include <sys/prctl.h>
namespace utils {
const int8_t PTHREAD_NAME_LEN = 15;

Thread::Thread(void)
    : mId(),
      mName("Thread"),
      mThreadCreated(false),
      mShutdown(false) {
}

Thread::~Thread(void) {
    if (false == mShutdown.load(std::memory_order_acquire)) {
        stopThread();
    }
}

int Thread::setThreadName(const std::string& inName) {
    mName = inName;
    mName.resize(PTHREAD_NAME_LEN);
    int err = pthread_setname_np(mId, static_cast<const char*>(mName.c_str()));
    return err;
}

int Thread::setThreadSched(const int inPolicy, const int inPriority) {
    int schedPolicy;
    struct sched_param param;
    int err = pthread_getschedparam(mId, &schedPolicy, &param);
    if (0 == err) {
        param.__sched_priority = (SCHED_OTHER == inPolicy) ? 0 : inPriority;
        err = pthread_setschedparam(mId, inPolicy, &param);
    }
    return err;
}

int Thread::startThread(void) {
    int rc = 0;
    if (!mThreadCreated) {
        rc = pthread_create(&mId, NULL, &Thread::threadFunc, this);
        if (0 == rc) {
            mThreadCreated = true;
        } else {
            rc = -1;
        }
    }
    return rc;
}

int Thread::stopThread(void) {
    int rc = 0;
    if (false == mShutdown.load(std::memory_order_acquire)) {
        mShutdown.store(true, std::memory_order_release);
        if (mThreadCreated) {
            rc = pthread_join(mId, NULL);
            if (0 == rc) {
                mThreadCreated = false;
            } else {
                rc = -1;
            }
        }
    }
    return rc;
}

void* Thread::threadFunc(void* pTr) {
    auto me = static_cast<Thread*>(pTr);
    if (nullptr != me) {
        me->workerThread();
    }
    return nullptr;
}

void Thread::workerThread(void) {
    if (mName.size() != 0) {
        pthread_setname_np(mId, static_cast<const char*>(mName.c_str()));
    }
    static_cast<Thread*>(this)->run();
    pthread_exit(NULL);
}
}
