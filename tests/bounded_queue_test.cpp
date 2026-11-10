#include "market_data/bounded_queue.hpp"

#include <cassert>
#include <iostream>
#include <thread>

int main() {
    trading::BoundedQueue<int> queue(2);

    // Test normal push/pop.
    assert(queue.try_push(10));
    assert(queue.try_push(20));

    auto first = queue.wait_pop();
    assert(first.has_value());
    assert(*first == 10);

    // Test that closing wakes a waiting consumer.
    queue.close();

    auto second = queue.wait_pop();
    assert(second.has_value());
    assert(*second == 20);

    auto empty = queue.wait_pop();
    assert(!empty.has_value());

    // Push after close must fail.
    assert(!queue.try_push(30));

    std::cout << "All BoundedQueue tests passed!\n";

    return 0;
}
