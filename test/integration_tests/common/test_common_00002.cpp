/*---------------------------------------------------------------------------*\
 *
 *  bitpit
 *
 *  Copyright (C) 2015-2021 OPTIMAD engineering Srl
 *
 *  -------------------------------------------------------------------------
 *  License
 *  This file is part of bitpit.
 *
 *  bitpit is free software: you can redistribute it and/or modify it
 *  under the terms of the GNU Lesser General Public License v3 (LGPL)
 *  as published by the Free Software Foundation.
 *
 *  bitpit is distributed in the hope that it will be useful, but WITHOUT
 *  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 *  FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public
 *  License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with bitpit. If not, see <http://www.gnu.org/licenses/>.
 *
\*---------------------------------------------------------------------------*/

#include <array>
#if BITPIT_ENABLE_MPI==1
#include <mpi.h>
#endif

#include "bitpit_common.hpp"

using namespace bitpit;

/*!
* Subtest 001
*
* Testing basic logger fatures.
*/
int subtest_001()
{
	int nProcessors;
	int rank;

#if BITPIT_ENABLE_MPI==1
	MPI_Comm_size(MPI_COMM_WORLD, &nProcessors);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
#else
	nProcessors = 1;
	rank        = 0;
#endif

	std::cout << "Testing basic logger fatures" << "\n";

	// Default logger
	log::manager().initialize(log::MODE_SEPARATE, false, nProcessors, rank);

	log::cout() << log::consoleVerbosity(log::LEVEL_INFO);
	log::cout() << log::fileVerbosity(log::LEVEL_INFO);

	log::cout() << log::defaultSeverity(log::LEVEL_INFO);
	log::cout() << "1-1\n2--2\n" << std::endl << "3---3" << std::endl;
	log::cout() << "4----4\n5-----5\n" << std::endl << "6--";
	log::cout() << "----6\n7-------7";
	log::cout() << std::endl;
	log::cout() << log::defaultSeverity(log::LEVEL_INFO);
	log::cout() << log::context("context-1");
	log::cout() << "1-1\n2--2\n" << std::endl << "3---3" << std::endl;
	log::cout() << "4----4\n5-----5\n" << std::endl << "6--";
	log::cout() << "----6\n7-------7";
	log::cout() << log::defaultSeverity(log::LEVEL_INFO);
	log::cout() << log::context("context-2");
	log::cout() << "1-1\n2--2\n" << std::endl << "3---3" << std::endl;
	log::cout() << "4----4\n5-----5\n" << std::endl << "6--";
	log::cout() << "----6\n7-------7";
	log::cout() << std::endl;

	log::cout() << log::defaultSeverity(log::LEVEL_INFO);
	log::critical() << log::context("critical");
	log::critical() << "1-1\n2--2\n" << std::endl << "3---3" << std::endl;
	log::critical() << "4----4\n5-----5\n" << std::endl << "6--";
	log::critical() << "----6\n7-------7";
	log::critical() << std::endl;
	log::error() << log::context("error");
	log::error() << "1-1\n2--2\n" << std::endl << "3---3" << std::endl;
	log::error() << "4----4\n5-----5\n" << std::endl << "6--";
	log::error() << "----6\n7-------7";
	log::error() << std::endl;
	log::warning() << log::context("warning");
	log::warning() << "1-1\n2--2\n" << std::endl << "3---3" << std::endl;
	log::warning() << "4----4\n5-----5\n" << std::endl << "6--";
	log::warning() << "----6\n7-------7";
	log::warning() << std::endl;
	log::info() << log::context("info");
	log::info() << "1-1\n2--2\n" << std::endl << "3---3" << std::endl;
	log::info() << "4----4\n5-----5\n" << std::endl << "6--";
	log::info() << "----6\n7-------7";
	log::info() << std::endl;
	log::debug() << log::context("debug");
	log::debug() << "1-1\n2--2\n" << std::endl << "3---3" << std::endl;
	log::debug() << "4----4\n5-----5\n" << std::endl << "6--";
	log::debug() << "----6\n7-------7";
	log::debug() << std::endl;

	BITPIT_DEBUG_COUT() << "Debug statement" << std::endl;

	// Log only on console
	log::manager().create("logger1", false, nProcessors, rank);

	log::cout("logger1") << log::consoleVerbosity(log::LEVEL_DEBUG);
	log::cout("logger1") << log::disableFile();

	log::cout("logger1") << log::defaultSeverity(log::LEVEL_DEBUG);
	log::cout("logger1") << log::context("logger1-A");
	log::cout("logger1") << "1-1\n2--2\n" << std::endl << "3---3" << std::endl;
	log::cout("logger1") << "4----4\n5-----5\n" << std::endl << "6--";
	log::cout("logger1") << "----6\n7-------7";
	log::cout("logger1") << std::endl;
	log::cout("logger1") << log::defaultSeverity(log::LEVEL_INFO);
	log::cout("logger1") << log::context("logger1-B");
	log::cout("logger1") << "1-1\n2--2\n" << std::endl << "3---3" << std::endl;
	log::cout("logger1") << "4----4\n5-----5\n" << std::endl << "6--";
	log::cout("logger1") << "----6\n7-------7";
	log::cout("logger1") << std::endl;

	BITPIT_DEBUG_COUT("logger1") << "Debug statement" << std::endl;

	// Log on console and also on file
	log::manager().create("logger2", false, nProcessors, rank);

	log::cout("logger2") << log::consoleVerbosity(log::LEVEL_INFO);
	log::cout("logger2") << log::fileVerbosity(log::LEVEL_INFO);

	log::cout("logger2") << log::defaultSeverity(log::LEVEL_DEBUG);
	log::cout("logger2") << log::context("logger2-A");
	log::cout("logger2") << "1-1\n2--2\n" << std::endl << "3---3" << std::endl;
	log::cout("logger2") << "4----4\n5-----5\n" << std::endl << "6--";
	log::cout("logger2") << "----6\n7-------7";
	log::cout("logger2") << std::endl;
	log::cout("logger2") << log::defaultSeverity(log::LEVEL_INFO);
	log::cout("logger2") << log::context("logger2-B");
	log::cout("logger2") << "1-1\n2--2\n" << std::endl << "3---3" << std::endl;
	log::cout("logger2") << "4----4\n5-----5\n" << std::endl << "6--";
	log::cout("logger2") << "----6\n7-------7";
	log::cout("logger2") << std::endl;

	BITPIT_DEBUG_COUT("logger2") << "Debug statement" << std::endl;

	// Log on console and also on file with different verbosities
	log::manager().create("logger3", true, nProcessors, rank);

	log::cout("logger3").setConsoleVerbosity(log::LEVEL_INFO);
	log::cout("logger3").setFileVerbosity(log::LEVEL_DEBUG);

	log::cout("logger3") << log::defaultSeverity(log::LEVEL_DEBUG);
	log::cout("logger3") << log::context("logger3-A");
	log::cout("logger3") << "1-1\n2--2\n" << std::endl << "3---3" << std::endl;
	log::cout("logger3") << "4----4\n5-----5\n" << std::endl << "6--";
	log::cout("logger3") << "----6\n7-------7";
	log::cout("logger3") << std::endl;
	log::cout("logger3") << log::defaultSeverity(log::LEVEL_INFO);
	log::cout("logger3") << log::context("logger3-B");
	log::cout("logger3") << "1-1\n2--2\n" << std::endl << "3---3" << std::endl;
	log::cout("logger3") << "4----4\n5-----5\n" << std::endl << "6--";
	log::cout("logger3") << "----6\n7-------7";
	log::cout("logger3") << std::endl;

	BITPIT_DEBUG_COUT("logger3") << "Debug statement" << std::endl;

	// Log using logger functions
	log::manager().create("logger4", true, nProcessors, rank);

	log::cout("logger4").setConsoleVerbosity(log::LEVEL_DEBUG);
	log::cout("logger4").setFileVerbosity(log::LEVEL_DEBUG);

	log::cout("logger4").setDefaultSeverity(log::LEVEL_INFO);
	log::cout("logger4").setContext("logger4-B");
	log::cout("logger4").println("1-1\n2--2\n\n3---3");
	log::cout("logger4").print("4----4\n5-----5\n\n6--");
	log::cout("logger4").print("----6\n7-------7");
	log::cout("logger4").println("");
	log::cout("logger4").setDefaultSeverity(log::LEVEL_INFO);
	log::cout("logger4").setContext("logger4-B");
	log::cout("logger4").println("1-1\n2--2\n\n3---3");
	log::cout("logger4").print("4----4\n5-----5\n\n6--");
	log::cout("logger4").print("----6\n7-------7");
	log::cout("logger4").println("");

	BITPIT_DEBUG_COUT("logger4") << "Debug statement" << std::endl;

	// Test indention
	log::cout().setConsoleVerbosity(log::LEVEL_DEBUG);
	log::cout().setFileVerbosity(log::LEVEL_DEBUG);

	log::cout().setDefaultSeverity(log::LEVEL_INFO);
	log::cout().setContext("indent");

	log::cout().println("012345678912345678901234567890123456789");
	log::cout().println("<------------------------->");
	log::cout().setIndentation(4);
	log::cout().println("<------------------------->");
	log::cout().setIndentation(4);
	log::cout().println("<------------------------->");
	log::cout().setIndentation(-2);
	log::cout().println("<------------------------->");
	log::cout().setIndentation(-2);
	log::cout().println("<------------------------->");
	log::cout().setIndentation(-4);
	log::cout().println("<------------------------->");

	log::cout() << "012345678912345678901234567890123456789" << "\n";
	log::cout() << "<------------------------->" << "\n";
	log::cout() << log::indent(4);
	log::cout() << "<------------------------->" << "\n";
	log::cout() << log::indent(4);
	log::cout() << "<------------------------->" << "\n";
	log::cout() << log::indent(-2);
	log::cout() << "<------------------------->" << "\n";
	log::cout() << log::indent(-2);
	log::cout() << "<------------------------->" << "\n";
	log::cout() << log::indent(-4);
	log::cout() << "<------------------------->" << "\n";

    return 0;
}

/*!
* Main program.
*/
int main(int argc, char *argv[])
{
#if BITPIT_ENABLE_MPI==1
	MPI_Init(&argc,&argv);
#else
	BITPIT_UNUSED(argc);
	BITPIT_UNUSED(argv);
#endif

	std::cout << "Testing basic logger fatures" << "\n";

	// Run the subtests
	std::cout << "Testing logger" << std::endl;

	int status;
	try {
		status = subtest_001();
		if (status != 0) {
			return status;
		}
	} catch (const std::exception &exception) {
		std::cout << exception.what();
		exit(1);
	}

#if BITPIT_ENABLE_MPI==1
	MPI_Finalize();
#endif
}
