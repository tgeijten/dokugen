#include "xo/system/log.h"
#include "xo/system/system_tools.h"
#include "xo/system/log_sink.h"

int main( int argc, char* argv[] )
{
	xo::log::console_sink sink( xo::log::info_level );
	xo::log::info( "Hello from xo!" );
	
	xo::wait_for_key();
	
	return 0;
}
