#include "dokugen.h"
#include <fstream>
#include "xo/system/log.h"
#include "xo/serialization/serialize.h"

#include "rapidxml.hpp"
#include "rapidxml_print.hpp"
#include "xo/filesystem/filesystem.h"
#include "xo/string/string_tools.h"

using namespace xo;
using namespace rapidxml;
using std::endl;


string fix_string( string str, const dokugen_settings& cfg ) {
	for ( auto& s : cfg.remove_strings )
		xo::replace_str( str, s, "" );
	return str;
}

string extract_ref( xml_node<>* node, const dokugen_settings& cfg )
{
	auto* id = node->first_attribute( "refid" );
	if ( id )
		return string( "[[" ) + fix_string( id->value(), cfg ) + "|" + node->value() + "]]";
	else return "";
}

string extract_text( xml_node<>* node, const dokugen_settings& cfg )
{
	string str = node->value();
	if ( auto* para = node->first_node( "para" ) )
	{
		for ( xml_node<>* child = para->first_node(); child; child = child->next_sibling() )
		{
			if ( child->type() == node_element )
				str += extract_ref( child, cfg );
			else str += child->value();
		}
	}
	return str;
}

int write_doku( const xo::path& input, const dokugen_settings& cfg )
{
	path output = cfg.output_dir / fix_string( input.filename().replace_extension( "txt" ).string(), cfg );

	rapidxml::xml_document<> doc;
	std::string file_contents = load_string( input );
	doc.parse< 0 >( &file_contents[ 0 ] );

	xml_node<>* root = doc.first_node( "doxygen" );
	xo_error_if( !root, "Could not find doxygen" );
	root = root->first_node( "compounddef" );
	xo_error_if( !root, "Could not find compounddef" );

	auto name = xo::clean_type_name( root->first_node( "compoundname" )->value() );
	auto brief = extract_text( root->first_node( "briefdescription" ), cfg );
	auto detailed = extract_text( root->first_node( "detaileddescription" ), cfg );

	if ( brief.empty() )
		return 0;

	auto str = std::ofstream( output.string() );
	xo_error_if( !str.good(), "Could not open " + output.string() );

	str << "====== " << name << " ======" << endl;
	str << brief << endl;
	if ( !detailed.empty() )
		str << detailed << endl;
	str << endl;

	//str << "==== Inheritance ====" << std::endl;
	auto base_count = 0;
	for ( auto* node = root->first_node( "basecompoundref" ); node; node = node->next_sibling( "basecompoundref" ) )
	{
		if ( auto s = extract_ref( node, cfg ); !s.empty() )
		{
			if ( base_count++ == 0 )
				str << "**Inherits from** " << s;
			else str << ", " << s;
		}
	}
	if ( base_count > 0 )
		str << "." << endl << endl;

	auto derived_count = 0;
	for ( auto* node = root->first_node( "derivedcompoundref" ); node; node = node->next_sibling( "derivedcompoundref" ) )
	{
		if ( auto s = extract_ref( node, cfg ); !s.empty() )
		{
			if ( derived_count++ == 0 )
				str << "**Inherited by** " << s;
			else str << ", " << s;
		}
	}
	if ( derived_count > 0 )
		str << "." << endl << endl;

	// write sections
	auto attrib_count = 0;
	for ( auto* section = root->first_node( "sectiondef" ); section; section = section->next_sibling( "sectiondef" ) )
	{
		string kind = section->first_attribute( "kind" )->value();
		if ( kind == "public-attrib" )
		{
			for ( auto* member = section->first_node( "memberdef" ); member; member = member->next_sibling( "memberdef" ) )
			{
				auto brief = extract_text( member->first_node( "briefdescription" ), cfg );
				if ( !brief.empty() )
				{
					if ( attrib_count++ == 0 )
					{
						str << "==== Public Attributes ====" << std::endl;
						str << "^ Parameter ^ Type ^ Description ^" << std::endl;
					}

					str << "^ " << member->first_node( "name" )->value();
					str << " | " << member->first_node( "type" )->value();
					str << " | " << brief;
					str << " |" << std::endl;
				}
			}
		}
	}

	str << endl << "<sub>Converted from doxygen using [[https://github.com/tgeijten/dokugen|dokugen]]</sub>" << endl;

	return attrib_count;
}
