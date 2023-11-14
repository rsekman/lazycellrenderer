#include <mutex>
#include <unordered_map>

template<typename K, typename V> class ThreadMap {
    private:
        std::unordered_map<K, V> m;
        std::mutex mx;
    public:
        ThreadMap(void) :
            m(), mx()
        {};
        auto find (const K& key) {
            std::lock_guard<std::mutex> lock(mx);
            return m.find(key);
        }
        auto count (const K& key) {
            std::lock_guard<std::mutex> lock(mx);
            return m.count(key);
        }
        void insert_or_assign(const K& key, V&& val){
            std::lock_guard<std::mutex> lock(mx);
            m.insert_or_assign(key, val);
        }
        auto operator[] (const K& key) {
            std::lock_guard<std::mutex> lock(mx);
            return m[key];
        }
};
