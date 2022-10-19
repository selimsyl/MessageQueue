#include <iostream>
#include <vector>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <optional>
#include <functional>
#include <algorithm>

template<typename Data>
class Queue {
public:

    /**
     * @brief Construct queue with desired size
     * @exception std::length_error
     */
    Queue(std::size_t size) noexcept(false) : m_maxCap{size} {
        m_buffer.reserve(size);
    }
    
    /**
     * @brief Blocking method wait until data available
     * @param No
     * @return if queue is not closed first data in the queue, otherwise null.
     * @details Thread safety is guaranteed. Return value must be checked.
     */
    [[nodiscard]] std::optional<Data> pop() {
        std::unique_lock lock{m_mutex};

        if (disableQueue)
            return std::optional<Data>{};

        cvBlockPop.wait(lock,[&](){return m_size > 0;});

        Data d = m_buffer.front();

        remove();

        return std::optional(d);
    }

    /**
     * @brief Non-blocking push.
     * @param data to be added to the queue
     * @return True if pushing is successful, otherwise false
     * @details Thread safety is guaranteed. Return value must be checked.
     */
    [[nodiscard]] bool push(Data data) {
        std::unique_lock lock{m_mutex};

        if (disableQueue)
            return false;
            
        if (m_size == m_maxCap) 
            return false;

        add(data);

        cvBlockPop.notify_all();

        return true;
    }

    /**
     * @brief Disables queue pop,push operations.
     * @details Thread safety is guaranteed
     */
    void close() noexcept {
        disableQueue=true;
    }

    /**
     * @brief Thread Safety, get first element satisfies predicate.
     *          Can be called after queue is closed
     * @return If predicate satisfied desired element, otherwise null. 
     * @param Predicate callable for desired element.
     * @details Thread safety is guaranteed. Return value must be checked.
     */
    [[nodiscard]] std::optional<Data> get(std::function<bool(Data &)> predicate) {
        std::unique_lock lock{m_mutex};

        auto findIter = std::find_if(m_buffer.begin(),m_buffer.end(),predicate);

        return findIter!=m_buffer.end()?*findIter:std::optional<Data>{};
    }

private:
    /**
     * @details Add data to underlying buffer
     *          Updates m_size attribute
     */
    void add(Data& data) {
        m_buffer.push_back(data);
        m_size = m_buffer.size();
    }

    /**
     * @details Remove first data from underlying buffer
     *          Updates m_size attribute
     */
    void remove() {
        m_buffer.erase(m_buffer.begin());
        m_size = m_buffer.size();
    }

    /**
     * @details Underlying buffer to store queue elements
     */
    std::vector<Data> m_buffer;

    /**
     * @details Mutex member used to sync multi thread applications
     */
    std::mutex m_mutex;

    /**
     * @details Used for blocking pop operation.
     */
    std::condition_variable cvBlockPop;

    /**
     * @details Used as alias to underlying buffer size
     */
    std::atomic<int> m_size{0};

    /**
     * @details Used as alias to underlying buffer capacity
     */
    const std::atomic<size_t> m_maxCap;

    /**
     * @details Disables queue pop,push operations.
     *          Get operations is also available after closing.
     */
    std::atomic<bool> disableQueue{false};
};