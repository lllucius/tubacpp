
#include <wx/types.h>
#include <wx/string.h>

#include "channel.h"

Channel::Channel()
{
}

Channel::~Channel()
{
}

bool
Channel::Init( const wxString & label, const wxString & name )
{
    mLabel = label;
    mName = name;

    return true;
}

wxString
Channel::GetOSCPattern()
{
    return mPattern;
}

void
Channel::SetOSCPattern( const wxString & pattern )
{
    mPattern = pattern;
}

void
Channel::SetName( const wxString & name )
{
    mName = name;
}

wxString
Channel::GetName()
{
    return mName;
}

/////////////////////////////////////////////////////////////////////////////

Channels::Channels()
{
}

Channels::Channels( const wxString & name )
{
    mName = name;
}

Channels::~Channels()
{
}

wxString
Channels::GetName()
{
    return mName;
}

void
Channels::SetName( const wxString & name )
{
    mName = name;
}

Channel *
Channels::GetChannel( int id )
{
    return &mChannels[ id ];
}

Channel *
Channels::GetChannel( const wxString & name )
{
    std::map< int, Channel >::iterator iter;

    for( iter = mChannels.begin(); iter != mChannels.end(); iter++ )
    {
        if( iter->second.GetName().IsSameAs( name ) )
        {
            return &iter->second;
        }
    }

    return NULL;
}

int
Channels::GetChannelID( const wxString & name )
{
    std::map< int, Channel >::iterator iter;

    for( iter = mChannels.begin(); iter != mChannels.end(); iter++ )
    {
        if( iter->second.GetName().IsSameAs( name ) )
        {
            return iter->first;
        }
    }

    return -1;
}

wxString
Channels::GetChannelName( int id )
{
    return mChannels[ id ].GetName();
}

void
Channels::SetChannelName( int id, const wxString & name )
{
    mChannels[ id ].SetName( name );
    
}

int
Channels::GetCount()
{
    return mChannels.size();
}


