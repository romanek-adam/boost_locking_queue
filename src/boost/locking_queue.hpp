/**
 * @file locking_queue.hpp Thread-safe queue-like container template class.
 * @author Adam Romanek <a.romanek -at- no.pl>
 */

// Copyright (c) Adam Romanek 2011. Use, modification and distribution is
// subject to the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(BOOST_LOCKING_QUEUE_HPP)
#define BOOST_LOCKING_QUEUE_HPP

#include <queue>
#include <boost/thread/mutex.hpp>                          // for boost::mutex
#include <boost/thread/condition_variable.hpp>             // for boost::condition_variable
#include <boost/date_time/posix_time/posix_time_types.hpp> // for boost::posix_time

namespace boost {

/**
 * Queue-like class with full locking support. Ideal for multi-threaded
 * purposes.
 *
 * @tparam T         type that is to be stored in locking queue
 * @tparam Container container type that will serve as underlying container
 *                   for queue values
 */
template<
    typename T,
    typename Container = std::queue<T>
>
class locking_queue {
protected:
    /**
     * Lock guard type.
     */
    typedef boost::lock_guard<boost::mutex> lock_guard;

    /**
     * Unique lock type.
     */
    typedef boost::unique_lock<boost::mutex> unique_lock;

public:
    /**
     * Empty queue exception type.
     */
    class queue_empty {};

    /**
     * Container type.
     */
    typedef Container container_type;

    /**
     * Value type.
     */
    typedef typename Container::value_type value_type;

    /**
     * Size type.
     */
    typedef typename Container::size_type size_type;

    /**
     * Constructs new locking queue.
     */
    locking_queue() : container(), mutex(), non_empty_cond() {}

    /**
     * Constructs new locking queue with the copy of the contents of
     * other container.
     *
     * @param[in] other other container to copy the contents from
     *                  when constructing new locking queue.
     */
    explicit locking_queue(const container_type& other)
        : container(other), mutex(), non_empty_cond() {}

    /**
     * Checks whether the queue is empty.
     * @return true if the queue is empty, false otherwise
     */
    bool empty() const {
        lock_guard guard(mutex);
        return container.empty();
    }

    /**
     * Returns the number of elements in the queue.
     * @return the number of elements in the queue.
     */
    size_type size() const {
        lock_guard guard(mutex);
        return container.size();
    }

    /**
     * Pops an element from the front of the queue and returns it to the
     * caller.
     *
     * If block is true and timeout is a positive integral value then wait for
     * the element to be available in the queue for at most timeout seconds
     * and then throw @a empty exception.
     * 
     * @param[in] block if true, then blocks until an element is available
     * @param[in] timeout number of seconds to wait for the element to be
     *                    available
     * @throws locking_queue::empty in case no elements were available
     * @return the first element of the queue
     */
    value_type pop(bool block = false, int timeout = 0) {
        unique_lock lock(mutex);

        if (block) {
            while (!container.empty()) {
                if (timeout > 0) {
                    if (!non_empty_cond.timed_wait(
                                lock, boost::posix_time::seconds(timeout))) {
                        throw queue_empty();
                    }
                } else {
                    non_empty_cond.wait(lock);
                }
            }
        } else {
            if (container.empty()) {
                throw queue_empty();
            }
        }

        value_type element(container.front());
        container.pop();

        return element;
    }

    /**
     * Pushes a new element to the back of the queue.
     * @param[in] element element to be pushed to the back of the queue
     */
    void push(const value_type& element) {
        lock_guard guard(mutex);
        container.push(element);
        non_empty_cond.notify_one();
    }

protected:
    /**
     * Underlying container that serves as a storage for the queue elements.
     */
    container_type container;

    /**
     * Container access mutex.
     */
    mutable boost::mutex mutex;

    /**
     * Non empty condition variable.
     */
    mutable boost::condition_variable non_empty_cond;
};

} // namespace boost

#endif /* BOOST_LOCKING_QUEUE_HPP */
