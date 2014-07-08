#include <iostream>
#include <stdexcept>

#include <lemon/arg_parser.h>
#include <lemon/time_measure.h>

#include "Verbosity.hpp"
#include "ListsParser.hpp"
#include "Solver.hpp"

#include "Config.hpp"

using namespace std;
using namespace xHeinz;

int main( int argc, char * argv[] ) {
  solver::Config config;
  int verbosityLevel = 2;
  int connectivityType = 0;
  string warmFilename;
  string outFilename;
  string errFilename;
  string logFilename;
  string solFilename;
  string nodeFile[2];
  string edgeFile[2];
  string cogFile;

  lemon::ArgParser ap( argc, argv );
  ap.boolOption( "version", "Show version number" )
    .refOption( "n1", "Graph 1: node file", nodeFile[0], false )
    .refOption( "e1", "Graph 1: edge file", edgeFile[0], false )
    .refOption( "n2", "Graph 2: node file", nodeFile[1], false )
    .refOption( "e2", "Graph 2: edge file", edgeFile[1], false )
    .refOption( "cog", "COG file", cogFile, false )
    .refOption( "a", "Conservation ratio threshold (default: 0.6)"
              , config.connectivityPercentage, false
              )
    .synonym("alpha", "a" )
#if 0
    .refOption( "b1", "Graph 1: positive node ratio threshold (default: 0.0) TODO"
              , config.positivePercentage[0], false
              )
    .synonym("beta1", "b1" )
    .refOption( "b2", "Graph 2: positive node ratio threshold (default: 0.0) TODO"
              , config.positivePercentage[1], false
              )
    .synonym("beta2", "b2" )
#endif
    .refOption( "w", "Specifies the type of mapping weighting:\n"
                     "         0 - Sum mapped nodes (default)\n"
                     "         1 - Sum mapped node weights\n"
                     "         2 - Sum positive mapped nodes\n"
                     "         3 - Sum positive mapped node weights"
              , connectivityType, false
              )
    .synonym( "omega", "w" )
    .refOption( "t", "Time limit (in seconds, default: ∞)"
              , config.timeLimit, false
              )
    .refOption( "rt", "Time limit for the root node (in seconds, default: ∞)"
              , config.rootTimeLimit, false
              )
    .refOption( "warm", "xHeinz output file used as a warm start (must be solved with >= alpha)"
              , warmFilename, false
              )
    .refOption( "out-file", "Standard output file (default to stdout)", outFilename, false )
    .refOption( "err-file", "Error output file (default to stderr)"   , errFilename, false )
    .refOption( "log-file", "Log output file (default to stderr)"     , logFilename, false )
    .refOption( "sol-file", "Solution output file"                    , solFilename, false )
    .refOption( "s", "Size constraint (default -1, unconstrained)"    , config.size, false )
    .refOption( "threads", "Specifies number of threads (default: 1)", config.numThreads, false )
    .refOption( "v", "Specifies the verbosity level:\n"
                     "         0 - No output\n"
                     "         1 - Only necessary output\n"
                     "         2 - More verbose output (default)\n"
                     "         3 - Debug output"
              , verbosityLevel, false)
    .synonym( "verbosity", "v" )
    .parse();

  ofstream outFile;
  streambuf * coutBuf = cout.rdbuf();
  if ( !outFilename.empty() ) {
    outFile.open( outFilename );
    cout.rdbuf( outFile.rdbuf() );   // redirect cout to outFilename
  }

  ofstream errFile;
  streambuf * cerrBuf = cerr.rdbuf();
  if ( !errFilename.empty() ) {
    errFile.open( errFilename );
    cerr.rdbuf( errFile.rdbuf() );   // redirect cerr to errFilename
  }

  ofstream logFile;
  streambuf * clogBuf = clog.rdbuf();
  if ( !logFilename.empty() ) {
    logFile.open( logFilename );
    clog.rdbuf( logFile.rdbuf() );   // redirect clog to logFilename
  }

  if ( verbosityLevel < 0 || verbosityLevel > 3 ) {
    cerr << "Invalid verbosity level" << endl;
    return 1;
  }
  g_verbosity = static_cast< VerbosityLevel >(verbosityLevel);

  if ( connectivityType < 0 || connectivityType > 3 ) {
    cerr << "Invalid omega (mapping weighting)" << endl;
    return 1;
  }
  config.connectivityType =
    static_cast< solver::Config::ConnectivityType >(connectivityType);

  if ( ap.given( "version" ) ) {
    cout << "Version number: " << XHEINZ_VERSION << endl;
    return 0;
  }

  if ( nodeFile[0].empty() || edgeFile[0].empty()
    || cogFile.empty()
    || nodeFile[1].empty() || edgeFile[1].empty()
     ) {
    cerr << "Invalid input files" << endl;
    return 1;
  }

  auto const & parsedGraph = ParseChainGraphListsFiles( nodeFile[0].c_str()
                                                      , edgeFile[0].c_str()
                                                      , cogFile.c_str()
                                                      , nodeFile[1].c_str()
                                                      , edgeFile[1].c_str()
                                                      );
  auto const & inputGraph = get< 0 >( parsedGraph );

  cout << "-- Solver configuration:\n" << config << endl;
  Solver solver( inputGraph, config );

  if ( !warmFilename.empty() ) {
    lemon::Timer t;
    try {
      solver.warm( solver::InputSolution( warmFilename.c_str()
                                        , get< 1 >( parsedGraph )
                                        , get< 2 >( parsedGraph )
                                        )
                 , warmFilename.c_str()
                 );
      clog << "Warm start installed in " << t.realTime() << "s" << endl;
    } catch ( exception const & ) {
      clog << "Skipping warm start" << endl;
    }
  }

  lemon::Timer t;
  auto sol = solver.solve();
  if ( sol ) {
    cout << "-- Solution:\n" << *sol << endl;
    if ( !solFilename.empty() ) {
      ofstream solFile( solFilename.c_str() );
      solFile << *sol << endl;
    }
  } else {
    cout << "-- No solution" << endl;
  }
  clog << "Solved in " << t.realTime() << "s" << endl;

  clog.rdbuf( clogBuf );
  cerr.rdbuf( cerrBuf );
  cout.rdbuf( coutBuf );

  return 0;
}

/* vim: set ts=8 sw=2 sts=2 et : */
