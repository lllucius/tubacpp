/* ====================================================================
||
|| VMAgui - GUI viewer/extractor/creator for VMARC Hives
||
|| This little utility allows you to view and extract subfiles from
|| archives in VMARC format.
||
|| Written by:  Leland Lucius (vma@homerow.net>
||
|| Copyright:  Public Domain (just use your conscience)
||
====================================================================*/

#include <map>
#include <queue>

#include <wx/defs.h>

#include <wx/bmpbuttn.h>
#include <wx/checkbox.h>
#include <wx/panel.h>
#include <wx/sizer.h>
#include <wx/slider.h>
#include <wx/socket.h>
#include <wx/timer.h>

#include "udp.h"
#include "channel.h"

enum
{
   BUS_INPUT,
   BUS_OUTPUT,
   BUS_PLAYBACK
};

// ====================================================================
// The application
// ====================================================================
class MyFrame;
class MyApp : public wxApp
{
public:
   MyApp(){};
   bool OnInit();

protected:
   void OnKeyDown(wxKeyEvent & event);

private:
   MyFrame *mFrame;

   DECLARE_EVENT_TABLE();
};

// ====================================================================
// The GUI dialog
// ====================================================================
class MyFrame : public wxFrame
{
public:
   MyFrame();

private:

   void OnClose(wxCloseEvent& event);

   void OnSocket(wxSocketEvent& event);

   void OnPhones(wxCommandEvent& event);
   void OnMain(wxCommandEvent& event);
   void OnMic1Vol(wxCommandEvent& event);
   void OnMic1Gain(wxCommandEvent& event);
   void OnMic2Vol(wxCommandEvent& event);
   void OnMic2Gain(wxCommandEvent& event);
   void OnMidi(wxCommandEvent& event);

   void OnBass(wxCommandEvent& event);
   void OnMid(wxCommandEvent& event);
   void OnTreble(wxCommandEvent& event);
   void OnEq(wxCommandEvent& event);

   void OnTimer(wxTimerEvent& event);

   void SendOSCMessage(const wxString & pattern, float value);
   void SendOSCMessage(const wxString & pattern, const wxString & value);
   void SendOSCFader(int page, int bus, const wxString & name, const wxString & pattern, int value);
   void SendOSCSet(const wxString & pattern);
   void SendOSCToggle(int page, int bus, const wxString & name, const wxString & pattern);
   void SendOSCString(const wxString & pattern, const wxString & value);
   void SendOSCSetChannel(const wxString & page, const wxString & bus, const wxString & channel);
   void QueueMessage(oscpkt::Message *msg);

private:
   bool mInitializing;
   bool mIsOutputSelected;
   bool mIsMainSelected;
   wxIPV4address mIp;
   wxDatagramSocket *mSock;
   wxTimer mTimer;
   wxString mActive;

   Channels mInput;
   Channels mOutput;
   Channels mPlayback;

   std::map< wxString, float > mValues;
   std::queue< oscpkt::PacketWriter * > mQueue;
   bool mQueued;

   wxSlider        *mPhones;
   wxSlider        *mMain;
   wxSlider        *mMic1Vol;
   wxSlider        *mMic1Gain;
   wxSlider        *mMic2Vol;
   wxSlider        *mMic2Gain;
   wxSlider        *mMidi;
   wxSlider        *mBass;
   wxSlider        *mMid;
   wxSlider        *mTreble;
   wxCheckBox      *mEq;

   wxConfigBase *m_Config;

   DECLARE_EVENT_TABLE()
};

// controls and menu constants
enum
{
   ID_NEW = wxID_HIGHEST + 1,
   ID_OPEN,
   ID_SAVE,
   ID_ADD,
   ID_DELETE,
   ID_EXTRACT,
   ID_EXTALL,
   ID_VIEW,
   ID_PROPS,
   ID_SETTINGS,

   ID_OUTPAT,
   ID_VALUE,
   ID_PHONES,
   ID_MAIN,
   ID_MIC1VOL,
   ID_MIC1GAIN,
   ID_MIC2VOL,
   ID_MIC2GAIN,
   ID_MIDI,
   ID_BASS,
   ID_MID,
   ID_TREBLE,
   ID_EQ,

   ID_AUTO,
   ID_TEXT,
   ID_BINARY,

   ID_LIST,

   ID_ENTER,
   ID_CTRL_A
};
