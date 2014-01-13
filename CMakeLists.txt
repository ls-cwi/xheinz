cmake_minimum_required( VERSION 2.8 )

project( xHeinz )

set( xHeinz_Hdr
  src/CppUtils.hpp
  src/Math.hpp

  src/BaseGraphTypes.hpp
  src/Graph.hpp
  src/GraphBuilder.hpp
  src/GraphComponent.hpp
  src/ChainGraph.hpp
  src/ThreeWayGraph.hpp

  src/Verbosity.hpp
  src/ListsParser.hpp

  src/Solver.hpp

  src/Solver/Config.hpp
  src/Solver/Solution.hpp
  src/Solver/BkMaxFlow.hpp
  src/Solver/BkCallback.hpp
  src/Solver/ExtChainGraph.hpp
  src/Solver/GraphVariables.hpp
  src/Solver/MinCutDigraph.hpp
  src/Solver/RoundingCallback.hpp
  src/Solver/MinCutDigraphComponent.hpp
)
set( xHeinz_Src
  src/xHeinz.cpp

  src/Verbosity.cpp
  src/ListsParser.cpp

  src/Solver.cpp

  src/Solver/Config.cpp
  src/Solver/Solution.cpp
  src/Solver/BkMaxFlow.cpp
  src/Solver/BkCallback.cpp
  src/Solver/RoundingCallback.cpp
  src/Solver/MinCutDigraphComponent.cpp
)

set( XHEINZ_EPSILON 1e-10 CACHE STRING "xHeinz epsilon (double type) value" )

set( GitCommand1 "symbolic-ref HEAD 2> /dev/null | cut -b 12-" )
execute_process( COMMAND git symbolic-ref HEAD  WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
                 COMMAND tail -c -7 COMMAND tr -d "\\n" OUTPUT_VARIABLE GitBranch )
set( GitCommand2 "log --pretty=format:\\\"%h\\\" -1" )
execute_process( COMMAND git log --pretty=format:%h WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
                 COMMAND head -n 1  COMMAND tr -d "\\n" OUTPUT_VARIABLE GitHashtag )
set( XHEINZ_VERSION \"${GitBranch}-${GitHashtag}\" )

set( Boost_USE_STATIC_LIBS ON )
find_package( Boost 1.49.0 REQUIRED )

set( LIBLEMON_ROOT "$ENV{HOME}/lemon" CACHE PATH "Additional search directory for lemon library" )

if( NOT CPLEX_INC_DIR OR NOT CPLEX_LIB_DIR OR NOT CONCERT_INC_DIR OR NOT CONCERT_LIB_DIR )
	set( CPLEX_AUTODETECTION_PREFIXES "$ENV{HOME}/ILOG;/opt/ibm/ILOG" CACHE STRING "CPLEX automatic lookup directories" )
	foreach( CplexInstallPrefix ${CPLEX_AUTODETECTION_PREFIXES} )
		file( GLOB_RECURSE CplexLibraryPaths ${CplexInstallPrefix}/*/cplex/lib/*/libcplex.a )
		if( CplexLibraryPaths )
			list( LENGTH CplexLibraryPaths NumPath )
			if( NumPath GREATER 1 )
				message( WARNING "Found multiple CPLEX installations in ${CplexInstallPrefix}, automatically selecting the first one, please set {CPLEX|CONCERT}_{INC|LIB}_DIR manually if this is not the right selection" )
			else()
				message( "Found CPLEX's installation in ${CPlexInstallPrefix}" )
			endif()
			list( GET CplexLibraryPaths 0 CplexLibraryPath )

			set( MyRegex "${CplexInstallPrefix}/(.*)/cplex/?/lib/(.*)/libcplex.a" )
			string( REGEX MATCH "${MyRegex}" ValidatedCplexLibPath ${CplexLibraryPath} )
			if( NOT ValidatedCplexLibPath )
				message( WARNING "Unable to recognize ${CplexLibraryPath} as a valid CPLEX path" )
			else()
				string( REGEX REPLACE "${MyRegex}" "\\1" CplexHome ${ValidatedCplexLibPath} )
				string( REGEX REPLACE "${MyRegex}" "\\2" CplexArch ${ValidatedCplexLibPath} )

				set( CPLEX_INC_DIR   "${CplexInstallPrefix}/${CplexHome}/cplex/include"            CACHE PATH "CPLEX's include dir" )
				set( CPLEX_LIB_DIR   "${CplexInstallPrefix}/${CplexHome}/cplex/lib/${CplexArch}"   CACHE PATH "CPLEX's library dir" )
				set( CONCERT_INC_DIR "${CplexInstallPrefix}/${CplexHome}/concert/include"          CACHE PATH "Concert's include dir" )
				set( CONCERT_LIB_DIR "${CplexInstallPrefix}/${CplexHome}/concert/lib/${CplexArch}" CACHE PATH "Concert's library dir" )

				break()
			endif()
		else()
			message( "Unable to find CPLEX and Concert installation in ${CplexInstallPrefix}" )
		endif()
	endforeach()

	if( NOT CPLEX_INC_DIR OR NOT CPLEX_LIB_DIR OR NOT CONCERT_INC_DIR OR NOT CONCERT_LIB_DIR )
		message( "Unable to autodeted CPLEX's and Concert's installation directories, please set {CPLEX|CONCERT}_{INC|LIB}_DIR manually" )
	else()
		message( "Autodetected CPLEX's include dir   : ${CPLEX_INC_DIR}" )
		message( "Autodetected CPLEX's library dir   : ${CPLEX_LIB_DIR}" )
		message( "Autodetected Concert's include dir : ${CONCERT_INC_DIR}" )
		message( "Autodetected Concert's library dir : ${CONCERT_LIB_DIR}" )
	endif()
endif()

set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wno-write-strings " )
#if( ${CMAKE_SYSTEM_NAME} MATCHES "Darwin" )
#	set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libstdc++ " )
#endif()
add_definitions( -DIL_STD )
configure_file( src/Config.hpp.in ${PROJECT_SOURCE_DIR}/src/Config.hpp )

set( CMAKE_CXX_FLAGS_DEBUG   "${CMAKE_CXX_FLAGS_DEBUG}   -Wall -DDEBUG -DVERBOSE " )
set( CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Ofast " )

include_directories( ${CPLEX_INC_DIR} ${CONCERT_INC_DIR} "src/externlib/boost_1_55_0_patched" ${Boost_INCLUDE_DIRS} "${LIBLEMON_ROOT}/include" "src" "src/externlib" )
link_directories( ${CPLEX_LIB_DIR} ${CONCERT_LIB_DIR} "${LIBLEMON_ROOT}/lib" )

set( CommonLibs
	m
	ilocplex
	cplex
	concert
	pthread
	emon
)

add_executable( xHeinz ${xHeinz_Src} ${xHeinz_Hdr} ${CommonObjs} )
target_link_libraries( xHeinz ${CommonLibs} )
