#ifndef GUI_LOG_H
#define GUI_LOG_H 1

/* guilog.h v0.2
   Copyright: Tomasz Pawlak (C) 2014-2022
   License:   GPLv3+
*/

#include <wx/log.h>
#include <wx/frame.h>
#include "rcode/rcode.h"

enum {
   msg_buf_sz = 4096
};

class guilog_cl {
   protected:
      int          status;
      wxLogWindow *logwin;

   public:
      wxString     slog;
      char         rcd_msg_buff[msg_buf_sz];

      inline bool isReady() {return (status>0);};

      void RcodeToString(rcode retU, wxString& str);
      void PrintRcode(rcode retU);

      void DoLog();
      void DoLog(const wxString& msg);

      rcode Create(wxWindow *parent, int w=640, int h=400);
      rcode Destroy();

      void Show();
      void ShowHide();

   guilog_cl();

   ~guilog_cl() {};
};

#endif /* GUI_LOG_H */
