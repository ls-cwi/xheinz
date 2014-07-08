#include "Solver/Solution.hpp"

#include <fstream>
#include <ostream>
#include <stdexcept>

#include "Verbosity.hpp"

using namespace std;

namespace xHeinz {
namespace solver {

 ostream & operator<<( ostream & out, OutputSolution const & sol ) {
   out << "-- Alpha = " << sol.alpha  << " (";
   bool first = true;
   for_each( sol.graphsSolutions, [&]( OutputSolution::GraphSolution const & gs ) {
     if ( !first ) {
       out << ", ";
     } else {
       first = false;
     }
     out << get< 2 >( gs );
   });
   out << ")\n-- Score = " << sol.weight << " (";
   first = true;
   for_each( sol.graphsSolutions, [&]( OutputSolution::GraphSolution const & gs ) {
     if ( !first ) {
       out << ", ";
     } else {
       first = false;
     }
     out << get< 1 >( gs );
   });
   out << ")";

   for ( int i = 0, e = sol.graphsSolutions.size(); i != e; ++i ) {
     auto const & graphSol = sol.graphsSolutions[i];
     out << "\n#" << i << " graph (alpha="
         << get< 2 >( graphSol ) << ", score="
         << get< 1 >( graphSol ) << ") nodes:";
     for_each( get< 0 >( graphSol ), [&]( std::pair< Graph::Node, bool > const & p ) {
       auto const & n = p.first;
       out << '\n' << n.component().label( n )
           << '\t' << n.component().weight( n );
       if ( p.second ) {
           out << "\t*";
       }
     });
   }
   return out;
 }

 ostream & operator<<( ostream & out, InputSolution const & sol ) {
   for ( int i = 0, e = sol.graphsSolutions.size(); i != e; ++i ) {
     auto const & nodesSet = sol.graphsSolutions[i];
     out << "\n#" << i << " graph nodes:";
     for_each( nodesSet, [&]( Graph::Node const & n ) {
       out << '\n' << n.component().label ( n );
     });
   }
   return out;
 }

 InputSolution::InputSolution( char const * filename
                             , LabelToGraphNodeMap const & labels0
                             , LabelToGraphNodeMap const & labels1
                             ) {
   ifstream inFile( filename );
   if ( !inFile.good() ) {
     if ( g_verbosity >= VERBOSE_ESSENTIAL ) {
       cerr << "Error: could not open file \"" << filename << "\" for reading" << endl;
     }
     throw runtime_error( "could not open file \"" + string( filename ) + "\" for reading" );
   }

   cout << "-- Parsing warm solution in \"" << filename << "\" (graph: ";
   cout.flush();

   string line;
   int graphIndex = -1;
   bool first = true;
   int lineNumber = 0;
   while ( getline( inFile, line ) ) {
     ++lineNumber;
     if ( line.size() < 2 || (line[0] == '-' && line[1] == '-') ) {
       continue;
     }

     if ( line[0] == '#' ) {
       stringstream stream( line.substr( 1 ) );
       stream >> graphIndex;
       if ( !first ) {
         cout << ", ";
       } else {
         first = false;
       }
       cout << graphIndex;
       cout.flush();
       graphsSolutions.emplace_back();

       if ( graphIndex < 0 || graphIndex > 1 ) {
         cerr << "Invalid graph index in warm start file, skipping section" << endl;
         graphIndex = -1;
       }
     } else {
       stringstream stream( line );
       string label;
       stream >> label;
       if ( !stream.good() || label.empty() ) {
         if ( g_verbosity >= VERBOSE_ESSENTIAL ) {
           cerr << "File error: wrong label at line " << lineNumber << endl;
         }
         throw runtime_error( "wrong label in warm file" );
       }
       LabelToGraphNodeMap::const_iterator it;
       if ( graphIndex == 0 ) {
         it = labels0.find( label );
         if ( it == labels0.end() ) {
           cerr << "Invalid label in warm file at line " << lineNumber
                << ", skipping" << endl;
           continue;
         }
       } else if ( graphIndex == 1 ) {
         it = labels1.find( label );
         if ( it == labels1.end() ) {
           cerr << "Invalid label in warm file at line " << lineNumber
                << ", skipping" << endl;
           continue;
         }
       } else {
         assert( !"Invalid graph number in warm start solution" );
       }

       assert( !graphsSolutions.empty() );
       graphsSolutions.back().push_back( it->second );
     }
   }
   cout << ")" << endl;
 }

} // namespace solver
} // namespace xHeinz

/* vim: set ts=8 sw=2 sts=2 et : */
