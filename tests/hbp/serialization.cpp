
/* Copyright (c) 2014-2015, Human Brain Project
 *                          Daniel Nachbaur <daniel.nachbaur@epfl.ch>
 *                          Grigori Chevtchenko <grigori.chevtchenko@epfl.ch>
 */

#define BOOST_TEST_MODULE hbp_serialization

#include <zeroeq/hbp/vocabulary.h>
#include <zeroeq/zeroeq.h>

#include <boost/test/unit_test.hpp>

typedef std::vector< uint32_t > uint32_ts;

BOOST_AUTO_TEST_CASE( selectionsEvent )
{
    unsigned int ids[] = {16,2,77,29};
    const std::vector< unsigned int > selection(
        ids, ids + sizeof(ids) / sizeof(unsigned int) );
    const zeroeq::Event& selectionEvent =
        zeroeq::hbp::serializeSelectedIDs( selection );
    const std::vector< unsigned int >& deserializedSelection =
            zeroeq::hbp::deserializeSelectedIDs( selectionEvent );
    BOOST_CHECK_EQUAL_COLLECTIONS(
        selection.begin(), selection.end(),
        deserializedSelection.begin(), deserializedSelection.end( ));
}

BOOST_AUTO_TEST_CASE( toggleRequestEvent )
{
    unsigned int ids[] = {16,2,77,29};
    const std::vector< unsigned int > selection(
        ids, ids + sizeof(ids) / sizeof(unsigned int) );
    const zeroeq::Event& toggleRequest_event =
        zeroeq::hbp::serializeToggleIDRequest( selection );
    const std::vector< unsigned int >& deserialized_toggleRequest =
            zeroeq::hbp::deserializeToggleIDRequest( toggleRequest_event );
    BOOST_CHECK_EQUAL_COLLECTIONS(
        selection.begin(), selection.end(),
        deserialized_toggleRequest.begin(), deserialized_toggleRequest.end( ));
}

BOOST_AUTO_TEST_CASE( cellSetBinaryOp )
{
  zeroeq::hbp::data::CellSetBinaryOp cellSet (
      { 0, 2, 4, 6 }, { 1, 3, 5, 7 },
      zeroeq::hbp::CELLSETOP_SYNAPTIC_PROJECTIONS );

  const zeroeq::Event& cellSetBinaryOpEvent =
      zeroeq::hbp::serializeCellSetBinaryOp( cellSet.first, cellSet.second,
                                          cellSet.operation );

  zeroeq::hbp::data::CellSetBinaryOp deserializedCellSetBinaryOp =
      zeroeq::hbp::deserializeCellSetBinaryOp( cellSetBinaryOpEvent );

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
