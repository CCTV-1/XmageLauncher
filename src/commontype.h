#pragma once
#ifndef COMMONTYPE_H
#define COMMONTYPE_H

#include <cstdint>

#include <glibmm/ustring.h>

enum class XmageType:std::uint8_t
{
    Beta,
    Release
};

Glib::ustring inline xmagetype_to_string( const XmageType& type )
{
    Glib::ustring str;
    switch ( type )
    {
        default:
        case XmageType::Release:
            str = "Release";
            break;
        case XmageType::Beta:
            str = "Beta";
            break;
    }

    return str;
}

XmageType inline string_to_xmagetype( const Glib::ustring& str )
{
    XmageType type = XmageType::Release;
    Glib::ustring diff_str = str.lowercase();
    if ( diff_str == "release" )
    {
        type = XmageType::Release;
    }
    if ( diff_str == "beta" )
    {
        type = XmageType::Beta;
    }

    return type;
}

#endif