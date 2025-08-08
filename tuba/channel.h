#if !defined(CHANNEL_H)
#define CHANNEL_H

#include <map>

#include <wx/string.h>

class Channel
{
public:
   Channel();
   virtual ~Channel();

   bool Init(const wxString & label, const wxString & name);

   wxString GetOSCPattern();
   void SetOSCPattern(const wxString & pattern);

   wxString GetName();
   void SetName(const wxString & name);

private:
   wxString mLabel;
   wxString mName;
   wxString mPattern;
};

class Channels
{
public:
   Channels();
   Channels(const wxString & name);
   virtual ~Channels();

   wxString GetName();
   void SetName(const wxString & name);

   Channel * GetChannel(int id);
   Channel * GetChannel(const wxString & name);

   int GetChannelID(const wxString & name);

   wxString GetChannelName(int id);
   void SetChannelName(int id, const wxString & name);

   int GetCount();

private:
   wxString mName;
   std::map< int, Channel > mChannels;
};


#endif
