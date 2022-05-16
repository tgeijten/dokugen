#pragma once

#include "xo/container/prop_node.h"
#include "xo/filesystem/path.h"

struct dokugen_settings
{
	xo::path output_dir;
	std::vector< std::string > remove_strings;
	bool remove_trailing_underscores = true;
};

int write_doku( const xo::path& input, const dokugen_settings& cfg );
