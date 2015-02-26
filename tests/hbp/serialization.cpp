
/* Copyright (c) 2014-2015, Human Brain Project
 *                     Daniel Nachbaur <daniel.nachbaur@epfl.ch>
 *                     Grigori Chevtchenko <grigori.chevtchenko@epfl.ch>
 */

#define BOOST_TEST_MODULE hbp_serialization

#include <zeq/hbp/vocabulary.h>
#include <zeq/hbp/imageJPEG_generated.h>
#include <zeq/zeq.h>

#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_CASE( test_cameraEvent )
{
    const std::vector< float > camera( 16, 42 );
    const zeq::Event& event = zeq::hbp::serializeCamera( camera );
    const std::vector< float >& deserialized =
            zeq::hbp::deserializeCamera( event );
    BOOST_CHECK_EQUAL_COLLECTIONS( camera.begin(), camera.end(),
                                   deserialized.begin(), deserialized.end( ));
}

BOOST_AUTO_TEST_CASE( test_selectionsEvent )
{

    unsigned int ids[] = {16,2,77,29};
    const std::vector< unsigned int > selection(
        ids, ids + sizeof(ids) / sizeof(unsigned int) );
    const zeq::Event& selectionEvent =
        zeq::hbp::serializeSelectedIDs( selection );
    const std::vector< unsigned int >& deserializedSelection =
            zeq::hbp::deserializeSelectedIDs( selectionEvent );
    BOOST_CHECK_EQUAL_COLLECTIONS(
        selection.begin(), selection.end(),
        deserializedSelection.begin(), deserializedSelection.end( ));
}

BOOST_AUTO_TEST_CASE( test_toggleRequestEvent )
{
    unsigned int ids[] = {16,2,77,29};
    const std::vector< unsigned int > selection(
        ids, ids + sizeof(ids) / sizeof(unsigned int) );
    const zeq::Event& toggleRequest_event =
        zeq::hbp::serializeToggleIDRequest( selection );
    const std::vector< unsigned int >& deserialized_toggleRequest =
            zeq::hbp::deserializeToggleIDRequest( toggleRequest_event );
    BOOST_CHECK_EQUAL_COLLECTIONS(
        selection.begin(), selection.end(),
        deserialized_toggleRequest.begin(), deserialized_toggleRequest.end( ));
}

BOOST_AUTO_TEST_CASE( test_imageJPEGEvent )
{
    uint8_t imageJPEGData[ 24u ] = { 13, 11, 17, 19, 34, 73, 25, 24, 36, 74, 21, 56,
                                     78, 23, 42, 23, 24, 42, 74, 32, 12, 35, 35, 13 };
    zeq::hbp::data::ImageJPEG image( 24u, &imageJPEGData[0] );

    const zeq::Event& imageEvent = zeq::hbp::serializeImageJPEG( image );
    zeq::hbp::data::ImageJPEG deserializedImage = zeq::hbp::deserializeImageJPEG( imageEvent );
    BOOST_CHECK_EQUAL( image.getSizeInBytes(), deserializedImage.getSizeInBytes() );
    BOOST_CHECK_EQUAL_COLLECTIONS( imageJPEGData, imageJPEGData + 24u,
                                   deserializedImage.getDataPtr(),
                                   deserializedImage.getDataPtr() + 24u );
}

BOOST_AUTO_TEST_CASE( test_requestEvent )
{
    lunchbox::uint128_t eventType( zeq::hbp::EVENT_IMAGEJPEG );
    const zeq::Event& requestEvent = zeq::hbp::serializeRequest( eventType );
    lunchbox::uint128_t deserializedEventType = zeq::hbp::deserializeRequest( requestEvent );
    BOOST_CHECK_EQUAL( eventType, deserializedEventType );
}

