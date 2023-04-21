/***************************************************************
 * Name:      wxEDID_Main.h
 * Purpose:   Defines Application Frame
 * Author:    Tomasz Pawlak (tomasz.pawlak@wp.eu)
 * Created:   2014-03-18
 * Copyright: Tomasz Pawlak (C) 2014-2022
 * License:   GPLv3+
 **************************************************************/

#ifndef wxEDIDMAIN_H
#define wxEDIDMAIN_H 1

#ifndef _RCD_AUTOGEN
   #include "config.h"
#else
   #define VERSION "1"
#endif

//(*Headers(wxEDID_Frame)
#include <wx/aui/aui.h>
#include <wx/frame.h>
#include <wx/grid.h>
#include <wx/menu.h>
#include <wx/panel.h>
#include <wx/sizer.h>
#include <wx/spinctrl.h>
#include <wx/stattext.h>
#include <wx/statusbr.h>
#include <wx/textctrl.h>
#include <wx/treectrl.h>
//*)

#include "rcode/rcode.h"
#include "EDID_class.h"
#include "guilog.h"

#include <wx/menu.h>
#include <wx/dcclient.h>
#include <wx/bitmap.h>

//config
typedef struct {
   bool     b_dtd_keep_aspect;
   bool     b_dta_grid_details;
   bool     b_have_last_fpath;
   bool     b_cmd_bin_file;
   bool     b_cmd_txt_file;
   bool     b_cmd_ignore_err;
   bool     b_cmd_ignore_rd;
   bool     b_have_layout;
   wxString cmd_open_file_path;
   wxString last_used_fpath;
   wxString last_used_fname;
   wxString aui_layout;
   wxPoint  win_pos;
   wxSize   win_size;
} config_t;

class blktree_cl;
class fgrid_cl;
class dtd_sct_cl;
class dtd_screen_cl;

enum { //cell column idx'es for BlkDataGrid
   DATGR_COL_NAME   = 0,
   DATGR_COL_TYPE   = 1,
   DATGR_COL_VAL    = 2,
   DATGR_COL_UNIT   = 3,
   DATGR_COL_FLG    = 4,

   DATGR_COL_OFFS   = 5,
   DATGR_COL_SHIFT  = 6,
   DATGR_COL_FSZ    = 7,

   DATGR_NCOL_1    = 5,
   DATGR_NCOL_2    = 8,
   DATGR_NCOL_DIFF = 3
};

enum { //status bar field idx
   SBAR_LOG     = 0,
   SBAR_GRPOFFS = 1
};

typedef union main_flags_u {
   u32_t u32;
   struct {
      uint ctrl_enabled  : 1;
      uint data_loadeed  : 1;
      uint edigridblk_ok : 1;
      uint edi_grp_rfsh  : 1;
      uint res           :28;
   } bits;
} mflags_t;

typedef union op_flags_u {
   u32_t u32;
   struct {
      uint CanCopy     : 1;
      uint CanPaste    : 1;
      uint CanCut      : 1;
      uint CanDelete   : 1;
      uint CanInsertUp : 1;
      uint CanInsInto  : 1;
      uint CanInsertDn : 1;
      uint CanMoveUp   : 1;
      uint CanMoveDn   : 1;
      uint CanReparse  : 1;
      uint res         :22;
   } bits;
} op_flags_t;

class wxEDID_Frame: public wxFrame {
    friend class dtd_screen_cl;

    public:
        rcode OpenEDID();
        rcode SaveEDID();
        rcode SaveRep_SubGrps(edi_grp_cl *pgrp, wxString& reps);
        rcode SaveReport();
        rcode ExportEDID_hex();
        rcode ImportEDID_hex();
        rcode AssembleEDID_main();

        rcode UpdateBlockTree();
        rcode UpdateDataGrid(edi_grp_cl* edigrp);
        void  DataGrid_ChangeView();
        rcode UpdateDataGridRow(int nrow, edi_dynfld_t *field);

        rcode DTD_Ctor_Recalc();
        rcode DTD_Ctor_read_field(dtd_sct_cl& sct, const edi_grp_cl& group, u32_t idx_field);
        rcode DTD_Ctor_read_all();
        rcode DTD_Ctor_ModeLine();
        rcode DTD_Ctor_WriteInt(dtd_sct_cl& sct);

        rcode SetFieldDesc(int row);
        rcode WriteField();

        rcode VerifyChksum(uint block);
        rcode CalcVerifyChksum(uint block);
        rcode Reparse();

        void  AppLayout();
        void  ClearAll(bool b_clrBlocks = true);
        void  EnableControl(bool enb);
        void  SetOpFlags();
        void  GetFullGroupName(edi_grp_cl* pgrp, wxString& grp_name);
        void  LogGroupOP(edi_grp_cl* pgrp, const wxString& opName);
        void  InitBlkTreeMenu();

        wxTreeItemId  BlkTreeInsGrp(wxTreeItemId trItemID, edi_grp_cl* pgrp, u32_t idx);
                void  BlkTreeDelGrp(edi_grp_cl* pgrp);
                void  BlkTreeUpdateGrp();
                rcode BlkTreeChangeGrpType();

        wxEDID_Frame(wxWindow* parent, wxWindowID id = -1);
        ~wxEDID_Frame();

        guilog_cl     GLog;
        bool          b_cmdln_have_file;

    private:
        //grid colors for marking data types:
        wxColour      grid_color_int;
        wxColour      grid_color_bit;
        wxColour      grid_color_float;
        wxColour      grid_color_hex;

        wxBitmap      bmpUp;
        wxBitmap      bmpIn;
        wxBitmap      bmpDn;
        wxBitmap      bmpNOK;

        wxMenu       *mnu_BlkTree;
        wxMenu       *mnu_SubInfo;
        wxMenuItem   *miInfoOK;
        wxMenuItem   *miInfoNOK;
        wxMenuItem   *miRemoved;
        wxMenuItem   *miSubInfo;

        wxAcceleratorEntry *accDelete;
        wxAcceleratorEntry *accMoveUp;
        wxAcceleratorEntry *accMoveDn;

        mflags_t      flags;

        EDID_cl       EDID;
        wxString      tmps;
        wxString      edid_file_name;

        wxTreeItemId  BT_Item_sel;
        wxTreeItemId  BT_Iparent;
        edi_grp_cl   *edigrp_sel;
        edi_grp_cl   *edigrp_src;        //EDID group to be cloned on paste/insert
        bool          b_srcgrp_orphaned; //cut/deleted group has no parent.
        int           subg_idx;          //index of selected subgroup: restore selection after reparse/re-init
        op_flags_t    opFlags;

        int           row_sel;
        int           row_op;

        bool          b_dta_grid_details;
        bool          b_dtd_keep_aspect;
        int           dtd_Htotal; //for DTD aspect ratio calculations, updated by evt_dtdctor_sct()
        int           dtd_Vtotal;

        //(*Handlers(wxEDID_Frame)
        //*)
        void evt_Quit               (wxCommandEvent    & evt);
        void evt_About              (wxCommandEvent    & evt);
        void evt_Flags              (wxCommandEvent    & evt);
        void evt_frame_size         (wxSizeEvent       & evt);
        void evt_ignore_rd          (wxCommandEvent    & evt);
        void evt_ignore_err         (wxCommandEvent    & evt);
        void evt_reparse            (wxCommandEvent    & evt);
        void evt_assemble_edid      (wxCommandEvent    & evt);
        void evt_log_win            (wxCommandEvent    & evt);
        void evt_dtd_asp            (wxCommandEvent    & evt);
        void evt_blk_fdetails       (wxCommandEvent    & evt);
        void evt_open_edid_bin      (wxCommandEvent    & evt);
        void evt_save_edid_bin      (wxCommandEvent    & evt);
        void evt_save_report        (wxCommandEvent    & evt);
        void evt_export_hex         (wxCommandEvent    & evt);
        void evt_import_hex         (wxCommandEvent    & evt);
        void evt_blktree_sel        (wxTreeEvent       & evt);
        void evt_blktree_rmb        (wxTreeEvent       & evt);
        void evt_blktree_key        (wxTreeEvent       & evt);
        void evt_blktree_focus      (wxFocusEvent      & evt) {evt.Skip(false);};
        void evt_datagrid_select    (wxGridEvent       & evt);
        void evt_datagrid_vsel      (wxGridEvent       & evt); //show value selector menu
        void evt_datagrid_edit_hide (wxGridEvent       & evt);
        void evt_datagrid_write     (wxGridEvent       & evt);
        void evt_ntbook_page        (wxAuiNotebookEvent& evt);
        void evt_dtdctor_sct        (wxSpinEvent       & evt);
        void evt_Deferred           (wxCommandEvent    & evt);

        void evt_blktree_reparse    (wxCommandEvent    & evt);
        void evt_blktree_copy       (wxCommandEvent    & evt);
        void evt_blktree_paste      (wxCommandEvent    & evt);
        void evt_blktree_cut        (wxCommandEvent    & evt);
        void evt_blktree_delete     (wxCommandEvent    & evt);
        void evt_blktree_insert     (wxCommandEvent    & evt);
        void evt_blktree_move       (wxCommandEvent    & evt);

        //(* Identifiers(wxEDID_Frame)
        static const long id_block_tree;
        static const long id_grid_blkdat;
        static const long id_txc_edid_info;
        static const long id_panel_edid;
        static const long ID_STATICTEXT5;
        static const long ID_STATICTEXT6;
        static const long id_sct_pixclk;
        static const long ID_STATICTEXT1;
        static const long id_txc_vrefresh;
        static const long ID_STATICTEXT2;
        static const long id_dtd_screen;
        static const long ID_STATICTEXT12;
        static const long id_sct_xres;
        static const long ID_STATICTEXT11;
        static const long id_txres;
        static const long ID_STATICTEXT33;
        static const long ID_STATICTEXT14;
        static const long id_sct_hborder;
        static const long ID_STATICTEXT13;
        static const long ID_STATICTEXT7;
        static const long id_sct_hblank;
        static const long ID_STATICTEXT3;
        static const long id_txc_thblank;
        static const long ID_STATICTEXT29;
        static const long ID_STATICTEXT8;
        static const long id_sct_hsoffs;
        static const long ID_STATICTEXT4;
        static const long id_txc_thsoffs;
        static const long ID_STATICTEXT30;
        static const long ID_STATICTEXT9;
        static const long id_sct_hswidth;
        static const long ID_STATICTEXT10;
        static const long id_thswidth;
        static const long ID_STATICTEXT31;
        static const long ID_STATICTEXT27;
        static const long id_txc_htotal;
        static const long ID_STATICTEXT32;
        static const long id_txc_thtotal;
        static const long ID_STATICTEXT28;
        static const long ID_STATICTEXT25;
        static const long id_txc_hfreq;
        static const long ID_STATICTEXT26;
        static const long ID_STATICTEXT15;
        static const long id_sct_vres;
        static const long ID_STATICTEXT16;
        static const long id_txc_tvres;
        static const long ID_STATICTEXT34;
        static const long ID_STATICTEXT17;
        static const long id_sct_vborder;
        static const long ID_STATICTEXT18;
        static const long ID_STATICTEXT19;
        static const long id_sct_vblank;
        static const long ID_STATICTEXT20;
        static const long id_txc_tvblank;
        static const long ID_STATICTEXT35;
        static const long ID_STATICTEXT21;
        static const long id_sct_vsoffs;
        static const long ID_STATICTEXT22;
        static const long id_txc_tvsoffs;
        static const long ID_STATICTEXT36;
        static const long ID_STATICTEXT23;
        static const long id_sct_vswidth;
        static const long ID_STATICTEXT24;
        static const long id_txc_vswidth;
        static const long ID_STATICTEXT37;
        static const long ID_STATICTEXT38;
        static const long id_txc_vtotal;
        static const long ID_STATICTEXT39;
        static const long id_txc_tvtotal;
        static const long ID_STATICTEXT40;
        static const long ID_STATICTEXT41;
        static const long id_txc_modeline;
        static const long id_panel_dtd;
        static const long id_ntbook;
        static const long id_mnu_imphex;
        static const long id_mnu_exphex;
        static const long id_mnu_parse;
        static const long id_mnu_asmchg;
        static const long id_mnu_ignerr;
        static const long id_mnu_allwr;
        static const long id_mnu_dtd_asp;
        static const long id_mnu_fdetails;
        static const long id_mnu_logw;
        static const long id_mnu_flags;
        static const long id_win_stat_bar;
        //*)
        static const long id_app_layout;
        static const long id_mnu_info;
        static const long id_mnu_ins_up;
        static const long id_mnu_ins_dn;
        static const long id_mnu_ins_in;

        //(* Declarations(wxEDID_Frame)
        dtd_screen_cl* dtd_screen;
        dtd_sct_cl* sct_hblank;
        dtd_sct_cl* sct_hborder;
        dtd_sct_cl* sct_hsoffs;
        dtd_sct_cl* sct_hswidth;
        dtd_sct_cl* sct_pixclk;
        dtd_sct_cl* sct_vblank;
        dtd_sct_cl* sct_vborder;
        dtd_sct_cl* sct_vres;
        dtd_sct_cl* sct_vsoffs;
        dtd_sct_cl* sct_vswidth;
        dtd_sct_cl* sct_xres;
        fgrid_cl* BlkDataGrid;
        wxAuiManager* AuiMgrEDID;
        wxAuiManager* AuiMgrMain;
        wxAuiNotebook* ntbook;
        wxBoxSizer* bs_dtd_main;
        wxFlexGridSizer* fgs_dtd;
        wxFlexGridSizer* fgs_dtd_bottom;
        wxFlexGridSizer* fgs_dtd_right;
        wxFlexGridSizer* fgs_dtd_top;
        wxMenu* Menu3;
        wxMenuItem* MenuItem1;
        wxMenuItem* mnu_allwritable;
        wxMenuItem* mnu_assemble;
        wxMenuItem* mnu_dtd_aspect;
        wxMenuItem* mnu_fdetails;
        wxMenuItem* mnu_exphex;
        wxMenuItem* mnu_ignore_err;
        wxMenuItem* mnu_imphex;
        wxMenuItem* mnu_logw;
        wxMenuItem* mnu_open_edi;
        wxMenuItem* mnu_reparse;
        wxMenuItem* mnu_save_edi;
        wxMenuItem* mnu_save_text;
        wxPanel* dtd_panel;
        wxPanel* edid_panel;
        wxStaticText* StaticText10;
        wxStaticText* StaticText11;
        wxStaticText* StaticText12;
        wxStaticText* StaticText13;
        wxStaticText* StaticText14;
        wxStaticText* StaticText15;
        wxStaticText* StaticText16;
        wxStaticText* StaticText17;
        wxStaticText* StaticText18;
        wxStaticText* StaticText19;
        wxStaticText* StaticText1;
        wxStaticText* StaticText20;
        wxStaticText* StaticText21;
        wxStaticText* StaticText22;
        wxStaticText* StaticText23;
        wxStaticText* StaticText24;
        wxStaticText* StaticText25;
        wxStaticText* StaticText26;
        wxStaticText* StaticText27;
        wxStaticText* StaticText28;
        wxStaticText* StaticText29;
        wxStaticText* StaticText2;
        wxStaticText* StaticText30;
        wxStaticText* StaticText31;
        wxStaticText* StaticText32;
        wxStaticText* StaticText33;
        wxStaticText* StaticText34;
        wxStaticText* StaticText35;
        wxStaticText* StaticText36;
        wxStaticText* StaticText37;
        wxStaticText* StaticText38;
        wxStaticText* StaticText39;
        wxStaticText* StaticText3;
        wxStaticText* StaticText40;
        wxStaticText* StaticText41;
        wxStaticText* StaticText4;
        wxStaticText* StaticText5;
        wxStaticText* StaticText6;
        wxStaticText* StaticText7;
        wxStaticText* StaticText8;
        wxStaticText* StaticText9;
        wxStatusBar* win_stat_bar;
        wxTextCtrl* txc_edid_info;
        wxTextCtrl* txc_hfreq;
        wxTextCtrl* txc_htotal;
        wxTextCtrl* txc_modeline;
        wxTextCtrl* txc_thblank;
        wxTextCtrl* txc_thsoffs;
        wxTextCtrl* txc_thswidth;
        wxTextCtrl* txc_thtotal;
        wxTextCtrl* txc_tvblank;
        wxTextCtrl* txc_tvres;
        wxTextCtrl* txc_tvsoffs;
        wxTextCtrl* txc_tvswidth;
        wxTextCtrl* txc_tvtotal;
        wxTextCtrl* txc_txres;
        wxTextCtrl* txc_vrefresh;
        wxTextCtrl* txc_vtotal;
        blktree_cl* BlockTree;
        //*)

        wxDECLARE_EVENT_TABLE();
};

class blktree_cl : public wxTreeCtrl {
   private:
      int   keycode;

      void  evt_key(wxKeyEvent& evt);

   public:
      bool    b_key_block;

   blktree_cl(wxWindow *parent, wxWindowID id=wxID_ANY,
              const wxPoint &pos=wxDefaultPosition,
              const wxSize &size=wxDefaultSize,
              long style=wxTR_DEFAULT_STYLE,
              const wxValidator &validator=wxDefaultValidator,
              const wxString &name=wxTreeCtrlNameStr) :
              wxTreeCtrl(parent, id, pos, size, style, validator, name),
              keycode(0), b_key_block(false)
              {};

   wxDECLARE_EVENT_TABLE();
};

class fgrid_cl : public wxGrid {
   private:
      wxString  tmps;

   public:
      void evt_datagrid_vmnu(wxCommandEvent& event);

   fgrid_cl(wxWindow* parentW, wxWindowID id = wxID_ANY,
            const wxPoint& pos = wxDefaultPosition,
            const wxSize& size = wxDefaultSize,
            long style = wxWANTS_CHARS,
            const wxString& name = wxPanelNameStr) :
            wxGrid(parentW, id, pos, size, style, name) {};

   wxDECLARE_EVENT_TABLE();
};

class dtd_screen_cl : public wxPanel {
   private:
      wxColour cScrArea, cHsync, cVsync, cSandC, cResStr;
      wxRect   rcHsync, rcVsync, rcScreen;
      wxSize   szHborder, szVborder;
      wxString tmps;

      wxEDID_Frame *pwin;

      void    scr_aspect_ratio(wxSize& dcsize, wxPaintDC& dc);
      rcode   calc_coords     (wxSize& dcsize);
      void    scr_area_str    (wxPaintDC& dc);
      void    evt_paint       (wxPaintEvent& event);

   public:
      void    SetParentFrame  (wxEDID_Frame *pM) {pwin = pM;};

   dtd_screen_cl(wxWindow* parentW, wxWindowID id = wxID_ANY,
                 const wxPoint& pos = wxDefaultPosition,
                 const wxSize& size = wxDefaultSize,
                 long style = wxTAB_TRAVERSAL, const wxString& name = "dtd") :
                 wxPanel(parentW, id, pos, size, style, name)
            {
               pwin      = dynamic_cast <wxEDID_Frame*> (parentW);

               cScrArea  = wxColour(0x88, 0x88, 0xFF);
               cHsync    = wxColour(0x77, 0x77, 0x99);
               cVsync    = wxColour(0xCC, 0xDD, 0x20);
               cSandC    = wxColour(0xFF, 0xFF, 0xFF);
               cResStr   = wxColour(0xFF, 0xFF, 0xFF);

               rcHsync   = wxRect(0,0,0,0);
               rcVsync   = rcHsync;
               rcScreen  = rcHsync;

            };

   wxDECLARE_EVENT_TABLE();
};

class dtd_sct_cl : public wxSpinCtrl {
   public:
      edi_dynfld_t *field;
      int           data;

   dtd_sct_cl(wxWindow* parent, wxWindowID id = -1, const wxString& value = wxEmptyString,
              const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize,
              long style = wxSP_ARROW_KEYS, int min = 0, int max = 1000, int initial = 0,
              const wxString& name = "wxSpinCtrl") :
              wxSpinCtrl(parent, id, value, pos, size, style, min, max, initial, name)
            {
               data  = 0;
               field = NULL;
            };
};

#endif // wxEDIDMAIN_H
