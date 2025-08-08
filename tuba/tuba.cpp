/* ====================================================================
||
|| Tuba - Totalmix UBA (ugly, but accessible)
||
|| Written by:  Leland Lucius (tuba@homerow.net>
||
|| Copyright:   GPL v3
||
==================================================================== */

// ====================================================================
// headers
// ====================================================================

#include <winsock2.h> 
#include <windows.h>
#include <ws2tcpip.h>
#include <CommCtrl.h>

#include <string>

#include <wx/defs.h>
#include <wx/app.h>
#include <wx/frame.h>
#include <wx/busyinfo.h>
#include <wx/button.h>
#include <wx/config.h>
#include <wx/dirdlg.h>
#include <wx/filedlg.h>
#include <wx/filename.h>
#include <wx/imaglist.h>
#include <wx/listctrl.h>
#include <wx/log.h>
#include <wx/msgdlg.h>
#include <wx/process.h>
#include <wx/radiobut.h>
#include <wx/sizer.h>
#include <wx/socket.h>
#include <wx/statbox.h>
#include <wx/stattext.h>
#include <wx/statusbr.h>
#include <wx/tokenzr.h>

#include "oscpkt.h"
#include "version.h"
#include "tuba.h"

// #define _DEBUG 1

#if defined( _DEBUG )
#define log wxLogMessage
#else
#define log
#endif

#if defined(_WIN32)
#define COMPARE stricmp
#else
#define COMPARE strcasecmp
#endif

#define ToWX(X) wxString((char *)(X), wxConvISO8859_1)

#define TITLE "TUBA" 

// ====================================================================
// We are an application (no, really, we are.)
// ====================================================================
IMPLEMENT_APP(MyApp)

// ====================================================================
// Application event table
// ====================================================================
BEGIN_EVENT_TABLE(MyApp, wxApp)

EVT_KEY_DOWN(MyApp::OnKeyDown)

END_EVENT_TABLE()

void MyApp::OnKeyDown(wxKeyEvent & event)
{
   if (event.GetKeyCode() == WXK_ESCAPE)
   {
//   wxExit();
      GetTopWindow()->Close();
      event.Skip( false );
      return;
   }

   event.Skip(true);
}

// ====================================================================
// Main entry method
// ====================================================================
bool MyApp::OnInit()
{
   if (!wxApp::OnInit())
   {
      return false;
   }

   // Create
   MyFrame *f = new MyFrame;
   f->Show();

   return true;
}

// ====================================================================
// Frame event table
// ====================================================================
BEGIN_EVENT_TABLE(MyFrame, wxFrame)
   EVT_CLOSE(MyFrame::OnClose)
   EVT_SLIDER(ID_PHONES, MyFrame::OnPhones)
   EVT_SLIDER(ID_MAIN, MyFrame::OnMain)
   EVT_SLIDER(ID_MIC1VOL, MyFrame::OnMic1Vol)
   EVT_SLIDER(ID_MIC1GAIN, MyFrame::OnMic1Gain)
   EVT_SLIDER(ID_MIC2VOL, MyFrame::OnMic2Vol)
   EVT_SLIDER(ID_MIC2GAIN, MyFrame::OnMic2Gain)
   EVT_SLIDER(ID_MIDI, MyFrame::OnMidi)
   EVT_SLIDER(ID_BASS, MyFrame::OnBass)
   EVT_SLIDER(ID_MID, MyFrame::OnMid)
   EVT_SLIDER(ID_TREBLE, MyFrame::OnTreble)
   EVT_CHECKBOX(ID_EQ, MyFrame::OnEq)
   EVT_SOCKET(wxID_ANY, MyFrame::OnSocket)
   EVT_TIMER(wxID_ANY, MyFrame::OnTimer)
END_EVENT_TABLE()

// ====================================================================
// Constructor
// ====================================================================
MyFrame::MyFrame()
: wxFrame(NULL,
          wxID_ANY,
          wxT(TITLE),
          wxPoint(0, 0),
          wxSize(800, 600),
          wxDEFAULT_FRAME_STYLE | wxNO_FULL_REPAINT_ON_RESIZE)
{
#if defined(_DEBUG)
   wxLog::SetActiveTarget( new wxLogWindow( this, wxT("Log") ) );
#endif
   // Get access to our config manager
   m_Config = wxConfig::Get();

   // Set to record all defaults
   m_Config->SetRecordDefaults();

   wxBoxSizer *szTop = new wxBoxSizer(wxVERTICAL);

   // Use a panel to get automatic TAB handling
   wxPanel *panel = new wxPanel(this, wxID_ANY);

   // Create base sizer
   wxBoxSizer *szBase = new wxBoxSizer(wxVERTICAL);

   // Create the volume grouping
   wxFlexGridSizer *szGrid = new wxFlexGridSizer(2, 3, 3);
   szGrid->SetFlexibleDirection(wxHORIZONTAL);
   szGrid->AddGrowableCol(1, 1);

   szGrid->Add(new wxStaticText(panel, wxID_ANY, wxT("Slave")),
      0,
      wxALIGN_CENTER_VERTICAL | wxALIGN_LEFT | wxTOP | wxBOTTOM, 3);
   mPhones = new wxSlider(panel,
      ID_PHONES,
      0,
      0,
      1000,
      wxDefaultPosition,
      wxDefaultSize,
      wxSL_HORIZONTAL);
   mPhones->SetLineSize(25);
   mPhones->SetName(wxT("Slave"));
   mPhones->SetLabel(wxT("Slave"));
   SetWindowLong((HWND)mPhones->GetHandle(), GWL_STYLE, GetWindowLong((HWND)mPhones->GetHandle(), GWL_STYLE) | TBS_DOWNISLEFT);
   szGrid->Add(mPhones, 0, wxEXPAND | wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxTOP | wxBOTTOM, 3);

   szGrid->Add(new wxStaticText(panel, wxID_ANY, wxT("Master")),
      0,
      wxALIGN_CENTER_VERTICAL | wxALIGN_LEFT | wxTOP | wxBOTTOM, 3);
   mMain = new wxSlider(panel,
      ID_MAIN,
      0,
      0,
      1000,
      wxDefaultPosition,
      wxDefaultSize,
      wxSL_HORIZONTAL);
   mMain->SetLineSize(25);
   mMain->SetName(wxT("Master"));
   mMain->SetLabel(wxT("Master"));
   SetWindowLong((HWND)mMain->GetHandle(), GWL_STYLE, GetWindowLong((HWND)mMain->GetHandle(), GWL_STYLE) | TBS_DOWNISLEFT);
   szGrid->Add(mMain, 0, wxEXPAND | wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxTOP | wxBOTTOM, 3);

   szGrid->Add(new wxStaticText(panel, wxID_ANY, wxT("Bass")),
      0,
      wxALIGN_CENTER_VERTICAL | wxALIGN_LEFT | wxTOP | wxBOTTOM, 3);
   mBass = new wxSlider(panel,
      ID_BASS,
      0,
      0,
      1000,
      wxDefaultPosition,
      wxDefaultSize,
      wxSL_HORIZONTAL);
   mBass->SetLineSize(25);
   mBass->SetName(wxT("Bass"));
   mBass->SetLabel(wxT("Bass"));
   SetWindowLong((HWND)mBass->GetHandle(), GWL_STYLE, GetWindowLong((HWND)mBass->GetHandle(), GWL_STYLE) | TBS_DOWNISLEFT);
   szGrid->Add(mBass, 0, wxEXPAND | wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxTOP | wxBOTTOM, 3);

   szGrid->Add(new wxStaticText(panel, wxID_ANY, wxT("Mid")),
      0,
      wxALIGN_CENTER_VERTICAL | wxALIGN_LEFT | wxTOP | wxBOTTOM, 3);
   mMid = new wxSlider(panel,
      ID_MID,
      0,
      0,
      1000,
      wxDefaultPosition,
      wxDefaultSize,
      wxSL_HORIZONTAL);
   mMid->SetLineSize(25);
   mMid->SetName(wxT("Mid"));
   mMid->SetLabel(wxT("Mid"));
   SetWindowLong((HWND)mMid->GetHandle(), GWL_STYLE, GetWindowLong((HWND)mMid->GetHandle(), GWL_STYLE) | TBS_DOWNISLEFT);
   szGrid->Add(mMid, 0, wxEXPAND | wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxTOP | wxBOTTOM, 3);

   szGrid->Add(new wxStaticText(panel, wxID_ANY, wxT("Treble")),
      0,
      wxALIGN_CENTER_VERTICAL | wxALIGN_LEFT | wxTOP | wxBOTTOM, 3);
   mTreble = new wxSlider(panel,
      ID_TREBLE,
      0,
      0,
      1000,
      wxDefaultPosition,
      wxDefaultSize,
      wxSL_HORIZONTAL);
   mTreble->SetLineSize(25);
   mTreble->SetName(wxT("Treble"));
   mTreble->SetLabel(wxT("Treble"));
   SetWindowLong((HWND)mTreble->GetHandle(), GWL_STYLE, GetWindowLong((HWND)mTreble->GetHandle(), GWL_STYLE) | TBS_DOWNISLEFT);
   szGrid->Add(mTreble, 0, wxEXPAND | wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxTOP | wxBOTTOM, 3);

   szGrid->Add(new wxStaticText(panel, wxID_ANY, wxT("Mic 1 Level")),
      0,
      wxALIGN_CENTER_VERTICAL | wxALIGN_LEFT | wxTOP | wxBOTTOM, 3);
   mMic1Vol = new wxSlider(panel,
      ID_MIC1VOL,
      0,
      0,
      1000,
      wxDefaultPosition,
      wxDefaultSize,
      wxSL_HORIZONTAL);
   mMic1Vol->SetLineSize(25);
   mMic1Vol->SetName(wxT("Mic 1 Level"));
   mMic1Vol->SetLabel(wxT("Mic 1 Level"));
   SetWindowLong((HWND)mMic1Vol->GetHandle(), GWL_STYLE, GetWindowLong((HWND)mMic1Vol->GetHandle(), GWL_STYLE) | TBS_DOWNISLEFT);
   szGrid->Add(mMic1Vol, 0, wxEXPAND | wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxTOP | wxBOTTOM, 3);

   szGrid->Add(new wxStaticText(panel, wxID_ANY, wxT("Mic 1 Gain")),
      0,
      wxALIGN_CENTER_VERTICAL | wxALIGN_LEFT | wxTOP | wxBOTTOM, 3);
   mMic1Gain = new wxSlider(panel,
      ID_MIC1GAIN,
      0,
      0,
      1000,
      wxDefaultPosition,
      wxDefaultSize,
      wxSL_HORIZONTAL);
   mMic1Gain->SetLineSize(25);
   mMic1Gain->SetName(wxT("Mic 1 Gain"));
   mMic1Gain->SetLabel(wxT("Mic 1 Gain"));
   SetWindowLong((HWND)mMic1Gain->GetHandle(), GWL_STYLE, GetWindowLong((HWND)mMic1Gain->GetHandle(), GWL_STYLE) | TBS_DOWNISLEFT);
   szGrid->Add(mMic1Gain, 0, wxEXPAND | wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxTOP | wxBOTTOM, 3);

   szGrid->Add(new wxStaticText(panel, wxID_ANY, wxT("Mic 2 Level")),
      0,
      wxALIGN_CENTER_VERTICAL | wxALIGN_LEFT | wxTOP | wxBOTTOM, 3);
   mMic2Vol = new wxSlider(panel,
      ID_MIC2VOL,
      0,
      0,
      1000,
      wxDefaultPosition,
      wxDefaultSize,
      wxSL_HORIZONTAL);
   mMic2Vol->SetLineSize(25);
   mMic2Vol->SetName(wxT("Mic 2 Level"));
   mMic2Vol->SetLabel(wxT("Mic 2 Level"));
   SetWindowLong((HWND)mMic2Vol->GetHandle(), GWL_STYLE, GetWindowLong((HWND)mMic2Vol->GetHandle(), GWL_STYLE) | TBS_DOWNISLEFT);
   szGrid->Add(mMic2Vol, 0, wxEXPAND | wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxTOP | wxBOTTOM, 3);

   szGrid->Add(new wxStaticText(panel, wxID_ANY, wxT("Mic 2 Gain")),
      0,
      wxALIGN_CENTER_VERTICAL | wxALIGN_LEFT | wxTOP | wxBOTTOM, 3);
   mMic2Gain = new wxSlider(panel,
      ID_MIC2GAIN,
      0,
      0,
      1000,
      wxDefaultPosition,
      wxDefaultSize,
      wxSL_HORIZONTAL);
   mMic2Gain->SetLineSize(25);
   mMic2Gain->SetName(wxT("Mic 2 Gain"));
   mMic2Gain->SetLabel(wxT("Mic 2 Gain"));
   SetWindowLong((HWND)mMic2Gain->GetHandle(), GWL_STYLE, GetWindowLong((HWND)mMic2Gain->GetHandle(), GWL_STYLE) | TBS_DOWNISLEFT);
   szGrid->Add(mMic2Gain, 0, wxEXPAND | wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxTOP | wxBOTTOM, 3);

   szGrid->Add(new wxStaticText(panel, wxID_ANY, wxT("Midi")),
      0,
      wxALIGN_CENTER_VERTICAL | wxALIGN_LEFT | wxTOP | wxBOTTOM, 3);
   mMidi = new wxSlider(panel,
      ID_MIDI,
      0,
      0,
      1000,
      wxDefaultPosition,
      wxDefaultSize,
      wxSL_HORIZONTAL);
   mMidi->SetLineSize(25);
   mMidi->SetName(wxT("Midi"));
   mMidi->SetLabel(wxT("Midi"));
   SetWindowLong((HWND)mMidi->GetHandle(), GWL_STYLE, GetWindowLong((HWND)mMidi->GetHandle(), GWL_STYLE) | TBS_DOWNISLEFT);
   szGrid->Add(mMidi, 0, wxEXPAND | wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxTOP | wxBOTTOM, 3);

   mEq = new wxCheckBox(panel,
      ID_EQ,
      wxT("Enable EQ"),
      wxDefaultPosition,
      wxDefaultSize,
      wxCHK_2STATE);
   mEq->SetName(wxT("E Q"));
   szGrid->Add(mEq, 0, wxEXPAND | wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxTOP | wxBOTTOM, 3);

   szBase->Add(szGrid, 0, wxALIGN_LEFT | wxEXPAND | wxALL, 10);

   // Tell the panel about the base sizer
   panel->SetSizer(szBase);

   szTop->Add(panel, 1, wxEXPAND);

   SetSizer(szTop);

   SetMinSize(wxSize(640, -1));
   Layout();
   Fit();
   CenterOnScreen();
   SetSizeHints(GetSize(), wxSize(-1, GetSize().y));

   mMain->SetFocus();

   mIp = wxIPV4address();
   mIp.LocalHost();
   mIp.Service(9001);

   mSock = new wxDatagramSocket(mIp, wxSOCKET_REUSEADDR);
   mSock->SetNotify(wxSOCKET_INPUT_FLAG | wxSOCKET_OUTPUT_FLAG);
   mSock->SetEventHandler(*this);
   mSock->Notify(true);

   mIp.Service(7001);

   mInput.SetName(wxT("Input"));
   mOutput.SetName(wxT("Output"));
   mPlayback.SetName(wxT("Playback"));

   mTimer.SetOwner(this);

   mInitializing = true;
   SendOSCSet(wxT("/1/busPlayback"));
   SendOSCSet(wxT("/1/busInput"));
   SendOSCSet(wxT("/1/busOutput"));
   SendOSCSet(wxT("/1/busPlayback"));

   SendOSCSetChannel(wxT("2"), wxT("Input"), wxT("Mic 1"));
   SendOSCSetChannel(wxT("2"), wxT("Input"), wxT("SPDIF"));
   SendOSCSetChannel(wxT("2"), wxT("Output"), wxT("Main"));
   SendOSCSetChannel(wxT("2"), wxT("Output"), wxT("Speaker B"));
//   SendOSCSetChannel(wxT("2"), wxT("Output"), wxT("AN 3/4"));

   return;
}

// ====================================================================
// Shutting down so destroy dialog
// ====================================================================
void MyFrame::OnClose(wxCloseEvent& event)
{
   mTimer.Stop();

   mSock->Close();
   mSock->Destroy();

   // Destroy dialog
   Destroy();

   return;
}

// ====================================================================
// 
// ====================================================================
void MyFrame::OnSocket(wxSocketEvent& event)
{
   wxDatagramSocket *sock = wxDynamicCast(event.GetSocket(), wxDatagramSocket);
   wxIPV4address ip = wxIPV4address();
   char buf[4096];
   ip.Hostname(wxT("127.0.0.1"));
   ip.Service(9001);

   while (sock->IsData())
   {
      sock->RecvFrom(ip, buf, sizeof(buf));
      int len = sock->LastCount();
      oscpkt::PacketReader pr(buf, len);
      oscpkt::Message *msg;

      while (pr.isOk() && (msg = pr.popMessage()) != 0)
      {
         wxString pat(msg->addressPattern());
         float val;
         msg->arg().popFloat(val);
         log("pat = %s, val %f", pat, val);
         log("active %s\n", mActive);
         if (msg->match("/*/bus*") && val == 1.0f)
         {
            mActive = pat;
         }
         else if (msg->match("/1/trackname*"))
         {
            Channels *chans;
            if (mActive.IsSameAs(wxT("/1/busInput")))
            {
               chans = &mInput;
            }
            else if (mActive.IsSameAs(wxT("/1/busOutput")))
            {
               chans = &mOutput;
            }
            else if (mActive.IsSameAs(wxT("/1/busPlayback")))
            {
               chans = &mPlayback;
            }
            std::string s;
            msg->arg().popStr(s);
            log("name %s", wxString(s));
            chans->SetChannelName(wxAtoi(pat.Right(1)), s.c_str());
         }
         else if (msg->match("/2/{volume,pan,mute,solo,gain,eqEnable,eqGain1,eqGain2,eqGain3}"))
         {
            mValues[pat] = val;
         }
         else if (msg->match("/2/trackname") && mQueued)
         {
            std::string str;
            msg->arg().popStr(str);
            log("str = %s, %f", wxString(str), mValues[ wxT("/2/volume") ]);
            if (str == "Mic 1")
            {
               mMic1Vol->SetValue((int)((mValues[wxT("/2/volume")] + 0.0005f) * 1000));
               mMic1Gain->SetValue((int)((mValues[wxT("/2/gain")] + 0.0005f) * 1000));
            }
            else if (str == "SPDIF")
            {
               mMidi->SetValue((int)((mValues[wxT("/2/volume")] + 0.0005f) * 1000));
            }
            else if (str == "Main")
            {
               mMain->SetValue((int)((mValues[wxT("/2/volume")] + 0.0005f) * 1000));
               mBass->SetValue((int)((mValues[wxT("/2/eqGain1")] + 0.0005f) * 1000));
               mMid->SetValue((int)((mValues[wxT("/2/eqGain2")] + 0.0005f) * 1000));
               mTreble->SetValue((int)((mValues[wxT("/2/eqGain3")] + 0.0005f) * 1000));
               mEq->SetValue(mValues[wxT("/2/eqEnable")] != 0.0f);
            }
            else if (str == "Speaker B")
            {
               mPhones->SetValue((int)((mValues[wxT("/2/volume")] + 0.0005f) * 1000));
            }
         }
#if 0
         if ( ! msg->match( "/" ) )
         {
            wxString os;
            os << wxT("osc_address: '") << msg->addressPattern() << wxT("', types: '") << msg->typeTags() << wxT("', timetag=") << msg->timeTag() << wxT(", args=[");
            oscpkt::Message::ArgReader arg(*msg);
            while (arg.nbArgRemaining() && arg.isOk())
            {
               if (arg.isBool()) { bool b; arg.popBool(b); os << (b?"True":"False"); }
               else if (arg.isInt32()) { wxInt32 i; arg.popInt32(i); os << i; }
               else if (arg.isInt64()) { wxInt64 h; arg.popInt64(h); os << h << "ll"; }
               else if (arg.isFloat()) { float f; arg.popFloat(f); os << f << "f"; }
               else if (arg.isDouble()) { double d; arg.popDouble(d); os << d; }
               else if (arg.isStr()) { std::string s; arg.popStr(s); os += "'" + s + "'"; }
               else if (arg.isBlob()) { std::vector<char> b; arg.popBlob(b); os << "Blob " << b.size() << " bytes"; }
               else assert(0); // I forgot a case..
               if (arg.nbArgRemaining()) os << ", ";
            }
            os += wxT("]");
            log( "%s", os );
         }
#endif
      }
   }

   if (mQueued)
   {
      if (!mQueue.empty())
      {
         oscpkt::PacketWriter *pw = mQueue.front();
         mSock->SendTo(mIp, pw->packetData(), pw->packetSize());
         delete pw;
         mQueue.pop();
      }
      else
      {
         if (mInitializing)
         {
            mInitializing = false;
            mTimer.Start(50);
            Update();
            Show();
         }
         mQueued = false;
      }
   }

   return;
}

// ====================================================================
// 
// ====================================================================

void MyFrame::OnTimer(wxTimerEvent& event)
{
   if (wxWindow::GetCapture() == NULL)
   {
      SendOSCSetChannel(wxT("2"), wxT("Input"), wxT("Mic 1"));
      SendOSCSetChannel(wxT("2"), wxT("Input"), wxT("SPDIF"));
      SendOSCSetChannel(wxT("2"), wxT("Output"), wxT("Main"));
      SendOSCSetChannel(wxT("2"), wxT("Output"), wxT("Speaker B"));
//      SendOSCSetChannel(wxT("2"), wxT("Output"), wxT("AN 3/4"));
   }
}

// ====================================================================
// 
// ====================================================================
void MyFrame::OnPhones(wxCommandEvent& event)
{
   SendOSCFader(2, BUS_OUTPUT, wxT("Speaker B"), wxT("volume"), mPhones->GetValue());
}

// ====================================================================
// 
// ====================================================================
void MyFrame::OnMain(wxCommandEvent& event)
{
   SendOSCFader(2, BUS_OUTPUT, wxT("Main"), wxT("volume"), mMain->GetValue());
}

// ====================================================================
// 
// ====================================================================
void MyFrame::OnMic1Vol(wxCommandEvent& event)
{
   SendOSCFader(2, BUS_INPUT, wxT("Mic 1"), wxT("volume"), mMic1Vol->GetValue());
}

// ====================================================================
// 
// ====================================================================
void MyFrame::OnMic1Gain(wxCommandEvent& event)
{
   SendOSCFader(2, BUS_INPUT, wxT("Mic 1"), wxT("gain"), mMic1Gain->GetValue());
}

// ====================================================================
// 
// ====================================================================
void MyFrame::OnMic2Vol(wxCommandEvent& event)
{
   SendOSCFader(2, BUS_INPUT, wxT("Mic 2"), wxT("volume"), mMic2Vol->GetValue());
}

// ====================================================================
// 
// ====================================================================
void MyFrame::OnMic2Gain(wxCommandEvent& event)
{
   SendOSCFader(2, BUS_INPUT, wxT("Mic 2"), wxT("gain"), mMic2Gain->GetValue());
}

// ====================================================================
// 
// ====================================================================
void MyFrame::OnMidi(wxCommandEvent& event)
{
   SendOSCFader(2, BUS_INPUT, wxT("SPDIF"), wxT("volume"), mMidi->GetValue());
}

// ====================================================================
// 
// ====================================================================
void MyFrame::OnBass(wxCommandEvent& event)
{
   SendOSCFader(2, BUS_OUTPUT, wxT("Main"), wxT("eqGain1"), mBass->GetValue());
   SendOSCFader(2, BUS_OUTPUT, wxT("Speaker B"), wxT("eqGain1"), mBass->GetValue());
}

// ====================================================================
// 
// ====================================================================
void MyFrame::OnMid(wxCommandEvent& event)
{
   SendOSCFader(2, BUS_OUTPUT, wxT("Main"), wxT("eqGain2"), mMid->GetValue());
   SendOSCFader(2, BUS_OUTPUT, wxT("Speaker B"), wxT("eqGain2"), mMid->GetValue());
}

// ====================================================================
// 
// ====================================================================
void MyFrame::OnTreble(wxCommandEvent& event)
{
   SendOSCFader(2, BUS_OUTPUT, wxT("Main"), wxT("eqGain3"), mTreble->GetValue());
   SendOSCFader(2, BUS_OUTPUT, wxT("Speaker B"), wxT("eqGain3"), mTreble->GetValue());
}

// ====================================================================
// 
// ====================================================================
void MyFrame::OnEq(wxCommandEvent& event)
{
   SendOSCToggle(2, BUS_OUTPUT, wxT("Main"), wxT("eqEnable"));
   SendOSCToggle(2, BUS_OUTPUT, wxT("Speaker B"), wxT("eqEnable"));
}

//#define ToStdString() c_str()
// ====================================================================
// 
// ====================================================================
void MyFrame::SendOSCSetChannel(const wxString & page, const wxString & bus, const wxString & name)
{
   Channels *chans;
   if (bus.IsSameAs(wxT("Input")))
   {
      chans = &mInput;
   }
   else if (bus.IsSameAs(wxT("Output")))
   {
      chans = &mOutput;
   }
   else if (bus.IsSameAs(wxT("Playback")))
   {
      chans = &mPlayback;
   }

   oscpkt::PacketWriter *pw = new oscpkt::PacketWriter();
   oscpkt::Message msg;

   pw->startBundle();
   msg.init(wxString::Format(wxT("/%s/bus%s"), page, bus).ToStdString()).pushFloat(1.0f);
   pw->addMessage(msg);

   int cnt = chans->GetCount();
   for (int i = 0; i < cnt; i++)
   {
      msg.init("/2/track-").pushFloat(1.0f);
      pw->addMessage(msg);
   }

   int id = chans->GetChannelID(name);
//   log(wxT("name = %s, id = %d"), name.c_str(), id );
   for (int i = 1; i < id; i++)
   {
      msg.init("/2/track+").pushFloat(1.0f);
      pw->addMessage(msg);
   }
   pw->endBundle();

   if (mQueued)
   {
      mQueue.push(pw);
   }
   else
   {
      mSock->SendTo(mIp, pw->packetData(), pw->packetSize());
      delete pw;
      mQueued = true;
   }
}

// ====================================================================
// 
// ====================================================================
void MyFrame::SendOSCFader(int page, int bus, const wxString & name, const wxString & pattern, int value)
{
   Channels *chans;
   switch (bus)
   {
   case BUS_INPUT:
      chans = &mInput;
      break;

   case BUS_OUTPUT:
      chans = &mOutput;
      break;

   case BUS_PLAYBACK:
      chans = &mPlayback;
      break;
   }

   oscpkt::PacketWriter pw;
   oscpkt::Message msg;

   pw.startBundle();
   msg.init(wxString::Format(wxT("/%d/bus%s"), page, chans->GetName()).ToStdString()).pushFloat(1.0f);
   pw.addMessage(msg);

   int cnt = chans->GetCount();
   for (int i = 0; i < cnt; i++)
   {
      msg.init(wxString::Format(wxT("/%d/track-"), page).ToStdString()).pushFloat(1.0f);
      pw.addMessage(msg);
   }

   int id = chans->GetChannelID(name);
//   log(wxT("name = %s, id = %d"), name.c_str(), id);
   for (int i = 1; i < id; i++)
   {
      msg.init(wxString::Format(wxT("/%d/track+"), page).ToStdString()).pushFloat(1.0f);
      pw.addMessage(msg);
   }

   msg.init(wxString::Format(wxT("/%d/%s"), page, pattern).ToStdString()).pushFloat(((float)value) / 1000.0f + 0.0005f);
   pw.addMessage(msg);
   pw.endBundle();

   mSock->SendTo(mIp, pw.packetData(), pw.packetSize());
}

// ====================================================================
// 
// ====================================================================
void MyFrame::SendOSCSet(const wxString & pattern)
{
   SendOSCMessage(pattern, 1.0);
}

// ====================================================================
// 
// ====================================================================
void MyFrame::SendOSCToggle(int page, int bus, const wxString & name, const wxString & pattern)
{
   Channels *chans;
   switch (bus)
   {
   case BUS_INPUT:
      chans = &mInput;
      break;

   case BUS_OUTPUT:
      chans = &mOutput;
      break;

   case BUS_PLAYBACK:
      chans = &mPlayback;
      break;
   }

   oscpkt::PacketWriter pw;
   oscpkt::Message msg;

   pw.startBundle();
   msg.init(wxString::Format(wxT("/%d/bus%s"), page, chans->GetName()).ToStdString()).pushFloat(1.0f);
   pw.addMessage(msg);

   int cnt = chans->GetCount();
   for (int i = 0; i < cnt; i++)
   {
      msg.init(wxString::Format(wxT("/%d/track-"), page).ToStdString()).pushFloat(1.0f);
      pw.addMessage(msg);
   }

   int id = chans->GetChannelID(name);
   for (int i = 1; i < id; i++)
   {
      msg.init(wxString::Format(wxT("/%d/track+"), page).ToStdString()).pushFloat(1.0f);
      pw.addMessage(msg);
   }

   msg.init(wxString::Format(wxT("/%d/%s"), page, pattern).ToStdString()).pushFloat(1.0f);
   pw.addMessage(msg);
   pw.endBundle();

   mSock->SendTo(mIp, pw.packetData(), pw.packetSize());
}

// ====================================================================
// 
// ====================================================================
void MyFrame::SendOSCString(const wxString & pattern, const wxString & value)
{
   SendOSCMessage(pattern, value);
}

// ====================================================================
// 
// ====================================================================
void MyFrame::SendOSCMessage(const wxString & pattern, const wxString & value)
{
   oscpkt::Message *msg = new oscpkt::Message(pattern.ToStdString());
   msg->pushStr(value.ToStdString());
   QueueMessage(msg);
}

void MyFrame::QueueMessage(oscpkt::Message *msg)
{
   oscpkt::PacketWriter *pw = new oscpkt::PacketWriter();
   pw->addMessage(*msg);
   delete msg;

   if (mQueued)
   {
      mQueue.push(pw);
   }
   else
   {
      mSock->SendTo(mIp, pw->packetData(), pw->packetSize());
      delete pw;
      mQueued = true;
   }
}

// ====================================================================
// 
// ====================================================================
void MyFrame::SendOSCMessage(const wxString & pattern, float value)
{
   oscpkt::Message *msg = new oscpkt::Message(pattern.ToStdString());
   msg->pushFloat(value);
   QueueMessage(msg);
}
