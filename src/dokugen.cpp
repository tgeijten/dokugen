#include "dokugen.h"
#include <fstream>
#include "xo/system/log.h"
#include "xo/serialization/serialize.h"

#include "rapidxml.hpp"
#include "rapidxml_print.hpp"
#include "xo/filesystem/filesystem.h"

using namespace xo;
using namespace rapidxml;
using std::endl;

string extract_ref( xml_node<>* node )
{
	auto* id = node->first_attribute( "refid" );
	if ( id )
		return string( "[[" ) + id->value() + "|" + node->value() + "]]";
	else return "";
}

string extract_text( xml_node<>* node )
{
	if ( auto* para = node->first_node( "para" ) )
	{
		char buffer[ 4096 ];
		char *end = print( buffer, *para, 0 );
		*end = '\0';
		return buffer;
	}
	else return "";

	//string str = node->value();
	//for ( auto* p = node->first_node(); p; p = p->next_sibling() )
	//{
	//	if ( p->name() == "ref" )
	//		str += extract_ref( p );
	//	else str += p->value();
	//}
	//return str;
}

int write_doku_xml( const xo::path& input, const xo::path& output )
{
	rapidxml::xml_document<> doc;
	std::string file_contents = load_string( input );
	doc.parse< 0 >( &file_contents[ 0 ] );

	xml_node<>* root = doc.first_node( "doxygen" );
	xo_error_if( !root, "Could not find doxygen" );
	root = root->first_node( "compounddef" );
	xo_error_if( !root, "Could not find compounddef" );

	auto name = root->first_node( "compoundname" )->value();
	auto brief = extract_text( root->first_node( "briefdescription" ) );
	auto detailed = extract_text( root->first_node( "detaileddescription" ) );

	if ( brief.empty() )
		return 0;

	auto str = std::ofstream( output.string() );
	xo_error_if( !str.good(), "Could not open " + output.string() );

	str << "====== " << name << " ======" << endl;
	str << brief << endl;
	if ( !detailed.empty() )
		str << detailed << endl;
	str << endl;

	//auto derived_count = 0;
	//for ( auto& derived : compounddef.select( "derivedcompoundref" ) )
	//{
	//	if ( derived_count++ == 0 )
	//		str << "==== Derived types ====" << endl;
	//	str << extract_ref( derived.second ) << endl;
	//}
	//if ( derived_count > 0 )
	//	str << endl;

	// write sections
	auto attrib_count = 0;
	for ( auto* section = root->first_node( "sectiondef" ); section; section = section->next_sibling( "sectiondef" ) )
	{
		string kind = section->first_attribute( "kind" )->value();
		if ( kind == "public-attrib" )
		{
			for ( auto* member = section->first_node( "memberdef" ); member; member = member->next_sibling( "memberdef" ) )
			{
				auto brief = extract_text( member->first_node( "briefdescription" ) );
				if ( !brief.empty() )
				{
					if ( attrib_count++ == 0 )
					{
						str << "==== Parameters ====" << std::endl;
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
	return attrib_count;
}

string extract_ref( const prop_node& pn )
{
	return "[[" + pn.get<string>( "refid" ) + "|" + pn.get_value() + "]]";
}

string extract_text( const prop_node& pn )
{
	string str = pn.get_value();
	if ( auto para = pn.try_get_child( "para" ) )
	{
		str += para->get_value();
		if ( auto ref = para->try_get_child( "ref" ) )
			str += extract_ref( *ref );
	}
	return str;
}

int write_doku( const xo::path& input, const xo::path& output )
{
	auto pn = load_file( input );
	auto& compounddef = pn[ "doxygen" ][ "compounddef" ];
	auto name = compounddef.get<string>( "compoundname" );
	auto brief = extract_text( compounddef.get_child( "briefdescription" ) );
	auto detailed = extract_text( compounddef.get_child( "detaileddescription" ) );

	if ( brief.empty() )
		return 0;

	auto str = std::ofstream( output.string() );
	xo_error_if( !str.good(), "Could not open " + output.string() );

	str << "====== " << name << " ======" << endl;
	str << brief << endl;
	if ( !detailed.empty() )
		str << detailed << endl;
	str << endl;

	auto derived_count = 0;
	for ( auto& derived : compounddef.select( "derivedcompoundref" ) )
	{
		if ( derived_count++ == 0 )
			str << "==== Derived types ====" << endl;
		str << extract_ref( derived.second ) << endl;
	}
	if ( derived_count > 0 )
		str << endl;

	// write sections
	auto attrib_count = 0;
	for ( auto& section : compounddef.select( "sectiondef" ) )
	{
		if ( section.second.get<string>( "kind" ) == "public-attrib" )
		{
			for ( auto& a : section.second.select( "memberdef" ) )
			{
				auto brief = extract_text( a.second.get_child( "briefdescription" ) );
				if ( !brief.empty() )
				{
					if ( attrib_count++ == 0 )
					{
						str << "==== Parameters ====" << std::endl;
						str << "^ Parameter ^ Type ^ Description ^" << std::endl;
					}

					str << "^ " << a.second.get<string>( "name" );
					str << " | " << a.second.get<string>( "type" );
					str << " | " << brief;
					str << " |" << std::endl;
				}
			}
		}
	}
	return attrib_count;
}

