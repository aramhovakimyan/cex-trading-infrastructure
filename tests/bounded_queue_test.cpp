#include "market_data/bounded_queue.hpp"

#include <cassert>
#include <iostream>

int main() {
    trading::BoundedQueue<int> queue(2);

    const bool pushed_first = queue.try_push(10);
    const bool pushed_second = queue.try_push(20);

    assert(pushed_first);
    assert(pushed_second);

    auto first = queue.wait_pop();

    assert(first.has_value());
    assert(*first == 10);

    queue.close();

    auto second = queue.wait_pop();

    assert(second.has_value());
    assert(*second == 20);

    auto empty = queue.wait_pop();

    assert(!empty.has_value());

    const bool pushed_after_close = queue.try_push(30);

    assert(!pushed_after_close);

    std::cout << "All BoundedQueue tests passed!\n";

    return 0;
}