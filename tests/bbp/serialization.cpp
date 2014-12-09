
/* Copyright (c) 2014, Human Brain Project
 *                     Daniel Nachbaur <daniel.nachbaur@epfl.ch>
 */

#define BOOST_TEST_MODULE bbp_serialization

#include <zeq/zeq.h>
#include <zeqBBP/zeqBBP.h>

#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_CASE(test_serialization)
{
    const std::vector< float > camera( 16, 42 );
    const zeq::Event& event = zeq::bbp::serializeCamera( camera );
    const std::vector< float >& deserialized =
            zeq::bbp::deserializeCamera( event );
    BOOST_CHECK_EQUAL_COLLECTIONS( camera.begin(), camera.end(),
                                   deserialized.begin(), deserialized.end( ));

    unsigned int ids[] = {16,2,77,29};
    const std::vector< unsigned int > selection(
        ids, ids + sizeof(ids) / sizeof(unsigned int) );
    const zeq::Event& selection_event =
        zeq::bbp::serializeSelection( selection );
    const std::vector< unsigned int >& deserialized_selection =
            zeq::bbp::deserializeSelection( selection_event );
    BOOST_CHECK_EQUAL_COLLECTIONS(
        selection.begin(), selection.end(),
        deserialized_selection.begin(), deserialized_selection.end( ));
}
