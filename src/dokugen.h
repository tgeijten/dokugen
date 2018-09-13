#pragma once

#include "xo/container/prop_node.h"
#include "xo/filesystem/path.h"

int write_doku( const xo::path& input, const xo::path& output );
int write_doku_xml( const xo::path& input, const xo::path& output );
