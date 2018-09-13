#include "xo/system/log.h"
#include "xo/system/system_tools.h"
#include "xo/system/log_sink.h"

#include <tclap/CmdLine.h>
#include <filesystem>
#include "xo/serialization/serialize.h"
#include "xo/container/prop_node.h"
#include "dokugen.h"
#include "xo/filesystem/filesystem.h"

using namespace xo;

int main( int argc, char* argv[] )
{
	xo::log::console_sink sink( xo::log::info_level );
	int converted = 0;

	try
	{
		TCLAP::CmdLine cmd( "dokugen", ' ', "1.0", true );
		TCLAP::UnlabeledValueArg< string > input( "input", "Input folder name", true, "", "Path of the file containing the XML doxygen output", cmd );
		TCLAP::UnlabeledValueArg< string > output( "output", "Output folder name", false, "", "Path of the file containing the dokuwiki output", cmd );
		cmd.parse( argc, argv );

		auto output_dir = path( output.getValue() );

		for ( auto& e : std::experimental::filesystem::v1::directory_iterator( input.getValue() ) )
		{
			auto input_path = xo::path( e.path().string() );
			auto output_path = output_dir / input_path.filename().replace_extension( "txt" );

			auto filename = input_path.filename().string();
			if ( input_path.extension() != "xml" )
				continue;
			if ( !str_begins_with( filename, "class" ) && !str_begins_with( filename, "struct" ) )
				continue;

			try
			{
				auto n = write_doku_xml( input_path, output_path );
				if ( n > 0 )
				{
					log::info( input_path.string(), ": ", n, " elements converted" );
					++converted;
				}
				else xo::remove( output_path );
			}
			catch ( std::exception& e )
			{
				log::error( input_path.string(), ": ", e.what() );
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
	xo::wait_for_key();
	return 0;
}
