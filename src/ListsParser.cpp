#include "ListsParser.hpp"

#include <string>
#include <fstream>
#include <stdexcept>

#include "Verbosity.hpp"
#include "GraphBuilder.hpp"

using namespace std;

namespace xHeinz {

 namespace {

  using GraphIndex  = int;

  using BuilderNode = GraphBuilder::Node;
  using LabelToBuilderNodeMap = std::map< std::string, std::pair< BuilderNode, GraphIndex > >;

  void ParseNodes( char const * filename
                 , GraphIndex   graphIndex
                 , GraphBuilder          & builder
                 , LabelToBuilderNodeMap & labelsToNode
                 , string const & rootName = string{}
                 ) {
    ifstream inFile( filename );
    if ( !inFile ) {
      if ( g_verbosity >= VERBOSE_ESSENTIAL ) {
        cerr << "Error: could not open file \"" << filename << "\" for reading" << endl;
      }
      throw runtime_error( "could not open file \"" + string( filename ) + "\" for reading" );
    }

    string line;
    int lineNumber = 0;
    while ( getline( inFile, line ) ) {
      ++lineNumber;
      if ( line.empty() || line[0] == '#' ) {
        continue;
      }

      stringstream lineStream( line );
      string label;
      lineStream >> label;
      if ( !lineStream.good()) {
        if ( g_verbosity >= VERBOSE_ESSENTIAL ) {
          cerr << "File error: wrong label at line " << lineNumber << endl;
        }
        throw runtime_error( "wrong label" );
      }

      double score;
      lineStream >> score;
      if ( lineStream.bad() || lineStream.fail() ) {
        if ( g_verbosity >= VERBOSE_ESSENTIAL ) {
          cerr << "File error: wrong score at line " << lineNumber << endl;
        }
        throw runtime_error( "wrong score" );
      }

      if ( !lineStream.eof() ) {
        if ( g_verbosity >= VERBOSE_DEBUG ) {
          cerr << "Warning: trailing characters at line " << lineNumber << endl;
        }
      }

      if ( labelsToNode.find( label ) != labelsToNode.end() ) {
        if ( g_verbosity >= VERBOSE_DEBUG ) {
          cerr << "Warning: ignoring duplicate node with label " << label
               << " at line " << lineNumber << endl;
        }
        continue;
      }

      auto x = builder.addNode();
      builder.setLabel( x, label );
      builder.setWeight( x, score );
  //    if ( label == rootName ) {
  //      out.root = x;
  //    }

      labelsToNode.insert( make_pair( label, make_pair( x, graphIndex ) ) );
    }
  }

  void ParseEdges( char const * filename, GraphBuilder & builder, LabelToBuilderNodeMap const & labelsToNode ) {
    ifstream inFile( filename );
    if ( !inFile.good() ) {
      if ( g_verbosity >= VERBOSE_ESSENTIAL ) {
        cerr << "Error: could not open file \"" << filename << "\" for reading" << endl;
      }
      throw runtime_error( "could not open file \"" + string( filename ) + "\" for reading" );
    }


    string line;
    int lineNumber = 0;
    while ( getline( inFile, line ) ) {
      ++lineNumber;
      if ( line.empty() || line[0] == '#' ) {
        continue;
      }

      stringstream lineStream( line );

      string label1;
      lineStream >> label1;
      if ( !lineStream.good() ) {
        if ( g_verbosity >= VERBOSE_ESSENTIAL ) {
          cerr << "File error: wrong first label at line " << lineNumber << endl;
        }
        throw runtime_error( "wrong first label" );
      }
      string label2;
      lineStream >> label2;
      if ( lineStream.bad() || lineStream.fail() ) {
        if ( g_verbosity >= VERBOSE_ESSENTIAL ) {
          cerr << "File error: wrong second label at line " << lineNumber << endl;
        }
        throw runtime_error( "wrong second label" );
      }

      if ( !lineStream.eof() ) {
        if ( g_verbosity >= VERBOSE_DEBUG ) {
          cerr << "Warning: trailing characters at line " << lineNumber << endl;
        }
      }

      auto it1 = labelsToNode.find( label1 );
      if ( it1 == labelsToNode.end() ) {
        if ( g_verbosity >= VERBOSE_DEBUG ) {
          cerr << "Warning: unknown node " << label1 << " at line " << lineNumber
               << ": skipping the edge" << endl;
        }
        continue;
      }

      auto it2 = labelsToNode.find( label2 );
      if ( it2 == labelsToNode.end() ) {
        if ( g_verbosity >= VERBOSE_DEBUG ) {
          cerr << "Warning: unknown node " << label2 << " at line " << lineNumber
               << ": skipping the edge" << endl;
        }
        continue;
      }

      if ( it1->second == it2->second ) {
        if ( g_verbosity >= VERBOSE_DEBUG ) {
          cerr << "Warning: self-loop with node " << label1 << " at line " << lineNumber
               << ": skipping the edge" << endl;
        }
        continue;
      }

      builder.addEdge( it1->second.first, it2->second.first );
    }

  }

  // XXX: Maybe return a ThreeWayGraph for consistency ? Don't change a winning team...
  void CreateFullCogsGraph( ThreeWayGraph & out ) {
    auto & red  = out.red ();
    auto & blue = out.blue();

    for_each( red.nodes(), [&]( Graph::Node const & rgn ) {
      for_each( blue.nodes(), [&]( Graph::Node const & bgn ) {
        auto rn = out.link().addRedNode();
        out.setNodeCorrespondence( rgn, rn );

        auto bn = out.link().addBlueNode();
        out.setNodeCorrespondence( bgn, bn );

        out.link().addEdge( rn, bn );
      });
    });
  }

  // XXX: Maybe return a ThreeWayGraph for consistency ? Don't change a winning team...
  void ParseInParanoidCogs( ThreeWayGraph & out
                          , char const * cogsFilename
                          , LabelToGraphNodeMap const & labels1
                          , LabelToGraphNodeMap const & labels2
                          ) {
    ifstream inFile( cogsFilename );
    if ( !inFile.good() ) {
      if ( g_verbosity >= VERBOSE_ESSENTIAL ) {
        cerr << "Error: could not open file \"" << cogsFilename << "\" for reading" << endl;
      }
      throw runtime_error( "could not open file \"" + string( cogsFilename ) + "\" for reading" );
    }

    string line;
    int lineNumber = 0;
    while ( getline( inFile, line ) ) {
      ++lineNumber;
      if ( line.empty() || line[0] == '#' ) {
        continue;
      }

      stringstream lineStream( line );

      string label1;
      lineStream >> label1;
      if ( !lineStream.good() ) {
        if ( g_verbosity >= VERBOSE_ESSENTIAL ) {
          cerr << "File error: wrong first label at line " << lineNumber << endl;
        }
        throw runtime_error( "wrong first label" );
      }
      string label2;
      lineStream >> label2;
      if ( lineStream.bad() || lineStream.fail() ) {
        if ( g_verbosity >= VERBOSE_ESSENTIAL ) {
          cerr << "File error: wrong second label at line " << lineNumber << endl;
        }
        throw runtime_error( "wrong second label" );
      }

      if ( !lineStream.eof() ) {
        if ( g_verbosity >= VERBOSE_DEBUG ) {
          cerr << "Warning: trailing characters at line " << lineNumber << endl;
        }
      }

      auto it1 = labels1.find( label1 );
      if ( it1 == labels1.end() ) {
        if ( g_verbosity >= VERBOSE_DEBUG ) {
          cerr << "Warning: unknown node " << label1 << " at line " << lineNumber
               << ": skipping the edge" << endl;
        }
        continue;
      }

      auto it2 = labels2.find( label2 );
      if ( it2 == labels2.end() ) {
        if ( g_verbosity >= VERBOSE_DEBUG ) {
          cerr << "Warning: unknown node " << label2 << " at line " << lineNumber
               << ": skipping the edge" << endl;
        }
        continue;
      }

      Graph::Node n1 = it1->second;
      Graph::Node n2 = it2->second;
      auto rn = out.toRedNode( n1 );
      if ( rn == lemon::INVALID ) {
        rn = out.link().addRedNode();
        out.setNodeCorrespondence( n1, rn );
      }

      auto bn = out.toBlueNode( n2 );
      if ( bn == lemon::INVALID ) {
        bn = out.link().addBlueNode();
        out.setNodeCorrespondence( n2, bn );
      }

      out.link().addEdge( rn, bn );
    }
    if ( g_verbosity >= VERBOSE_ESSENTIAL ) {
      cout << "-- Parsed " << cogsFilename << ": |V_1| = "
           << lemon::countRedNodes ( out.link() ) << ", |V_2| = "
           << lemon::countBlueNodes( out.link() ) << ", |E| = "
           << lemon::countEdges    ( out.link() )
           << endl;
    }
  }

  LabelToGraphNodeMap ConstructLabelMap( Graph const & g ) {
    LabelToGraphNodeMap retLabels;

    for_each( g.nodes(), [&]( Graph::Node const & n ) {
      retLabels[g.label( n )] = n;
    });

    return retLabels;
  }

  Graph ParseGraphListsFiles( GraphIndex index
                            , char const * nodesFilename, char const * edgesFilename
                            ) {
    GraphBuilder          builder;
    LabelToBuilderNodeMap labels;
    ParseNodes( nodesFilename, index, builder, labels );
    ParseEdges( edgesFilename,        builder, labels );
    auto g = builder.build();
    if ( g_verbosity >= VERBOSE_ESSENTIAL ) {
      int numPosNodes = 0;
      for ( auto n : g.nodes() ) {
        if ( g.weight(n) >= 0 ) {
          numPosNodes++;
        }
      }
      cout << "-- Parsed " << nodesFilename << ": "
           << "|V| = " << g.numNodes()
           << " (" << numPosNodes << " positive)"
           << ", |E| = " << g.numEdges()
           << ", # components = " << g.numComponents()
           << endl;
    }
    return g;
  }

 } // namespace <anonymous>

 tuple< ChainGraph, LabelToGraphNodeMap, LabelToGraphNodeMap >
 ParseChainGraphListsFiles( char const * nodesFilename0
                          , char const * edgesFilename0
                          , char const * cogsFilename
                          , char const * nodesFilename1
                          , char const * edgesFilename1
                          ) {
   ChainGraph ret;
   ret.addGraph( ParseGraphListsFiles( 0, nodesFilename0, edgesFilename0 ) );

   auto twg = ret.addGraph(
     ParseGraphListsFiles( 1, nodesFilename1, edgesFilename1 )
   );

   LabelToGraphNodeMap redGraphLabels  = ConstructLabelMap( twg->red () );
   LabelToGraphNodeMap blueGraphLabels = ConstructLabelMap( twg->blue() );
   if ( strcmp( cogsFilename, "" ) != 0 ) {
     ParseInParanoidCogs( *twg, cogsFilename, redGraphLabels, blueGraphLabels );
   } else {
     CreateFullCogsGraph( *twg );
   }

   return make_tuple( move( ret ), move( redGraphLabels ), move( blueGraphLabels ) );
 }

} // namespace xHeinz

/* vim: set ts=8 sw=2 sts=2 et : */
