#ifndef xHeinz_ListsParser_HPP
#define xHeinz_ListsParser_HPP

#include <cstring>

#include <map>
#include <tuple>

#include "ChainGraph.hpp"
#include "Verbosity.hpp"

namespace xHeinz {

 std::tuple< ChainGraph, LabelToGraphNodeMap, LabelToGraphNodeMap >
 ParseChainGraphListsFiles( char const * nodesFilename0
                          , char const * edgesFilename0
                          , char const * cogsFilename
                          , char const * nodesFilename1
                          , char const * edgesFilename1
                          );

} // namespace xHeinz

#endif // xHeinz_ListsParser_HPP

/* vim: set ts=8 sw=2 sts=2 et : */
