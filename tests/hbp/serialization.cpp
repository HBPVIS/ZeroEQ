
/* Copyright (c) 2014-2015, Human Brain Project
 *                          Daniel Nachbaur <daniel.nachbaur@epfl.ch>
 *                          Grigori Chevtchenko <grigori.chevtchenko@epfl.ch>
 */

#define BOOST_TEST_MODULE hbp_serialization

#include <zeq/hbp/vocabulary.h>
#include <zeq/zeq.h>

#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_CASE( cameraEvent )
{
    const std::vector< float > camera( 16, 42 );
    const zeq::Event& event = zeq::hbp::serializeCamera( camera );
    const std::vector< float >& deserialized =
            zeq::hbp::deserializeCamera( event );
    BOOST_CHECK_EQUAL_COLLECTIONS( camera.begin(), camera.end(),
                                   deserialized.begin(), deserialized.end( ));
}

BOOST_AUTO_TEST_CASE( frameEvent )
{
    {
        const zeq::hbp::data::Frame frame( 1, 2, 3, 0 );
        const zeq::Event& event = zeq::hbp::serializeFrame( frame );
        const zeq::hbp::data::Frame& out = zeq::hbp::deserializeFrame( event );

        BOOST_CHECK_EQUAL( frame, out );
    }
    {
        const zeq::hbp::data::Frame frame( 3, 2, 1, -1 );
        const zeq::Event& event = zeq::hbp::serializeFrame( frame );
        const zeq::hbp::data::Frame& out = zeq::hbp::deserializeFrame( event );
        const zeq::hbp::data::Frame expected( 3, 3, 3, -1 );

        BOOST_CHECK_EQUAL( expected, out );
    }
}

BOOST_AUTO_TEST_CASE( selectionsEvent )
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

BOOST_AUTO_TEST_CASE( toggleRequestEvent )
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

BOOST_AUTO_TEST_CASE( lookupTable1D )
{
    const std::vector< uint8_t > lut( 1024 );
    const zeq::Event& lookupTableEvent = zeq::hbp::serializeLookupTable1D( lut );
    const std::vector< uint8_t >& deserializedLut =
            zeq::hbp::deserializeLookupTable1D( lookupTableEvent );
    BOOST_CHECK_EQUAL_COLLECTIONS(
        lut.begin(), lut.end(), deserializedLut.begin(), deserializedLut.end());
}

BOOST_AUTO_TEST_CASE( imageJPEGEvent )
{
    const size_t size = 24;
    const uint8_t imageJPEGData[ size ] = { 13, 11, 17, 19, 34, 73, 25, 24, 36,
                                            74, 21, 56, 78, 23, 42, 23, 24, 42,
                                            74, 32, 12, 35, 35, 13 };
    zeq::hbp::data::ImageJPEG image( size, &imageJPEGData[0] );

    const zeq::Event& imageEvent = zeq::hbp::serializeImageJPEG( image );
    const zeq::hbp::data::ImageJPEG& deserializedImage =
            zeq::hbp::deserializeImageJPEG( imageEvent );
    BOOST_CHECK_EQUAL( image.getSizeInBytes(),
                       deserializedImage.getSizeInBytes( ));
    BOOST_CHECK_EQUAL_COLLECTIONS( imageJPEGData, imageJPEGData + size,
                                   deserializedImage.getDataPtr(),
                                   deserializedImage.getDataPtr() + size );
}

BOOST_AUTO_TEST_CASE( cellSetBinaryOp )
{
  zeq::hbp::data::CellSetBinaryOp cellSet ({ 0, 2, 4, 6 },
                                           { 1, 3, 5, 7 },
                                           zeq::hbp::CellSetOpType::
                                           CellSetOpType_SYNAPTIC_PROJECTION);
//  cellSet.first = ;
//  cellSet.second = ;
//  cellSet.operation =
//      zeq::hbp::CellSetOpType::CellSetOpType_SYNAPTIC_PROJECTION;

  const zeq::Event& cellSetBinaryOpEvent =
      zeq::hbp::serializeCellSetBinaryOp( cellSet );

  zeq::hbp::data::CellSetBinaryOp deserializedCellSetBinaryOp =
      zeq::hbp::deserializeCellSetBinaryOp( cellSetBinaryOpEvent );

  BOOST_CHECK_EQUAL( cellSet.operation,
                     deserializedCellSetBinaryOp.operation);

  BOOST_CHECK_EQUAL_COLLECTIONS( cellSet.first.begin( ),
                                 cellSet.first.end( ),
                                 deserializedCellSetBinaryOp.first.begin( ),
                                 deserializedCellSetBinaryOp.first.end( ));

  BOOST_CHECK_EQUAL_COLLECTIONS( cellSet.second.begin( ),
                                 cellSet.second.end( ),
                                 deserializedCellSetBinaryOp.second.begin( ),
                                 deserializedCellSetBinaryOp.second.end( ));
}
