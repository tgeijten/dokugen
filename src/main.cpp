#include "xo/system/log.h"
#include "xo/system/system_tools.h"
#include "xo/system/log_sink.h"

#include <tclap/CmdLine.h>
#include <filesystem>
#include "xo/serialization/serialize.h"
#include "xo/container/prop_node.h"
#include "dokugen.h"
#include "xo/filesystem/filesystem.h"
#include "xo/system/version.h"

using namespace xo;

const xo::version dokugen_version = xo::version( 1, 0, 0 );

int main( int argc, char* argv[] )
{
	xo::log::console_sink sink( xo::log::info_level );
	int converted = 0;

	try
	{
		std::cout << "Dokugen version " << to_str( dokugen_version ) << std::endl;
		std::cout << "(C) Copyright 2018-2019 by Thomas Geijtenbeek" << std::endl << std::endl;

		TCLAP::CmdLine cmd( "dokugen", ' ', to_str( dokugen_version ), true );
		TCLAP::UnlabeledValueArg< string > input( "input", "Folder from where to read XML doxygen output", true, "", "Folder", cmd );
		TCLAP::UnlabeledValueArg< string > output( "output", "Folder where to write dokuwiki output", false, "", "Folder", cmd );
		TCLAP::MultiArg< string > remove( "r", "remove", "Remove part of name", false, "String", cmd );
		cmd.parse( argc, argv );

		dokugen_settings cfg;
		cfg.output_dir = path( output.getValue() );
		xo::create_directories( cfg.output_dir );
		for ( auto& r : remove )
			cfg.remove_strings.emplace_back( r );

		for ( auto& e : std::experimental::filesystem::v1::directory_iterator( input.getValue() ) )
		{
			auto input_path = xo::path( e.path().string() );
			auto filename = input_path.filename().str();
			if ( input_path.extension() != "xml" )
				continue;
			if ( !str_begins_with( filename, "class" ) && !str_begins_with( filename, "struct" ) )
				continue;

			try
			{
				auto n = write_doku( input_path, cfg );
				log::info( input_path.str(), ": ", n, " elements converted" );
				++converted;
			}
			catch ( std::exception& e )
			{
				log::error( input_path.str(), ": ", e.what() );
			}
		}
	}
	catch ( std::exception& e )
	{
		log::critical( e.what() );
	}
	catch ( TCLAP::ExitException& e )
	{
		return e.getExitStatus();
	}

	log::info( "Successfully converted ", converted, " files..." );
	return 0;
}
