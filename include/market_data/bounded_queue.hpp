#pragma once

#include <condition_variable>
#include <cstddef>
#include <mutex>
#include <optional>
#include <queue>
#include <utility>

namespace trading {

    template <typename T>
    class BoundedQueue {
    public:
        explicit BoundedQueue(std::size_t capacity)
            : capacity_(capacity) {}

        bool try_push(T value) {
            {
                std::lock_guard lock(mutex_);

                if (closed_ || queue_.size() >= capacity_) {
                    return false;
                }

                queue_.push(std::move(value));
            }

            condition_.notify_one();
            return true;
        }

        std::optional<T> wait_pop() {
            std::unique_lock lock(mutex_);

            condition_.wait(lock, [this] {
                return closed_ || !queue_.empty();
            });

            if (queue_.empty()) {
                return std::nullopt;
            }

            T value = std::move(queue_.front());
            queue_.pop();

            return value;
        }

        void close() {
            {
                std::lock_guard lock(mutex_);
                closed_ = true;
            }

            condition_.notify_all();
        }

        [[nodiscard]]
        bool is_closed() const {
            std::lock_guard lock(mutex_);
            return closed_;
        }

    private:
        const std::size_t capacity_;

        mutable std::mutex mutex_;
        std::condition_variable condition_;
        std::queue<T> queue_;
        bool closed_{false};
    };

} // namespace trading