#include "dokugen.h"
#include <fstream>
#include "xo/system/log.h"
#include "xo/serialization/serialize.h"

#include "rapidxml.hpp"
#include "rapidxml_print.hpp"
#include "xo/filesystem/filesystem.h"
#include "xo/string/string_tools.h"
#include "xo/utility/hash.h"

using namespace xo;
using namespace rapidxml;

using std::endl, std::ofstream, std::string;

#define FOR_EACH_XML_NODE( _parent_, _child_, _name_ ) \
for ( auto* _child_ = _parent_->first_node( _name_ ); _child_; _child_ = _child_->next_sibling( _name_ ) )


string fix_string( string str, const dokugen_settings& cfg ) {
	for ( auto& s : cfg.remove_strings )
		xo::replace_str( str, s, "" );
	if ( cfg.remove_trailing_underscores )
		xo::trim_right_str( str, "_" );
	return str;
}

string extract_ref( xml_node<>* node, const dokugen_settings& cfg )
{
	if ( auto* id = node->first_attribute( "refid" ) )
		return string( "[[" ) + fix_string( id->value(), cfg ) + "|" + node->value() + "]]";
	else return "";
}

string extract_text( xml_node<>* node, const dokugen_settings& cfg )
{
	string result;

	for ( xml_node<>* child = node->first_node(); child; child = child->next_sibling() )
	{
		if ( child->type() == node_element )
		{
			auto name = string( child->name() );
			switch ( xo::hash( name ) )
			{
			case "para"_hash: result += extract_text( child, cfg ); break;
			case "ref"_hash: result += extract_ref( child, cfg ); break;
			case "emphasis"_hash: result += "//" + extract_text( child, cfg ) + "//"; break;
			case "bold"_hash: result += "**" + extract_text( child, cfg ) + "**"; break;
			case "subscript"_hash: result += "<sub>" + extract_text( child, cfg ) + "</sub>"; break;
			case "verbatim"_hash: result += "<code>" + extract_text( child, cfg ) + "</code>"; break;
			}
		}
		else result += child->value();
	}
	return result;
}

int write_inherited_from( xml_node<>* root, const dokugen_settings& cfg, ofstream& str )
{
	auto base_count = 0;
	FOR_EACH_XML_NODE( root, node, "basecompoundref" )
	{
		if ( auto s = extract_ref( node, cfg ); !s.empty() )
		{
			if ( base_count++ == 0 )
				str << endl << "**Inherits from** " << s;
			else str << ", " << s;
		}
	}
	if ( base_count > 0 )
		str << "." << endl;
	return base_count;
}

int write_inherited_by( xml_node<>* root, const dokugen_settings& cfg, ofstream& str )
{
	auto derived_count = 0;
	FOR_EACH_XML_NODE( root, node, "derivedcompoundref" )
	{
		if ( auto s = extract_ref( node, cfg ); !s.empty() )
		{
			if ( derived_count++ == 0 )
				str << endl << "**Inherited by** " << s;
			else str << ", " << s;
		}
	}
	if ( derived_count > 0 )
		str << "." << endl;
	return derived_count;
}

int write_attributes( xml_node<>* root, string &brief, const dokugen_settings& cfg, ofstream &str )
{
	auto attrib_count = 0;
	FOR_EACH_XML_NODE( root, section, "sectiondef" )
	{
		string kind = section->first_attribute( "kind" )->value();
		if ( kind == "public-attrib" )
		{
			FOR_EACH_XML_NODE( section, member, "memberdef" )
			{
				auto brief = xo::trim_str( extract_text( member->first_node( "briefdescription" ), cfg ) );
				if ( !brief.empty() )
				{
					if ( attrib_count++ == 0 )
					{
						str << endl << "==== Public Attributes ====" << endl;
						str << "^ Parameter ^ Type ^ Description ^" << endl;
					}

					str << "^ " << member->first_node( "name" )->value();
					str << " | " << extract_text( member->first_node( "type" ), cfg );
					str << " | " << brief;
					str << " |" << endl;
				}
			}
		}
	}
	return attrib_count;
}

int write_members( xml_node<>* root, string &brief, const dokugen_settings& cfg, ofstream &str )
{
	auto count = 0;
	FOR_EACH_XML_NODE( root, section, "sectiondef" )
	{
		string kind = section->first_attribute( "kind" )->value();
		if ( kind == "public-func" || kind == "public-static-func" )
		{
			FOR_EACH_XML_NODE( section, member, "memberdef" )
			{
				auto brief = xo::trim_str( extract_text( member->first_node( "briefdescription" ), cfg ) );
				if ( !brief.empty() )
				{
					if ( count++ == 0 )
					{
						str << endl << "==== Public Functions ====" << endl;
						str << "^ Function ^ Description ^" << endl;
					}

					str << "| " << extract_text( member->first_node( "type" ), cfg );
					str << " **" << member->first_node( "name" )->value() << "**";
					str << extract_text( member->first_node( "argsstring" ), cfg );
					str << " | " << brief;
					str << " |" << endl;
				}
			}
		}
	}
	return count;
}

int write_doku( const xo::path& input, const dokugen_settings& cfg )
{
	path output = cfg.output_dir / fix_string( input.filename().replace_extension( "txt" ).str(), cfg );

	rapidxml::xml_document<> doc;
	string file_contents = load_string( input );
	doc.parse< 0 >( &file_contents[ 0 ] );

	xml_node<>* root = doc.first_node( "doxygen" );
	xo_error_if( !root, "Could not find doxygen" );
	root = root->first_node( "compounddef" );
	xo_error_if( !root, "Could not find compounddef" );

	auto name = xo::tidy_type_name( root->first_node( "compoundname" )->value() );
	auto brief = extract_text( root->first_node( "briefdescription" ), cfg );
	auto detailed = extract_text( root->first_node( "detaileddescription" ), cfg );

	if ( brief.empty() )
		return 0;

	ofstream str( output.str() );
	xo_error_if( !str.good(), "Could not open " + output.str() );

	// title + description
	str << "====== " << name << " ======" << endl;
	str << brief << endl;
	if ( !detailed.empty() )
		str << endl << detailed << endl;

	int elem = 0;

	// inherited from
	elem += write_inherited_from( root, cfg, str );

	// inherited by
	elem += write_inherited_by( root, cfg, str );

	// public attributes
	elem += write_attributes( root, brief, cfg, str );

	// public members
	elem += write_members( root, brief, cfg, str );

	str << endl << "<sub>Converted from doxygen using [[https://github.com/tgeijten/dokugen|dokugen]]</sub>" << endl;

	return elem;
}
