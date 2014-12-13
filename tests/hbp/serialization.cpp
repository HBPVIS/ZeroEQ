
/* Copyright (c) 2014, Human Brain Project
 *                     Daniel Nachbaur <daniel.nachbaur@epfl.ch>
 */

#define BOOST_TEST_MODULE hbp_serialization

#include <zeq/zeq.h>
#include <zeqHBP/zeqHBP.h>

#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_CASE(test_serialization)
{
    const std::vector< float > camera( 16, 42 );
    const zeq::Event& event = zeq::hbp::serializeCamera( camera );
    const std::vector< float >& deserialized =
            zeq::hbp::deserializeCamera( event );
    BOOST_CHECK_EQUAL_COLLECTIONS( camera.begin(), camera.end(),
                                   deserialized.begin(), deserialized.end( ));

    unsigned int ids[] = {16,2,77,29};
    const std::vector< unsigned int > selection(
        ids, ids + sizeof(ids) / sizeof(unsigned int) );
    const zeq::Event& selection_event =
        zeq::hbp::serializeSelectedIDs( selection );
    const std::vector< unsigned int >& deserialized_selection =
            zeq::hbp::deserializeSelectedIDs( selection_event );
    BOOST_CHECK_EQUAL_COLLECTIONS(
        selection.begin(), selection.end(),
        deserialized_selection.begin(), deserialized_selection.end( ));

    const zeq::Event& toggleRequest_event =
        zeq::hbp::serializeToggleIDRequest( selection );
    const std::vector< unsigned int >& deserialized_toggleRequest =
            zeq::hbp::deserializeToggleIDRequest( toggleRequest_event );
    BOOST_CHECK_EQUAL_COLLECTIONS(
        selection.begin(), selection.end(),
        deserialized_toggleRequest.begin(), deserialized_toggleRequest.end( ));
}
