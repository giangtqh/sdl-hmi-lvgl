
#include <algorithm>
#include <atomic>
#include <condition_variable>
#include <mutex>
#include <queue>

namespace utils {

template <typename T, class Q = std::queue<T> >
class MessageQueue {
public:
    typedef Q Queue;
    /**
     * \brief Default constructor
     */
    MessageQueue();

    /**
     * \brief Destructor
     */
    ~MessageQueue();

    /**
     * \brief Returns size of the queue.
     * \return Size of the queue.
     */
    size_t size();

    /**
     * \brief If queue is empty.
     * \return Is queue empty.
     */
    bool empty();

    /**
     * \brief Tells if queue is being shut down
     */
    bool IsShuttingDown() const;

    /**
     * \brief Adds element to the queue.
     * \param element Element to be added to the queue.n
     */
    void push(const T& element);

    /**
     * \brief Removes element from the queue and returns it
     * \param element Element to be returned
     * \return True on success, false if queue is empty
     */
    bool pop(T& element);

    /**
     * \brief Conditional wait.
     */
    void wait();

    /**
     * \brief waitUntilEmpty message queue
     * Wait until message queue is empty
     */
    void WaitUntilEmpty();

    /**
     * \brief Shutdown the queue.
     * This leads to waking up everyone waiting on the queue
     * Queue being shut down can be drained ( with pop() )
     * But nothing must be added to the queue after it began
     * shutting down
     */
    void Shutdown();

    /**
     * \brief Clears queue.
     */
    void Reset();

private:
    /**
     *\brief Queue
     */
    Queue queue_;
    std::atomic<bool> shutting_down_;
    std::mutex mMutex;
    std::condition_variable mCondVar;
};

template <typename T, class Q>
MessageQueue<T, Q>::MessageQueue() : shutting_down_(false) {}

template <typename T, class Q>
MessageQueue<T, Q>::~MessageQueue() {}

template <typename T, class Q>
void MessageQueue<T, Q>::wait() {
  std::unique_lock<std::mutex> lock(mMutex);
  while ((!shutting_down_) && queue_.empty()) {
    mCondVar.wait(lock);
  }
}

template <typename T, class Q>
void MessageQueue<T, Q>::WaitUntilEmpty() {
  std::unique_lock<std::mutex> lock(mMutex);
  while ((!shutting_down_) && !queue_.empty()) {
    mCondVar.wait(lock);
  }
}

template <typename T, class Q>
size_t MessageQueue<T, Q>::size() {
  std::lock_guard<std::mutex> guard(mMutex);
  return queue_.size();
}

template <typename T, class Q>
bool MessageQueue<T, Q>::empty() {
  std::lock_guard<std::mutex> guard(mMutex);
  return queue_.empty();
}

template <typename T, class Q>
bool MessageQueue<T, Q>::IsShuttingDown() const {
  return shutting_down_;
}

template <typename T, class Q>
void MessageQueue<T, Q>::push(const T& element) {
  {
    std::lock_guard<std::mutex> guard(mMutex);
    if (shutting_down_) {
      return;
    }
    queue_.push(element);
  }
  mCondVar.notify_all();
}

template <typename T, class Q>
bool MessageQueue<T, Q>::pop(T& element) {
  std::lock_guard<std::mutex> guard(mMutex);
  if (queue_.empty()) {
    return false;
  }
  element = queue_.front();
  queue_.pop();
  mCondVar.notify_one();
  return true;
}

template <typename T, class Q>
void MessageQueue<T, Q>::Shutdown() {
  std::lock_guard<std::mutex> guard(mMutex);
  shutting_down_ = true;
  if (!queue_.empty()) {
    Queue empty_queue;
    std::swap(queue_, empty_queue);
  }
  mCondVar.notify_all();
}

template <typename T, class Q>
void MessageQueue<T, Q>::Reset() {
  std::lock_guard<std::mutex> guard(mMutex);
  shutting_down_ = false;
  if (!queue_.empty()) {
    Queue empty_queue;
    std::swap(queue_, empty_queue);
  }
}

}