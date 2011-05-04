#include "boost/locking_queue.hpp"
#include <queue>
#define BOOST_TEST_MODULE locking_queue_test
#include <boost/test/unit_test.hpp>

using namespace boost;

BOOST_AUTO_TEST_CASE( basic_functionality )
{
    locking_queue<int> lq;
    BOOST_CHECK( lq.empty() == true );
    BOOST_CHECK( lq.size() == 0 );

    int value = 5;

    lq.push(value);
    BOOST_CHECK( lq.empty() == false );
    BOOST_CHECK( lq.size() == 1 );

    int ret = lq.pop();
    BOOST_CHECK( ret == value );
    BOOST_CHECK( lq.empty() == true );
    BOOST_CHECK( lq.size() == 0);
}

BOOST_AUTO_TEST_CASE( copy_constructor )
{
    std::queue<int> q;
    locking_queue<int> lq2(q);

    int value = 5;
    q.push(value);

    locking_queue<int> lq3(q);
    BOOST_CHECK( lq3.empty() == false );
    BOOST_CHECK( lq3.size() == 1 );
    BOOST_CHECK( lq3.pop() == value );
}

BOOST_AUTO_TEST_CASE( pop_non_blocking )
{
    typedef locking_queue<int>::queue_empty empty;

    locking_queue<int> lq;
    BOOST_CHECK_THROW(lq.pop(false), empty);

    // ignore timeout parameter if block is false
    BOOST_CHECK_THROW(lq.pop(false, 5), empty);
}
