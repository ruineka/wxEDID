
#include "rcdunits.h"
#ifndef idGUI_LOG
   #error "guilog.cpp: missing unit ID"
#endif
#define RCD_UNIT idGUI_LOG
#include "rcode/rcode.h"

#include "wxedid_rcd_scope.h"

#include "guilog.h"


void guilog_cl::RcodeToString(rcode retU, wxString& str) {
   //assemble the message
   wxedid_RCD_GET_MSG(retU, rcd_msg_buff, msg_buf_sz);

   str = wxString::FromAscii(rcd_msg_buff, msg_buf_sz);
}

void guilog_cl::PrintRcode(rcode retU) {
   wxedid_RCD_GET_MSG(retU, rcd_msg_buff, msg_buf_sz);
   wxLogStatus(rcd_msg_buff);
}

void guilog_cl::DoLog() {
   wxLogStatus(slog.GetData());
   slog.Empty();
};

void guilog_cl::DoLog(const wxString& msg) {
   wxLogStatus(msg.GetData());
};

rcode guilog_cl::Create(wxWindow *parent, int w, int h) {
   rcode retU;

   if ((status != 0) || (logwin != NULL)) {
      RCD_RETURN_FAULT(retU);
   }
   logwin = new wxLogWindow(parent, "LOG", false);
   if (logwin == NULL) {
      RCD_RETURN_FAULT(retU);
   }
   logwin->GetFrame()->SetSize(wxDefaultCoord, wxDefaultCoord, w, h);
   status++ ;

   RCD_RETURN_OK(retU);
};

rcode guilog_cl::Destroy() {
   rcode retU;

   if ((status != 0) && (logwin != NULL)) {
      wxFrame * frm = logwin->GetFrame();
      if (frm != NULL) frm->Destroy();
      RCD_RETURN_OK(retU);
   }
   RCD_RETURN_FAULT(retU);
};

void guilog_cl::Show() {
   if ((status != 0) || (logwin != NULL)) {
      logwin->Show(true);
   }
};

void guilog_cl::ShowHide() {
   if ((status != 0) || (logwin != NULL)) {
      logwin->Show(! logwin->GetFrame()->IsShownOnScreen());
   }
   //visible = logwin->GetFrame()->IsShownOnScreen();
};

guilog_cl::guilog_cl(): status(0), logwin(NULL) {};


