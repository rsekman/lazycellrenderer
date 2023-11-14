#include <condition_variable>
#include <mutex>
#include <optional>
#include <queue>

template<typename T> class ThreadQueue {
    private:
        std::queue<T> q;
        std::condition_variable c;
        std::mutex m;
        bool isOpen;
    public:
        ThreadQueue(void) :
            q(), c(), m()
        {
            isOpen = true;
        }
        //for copy types
        void push(const T& value) {
            if (!isOpen) {
                return;
            }
            std::lock_guard<std::mutex> lock(m);
            q.push(value);
            c.notify_one();
        }
        //for move-only types
        void push(T&& value) {
            if (!isOpen) {
                return;
            }
            std::lock_guard<std::mutex> lock(m);
            q.push(std::move(value));
            c.notify_one();
        }
        std::optional<T> pop() {
            std::unique_lock<std::mutex> lock(m);
            c.wait( lock, [this] {
                return !this->q.empty() || !this->isOpen;
            } );
            std::optional<T> out;
            if ( !this->q.empty() ){
                out = std::move(q.front());
                q.pop();
            }
            return out;
        }
        bool is_open() {
            std::lock_guard<std::mutex> lock(m);
            return isOpen;
        }
        void close () {
            std::lock_guard<std::mutex> lock(m);
            isOpen = false;
            c.notify_all();
        }
};
