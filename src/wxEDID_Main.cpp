/***************************************************************
 * Name:      wxEDID_Main.cpp
 * Purpose:   Code for Application Frame
 * Author:    Tomasz Pawlak (tomasz.pawlak@wp.eu)
 * Created:   2014-03-18
 * Copyright: Tomasz Pawlak (C) 2014-2022
 * License:   GPLv3+
 **************************************************************/

#include "debug.h"
#include "rcdunits.h"
#ifndef idMAIN
   #error "wxEDID_Main.cpp: missing unit ID"
#endif
#define RCD_UNIT idMAIN
#include "rcode/rcode.h"

#include "wxedid_rcd_scope.h"

RCD_AUTOGEN_DEFINE_UNIT

#include <wx/event.h>

#include "wxEDID_Main.h"

#include <wx/display.h>
#include <wx/artprov.h>
#include <wx/msgdlg.h>
#include <wx/filedlg.h>
#include <wx/file.h>
#include <string.h>

//(*InternalHeaders(wxEDID_Frame)
#include <wx/intl.h>
#include <wx/string.h>
//*)

//helper functions
enum wxbuildinfoformat {
    short_f, long_f };

wxString wxbuildinfo(wxbuildinfoformat format) {
    wxString wxbuild(wxVERSION_STRING);

    if (format == long_f )
    {
#if defined(__WXMSW__)
        wxbuild << _T("-Windows");
#elif defined(__UNIX__)
        wxbuild << _T("-Linux");
#endif

#if wxUSE_UNICODE
        wxbuild << _T("-Unicode build");
#else
        wxbuild << _T("-ANSI build");
#endif // wxUSE_UNICODE
    }

    return wxbuild;
}

//config vars
config_t config;

//common messages / strings
static const wxString ver = VERSION;

static const char wxEDID_About[] =
"wxEDID v" VERSION "\n\n"
"Extended Display Identification Data (EDID)\n"
"structure editor and parser.\n"
"Supported structures:\n"
"EDID v1.3+\n"
"CEA/CTA-861-G\n\n"
"Author: Tomasz Pawlak\n"
"e-mail: tomasz.pawlak@wp.eu\n"
"License: GPLv3+\n\n";

static const char Hlp_Flg_Type[] =
"Types:\n"
"Bit: single bit value\n"
"BitFld: bit field\n"
"Byte: byte\n"
"Int: integer\n"
"Float: real/floating point value\n"
"Hex: value expressed in hexadecimal format\n"
"String: string of bytes\n"
"LE: little endian\n\n"
"Flags:\n"
"RD: read-only\n"
"NU: field not used\n"
"FR: forced refresh of block data\n"
"VS: value selector menu available\n"
"GD: group descriptor: no dedicated description\n"
"NI: internal checks: value is not an integer\n";

#define wxLF   '\n'
#define wxTAB  '\t'
#define wxIDNT "  "
#define wxSP   ' '

static const wxString txt_WARNING = "WARNING";
static const wxString txt_REMARK  = "Remark";
static const wxString msgF_LOAD   =
"Failed to load EDID: incorrect/damaged data structure.\n\n"
"You may use Options->\"Ignore EDID Errors\" and then \"Reparse EDID buffer\" to view the data anyway.";

static const char AUI_DefLayout[] =
"layout2|"
"name=TreeDataCtl;caption=Block Tree;state=5892;dir=4;layer=0;row=0;pos=0;"
"prop=100000;bestw=150;besth=80;minw=150;minh=-1;maxw=-1;maxh=-1;"
"floatx=-1;floaty=-1;floatw=-1;floath=-1|"

"name=GridDataCtl;caption=Block Data;state=5896;dir=5;layer=0;row=0;pos=0;"
"prop=137878;bestw=82;besth=16;minw=-1;minh=-1;maxw=-1;maxh=-1;"
"floatx=-1;floaty=-1;floatw=-1;floath=-1|"

"name=InfoCtl;caption=Info;state=5892;dir=5;layer=0;row=0;pos=1;"
"prop=62122;bestw=80;besth=100;minw=-1;minh=100;maxw=-1;maxh=-1;"
"floatx=-1;floaty=-1;floatw=-1;floath=-1|"

"dock_size(4,0,0)=215|dock_size(5,0,0)=10|";


//(*IdInit(wxEDID_Frame)
const long wxEDID_Frame::id_block_tree = wxNewId();
const long wxEDID_Frame::id_grid_blkdat = wxNewId();
const long wxEDID_Frame::id_txc_edid_info = wxNewId();
const long wxEDID_Frame::id_panel_edid = wxNewId();
const long wxEDID_Frame::ID_STATICTEXT5 = wxNewId();
const long wxEDID_Frame::ID_STATICTEXT6 = wxNewId();
const long wxEDID_Frame::id_sct_pixclk = wxNewId();
const long wxEDID_Frame::ID_STATICTEXT1 = wxNewId();
const long wxEDID_Frame::id_txc_vrefresh = wxNewId();
const long wxEDID_Frame::ID_STATICTEXT2 = wxNewId();
const long wxEDID_Frame::id_dtd_screen = wxNewId();
const long wxEDID_Frame::ID_STATICTEXT12 = wxNewId();
const long wxEDID_Frame::id_sct_xres = wxNewId();
const long wxEDID_Frame::ID_STATICTEXT11 = wxNewId();
const long wxEDID_Frame::id_txres = wxNewId();
const long wxEDID_Frame::ID_STATICTEXT33 = wxNewId();
const long wxEDID_Frame::ID_STATICTEXT14 = wxNewId();
const long wxEDID_Frame::id_sct_hborder = wxNewId();
const long wxEDID_Frame::ID_STATICTEXT13 = wxNewId();
const long wxEDID_Frame::ID_STATICTEXT7 = wxNewId();
const long wxEDID_Frame::id_sct_hblank = wxNewId();
const long wxEDID_Frame::ID_STATICTEXT3 = wxNewId();
const long wxEDID_Frame::id_txc_thblank = wxNewId();
const long wxEDID_Frame::ID_STATICTEXT29 = wxNewId();
const long wxEDID_Frame::ID_STATICTEXT8 = wxNewId();
const long wxEDID_Frame::id_sct_hsoffs = wxNewId();
const long wxEDID_Frame::ID_STATICTEXT4 = wxNewId();
const long wxEDID_Frame::id_txc_thsoffs = wxNewId();
const long wxEDID_Frame::ID_STATICTEXT30 = wxNewId();
const long wxEDID_Frame::ID_STATICTEXT9 = wxNewId();
const long wxEDID_Frame::id_sct_hswidth = wxNewId();
const long wxEDID_Frame::ID_STATICTEXT10 = wxNewId();
const long wxEDID_Frame::id_thswidth = wxNewId();
const long wxEDID_Frame::ID_STATICTEXT31 = wxNewId();
const long wxEDID_Frame::ID_STATICTEXT27 = wxNewId();
const long wxEDID_Frame::id_txc_htotal = wxNewId();
const long wxEDID_Frame::ID_STATICTEXT32 = wxNewId();
const long wxEDID_Frame::id_txc_thtotal = wxNewId();
const long wxEDID_Frame::ID_STATICTEXT28 = wxNewId();
const long wxEDID_Frame::ID_STATICTEXT25 = wxNewId();
const long wxEDID_Frame::id_txc_hfreq = wxNewId();
const long wxEDID_Frame::ID_STATICTEXT26 = wxNewId();
const long wxEDID_Frame::ID_STATICTEXT15 = wxNewId();
const long wxEDID_Frame::id_sct_vres = wxNewId();
const long wxEDID_Frame::ID_STATICTEXT16 = wxNewId();
const long wxEDID_Frame::id_txc_tvres = wxNewId();
const long wxEDID_Frame::ID_STATICTEXT34 = wxNewId();
const long wxEDID_Frame::ID_STATICTEXT17 = wxNewId();
const long wxEDID_Frame::id_sct_vborder = wxNewId();
const long wxEDID_Frame::ID_STATICTEXT18 = wxNewId();
const long wxEDID_Frame::ID_STATICTEXT19 = wxNewId();
const long wxEDID_Frame::id_sct_vblank = wxNewId();
const long wxEDID_Frame::ID_STATICTEXT20 = wxNewId();
const long wxEDID_Frame::id_txc_tvblank = wxNewId();
const long wxEDID_Frame::ID_STATICTEXT35 = wxNewId();
const long wxEDID_Frame::ID_STATICTEXT21 = wxNewId();
const long wxEDID_Frame::id_sct_vsoffs = wxNewId();
const long wxEDID_Frame::ID_STATICTEXT22 = wxNewId();
const long wxEDID_Frame::id_txc_tvsoffs = wxNewId();
const long wxEDID_Frame::ID_STATICTEXT36 = wxNewId();
const long wxEDID_Frame::ID_STATICTEXT23 = wxNewId();
const long wxEDID_Frame::id_sct_vswidth = wxNewId();
const long wxEDID_Frame::ID_STATICTEXT24 = wxNewId();
const long wxEDID_Frame::id_txc_vswidth = wxNewId();
const long wxEDID_Frame::ID_STATICTEXT37 = wxNewId();
const long wxEDID_Frame::ID_STATICTEXT38 = wxNewId();
const long wxEDID_Frame::id_txc_vtotal = wxNewId();
const long wxEDID_Frame::ID_STATICTEXT39 = wxNewId();
const long wxEDID_Frame::id_txc_tvtotal = wxNewId();
const long wxEDID_Frame::ID_STATICTEXT40 = wxNewId();
const long wxEDID_Frame::ID_STATICTEXT41 = wxNewId();
const long wxEDID_Frame::id_txc_modeline = wxNewId();
const long wxEDID_Frame::id_panel_dtd = wxNewId();
const long wxEDID_Frame::id_ntbook = wxNewId();
const long wxEDID_Frame::id_mnu_imphex = wxNewId();
const long wxEDID_Frame::id_mnu_exphex = wxNewId();
const long wxEDID_Frame::id_mnu_parse = wxNewId();
const long wxEDID_Frame::id_mnu_asmchg = wxNewId();
const long wxEDID_Frame::id_mnu_ignerr = wxNewId();
const long wxEDID_Frame::id_mnu_allwr = wxNewId();
const long wxEDID_Frame::id_mnu_dtd_asp = wxNewId();
const long wxEDID_Frame::id_mnu_fdetails = wxNewId();
const long wxEDID_Frame::id_mnu_logw = wxNewId();
const long wxEDID_Frame::id_mnu_flags = wxNewId();
const long wxEDID_Frame::id_win_stat_bar = wxNewId();
//*)
const long wxEDID_Frame::id_app_layout = wxNewId();
const long wxEDID_Frame::id_mnu_info   = wxNewId();
const long wxEDID_Frame::id_mnu_ins_up = wxNewId();
const long wxEDID_Frame::id_mnu_ins_dn = wxNewId();
const long wxEDID_Frame::id_mnu_ins_in = wxNewId();

wxDECLARE_EVENT(wxEVT_DEFERRED, wxCommandEvent);
wxDEFINE_EVENT (wxEVT_DEFERRED, wxCommandEvent);

wxBEGIN_EVENT_TABLE(wxEDID_Frame, wxFrame)
    //(*EventTable(wxEDID_Frame)
    //*)
    EVT_SIZE                   (                 wxEDID_Frame::evt_frame_size        )
    EVT_GRID_SELECT_CELL       (                 wxEDID_Frame::evt_datagrid_select   )
  //EVT_GRID_CELL_CHANGED      (                 wxEDID_Frame::evt_datagrid_write    ) //dynamic
    EVT_GRID_CMD_EDITOR_SHOWN  (id_grid_blkdat , wxEDID_Frame::evt_datagrid_vsel     )
    EVT_GRID_CMD_EDITOR_HIDDEN (id_grid_blkdat , wxEDID_Frame::evt_datagrid_edit_hide)
    EVT_TREE_SEL_CHANGED       (id_block_tree  , wxEDID_Frame::evt_blktree_sel       )
    EVT_TREE_ITEM_RIGHT_CLICK  (id_block_tree  , wxEDID_Frame::evt_blktree_rmb       )
    EVT_TREE_KEY_DOWN          (id_block_tree  , wxEDID_Frame::evt_blktree_key       )

    EVT_COMMAND                (wxID_ANY       , wxEVT_DEFERRED, wxEDID_Frame::evt_Deferred)
    EVT_AUINOTEBOOK_PAGE_CHANGING(id_ntbook    , wxEDID_Frame::evt_ntbook_page       )

    EVT_MENU                   (wxID_OPEN      , wxEDID_Frame::evt_open_edid_bin     )
    EVT_MENU                   (id_mnu_imphex  , wxEDID_Frame::evt_import_hex        )
    EVT_MENU                   (wxID_SAVE      , wxEDID_Frame::evt_save_edid_bin     )
    EVT_MENU                   (wxID_SAVEAS    , wxEDID_Frame::evt_save_report       )
    EVT_MENU                   (id_mnu_exphex  , wxEDID_Frame::evt_export_hex        )
    EVT_MENU                   (wxID_EXIT      , wxEDID_Frame::evt_Quit              )
    EVT_MENU                   (wxID_ABOUT     , wxEDID_Frame::evt_About             )
    EVT_MENU                   (id_mnu_flags   , wxEDID_Frame::evt_Flags             )
    EVT_MENU                   (id_mnu_logw    , wxEDID_Frame::evt_log_win           )
    EVT_MENU                   (id_mnu_allwr   , wxEDID_Frame::evt_ignore_rd         )
    EVT_MENU                   (id_mnu_ignerr  , wxEDID_Frame::evt_ignore_err        )
    EVT_MENU                   (id_mnu_parse   , wxEDID_Frame::evt_reparse           )
    EVT_MENU                   (id_mnu_asmchg  , wxEDID_Frame::evt_assemble_edid     )
    EVT_MENU                   (id_mnu_dtd_asp , wxEDID_Frame::evt_dtd_asp           )
    EVT_MENU                   (id_mnu_fdetails, wxEDID_Frame::evt_blk_fdetails      )

    //RMB menu actions for BlockTree
    EVT_MENU                   (wxID_EXECUTE   , wxEDID_Frame::evt_blktree_reparse)
    EVT_MENU                   (wxID_COPY      , wxEDID_Frame::evt_blktree_copy   )
    EVT_MENU                   (wxID_PASTE     , wxEDID_Frame::evt_blktree_paste  )
    EVT_MENU                   (wxID_DELETE    , wxEDID_Frame::evt_blktree_delete )
    EVT_MENU                   (wxID_CUT       , wxEDID_Frame::evt_blktree_cut    )
    EVT_MENU                   (id_mnu_ins_up  , wxEDID_Frame::evt_blktree_insert )
    EVT_MENU                   (id_mnu_ins_in  , wxEDID_Frame::evt_blktree_insert )
    EVT_MENU                   (id_mnu_ins_dn  , wxEDID_Frame::evt_blktree_insert )
    EVT_MENU                   (wxID_UP        , wxEDID_Frame::evt_blktree_move   )
    EVT_MENU                   (wxID_DOWN      , wxEDID_Frame::evt_blktree_move   )
wxEND_EVENT_TABLE()

#pragma GCC diagnostic ignored "-Wunused-parameter"

// Workaround for gcc v8.x :
// Warnings of type mismatch for event handlers cast using "wxObjectEventFunction"
#if wxCHECK_GCC_VERSION(8, 0)
   #pragma GCC diagnostic ignored "-Wcast-function-type"
#endif

wxEDID_Frame::wxEDID_Frame(wxWindow* parent,wxWindowID id) :
   grid_color_int    (wxColour(0xFF, 0xFF, 0xFF)),
   grid_color_bit    (wxColour(0xFF, 0xFF, 0xCC)),
   grid_color_float  (wxColour(0xFF, 0xDD, 0xFF)),
   grid_color_hex    (wxColour(0xDD, 0xDD, 0xFF)),
   b_dta_grid_details(config.b_dta_grid_details ),
   b_dtd_keep_aspect (config.b_dtd_keep_aspect  )
{
    //(*Initialize(wxEDID_Frame)
    wxBoxSizer* BoxSizer5;
    wxMenu* Menu1;
    wxMenu* Menu2;
    wxMenuBar* MenuBar1;
    wxMenuItem* mnu_about;
    wxMenuItem* mnu_quit;

    Create(parent, wxID_ANY, _("wxEDID"), wxDefaultPosition, wxDefaultSize, wxDEFAULT_FRAME_STYLE, _T("wxID_ANY"));
    SetMinSize(wxSize(640,440));
    AuiMgrMain = new wxAuiManager(this, 0);
    ntbook = new wxAuiNotebook(this, id_ntbook, wxDefaultPosition, wxDefaultSize, 0);
    edid_panel = new wxPanel(ntbook, id_panel_edid, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("id_panel_edid"));
    AuiMgrEDID = new wxAuiManager(edid_panel, wxAUI_MGR_RECTANGLE_HINT);
    BlockTree = new blktree_cl(edid_panel, id_block_tree, wxDefaultPosition, wxDefaultSize, wxTR_HAS_BUTTONS|wxTR_SINGLE|wxTAB_TRAVERSAL, wxDefaultValidator, _T("id_block_tree"));
    BlockTree->SetMinSize(wxSize(150,-1));
    AuiMgrEDID->AddPane(BlockTree, wxAuiPaneInfo().Name(_T("TreeDataCtl")).DefaultPane().Caption(_("Block Tree")).CloseButton(false).Left().TopDockable(false).BottomDockable(false).RightDockable(false).Floatable(false).MinSize(wxSize(150,-1)).Movable(false).DestroyOnClose());
    BlkDataGrid = new fgrid_cl(edid_panel, id_grid_blkdat, wxDefaultPosition, wxDefaultSize, 0, _T("id_grid_blkdat"));
    BlkDataGrid->CreateGrid(8,5);
    BlkDataGrid->Disable();
    BlkDataGrid->EnableEditing(true);
    BlkDataGrid->EnableGridLines(true);
    BlkDataGrid->SetColLabelValue(0, _("Name"));
    BlkDataGrid->SetColLabelValue(1, _("Type"));
    BlkDataGrid->SetColLabelValue(2, _("Value"));
    BlkDataGrid->SetColLabelValue(3, _("Unit"));
    BlkDataGrid->SetColLabelValue(4, _("Flags"));
    BlkDataGrid->SetDefaultCellFont( BlkDataGrid->GetFont() );
    BlkDataGrid->SetDefaultCellTextColour( BlkDataGrid->GetForegroundColour() );
    AuiMgrEDID->AddPane(BlkDataGrid, wxAuiPaneInfo().Name(_T("GridDataCtl")).DefaultPane().Caption(_("Block Data")).CloseButton(false).Center().TopDockable(false).BottomDockable(false).LeftDockable(false).Floatable(false).Movable(false).DestroyOnClose());
    txc_edid_info = new wxTextCtrl(edid_panel, id_txc_edid_info, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE|wxTE_READONLY|wxTE_WORDWRAP|wxVSCROLL, wxDefaultValidator, _T("id_txc_edid_info"));
    txc_edid_info->SetMinSize(wxSize(-1,100));
    txc_edid_info->SetForegroundColour(wxColour(0,0,0));
    txc_edid_info->SetBackgroundColour(wxColour(255,255,255));
    AuiMgrEDID->AddPane(txc_edid_info, wxAuiPaneInfo().Name(_T("InfoCtl")).DefaultPane().Caption(_("Info")).CloseButton(false).Position(1).Center().TopDockable(false).BottomDockable(false).RightDockable(false).Floatable(false).MinSize(wxSize(-1,100)).Movable(false).DestroyOnClose());
    AuiMgrEDID->Update();
    dtd_panel = new wxPanel(ntbook, id_panel_dtd, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("id_panel_dtd"));
    bs_dtd_main = new wxBoxSizer(wxVERTICAL);
    fgs_dtd = new wxFlexGridSizer(3, 2, 0, 0);
    fgs_dtd_top = new wxFlexGridSizer(4, 4, 0, 0);
    StaticText5 = new wxStaticText(dtd_panel, ID_STATICTEXT5, _("Pixel Clock"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT5"));
    fgs_dtd_top->Add(StaticText5, 1, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    fgs_dtd_top->Add(-1,-1,1, wxLEFT|wxRIGHT|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 8);
    StaticText6 = new wxStaticText(dtd_panel, ID_STATICTEXT6, _("V-Refresh"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT6"));
    fgs_dtd_top->Add(StaticText6, 1, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    fgs_dtd_top->Add(-1,-1,1, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    sct_pixclk = new dtd_sct_cl(dtd_panel, id_sct_pixclk, _T("0"), wxDefaultPosition, wxDefaultSize, 0, 1, 655350, 0, _T("id_sct_pixclk"));
    sct_pixclk->SetValue(_T("0"));
    fgs_dtd_top->Add(sct_pixclk, 1, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticText1 = new wxStaticText(dtd_panel, ID_STATICTEXT1, _("x10kHz"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT1"));
    fgs_dtd_top->Add(StaticText1, 2, wxRIGHT|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 8);
    txc_vrefresh = new wxTextCtrl(dtd_panel, id_txc_vrefresh, _("0"), wxDefaultPosition, wxDefaultSize, wxTE_READONLY, wxDefaultValidator, _T("id_txc_vrefresh"));
    txc_vrefresh->SetMinSize(wxSize(70,-1));
    txc_vrefresh->SetBackgroundColour(wxColour(224,224,224));
    fgs_dtd_top->Add(txc_vrefresh, 1, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    StaticText2 = new wxStaticText(dtd_panel, ID_STATICTEXT2, _("Hz"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT2"));
    fgs_dtd_top->Add(StaticText2, 2, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    fgs_dtd->Add(fgs_dtd_top, 1, wxTOP|wxBOTTOM|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 6);
    fgs_dtd->Add(-1,-1,1, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    dtd_screen = new dtd_screen_cl(dtd_panel, id_dtd_screen, wxDefaultPosition, wxDefaultSize, wxFULL_REPAINT_ON_RESIZE, _T("id_dtd_screen"));
    fgs_dtd->Add(dtd_screen, 1, wxEXPAND, 4);
    fgs_dtd_right = new wxFlexGridSizer(7, 5, 1, 5);
    StaticText12 = new wxStaticText(dtd_panel, ID_STATICTEXT12, _("X-res"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT12"));
    fgs_dtd_right->Add(StaticText12, 1, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    sct_xres = new dtd_sct_cl(dtd_panel, id_sct_xres, _T("0"), wxDefaultPosition, wxDefaultSize, 0, 1, 4095, 0, _T("id_sct_xres"));
    sct_xres->SetValue(_T("0"));
    fgs_dtd_right->Add(sct_xres, 1, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5);
    StaticText11 = new wxStaticText(dtd_panel, ID_STATICTEXT11, _("pix"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT11"));
    fgs_dtd_right->Add(StaticText11, 1, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    txc_txres = new wxTextCtrl(dtd_panel, id_txres, _("0"), wxDefaultPosition, wxDefaultSize, wxTE_READONLY, wxDefaultValidator, _T("id_txres"));
    txc_txres->SetMinSize(wxSize(70,-1));
    txc_txres->SetBackgroundColour(wxColour(224,224,224));
    fgs_dtd_right->Add(txc_txres, 1, wxLEFT|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    StaticText33 = new wxStaticText(dtd_panel, ID_STATICTEXT33, _("us"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT33"));
    fgs_dtd_right->Add(StaticText33, 1, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    StaticText14 = new wxStaticText(dtd_panel, ID_STATICTEXT14, _("H-Border"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT14"));
    fgs_dtd_right->Add(StaticText14, 1, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    sct_hborder = new dtd_sct_cl(dtd_panel, id_sct_hborder, _T("0"), wxDefaultPosition, wxDefaultSize, 0, 0, 255, 0, _T("id_sct_hborder"));
    sct_hborder->SetValue(_T("0"));
    fgs_dtd_right->Add(sct_hborder, 1, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5);
    StaticText13 = new wxStaticText(dtd_panel, ID_STATICTEXT13, _("pix"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT13"));
    fgs_dtd_right->Add(StaticText13, 1, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    fgs_dtd_right->Add(-1,-1,1, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    fgs_dtd_right->Add(-1,-1,1, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticText7 = new wxStaticText(dtd_panel, ID_STATICTEXT7, _("H-Blank"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT7"));
    fgs_dtd_right->Add(StaticText7, 1, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    sct_hblank = new dtd_sct_cl(dtd_panel, id_sct_hblank, _T("0"), wxDefaultPosition, wxDefaultSize, 0, 1, 4095, 0, _T("id_sct_hblank"));
    sct_hblank->SetValue(_T("0"));
    fgs_dtd_right->Add(sct_hblank, 1, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5);
    StaticText3 = new wxStaticText(dtd_panel, ID_STATICTEXT3, _("pix"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT3"));
    fgs_dtd_right->Add(StaticText3, 1, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    txc_thblank = new wxTextCtrl(dtd_panel, id_txc_thblank, _("0"), wxDefaultPosition, wxSize(50,22), wxTE_READONLY, wxDefaultValidator, _T("id_txc_thblank"));
    txc_thblank->SetMinSize(wxSize(70,-1));
    txc_thblank->SetBackgroundColour(wxColour(224,224,224));
    fgs_dtd_right->Add(txc_thblank, 1, wxLEFT|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    StaticText29 = new wxStaticText(dtd_panel, ID_STATICTEXT29, _("us"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT29"));
    fgs_dtd_right->Add(StaticText29, 1, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    StaticText8 = new wxStaticText(dtd_panel, ID_STATICTEXT8, _("H-Sync offs."), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT8"));
    fgs_dtd_right->Add(StaticText8, 1, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    sct_hsoffs = new dtd_sct_cl(dtd_panel, id_sct_hsoffs, _T("0"), wxDefaultPosition, wxDefaultSize, 0, 0, 1023, 0, _T("id_sct_hsoffs"));
    sct_hsoffs->SetValue(_T("0"));
    fgs_dtd_right->Add(sct_hsoffs, 1, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5);
    StaticText4 = new wxStaticText(dtd_panel, ID_STATICTEXT4, _("pix"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT4"));
    fgs_dtd_right->Add(StaticText4, 1, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    txc_thsoffs = new wxTextCtrl(dtd_panel, id_txc_thsoffs, _("0"), wxDefaultPosition, wxSize(50,22), wxTE_READONLY, wxDefaultValidator, _T("id_txc_thsoffs"));
    txc_thsoffs->SetMinSize(wxSize(70,-1));
    txc_thsoffs->SetBackgroundColour(wxColour(224,224,224));
    fgs_dtd_right->Add(txc_thsoffs, 1, wxLEFT|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    StaticText30 = new wxStaticText(dtd_panel, ID_STATICTEXT30, _("us"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT30"));
    fgs_dtd_right->Add(StaticText30, 1, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    StaticText9 = new wxStaticText(dtd_panel, ID_STATICTEXT9, _("H-Sync width"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT9"));
    fgs_dtd_right->Add(StaticText9, 1, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    sct_hswidth = new dtd_sct_cl(dtd_panel, id_sct_hswidth, _T("0"), wxDefaultPosition, wxDefaultSize, 0, 1, 1023, 0, _T("id_sct_hswidth"));
    sct_hswidth->SetValue(_T("0"));
    fgs_dtd_right->Add(sct_hswidth, 1, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5);
    StaticText10 = new wxStaticText(dtd_panel, ID_STATICTEXT10, _("pix"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT10"));
    fgs_dtd_right->Add(StaticText10, 1, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    txc_thswidth = new wxTextCtrl(dtd_panel, id_thswidth, _("0"), wxDefaultPosition, wxDefaultSize, wxTE_READONLY, wxDefaultValidator, _T("id_thswidth"));
    txc_thswidth->SetMinSize(wxSize(70,-1));
    txc_thswidth->SetBackgroundColour(wxColour(224,224,224));
    fgs_dtd_right->Add(txc_thswidth, 1, wxLEFT|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    StaticText31 = new wxStaticText(dtd_panel, ID_STATICTEXT31, _("us"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT31"));
    fgs_dtd_right->Add(StaticText31, 1, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    StaticText27 = new wxStaticText(dtd_panel, ID_STATICTEXT27, _("H total"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT27"));
    fgs_dtd_right->Add(StaticText27, 1, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    txc_htotal = new wxTextCtrl(dtd_panel, id_txc_htotal, _("0"), wxDefaultPosition, wxSize(60,22), wxTE_READONLY, wxDefaultValidator, _T("id_txc_htotal"));
    txc_htotal->SetMinSize(wxSize(80,-1));
    txc_htotal->SetBackgroundColour(wxColour(224,224,224));
    fgs_dtd_right->Add(txc_htotal, 1, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5);
    StaticText32 = new wxStaticText(dtd_panel, ID_STATICTEXT32, _("pix"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT32"));
    fgs_dtd_right->Add(StaticText32, 1, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    txc_thtotal = new wxTextCtrl(dtd_panel, id_txc_thtotal, _("0"), wxDefaultPosition, wxDefaultSize, wxTE_READONLY, wxDefaultValidator, _T("id_txc_thtotal"));
    txc_thtotal->SetMinSize(wxSize(70,-1));
    txc_thtotal->SetBackgroundColour(wxColour(224,224,224));
    fgs_dtd_right->Add(txc_thtotal, 1, wxLEFT|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    StaticText28 = new wxStaticText(dtd_panel, ID_STATICTEXT28, _("us"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT28"));
    fgs_dtd_right->Add(StaticText28, 1, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    StaticText25 = new wxStaticText(dtd_panel, ID_STATICTEXT25, _("H-Freq"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT25"));
    fgs_dtd_right->Add(StaticText25, 1, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    txc_hfreq = new wxTextCtrl(dtd_panel, id_txc_hfreq, _("0"), wxDefaultPosition, wxDefaultSize, wxTE_READONLY, wxDefaultValidator, _T("id_txc_hfreq"));
    txc_hfreq->SetMinSize(wxSize(80,-1));
    txc_hfreq->SetBackgroundColour(wxColour(224,224,224));
    fgs_dtd_right->Add(txc_hfreq, 1, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5);
    StaticText26 = new wxStaticText(dtd_panel, ID_STATICTEXT26, _("kHz"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT26"));
    fgs_dtd_right->Add(StaticText26, 1, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    fgs_dtd_right->Add(-1,-1,1, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    fgs_dtd_right->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    fgs_dtd->Add(fgs_dtd_right, 1, wxLEFT|wxRIGHT|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 4);
    fgs_dtd_bottom = new wxFlexGridSizer(6, 5, 1, 5);
    StaticText15 = new wxStaticText(dtd_panel, ID_STATICTEXT15, _("V-res"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT15"));
    fgs_dtd_bottom->Add(StaticText15, 1, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    sct_vres = new dtd_sct_cl(dtd_panel, id_sct_vres, _T("0"), wxDefaultPosition, wxDefaultSize, 0, 1, 4095, 0, _T("id_sct_vres"));
    sct_vres->SetValue(_T("0"));
    fgs_dtd_bottom->Add(sct_vres, 1, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5);
    StaticText16 = new wxStaticText(dtd_panel, ID_STATICTEXT16, _("pix"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT16"));
    fgs_dtd_bottom->Add(StaticText16, 1, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    txc_tvres = new wxTextCtrl(dtd_panel, id_txc_tvres, _("0"), wxDefaultPosition, wxDefaultSize, wxTE_READONLY, wxDefaultValidator, _T("id_txc_tvres"));
    txc_tvres->SetMinSize(wxSize(70,-1));
    txc_tvres->SetBackgroundColour(wxColour(224,224,224));
    fgs_dtd_bottom->Add(txc_tvres, 1, wxLEFT|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticText34 = new wxStaticText(dtd_panel, ID_STATICTEXT34, _("ms"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT34"));
    fgs_dtd_bottom->Add(StaticText34, 1, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    StaticText17 = new wxStaticText(dtd_panel, ID_STATICTEXT17, _("V-Border"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT17"));
    fgs_dtd_bottom->Add(StaticText17, 1, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    sct_vborder = new dtd_sct_cl(dtd_panel, id_sct_vborder, _T("0"), wxDefaultPosition, wxDefaultSize, 0, 0, 255, 0, _T("id_sct_vborder"));
    sct_vborder->SetValue(_T("0"));
    fgs_dtd_bottom->Add(sct_vborder, 1, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5);
    StaticText18 = new wxStaticText(dtd_panel, ID_STATICTEXT18, _("lines"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT18"));
    fgs_dtd_bottom->Add(StaticText18, 1, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    fgs_dtd_bottom->Add(-1,-1,1, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    fgs_dtd_bottom->Add(-1,-1,1, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticText19 = new wxStaticText(dtd_panel, ID_STATICTEXT19, _("V-Blank"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT19"));
    fgs_dtd_bottom->Add(StaticText19, 1, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    sct_vblank = new dtd_sct_cl(dtd_panel, id_sct_vblank, _T("0"), wxDefaultPosition, wxDefaultSize, 0, 1, 4095, 0, _T("id_sct_vblank"));
    sct_vblank->SetValue(_T("0"));
    fgs_dtd_bottom->Add(sct_vblank, 1, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5);
    StaticText20 = new wxStaticText(dtd_panel, ID_STATICTEXT20, _("lines"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT20"));
    fgs_dtd_bottom->Add(StaticText20, 1, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    txc_tvblank = new wxTextCtrl(dtd_panel, id_txc_tvblank, _("0"), wxDefaultPosition, wxDefaultSize, wxTE_READONLY, wxDefaultValidator, _T("id_txc_tvblank"));
    txc_tvblank->SetMinSize(wxSize(70,-1));
    txc_tvblank->SetBackgroundColour(wxColour(224,224,224));
    fgs_dtd_bottom->Add(txc_tvblank, 1, wxLEFT|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticText35 = new wxStaticText(dtd_panel, ID_STATICTEXT35, _("ms"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT35"));
    fgs_dtd_bottom->Add(StaticText35, 1, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    StaticText21 = new wxStaticText(dtd_panel, ID_STATICTEXT21, _("V-Sync offs."), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT21"));
    fgs_dtd_bottom->Add(StaticText21, 1, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    sct_vsoffs = new dtd_sct_cl(dtd_panel, id_sct_vsoffs, _T("0"), wxDefaultPosition, wxDefaultSize, 0, 0, 1023, 0, _T("id_sct_vsoffs"));
    sct_vsoffs->SetValue(_T("0"));
    fgs_dtd_bottom->Add(sct_vsoffs, 1, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5);
    StaticText22 = new wxStaticText(dtd_panel, ID_STATICTEXT22, _("lines"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT22"));
    fgs_dtd_bottom->Add(StaticText22, 1, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    txc_tvsoffs = new wxTextCtrl(dtd_panel, id_txc_tvsoffs, _("0"), wxDefaultPosition, wxDefaultSize, wxTE_READONLY, wxDefaultValidator, _T("id_txc_tvsoffs"));
    txc_tvsoffs->SetMinSize(wxSize(70,-1));
    txc_tvsoffs->SetBackgroundColour(wxColour(224,224,224));
    fgs_dtd_bottom->Add(txc_tvsoffs, 1, wxLEFT|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticText36 = new wxStaticText(dtd_panel, ID_STATICTEXT36, _("ms"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT36"));
    fgs_dtd_bottom->Add(StaticText36, 1, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    StaticText23 = new wxStaticText(dtd_panel, ID_STATICTEXT23, _("V-Sync width"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT23"));
    fgs_dtd_bottom->Add(StaticText23, 1, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    sct_vswidth = new dtd_sct_cl(dtd_panel, id_sct_vswidth, _T("0"), wxDefaultPosition, wxDefaultSize, 0, 1, 1023, 0, _T("id_sct_vswidth"));
    sct_vswidth->SetValue(_T("0"));
    fgs_dtd_bottom->Add(sct_vswidth, 1, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5);
    StaticText24 = new wxStaticText(dtd_panel, ID_STATICTEXT24, _("lines"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT24"));
    fgs_dtd_bottom->Add(StaticText24, 1, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    txc_tvswidth = new wxTextCtrl(dtd_panel, id_txc_vswidth, _("0"), wxDefaultPosition, wxDefaultSize, wxTE_READONLY, wxDefaultValidator, _T("id_txc_vswidth"));
    txc_tvswidth->SetMinSize(wxSize(70,-1));
    txc_tvswidth->SetBackgroundColour(wxColour(224,224,224));
    fgs_dtd_bottom->Add(txc_tvswidth, 1, wxLEFT|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticText37 = new wxStaticText(dtd_panel, ID_STATICTEXT37, _("ms"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT37"));
    fgs_dtd_bottom->Add(StaticText37, 1, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    StaticText38 = new wxStaticText(dtd_panel, ID_STATICTEXT38, _("V total"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT38"));
    fgs_dtd_bottom->Add(StaticText38, 1, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    txc_vtotal = new wxTextCtrl(dtd_panel, id_txc_vtotal, _("0"), wxDefaultPosition, wxDefaultSize, wxTE_READONLY, wxDefaultValidator, _T("id_txc_vtotal"));
    txc_vtotal->SetMinSize(wxSize(70,-1));
    txc_vtotal->SetBackgroundColour(wxColour(224,224,224));
    fgs_dtd_bottom->Add(txc_vtotal, 1, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5);
    StaticText39 = new wxStaticText(dtd_panel, ID_STATICTEXT39, _("lines"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT39"));
    fgs_dtd_bottom->Add(StaticText39, 1, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    txc_tvtotal = new wxTextCtrl(dtd_panel, id_txc_tvtotal, _("0"), wxDefaultPosition, wxDefaultSize, wxTE_READONLY, wxDefaultValidator, _T("id_txc_tvtotal"));
    txc_tvtotal->SetMinSize(wxSize(70,-1));
    txc_tvtotal->SetBackgroundColour(wxColour(224,224,224));
    fgs_dtd_bottom->Add(txc_tvtotal, 1, wxLEFT|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticText40 = new wxStaticText(dtd_panel, ID_STATICTEXT40, _("ms"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT40"));
    fgs_dtd_bottom->Add(StaticText40, 1, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    fgs_dtd->Add(fgs_dtd_bottom, 1, wxTOP|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 4);
    bs_dtd_main->Add(fgs_dtd, 1, wxLEFT|wxEXPAND, 16);
    BoxSizer5 = new wxBoxSizer(wxHORIZONTAL);
    StaticText41 = new wxStaticText(dtd_panel, ID_STATICTEXT41, _("X11 ModeLine"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT41"));
    BoxSizer5->Add(StaticText41, 4, wxLEFT|wxRIGHT|wxALIGN_CENTER_VERTICAL, 5);
    txc_modeline = new wxTextCtrl(dtd_panel, id_txc_modeline, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_READONLY, wxDefaultValidator, _T("id_txc_modeline"));
    txc_modeline->SetBackgroundColour(wxColour(224,224,224));
    BoxSizer5->Add(txc_modeline, 19, wxLEFT|wxRIGHT|wxEXPAND, 4);
    bs_dtd_main->Add(BoxSizer5, 0, wxTOP|wxBOTTOM|wxEXPAND, 4);
    dtd_panel->SetSizer(bs_dtd_main);
    bs_dtd_main->Fit(dtd_panel);
    bs_dtd_main->SetSizeHints(dtd_panel);
    ntbook->AddPage(edid_panel, _("EDID"));
    ntbook->AddPage(dtd_panel, _("DTD Constructor"));
    AuiMgrMain->AddPane(ntbook, wxAuiPaneInfo().Name(_T("AUInbook")).DefaultPane().Caption(_("Nbook")).CaptionVisible(false).CloseButton(false).Center().Floatable(false).Movable(false).PaneBorder(false));
    AuiMgrMain->Update();
    MenuBar1 = new wxMenuBar();
    Menu1 = new wxMenu();
    mnu_open_edi = new wxMenuItem(Menu1, wxID_OPEN, _("Open EDID binary\tctrl-O"), wxEmptyString, wxITEM_NORMAL);
    Menu1->Append(mnu_open_edi);
    mnu_save_edi = new wxMenuItem(Menu1, wxID_SAVE, _("Save EDID binary\tctrl-S"), wxEmptyString, wxITEM_NORMAL);
    Menu1->Append(mnu_save_edi);
    mnu_save_edi->Enable(false);
    mnu_save_text = new wxMenuItem(Menu1, wxID_SAVEAS, _("Save as text"), _("Saves EDID as human readable text file"), wxITEM_NORMAL);
    Menu1->Append(mnu_save_text);
    mnu_save_text->Enable(false);
    mnu_imphex = new wxMenuItem(Menu1, id_mnu_imphex, _("Import EDID from HEX (ASCII)"), wxEmptyString, wxITEM_NORMAL);
    Menu1->Append(mnu_imphex);
    mnu_exphex = new wxMenuItem(Menu1, id_mnu_exphex, _("Export EDID as HEX (ASCII)"), wxEmptyString, wxITEM_NORMAL);
    Menu1->Append(mnu_exphex);
    mnu_exphex->Enable(false);
    mnu_quit = new wxMenuItem(Menu1, wxID_EXIT, _("Quit\tAlt-F4"), _("Quit the application"), wxITEM_NORMAL);
    Menu1->Append(mnu_quit);
    MenuBar1->Append(Menu1, _("&File"));
    Menu3 = new wxMenu();
    mnu_reparse = new wxMenuItem(Menu3, id_mnu_parse, _("Reparse EDID buffer"), _("Reinterpret EDID data"), wxITEM_NORMAL);
    Menu3->Append(mnu_reparse);
    mnu_reparse->Enable(false);
    mnu_assemble = new wxMenuItem(Menu3, id_mnu_asmchg, _("Assemble EDID"), _("Apply changes, update checksums"), wxITEM_NORMAL);
    Menu3->Append(mnu_assemble);
    mnu_assemble->Enable(false);
    mnu_ignore_err = new wxMenuItem(Menu3, id_mnu_ignerr, _("Ignore EDID Errors"), wxEmptyString, wxITEM_CHECK);
    Menu3->Append(mnu_ignore_err);
    mnu_allwritable = new wxMenuItem(Menu3, id_mnu_allwr, _("Ignore Read-Only flags"), wxEmptyString, wxITEM_CHECK);
    Menu3->Append(mnu_allwritable);
    mnu_dtd_aspect = new wxMenuItem(Menu3, id_mnu_dtd_asp, _("DTD preview: keep aspect ratio"), wxEmptyString, wxITEM_CHECK);
    Menu3->Append(mnu_dtd_aspect);
    mnu_dtd_aspect->Check(true);
    mnu_fdetails = new wxMenuItem(Menu3, id_mnu_fdetails, _("Block Data: show field details"), wxEmptyString, wxITEM_CHECK);
    Menu3->Append(mnu_fdetails);
    mnu_logw = new wxMenuItem(Menu3, id_mnu_logw, _("Log Window\tctrl-L"), wxEmptyString, wxITEM_NORMAL);
    Menu3->Append(mnu_logw);
    MenuBar1->Append(Menu3, _("Options"));
    Menu2 = new wxMenu();
    mnu_about = new wxMenuItem(Menu2, wxID_ABOUT, _("About\tF1"), _("Show info about this application"), wxITEM_NORMAL);
    Menu2->Append(mnu_about);
    MenuItem1 = new wxMenuItem(Menu2, id_mnu_flags, _("Flags & Types\tF3"), wxEmptyString, wxITEM_NORMAL);
    Menu2->Append(MenuItem1);
    MenuBar1->Append(Menu2, _("Help"));
    SetMenuBar(MenuBar1);
    win_stat_bar = new wxStatusBar(this, id_win_stat_bar, wxST_SIZEGRIP, _T("id_win_stat_bar"));
    int __wxStatusBarWidths_1[2] = { -300, 300 };
    int __wxStatusBarStyles_1[2] = { wxSB_NORMAL, wxSB_NORMAL };
    win_stat_bar->SetFieldsCount(2,__wxStatusBarWidths_1);
    win_stat_bar->SetStatusStyles(2,__wxStatusBarStyles_1);
    SetStatusBar(win_stat_bar);
    Center();
    //*)

    Connect(wxID_ANY, wxEVT_GRID_CELL_CHANGED,
            (wxObjectEventFunction) &wxEDID_Frame::evt_datagrid_write);

    InitBlkTreeMenu();

    row_sel    = -1;
    subg_idx   = -1;
    edigrp_sel = NULL;
    edigrp_src = NULL;
    b_srcgrp_orphaned = false;

    GLog.Create(this);

    EDID.SetGuiLogPtr(&GLog);

    dtd_panel ->Enable(false);
    dtd_screen->Enable(false);
    dtd_screen->SetParentFrame(this); //needed to have a pointer to a DTD values

    mnu_dtd_aspect->Check(b_dtd_keep_aspect );
    mnu_fdetails  ->Check(b_dta_grid_details);

    DataGrid_ChangeView();

    flags.u32      = 0;
    edid_file_name = "EDID";

    DeletePendingEvents();

    {  //calc win size based on DTD panel.
       wxSize minsz, actsz, winsz, delta;

       winsz    = GetSize();
       actsz    = dtd_panel->GetSize();
       minsz    = bs_dtd_main->ComputeFittingWindowSize(this);

       delta.x  = actsz.x - minsz.x;
       delta.x  = (delta.x > 0) ? 0 : -delta.x;
       delta.x += 8; //magic: compensation for dtd_screen border
       delta.y  = actsz.y - minsz.y;
       delta.y  = (delta.y > 0) ? 0 : -delta.y;

       winsz   += delta;

       SetMinSize(winsz);
    }

    //config::flags
    if (config.b_cmd_ignore_err) {
       mnu_ignore_err->Check(config.b_cmd_ignore_err);
       EDID.Set_RD_Ignore(true);
    }

    if (config.b_cmd_ignore_err) {
       mnu_allwritable->Check(config.b_cmd_ignore_rd);
       EDID.Set_ERR_Ignore(true);
    }

    if (config.b_have_layout) {
       wxCommandEvent evt(wxEVT_DEFERRED, id_app_layout);
       AddPendingEvent(evt);
    } else {
       //EDID panel, default layout
       tmps = wxString::FromAscii(AUI_DefLayout);
       AuiMgrEDID->LoadPerspective(tmps);
    }
    //load/import file from cmd line arg?
    if (config.b_cmd_bin_file) {
       wxCommandEvent evt(wxEVT_DEFERRED, wxID_OPEN);
       AddPendingEvent(evt);
       return;
    }
    if (config.b_cmd_txt_file) {
       wxCommandEvent evt(wxEVT_DEFERRED, id_mnu_imphex);
       AddPendingEvent(evt);
    }
}
#if wxCHECK_GCC_VERSION(8, 0)
   #pragma GCC diagnostic warning "-Wcast-function-type"
#endif
#pragma GCC diagnostic warning "-Wunused-parameter"

wxEDID_Frame::~wxEDID_Frame() {

   config.aui_layout = AuiMgrEDID->SavePerspective();
   config.win_pos    = GetPosition();
   config.win_size   = GetSize();

   if (b_srcgrp_orphaned) delete edigrp_src;

   delete mnu_BlkTree;
   delete miRemoved;
   delete accDelete;
   delete accMoveUp;
   delete accMoveDn;

   AuiMgrEDID->UnInit();
   AuiMgrMain->UnInit();

   delete AuiMgrEDID;
   delete AuiMgrMain;
}

void wxEDID_Frame::evt_frame_size(wxSizeEvent& evt) {
   wxSize NewSize;

   NewSize.y  = fgs_dtd->GetSize().y;
   NewSize.y -= fgs_dtd_top->GetSize().y;
   NewSize.y -= fgs_dtd_bottom->GetSize().y;

   NewSize.x  = fgs_dtd->GetSize().x;
   NewSize.x -= fgs_dtd_right->GetSize().x;

   NewSize.DecBy(10, 16); //borders

   dtd_screen->SetMinSize(NewSize);
   bs_dtd_main->Layout();

   evt.Skip(true);
}

void wxEDID_Frame::evt_datagrid_select(wxGridEvent& evt) {
   rcode retU;
   row_sel = evt.GetRow();
   BlkDataGrid->SelectRow(row_sel);
   retU = SetFieldDesc(row_sel);
   evt.Skip(true);
}

#pragma GCC diagnostic ignored "-Wunused-parameter"
void wxEDID_Frame::evt_datagrid_vsel(wxGridEvent& evt) {
   //EVT_GRID_CMD_EDITOR_SHOWN: show value selector menu for fields with EF_VS
   rcode  retU;

   if (edigrp_sel == NULL) {
      RCD_SET_FAULT(retU);
      GLog.PrintRcode(retU);
      GLog.Show();
      return;
   }

   edi_dynfld_t *p_field = edigrp_sel->FieldsAr.Item(row_sel);

   if (p_field == NULL) {
      RCD_SET_FAULT(retU);
      GLog.PrintRcode(retU);
      GLog.Show();
      return;
   }

   if ((p_field->field.flags & EF_VS) != 0) {
      if(p_field->selector != NULL) BlkDataGrid->PopupMenu(p_field->selector);
   }
}

void wxEDID_Frame::evt_datagrid_edit_hide(wxGridEvent& evt) {
   //keep row selected after CellEditControl is removed
   BlkDataGrid->SelectRow(row_sel);
}


#if wxCHECK_GCC_VERSION(8, 0)
   #pragma GCC diagnostic ignored "-Wcast-function-type"
#endif
void wxEDID_Frame::evt_datagrid_write(wxGridEvent& evt) {
   static const wxString fmsgU = "FAULT: UpdateDataGrid()";
   static const wxString fmsgW = "FAULT: WriteField()";
   rcode retU;

   row_op = evt.GetRow();
   //Call field handler(write), then re-read the value to verify it
   retU = WriteField();
   //refresh the cell value: (fn name is misleading, but it works)
   BlkDataGrid->ShowCellEditControl();
   BlkDataGrid->HideCellEditControl();

   if (!RCD_IS_OK(retU)) {
      GLog.DoLog(fmsgW);
      GLog.PrintRcode(retU);
      GLog.Show();
   }

   //check Group Refresh flag:
   if (flags.bits.edi_grp_rfsh != 0) {

      flags.bits.edi_grp_rfsh = 0;
      GLog.DoLog("Group Refresh forced.");
      // prevent evt nesting
      Disconnect(wxID_ANY, wxEVT_GRID_CELL_CHANGED,
                 (wxObjectEventFunction) &wxEDID_Frame::evt_datagrid_write);

      retU = UpdateDataGrid(edigrp_sel);
      if (!RCD_IS_OK(retU)) {
         GLog.DoLog(fmsgU);
         GLog.PrintRcode(retU);
         GLog.Show();
      }

      //BlkDataGrid->ForceRefresh();
      Connect(wxID_ANY, wxEVT_GRID_CELL_CHANGED,
              (wxObjectEventFunction) &wxEDID_Frame::evt_datagrid_write);
   }
}

void wxEDID_Frame::evt_blktree_sel(wxTreeEvent& evt) {
   rcode       retU;
   edi_grp_cl *pgrp;

   BT_Item_sel = evt.GetItem(); //REMOVE BT_Item_sel, use local var
   BT_Iparent  = BlockTree->GetItemParent(BT_Item_sel);
   pgrp        = reinterpret_cast <edi_grp_cl*> (BlockTree->GetItemData(BT_Item_sel));
   opFlags.u32 = 0;

   //on block sel. change always hide cell editor and clear grid selection:
   BlkDataGrid->HideCellEditControl();
   BlkDataGrid->ClearSelection();

   if ( (!BT_Item_sel.IsOk()) || (pgrp == NULL)) {
      txc_edid_info->Clear();
      BlkDataGrid->Enable(false);
      tmps.Empty();
      win_stat_bar->SetStatusText(tmps, SBAR_GRPOFFS);
      edigrp_sel = NULL;
      return;
   }
   BlkDataGrid->Enable(flags.bits.ctrl_enabled);
   edigrp_sel = pgrp;
   SetOpFlags();
   //display group info
   txc_edid_info->SetValue(pgrp->GroupDesc);
   {
      u32_t abs_offs;
      u32_t rel_offs;
      abs_offs = pgrp->getAbsOffs();
      rel_offs = pgrp->getRelOffs();
      tmps.Printf("Offset: Abs:%d (0x%04X), Rel: %d (0x%02X)",
                  abs_offs, abs_offs, rel_offs, rel_offs);
      win_stat_bar->SetStatusText(tmps, SBAR_GRPOFFS);
   }

   retU = UpdateDataGrid(pgrp);
   if (!RCD_IS_OK(retU)) {
      GLog.PrintRcode(retU);
      GLog.Show();
   }

   {
      bool enb = (pgrp->getTypeID() == ID_DTD);
      if (enb) {
         Connect(wxID_ANY, wxEVT_COMMAND_SPINCTRL_UPDATED,
                 (wxObjectEventFunction) &wxEDID_Frame::evt_dtdctor_sct);
      } else {
         Disconnect(wxID_ANY, wxEVT_COMMAND_SPINCTRL_UPDATED,
                    (wxObjectEventFunction) &wxEDID_Frame::evt_dtdctor_sct);
      }
      dtd_panel ->Enable(enb);
      dtd_screen->Enable(enb);
   }
}

void wxEDID_Frame::BlkTreeUpdateGrp() {
   rcode         retU;
   wxTreeItemId  trItemID;
   edi_grp_cl   *grp_copy;
   GroupAr_cl   *grp_ar;
   u32_t         grp_idx;
   bool          b_expand;

   RCD_SET_FAULT(retU);

   //BlockTree is refreshed even when sub-groups are not present

   //clone the group on length change -> re-init().
   grp_copy = edigrp_sel->Clone(retU, T_MODE_EDIT);
   if ((NULL == grp_copy) || !RCD_IS_OK(retU)) {
      GLog.PrintRcode(retU);
      if (! EDID.Get_ERR_Ignore()) {
         GLog.Show();
      }
      if (NULL == grp_copy) return;
   }

   grp_idx  = edigrp_sel->getParentArIdx();
   grp_ar   = edigrp_sel->getParentAr();
   trItemID = edigrp_sel->GetId();
   b_expand = BlockTree->IsExpanded(trItemID);

   if (edigrp_src == edigrp_sel) edigrp_src = grp_copy;

   BlkTreeDelGrp (edigrp_sel);

   grp_ar->Delete  (grp_idx);
   grp_ar->InsertUp(grp_idx, grp_copy);
   //delete edigrp_sel; edigrp_sel = NULL;

   trItemID = BlkTreeInsGrp (BT_Iparent, grp_copy, grp_idx);

   //prevent evt nesting
   //WriteField->BlkTreeUpdateGrp()->BlockTree->SelectItem
   Disconnect(wxID_ANY, wxEVT_GRID_CELL_CHANGED,
              (wxObjectEventFunction) &wxEDID_Frame::evt_datagrid_write);

   if (subg_idx >= 0) { //restore sub-group selection
      if (subg_idx > (int) grp_copy->getSubGrpCount()) {
         subg_idx  = grp_copy->getSubGrpCount() -1;
      }
      grp_ar     = grp_copy->getSubArray();
      edigrp_sel = grp_copy->getSubGroup(subg_idx);
      trItemID   = edigrp_sel->GetId();
      BlockTree  ->SelectItem (trItemID);
      subg_idx   = -1;
   } else {
      BlockTree->SelectItem (trItemID);
      if (b_expand) BlockTree->Expand(trItemID);
   }

   Connect(wxID_ANY, wxEVT_GRID_CELL_CHANGED,
           (wxObjectEventFunction) &wxEDID_Frame::evt_datagrid_write);
}

rcode wxEDID_Frame::BlkTreeChangeGrpType() {
   rcode         retU;
   wxTreeItemId  trItemID;
   edi_grp_cl   *grp_new;
   u8_t         *grp_inst;
   GroupAr_cl   *grp_ar;
   u32_t         grp_idx;
   u32_t         gp_abs_offs;
   u32_t         gp_rel_offs;
   bool          b_expand;

   grp_inst    = edigrp_sel->getInsPtr();
   grp_idx     = edigrp_sel->getParentArIdx();
   gp_abs_offs = edigrp_sel->getAbsOffs();
   gp_rel_offs = edigrp_sel->getRelOffs();
   grp_ar      = edigrp_sel->getParentAr();
   trItemID    = edigrp_sel->GetId();
   b_expand    = BlockTree->IsExpanded(trItemID);

   //spawn new group after tag code change
   retU = EDID.ParseDBC_TAG(grp_inst, &grp_new);
   if (! RCD_IS_OK(retU)) {
      GLog.PrintRcode(retU);
      if (! EDID.Get_ERR_Ignore() ) GLog.Show();
   }
   if (NULL == grp_new) {
      RCD_SET_FAULT(retU);
      goto error;
   }

   grp_new->setAbsOffs(gp_abs_offs);
   grp_new->setRelOffs(gp_rel_offs);

   retU = grp_new->init(grp_inst, T_MODE_EDIT, NULL);
   if (!RCD_IS_OK(retU)) {
      //the fault message is logged, but ignored
      if ( (retU.detail.rcode > RCD_FVMSG) || !EDID.Get_ERR_Ignore() ) {
         delete grp_new;
         goto error;
      } else {
         GLog.PrintRcode(retU);
         if (! EDID.Get_ERR_Ignore() ) GLog.Show();
      }
   }

   if (edigrp_src == edigrp_sel) edigrp_src = grp_new;

   BlkTreeDelGrp (edigrp_sel);

   grp_ar->Delete  (grp_idx);
   grp_ar->InsertUp(grp_idx, grp_new);
   //delete edigrp_sel; edigrp_sel = NULL;

   trItemID = BlkTreeInsGrp (BT_Iparent, grp_new, grp_idx);

   //prevent evt nesting
   Disconnect(wxID_ANY, wxEVT_GRID_CELL_CHANGED,
              (wxObjectEventFunction) &wxEDID_Frame::evt_datagrid_write);

   BlockTree->SelectItem (trItemID);
   if (b_expand) BlockTree->Expand(trItemID);

   Connect(wxID_ANY, wxEVT_GRID_CELL_CHANGED,
           (wxObjectEventFunction) &wxEDID_Frame::evt_datagrid_write);

   RCD_RETURN_OK(retU);

error:
   GLog.PrintRcode(retU);
   GLog.Show();
   return retU;
}
#if wxCHECK_GCC_VERSION(8, 0)
   #pragma GCC diagnostic warning "-Wcast-function-type"
#endif

void wxEDID_Frame::SetOpFlags() {
   GroupAr_cl *grp_ar;
   u32_t       grp_idx;
   u32_t       type_id;
   bool        b_Reparse;

   type_id = edigrp_sel->getTypeID();
   grp_ar  = edigrp_sel->getParentAr();
   grp_idx = edigrp_sel->getParentArIdx();

   if (edigrp_src != NULL) {
      opFlags.bits.CanInsertUp = grp_ar->CanInsertUp(grp_idx, edigrp_src);
      opFlags.bits.CanInsInto  = grp_ar->CanInsInto (grp_idx, edigrp_src);
      opFlags.bits.CanInsertDn = grp_ar->CanInsertDn(grp_idx, edigrp_src);
      opFlags.bits.CanPaste    = grp_ar->CanPaste   (grp_idx, edigrp_src);
   }

   b_Reparse  = (0 != (type_id & (ID_CEA_MASK  | ID_CEA_EXT_MASK) ));
   b_Reparse &= (0 == (type_id & (T_EDID_FIXED | T_DBC_SUBGRP   ) ));

   opFlags.bits.CanReparse = b_Reparse;
   opFlags.bits.CanCopy    = (0 == (type_id &  T_EDID_FIXED) );
   opFlags.bits.CanDelete  = grp_ar->CanDelete(grp_idx);
   opFlags.bits.CanCut     = grp_ar->CanCut   (grp_idx);
   opFlags.bits.CanMoveUp  = grp_ar->CanMoveUp(grp_idx);
   opFlags.bits.CanMoveDn  = grp_ar->CanMoveDn(grp_idx);
}

void wxEDID_Frame::evt_blktree_key(wxTreeEvent& evt) {
   int            keycode;
   wxCommandEvent op_evt(wxEVT_MENU);
   wxKeyEvent     kevt = evt.GetKeyEvent();

   if (BlockTree->b_key_block) goto skip_evt;
   if (edigrp_sel == NULL    ) goto skip_evt;

   keycode = evt.GetKeyCode();

   if ((keycode == WXK_DELETE) || (keycode == WXK_BACK)) {
      if (! opFlags.bits.CanDelete) return;
      evt_blktree_delete(evt); //wxID_DELETE ignored
      return;
   }

   if (! kevt.ControlDown()) goto skip_evt;

   switch (keycode) {
      case WXK_UP:
      case WXK_NUMPAD_UP:
         if (! opFlags.bits.CanMoveUp) return;
         op_evt.SetId(wxID_UP);
         evt_blktree_move(op_evt);
         return;
      case WXK_DOWN:
      case WXK_NUMPAD_DOWN:
         if (! opFlags.bits.CanMoveDn) return;
         op_evt.SetId(wxID_DOWN);
         evt_blktree_move(op_evt);
         return;
      default:
         break;
   }

   if (keycode == 'C') {
      if (! opFlags.bits.CanCopy) return;
      evt_blktree_copy(op_evt); //wxID_COPY ignored
      return;
   }
   if (keycode == 'V') {
      if (! opFlags.bits.CanPaste) return;
      evt_blktree_paste(op_evt); //wxID_PASTE ignored
      return;
   }
   if (keycode == 'X') {
      if (! opFlags.bits.CanCut) return;
      evt_blktree_cut(op_evt); //wxID_CUT ignored
      return;
   }

skip_evt:
   evt.Skip(true);
   return;
}

void wxEDID_Frame::evt_blktree_rmb(wxTreeEvent& evt) {
   GroupAr_cl *grp_ar;
   u32_t       blk_free;
   u32_t       grp_free;

   if (edigrp_sel == NULL) return;

   grp_ar = edigrp_sel->getParentAr();

   if (grp_ar->getParentArray() != NULL) {
      blk_free = grp_ar->getParentArray()->getFreeSpace();
      grp_free = grp_ar->getFreeSpace();
   } else {
      blk_free = grp_ar->getFreeSpace();
      u32_t  tid;
      //if DBC group, read length from header
      //for T_EDID_FIXED zero free space is reported. f.e. ID_CHD
      tid  = edigrp_sel->getTypeID();
      tid &= ~(ID_CEA_MASK|ID_CEA_EXT_MASK);
      if (tid == 0) {
         u8_t  *inst;
         u32_t  dlen;
         inst     = edigrp_sel->getInsPtr();
         dlen     = reinterpret_cast <bhdr_t*> (inst)->tag.blk_len;
         grp_free = (31 - dlen);
      } else {
         grp_free = 0;
      }
   }

   if (edigrp_src != NULL) {
      //miInfoOK if types are matching && there's enough free space left
      if (opFlags.bits.CanPaste | opFlags.bits.CanInsInto) {
         if (miInfoOK != mnu_BlkTree->FindItemByPosition(0)) {
            miRemoved  = mnu_BlkTree->Remove(id_mnu_info);
            miInfoOK   ->SetSubMenu(mnu_SubInfo);
            miInfoNOK  ->SetSubMenu(NULL);
            mnu_BlkTree->Insert(0, miInfoOK);
         }
      } else {
         if (miInfoNOK != mnu_BlkTree->FindItemByPosition(0)) {
            miRemoved  = mnu_BlkTree->Remove(id_mnu_info);
            miInfoOK   ->SetSubMenu(NULL);
            miInfoNOK  ->SetSubMenu(mnu_SubInfo);
            mnu_BlkTree->Insert(0, miInfoNOK);
         }
      }

      tmps.Printf("Src: %s\nSel: %s", edigrp_src->CodeName, edigrp_sel->CodeName );
      mnu_BlkTree->SetLabel(id_mnu_info, tmps);

      if (b_srcgrp_orphaned) {
         GLog.slog = "<no parent>";
      } else {
         GLog.slog.Printf("offs=%d (%d)", edigrp_src->getAbsOffs(), edigrp_src->getRelOffs());
      }

      tmps.Printf("Src: %s, %s, size=%d\n"
                  "Sel: %s, offs=%d (%d)\n"
                  "Free space: (bytes)\n"
                  "group: %d\n"
                  "block: %d",
                  edigrp_src->CodeName, GLog.slog               , edigrp_src->getTotalSize(),
                  edigrp_sel->CodeName, edigrp_sel->getAbsOffs(), edigrp_sel->getRelOffs  (),
                  grp_free, blk_free );

   } else {
      tmps.Printf("Src: <none>\nSel: %s", edigrp_sel->CodeName );
      mnu_BlkTree->SetLabel(id_mnu_info, tmps);

      tmps.Printf("Sel: %s, offs=%d (%d)\n"
                  "Free space: (bytes)\n"
                  "group: %d\n"
                  "block: %d",
                  edigrp_sel->CodeName, edigrp_sel->getAbsOffs(),
                  edigrp_sel->getRelOffs(), grp_free, blk_free );
   }

   miSubInfo->SetItemLabel(tmps);

   mnu_BlkTree->Enable(wxID_EXECUTE , opFlags.bits.CanReparse );
   mnu_BlkTree->Enable(wxID_PASTE   , opFlags.bits.CanPaste   );
   mnu_BlkTree->Enable(wxID_CUT     , opFlags.bits.CanCut     );
   mnu_BlkTree->Enable(wxID_COPY    , opFlags.bits.CanCopy    );
   mnu_BlkTree->Enable(wxID_DELETE  , opFlags.bits.CanDelete  );
   mnu_BlkTree->Enable(id_mnu_ins_up, opFlags.bits.CanInsertUp);
   mnu_BlkTree->Enable(id_mnu_ins_in, opFlags.bits.CanInsInto );
   mnu_BlkTree->Enable(id_mnu_ins_dn, opFlags.bits.CanInsertDn);
   mnu_BlkTree->Enable(wxID_UP      , opFlags.bits.CanMoveUp  );
   mnu_BlkTree->Enable(wxID_DOWN    , opFlags.bits.CanMoveDn  );

   PopupMenu(mnu_BlkTree);
}

void wxEDID_Frame::evt_blktree_copy(wxCommandEvent& evt) {

   LogGroupOP(edigrp_sel, "Copy");
   edigrp_src = edigrp_sel;
   //immediately check conditions for selected group
   //removes warning flag from id_mnu_info
   //otherwise the flags are updated only on selection change
   opFlags.u32 = 0;
   SetOpFlags();
}

void wxEDID_Frame::evt_blktree_paste(wxCommandEvent& evt) {
   rcode         retU;
   wxTreeItemId  trItemID;
   edi_grp_cl   *grp_copy;
   GroupAr_cl   *grp_ar;
   u32_t         grp_idx;

   LogGroupOP(edigrp_sel, "Paste");

   if (! b_srcgrp_orphaned) {
      grp_copy = edigrp_src->Clone(retU, 0);
      if ((NULL == grp_copy) || !RCD_IS_OK(retU)) {
         GLog.PrintRcode(retU);
         GLog.Show();
         if (NULL == grp_copy) return;
      }
   } else { //orphaned group is pasted directly (no parent)
      grp_copy          = edigrp_src;
      b_srcgrp_orphaned = false;
   }

   grp_ar  = edigrp_sel->getParentAr();
   grp_idx = edigrp_sel->getParentArIdx();

   BlkTreeDelGrp(edigrp_sel);
   grp_ar->Paste(grp_idx, grp_copy);

   edigrp_sel = grp_copy;

   retU = UpdateDataGrid(grp_copy);
   if (!RCD_IS_OK(retU)) {
      GLog.DoLog("Paste() FAILED.");
      GLog.PrintRcode(retU);
      GLog.Show();
   }

   //OPTION: keep expanded state
   trItemID = BlkTreeInsGrp(BT_Iparent, grp_copy, grp_idx);
   BlockTree->SelectItem(trItemID);
}

void wxEDID_Frame::evt_blktree_delete(wxCommandEvent& evt) {
   rcode         retU;
   wxTreeItemId  trItemID;
   GroupAr_cl   *grp_ar;
   u32_t         grp_idx;

   LogGroupOP(edigrp_sel, "Delete");

   grp_ar   = edigrp_sel->getParentAr();
   grp_idx  = edigrp_sel->getParentArIdx();
   trItemID = edigrp_sel->GetId();

   //deleting source group: create a clone
   if (edigrp_sel == edigrp_src) {
      edigrp_src = edigrp_sel->Clone(retU, 0);
      if ((NULL == edigrp_src) || !RCD_IS_OK(retU)) {
         GLog.PrintRcode(retU);
         GLog.Show();
         if (NULL == edigrp_src) return;
      }
      b_srcgrp_orphaned = true;
   }

   BlkTreeDelGrp (edigrp_sel);
   grp_ar->Delete(grp_idx   );

   { //select next group
      u32_t grp_cnt = grp_ar->GetCount();

      if (grp_cnt > 0) {
         grp_idx    = (grp_idx < grp_cnt) ? grp_idx : (grp_cnt-1);
         edigrp_sel = grp_ar->Item(grp_idx);
         trItemID   = edigrp_sel->GetId();
         goto update;
      }
   }
   //empty group: select parent
   trItemID = BT_Iparent;

update:
   BlockTree->SelectItem(trItemID); //triggers evt_blktree_sel()
}

void wxEDID_Frame::evt_blktree_cut(wxCommandEvent& evt) {
   wxTreeItemId  trItemID;
   GroupAr_cl   *grp_ar;
   u32_t         grp_idx;

   LogGroupOP(edigrp_sel, "Cut");

   grp_ar   = edigrp_sel->getParentAr();
   grp_idx  = edigrp_sel->getParentArIdx();

   edigrp_src        = grp_ar->Cut(grp_idx);
   b_srcgrp_orphaned = true;

   BlkTreeDelGrp(edigrp_sel);

   { //select next group
      u32_t grp_cnt;

      grp_cnt = grp_ar->GetCount();

      if (grp_cnt > 0) {
         grp_idx    = (grp_idx < grp_cnt) ? grp_idx : (grp_cnt-1);
         edigrp_sel = grp_ar->Item(grp_idx);
         trItemID   = edigrp_sel->GetId();
         goto update;
      }
   }
   //empty group: select parent
   trItemID = BT_Iparent;

update:
   BlockTree->SelectItem(trItemID); //triggers evt_blktree_sel()
}

void wxEDID_Frame::evt_blktree_insert(wxCommandEvent& evt) {
   rcode         retU;
   wxTreeItemId  trItemID;
   edi_grp_cl   *grp_copy;
   GroupAr_cl   *grp_ar;
   u32_t         grp_idx;
   int           evtid;

   if (! b_srcgrp_orphaned) {
      grp_copy = edigrp_src->Clone(retU, 0);
      if ((NULL == grp_copy) || !RCD_IS_OK(retU)) {
         GLog.PrintRcode(retU);
         GLog.Show();
         if (NULL == grp_copy) return;
      }
   } else { //orphaned group is inserted directly (no parent)
      grp_copy          = edigrp_src;
      b_srcgrp_orphaned = false;
   }

   evtid   = evt.GetId();
   grp_ar  = edigrp_sel->getParentAr();
   grp_idx = edigrp_sel->getParentArIdx();

   if (id_mnu_ins_up == evtid) {
      LogGroupOP(edigrp_sel, "Insert Above");

      grp_ar   ->InsertUp(grp_idx, grp_copy);
      trItemID = BlkTreeInsGrp(BT_Iparent, grp_copy, grp_idx);
      goto select_item;
   }
   if (id_mnu_ins_in == evtid) {
      LogGroupOP(edigrp_sel, "Insert Into");

      grp_ar     = edigrp_sel->getSubArray();
      grp_ar     ->InsertInto(edigrp_sel, grp_copy); //always insert @idx=0
      BT_Iparent = edigrp_sel->GetId();
      trItemID   = BlkTreeInsGrp(BT_Iparent, grp_copy, 0);
      goto select_item;
   }
   // id_mnu_ins_dn
   LogGroupOP(edigrp_sel, "Insert Below");

   grp_ar    ->InsertDn(grp_idx, grp_copy);
   grp_idx  ++ ;
   trItemID  = BlkTreeInsGrp(BT_Iparent, grp_copy, grp_idx);

select_item:
   BlockTree->SelectItem(trItemID);
}

void wxEDID_Frame::evt_blktree_move(wxCommandEvent& evt) {
   wxTreeItemId  trItemID;
   GroupAr_cl   *grp_ar;
   u32_t         grp_idx;
   int           evtid;
   bool          b_Expanded;

   evtid      = evt.GetId();
   grp_ar     = edigrp_sel->getParentAr();
   grp_idx    = edigrp_sel->getParentArIdx();
   trItemID   = edigrp_sel->GetId();
   b_Expanded = BlockTree->IsExpanded(trItemID);

   //don't delete edi_grp_cl::wxTreeItemData
   BlkTreeDelGrp(edigrp_sel);

   if (evtid == wxID_UP) {

      LogGroupOP(edigrp_sel, "Move Up");

      grp_ar->MoveUp(grp_idx);
      grp_idx -- ;
      trItemID = BlkTreeInsGrp(BT_Iparent, edigrp_sel, grp_idx);

      goto update;
   }
   // wxID_DOWN: move down
   LogGroupOP(edigrp_sel, "Move Down");

   grp_ar->MoveDn(grp_idx);

   grp_idx  ++ ;
   trItemID  = BlkTreeInsGrp(BT_Iparent, edigrp_sel, grp_idx);

update:
   BlockTree->SelectItem(trItemID);
   if (b_Expanded) BlockTree->Expand(trItemID);
}

void wxEDID_Frame::evt_blktree_reparse(wxCommandEvent& evt) {
   rcode retU;

   retU = edigrp_sel->AssembleGroup();
   if (!RCD_IS_OK(retU)) {
      GLog.PrintRcode(retU);
      if (! EDID.Get_ERR_Ignore()) GLog.Show();
   }

   BlkTreeUpdateGrp();
}


void wxEDID_Frame::evt_open_edid_bin(wxCommandEvent& evt) {
   rcode retU;
   retU = OpenEDID();
   if (!RCD_IS_OK(retU)) {
      GLog.PrintRcode(retU);

      if (! EDID.Get_ERR_Ignore()) {
         tmps = msgF_LOAD;
         wxMessageBox(tmps, txt_REMARK, wxOK|wxCENTRE|wxICON_INFORMATION);
      } else {
         GLog.Show();
      }
   }
   mnu_reparse ->Enable(flags.bits.data_loadeed);
   mnu_assemble->Enable(flags.bits.data_loadeed);
}

void wxEDID_Frame::evt_save_edid_bin(wxCommandEvent& evt) {
   rcode retU;

   retU = SaveEDID();
   if (!RCD_IS_OK(retU)) {
      GLog.DoLog("SaveEDID() FAILED.");
      GLog.PrintRcode(retU);
      GLog.Show();
   }

   evt.Skip(true);
}

void wxEDID_Frame::evt_save_report(wxCommandEvent& evt) {
   rcode retU;

   retU = SaveReport();
   if (!RCD_IS_OK(retU)) {
      GLog.DoLog("SaveReport() FAILED.");
      GLog.PrintRcode(retU);
      GLog.Show();
   }
}

void wxEDID_Frame::evt_export_hex(wxCommandEvent& evt) {
   rcode retU;

   retU = ExportEDID_hex();
   if (!RCD_IS_OK(retU)) {
      GLog.DoLog("ExportEDID_hex() FAILED.");
      GLog.PrintRcode(retU);
      GLog.Show();
   }
}

void wxEDID_Frame::evt_import_hex(wxCommandEvent& evt) {
   rcode retU;

   retU = ImportEDID_hex();
   if (! RCD_IS_OK(retU)) {
      GLog.PrintRcode(retU);

      if (! EDID.Get_ERR_Ignore()) {
         tmps = msgF_LOAD;
         wxMessageBox(tmps, txt_REMARK, wxOK|wxCENTRE|wxICON_INFORMATION);
      } else {
         GLog.Show();
      }
   }
   mnu_reparse->Enable(flags.bits.data_loadeed);
   mnu_assemble ->Enable(flags.bits.data_loadeed);
}

void wxEDID_Frame::evt_ignore_rd(wxCommandEvent& evt) {

   EDID.Set_RD_Ignore(mnu_allwritable->IsChecked());

   if (mnu_allwritable->IsChecked()) {
      tmps = "Write protection for ALL fields is disabled.";
      wxMessageBox(tmps, txt_WARNING, wxOK|wxCENTRE|wxICON_EXCLAMATION);
      UpdateDataGrid(edigrp_sel);
   } else {
      tmps = "Write protection is enabled for fields with RD flag set.";
      wxMessageBox(tmps, txt_REMARK, wxOK|wxCENTRE|wxICON_INFORMATION);
      UpdateDataGrid(edigrp_sel);
   }
}

void wxEDID_Frame::evt_ignore_err(wxCommandEvent& evt) {

   EDID.Set_ERR_Ignore(mnu_ignore_err->IsChecked());

   if (mnu_ignore_err->IsChecked()) {
      tmps = "EDID errors will be IGNORED.\n\n"
             "This allows to bypass minor bugs in loaded files, "
             "but it won't help if the file contains garbage.";
      wxMessageBox(tmps, txt_WARNING, wxOK|wxCENTRE|wxICON_EXCLAMATION);
      GLog.DoLog("Ignoring of EDID errors ENABLED.");
   } else {
      tmps = "Critical EDID error will disable the editor.";
      wxMessageBox(tmps, txt_REMARK, wxOK|wxCENTRE|wxICON_INFORMATION);
      GLog.DoLog("Ignoring of EDID errors DISABLED.");
   }
}

void wxEDID_Frame::evt_dtdctor_sct(wxSpinEvent& evt) {
   float  pixclk;
   int    limit, val;
   bool   brecalc     = true;
   bool   updt_htotal = false;
   bool   updt_vtotal = false;
   int    evtid       = evt.GetId();

   //invalidate grid data -> refresh
   if (evtid != wxID_ANY) flags.bits.edigridblk_ok = 0;

   if (evtid == id_sct_pixclk) {
      sct_pixclk->data = sct_pixclk->GetValue();
      if (!RCD_IS_OK(DTD_Ctor_WriteInt(*sct_pixclk))) return;
      evtid = wxID_ANY;
      //DTD_Ctor_Recalc();
      //return;
   }
   pixclk  = (sct_pixclk->data * 10000);

   if ((evtid == wxID_ANY) || (evtid == id_sct_xres)) {
      sct_xres->data = sct_xres->GetValue();
      if (evtid != wxID_ANY) {
         if (!RCD_IS_OK(DTD_Ctor_WriteInt(*sct_xres))) return;
      }
      tmps.Printf("%.03f", ((sct_xres->data * 1000000.0) / pixclk));
      txc_txres->SetValue(tmps);
      updt_htotal = updt_vtotal = true;
      evtid = wxID_ANY;
   }
   if ((evtid == wxID_ANY) || (evtid == id_sct_hblank)) {
      limit = (sct_hsoffs->data + sct_hswidth->data);
      val   = sct_hblank->GetValue();
      if (val < limit) {
         val = limit;
         sct_hblank->SetValue(val);
      }
      sct_hblank->data = val;
      if (evtid != wxID_ANY) {
         if (!RCD_IS_OK(DTD_Ctor_WriteInt(*sct_hblank))) return;
      }
      tmps.Printf("%.04f", ((val * 1000000.0) / pixclk));
      txc_thblank->SetValue(tmps);
      updt_htotal = updt_vtotal = true;
   }
   if ((evtid == wxID_ANY) || (evtid == id_sct_hsoffs)) {
      limit = (sct_hblank->data - sct_hswidth->data);
      val   = sct_hsoffs->GetValue();
      if (val > limit) {
         val = limit;
         sct_hsoffs->SetValue(val);
      }
      sct_hsoffs->data = val;
      if (evtid != wxID_ANY) {
         if (!RCD_IS_OK(DTD_Ctor_WriteInt(*sct_hsoffs))) return;
      }
      tmps.Printf("%.04f", ((val * 1000000.0) / pixclk));
      txc_thsoffs->SetValue(tmps);
      brecalc = false;
   }
   if ((evtid == wxID_ANY) || (evtid == id_sct_hswidth)) {
      limit = (sct_hblank->data - sct_hsoffs->data);
      val   = sct_hswidth->GetValue();
      if (val > limit) {
         val = limit;
         sct_hswidth->SetValue(val);
      }
      sct_hswidth->data = val;
      if (evtid != wxID_ANY) {
         if (!RCD_IS_OK(DTD_Ctor_WriteInt(*sct_hswidth))) return;
      }
      tmps.Printf("%.04f", ((val * 1000000.0) / pixclk));
      txc_thswidth->SetValue(tmps);
      brecalc = false;
   }
   if (evtid == id_sct_hborder) {
      sct_hborder->data = sct_hborder->GetValue();
      if (!RCD_IS_OK(DTD_Ctor_WriteInt(*sct_hborder))) return;
      brecalc = false;
   }

   dtd_Htotal = (sct_hblank->data + sct_xres->data);
   if ((evtid == wxID_ANY) || (evtid == id_sct_vres)) {
      sct_vres->data = sct_vres->GetValue();
      if (evtid != wxID_ANY) {
         if (!RCD_IS_OK(DTD_Ctor_WriteInt(*sct_vres))) return;
      }
      tmps.Printf("%.03f", ((sct_vres->data * dtd_Htotal * 1000.0) / pixclk));
      txc_tvres->SetValue(tmps);
      updt_vtotal = true;
   }
   if ((evtid == wxID_ANY) || (evtid == id_sct_vblank)) {
      limit = (sct_vsoffs->data + sct_vswidth->data);
      val   = sct_vblank->GetValue();
      if (val < limit) {
         val = limit;
         sct_vblank->SetValue(val);
      }
      sct_vblank->data = val;
      if (evtid != wxID_ANY) {
         if (!RCD_IS_OK(DTD_Ctor_WriteInt(*sct_vblank))) return;
      }
      tmps.Printf("%.04f", ((val * dtd_Htotal * 1000.0) / pixclk));
      txc_tvblank->SetValue(tmps);
      updt_vtotal = true;
   }
   if ((evtid == wxID_ANY) || (evtid == id_sct_vsoffs)) {
      limit = (sct_vblank->data - sct_vswidth->data);
      val   = sct_vsoffs->GetValue();
      if (val > limit) {
         val = limit;
         sct_vsoffs->SetValue(val);
      }
      sct_vsoffs->data = val;
      if (evtid != wxID_ANY) {
         if (!RCD_IS_OK(DTD_Ctor_WriteInt(*sct_vsoffs))) return;
      }
      tmps.Printf("%.04f", ((val * dtd_Htotal * 1000.0) / pixclk));
      txc_tvsoffs->SetValue(tmps);
      brecalc = false;
   }
   if ((evtid == wxID_ANY) || (evtid == id_sct_vswidth)) {
      limit = (sct_vblank->data - sct_vsoffs->data);
      val   = sct_vswidth->GetValue();
      if (val > limit) {
         val = limit;
         sct_vswidth->SetValue(val);
      }
      sct_vswidth->data = val;
      if (evtid != wxID_ANY) {
         if (!RCD_IS_OK(DTD_Ctor_WriteInt(*sct_vswidth))) return;
      }
      tmps.Printf("%.04f", ((val * dtd_Htotal * 1000.0) / pixclk));
      txc_tvswidth->SetValue(tmps);
      brecalc = false;
   }
   if (evtid == id_sct_vborder) {
      sct_vborder->data = sct_vborder->GetValue();
      if (!RCD_IS_OK(DTD_Ctor_WriteInt(*sct_vborder))) return;
      brecalc = false;
   }

   if (updt_htotal) {
      tmps.Printf("%d", dtd_Htotal);
      txc_htotal->SetValue(tmps);
      tmps.Printf("%.03f", ((dtd_Htotal * 1000000.0) / pixclk));
      txc_thtotal->SetValue(tmps);
   }

   if (updt_vtotal) {
      dtd_Vtotal = (sct_vres->data + sct_vblank->data);
      tmps.Printf("%d", dtd_Vtotal);
      txc_vtotal->SetValue(tmps);
      tmps.Printf("%.03f", ((dtd_Htotal * 1000.0 * dtd_Vtotal) / pixclk));
      txc_tvtotal->SetValue(tmps);
   }

   if (brecalc || (evtid == wxID_ANY)) DTD_Ctor_Recalc();

   DTD_Ctor_ModeLine();

   dtd_screen->Refresh();
}

void wxEDID_Frame::evt_ntbook_page(wxAuiNotebookEvent& evt) {
   rcode retU;
   int page = evt.GetSelection();

   if (page == 0) {
      //update edi block data grid after switching from DTD Ctor panel
      if ( !flags.bits.edigridblk_ok && (edigrp_sel != NULL)) {
         retU = UpdateDataGrid(edigrp_sel);
         if (!RCD_IS_OK(retU)) {
            GLog.DoLog("UpdateDataGrid() FAILED.");
            GLog.PrintRcode(retU);
            GLog.Show();
         }
      }

      edid_panel->SetFocus(); //prevent tree item from stealing focus

      //re-enable refreshing only when the panel is visible
      AuiMgrEDID->SetEvtHandlerEnabled(true);
      AuiMgrEDID->Update();

      return;
   }

   //wxWidgets3.x, wxAUI, GTK3: BUG: the AUI managed panel gets refreshed even if it's invisible,
   //what causes horrible flickering during window resizing.
   AuiMgrEDID->SetEvtHandlerEnabled(false);

   if (!dtd_panel->IsEnabled()) return;

   retU = DTD_Ctor_read_all();

   if (!RCD_IS_OK(retU)) {
      GLog.DoLog("DTD_Ctor_read_all() FAILED.");
      GLog.PrintRcode(retU);
      GLog.Show();
   }

   //push evt to refresh all the spin controls
   wxSpinEvent evtsp(wxEVT_COMMAND_SPINCTRL_UPDATED, wxID_ANY);
   ProcessEvent(evtsp);

   evt.Skip(true);
}

void wxEDID_Frame::evt_reparse(wxCommandEvent& evt) {
   rcode retU;

   retU = Reparse();
   if (!RCD_IS_OK(retU)) {
      GLog.DoLog("Reparse() FAILED.");
      GLog.PrintRcode(retU);
      GLog.Show();
   }
}

void wxEDID_Frame::evt_assemble_edid(wxCommandEvent& evt) {
   AssembleEDID_main();
}

void wxEDID_Frame::evt_log_win(wxCommandEvent& evt) {
   GLog.ShowHide();
   this->Raise();
   this->SetFocus();
}

void wxEDID_Frame::evt_blk_fdetails(wxCommandEvent& evt) {
   b_dta_grid_details        = mnu_fdetails->IsChecked();
   config.b_dta_grid_details = b_dta_grid_details;

   DataGrid_ChangeView();
}

void wxEDID_Frame::evt_dtd_asp(wxCommandEvent& evt) {

   b_dtd_keep_aspect        = mnu_dtd_aspect->IsChecked();
   config.b_dtd_keep_aspect = b_dtd_keep_aspect;

   mnu_dtd_aspect->Check(b_dtd_keep_aspect);

   if (dtd_panel->IsShown() && dtd_panel->IsEnabled()) {
      //don't post refresh evt if the dtd panel is not active
      dtd_panel->Refresh();
   }
}

void wxEDID_Frame::evt_Quit(wxCommandEvent& evt) {

   Close();
}

void wxEDID_Frame::evt_About(wxCommandEvent& evt) {
   tmps = wxString::FromAscii(wxEDID_About);
   tmps << wxbuildinfo(long_f);
   wxMessageBox(tmps, "wxEDID::About");
}

void wxEDID_Frame::evt_Flags(wxCommandEvent& evt) {
   tmps = wxString::FromAscii(Hlp_Flg_Type);
   wxMessageBox(tmps, "Help::Flags & Types");
}
#pragma GCC diagnostic warning "-Wunused-parameter"

void wxEDID_Frame::evt_Deferred(wxCommandEvent& evt) {

   if (evt.GetId() == wxID_OPEN) {
      evt_open_edid_bin(evt);
      return;
   }

   if (evt.GetId() == id_mnu_imphex) {
      evt_import_hex(evt);
      return;
   }

   if (evt.GetId() == id_app_layout) {
      AppLayout();
   }
}


rcode wxEDID_Frame::OpenEDID() {
   rcode  retU;
   wxFile file;

   GLog.DoLog("OpenEDID()");

   if (! config.b_cmd_bin_file) { //menu event: open EDID binary

      wxFileDialog dlg_open(this, "Open EDID binary file", "", "",
                            "Binary (*.bin)|*.bin|All (*.*)|*", wxFD_DEFAULT_STYLE);
      //set last used path
      if (config.b_have_last_fpath) {
         dlg_open.SetPath(config.last_used_fpath);
      }

      if (dlg_open.ShowModal() != wxID_OK) {
         GLog.DoLog("OpenEDID() canceled by user.");
         RCD_RETURN_OK(retU);
      }
      edid_file_name = dlg_open.GetFilename();
      tmps           = dlg_open.GetPath();

   } else {
      //open EDID binary file provided as cmd line arg
      edid_file_name = config.last_used_fname;
      tmps           = config.cmd_open_file_path;
      config.b_cmd_bin_file = false; //single shot
   }

   if (! file.Open(tmps, wxFile::read) ) RCD_RETURN_FAULT(retU);

   //remember last used file path
   config.last_used_fpath   = tmps;
   config.b_have_last_fpath = true;

   GLog.slog = "File opened:\n ";
   GLog.slog << tmps;
   GLog.DoLog();

   RCD_SET_OK(retU);
   ClearAll();
   EDID.Clear();

   u32_t idx    = EDI_BASE_IDX;
   u32_t idxmax = 0;
   ediblk_t *pblk = EDID.getEDID()->blk;
   do {
      if ( EDI_BLK_SIZE != file.Read(&pblk[idx], EDI_BLK_SIZE) ) {
         RCD_SET_FAULT(retU);
         GLog.slog.Printf("Failed to load EDID block[%d]: incorrect size.", idx);
         GLog.DoLog();
      } else if (! EDID.VerifyChksum(idx)) {
         RCD_SET_FAULT(retU);
      }
      if (idx == 0) {
         //only 1 extension block is currently supported (CEA)
         idxmax = EDID.getEDID()->edi.base.num_extblk;
         if (idxmax > EDI_EXT1_IDX) idxmax = EDI_EXT1_IDX;
      }
      idx++ ;
   } while (idx <= idxmax);

   flags.bits.data_loadeed = 1;

   file.Close();
   if (!RCD_IS_OK(retU)) {
      GLog.DoLog("OpenEDID() FAILED.");

      if (! EDID.Get_ERR_Ignore()) {
         return retU;
      } else {
         GLog.DoLog("Error ignored: trying to Reparse().");
      }
   }

   GLog.slog.Printf("Loaded %u EDID block(s)\n", idx);
   GLog.DoLog();

   retU = Reparse();
   if (!RCD_IS_OK(retU)) {
      GLog.DoLog("OpenEDID()->Reparse() FAILED.");
   }

   return retU;
}

rcode wxEDID_Frame::SaveEDID() {
   rcode retU;
   RCD_SET_OK(retU);

   GLog.DoLog("SaveEDID()");

   uint dtalen = (EDID.getNumValidBlocks() * EDI_BLK_SIZE);
   if ((dtalen == 0) || (dtalen > sizeof(edi_buf_t)) ) {
      RCD_RETURN_FAULT(retU);
   }

   retU = AssembleEDID_main();
   if (!RCD_IS_OK(retU)) return retU;

   wxFileDialog dlg_open(this, "Save EDID binary file", "", "",
                         "Binary (*.bin)|*.bin|All (*.*)|*.*", wxFD_SAVE|wxFD_OVERWRITE_PROMPT);
   //set last used path
   if (config.b_have_last_fpath) {
      dlg_open.SetPath(config.last_used_fpath);
   }

   if (dlg_open.ShowModal() != wxID_OK) {
      GLog.DoLog("SaveEDID() cancelled by user.");
      RCD_RETURN_OK(retU);
   }

   wxFile file;
   tmps = dlg_open.GetPath();
   if (! file.Open(tmps, wxFile::write) ) {
      RCD_RETURN_FAULT(retU);
   }
   //save edid_t + extensions
   if ( dtalen != file.Write(EDID.getEDID()->buff, dtalen) ) {
      RCD_SET_FAULT(retU);
   }
   file.Close();
   if (RCD_IS_OK(retU)) {
      GLog.slog = "File saved:\n ";
      GLog.slog << tmps;
      GLog.DoLog();
   }

   return retU;
}

rcode wxEDID_Frame::SaveRep_SubGrps(edi_grp_cl *pgrp, wxString& reps) {
   static int rcdepth = -1;
   rcode      retU;
   wxString   grpINDENT;
   wxString   sval;
   u32_t      tmpi;
   u32_t      ival;
   u32_t      typeID;


   RCD_SET_OK(retU);

   rcdepth++ ;
   if (rcdepth > 1) { //hardcoded nesting depth limit
      rcdepth-- ;
      RCD_RETURN_FAULT(retU);
   }

   //indentation based on nesting depth
   for (int nsub=0; nsub<rcdepth; nsub++) {
      grpINDENT << wxIDNT;
   }

   sval.Printf("offs=%u (0x%04X): ", pgrp->getAbsOffs(), pgrp->getAbsOffs());
   reps << wxLF << grpINDENT << sval << pgrp->GroupName << wxLF;

   //special case for:
   //CEA:VDB:SVD: SVD
   //CEA-EXT:VFPD: SVR
   typeID = pgrp->getTypeID() & (ID_CEA_MASK | ID_CEA_EXT_MASK);

   if ( (ID_VDB  == typeID) ||
        (ID_VFPD == typeID)  ) {

      for (u32_t sgp=0; sgp<pgrp->getSubGrpCount(); sgp++ ) {
         u32_t         mval;
         edi_grp_cl   *subgrp;
         edi_dynfld_t *p_field;

         subgrp  = pgrp->getSubGroup(sgp); //SVD or SVR
         p_field = subgrp->FieldsAr.Item(0); //VIC field or SVR

         sval.Printf("offs=%u (0x%04X): ", subgrp->getAbsOffs(), subgrp->getAbsOffs());
         reps << wxLF << wxIDNT << sval << subgrp->GroupName << wxLF;

         reps << wxIDNT << wxIDNT << wxString::FromAscii(p_field->field.name);
         //align values:
         tmpi = (16 - strlen(p_field->field.name));
         if (tmpi > 16) tmpi = 1;
         sval.Empty();
         sval.Pad(tmpi, wxSP);
         reps << sval;
         //p_field value
         retU = (EDID.*p_field->field.handlerfn)(OP_READ, sval, ival, p_field);
         if (!RCD_IS_OK(retU)) break;
         if (ID_VDB  == typeID) {
            ival = EDID.CEA_VDB_SVD_decode(ival, mval);
         } else {
            ival = EDID.CEA_VFPD_SVR_decode(ival, mval); //mval != 0 for DTD
         }
         sval.Empty();
         sval << ival;
         reps << sval;
         //align units / vmap strings
         tmpi = (6 - sval.Len());
         if (tmpi > 6) tmpi = 1;
         sval.Empty();
         sval.Pad(tmpi, wxSP);
         reps << sval;
         //VIC value interpretation:
         if (ID_VDB  == typeID) {
            EDID.getVDesc(sval, p_field, ival);
         } else {
            if (mval != 0) {
               sval.Printf("DTD number %u", mval);
            } else {
               EDID.getVDesc(sval, p_field, ival);
            }
            reps << sval << wxLF;
            continue;
         }
         reps << sval << wxLF;

         p_field = subgrp->FieldsAr.Item(1); //SVD Native flag field

         reps << wxIDNT << wxIDNT << wxString::FromAscii(p_field->field.name);
         //align values:
         tmpi = (16 - strlen(p_field->field.name));
         if (tmpi > 16) tmpi = 1;
         sval.Empty();
         sval.Pad(tmpi, wxSP);
         reps << sval;
         //p_field value
         reps << mval << wxLF;
      }

      RCD_SET_OK(retU);
      goto exit;
   }


   for (u32_t itf=0; itf<(pgrp->FieldsAr.GetCount()); itf++) {
      u32_t         ival;
      edi_dynfld_t *p_field = pgrp->FieldsAr.Item(itf);
      if (p_field == NULL) {
         RCD_SET_FAULT(retU); break;
      }
      reps << wxIDNT << grpINDENT << wxString::FromAscii(p_field->field.name);
      //align values:
      tmpi = (16 - strlen(p_field->field.name));
      if (tmpi > 16) tmpi = 1;
      sval.Empty();
      sval.Pad(tmpi, wxSP);
      reps << sval;
      //p_field value
      sval.Empty();
      retU = (EDID.*p_field->field.handlerfn)(OP_READ, sval, ival, p_field);
      if (!RCD_IS_OK(retU)) break;
      reps << sval;
      //align units / vmap strings
      tmpi = (6 - sval.Len());
      if (tmpi > 6) tmpi = 1;
      sval.Empty();
      sval.Pad(tmpi, wxSP);
      reps << sval;
      //value interpretation (mapped)
      if ( ((p_field->field.flags & EF_VS) != 0) && (p_field->field.vmap != NULL)) {
         if (ival < p_field->field.vmap->nval) {
            EDID.getVDesc(sval, p_field, ival);
         }
         reps << sval;
      }
      //value unit
      retU = EDID.getValUnitName(sval, p_field->field.flags);
      if (!RCD_IS_OK(retU)) break;

      reps << sval;
      if ((p_field->field.flags & EF_NU) != 0) reps << wxTAB << "<unused>";
      reps << wxLF;

   }
   if (! RCD_IS_OK(retU)) {
      /* this can happen if edid field definition contains bad combination
         of flags and handler type */
      reps << "\n< internal error! >\n";
      rcdepth-- ;
      return retU;
   }

   //subgroups/subfields
   for (u32_t sgp=0; sgp<pgrp->getSubGrpCount(); sgp++ ) {
      edi_grp_cl *subgrp;
      subgrp = pgrp->getSubGroup(sgp);

      retU = SaveRep_SubGrps(subgrp, reps);
      if (!RCD_IS_OK(retU)) break;
   }

exit:
   rcdepth-- ;

   return retU;
}

rcode wxEDID_Frame::SaveReport() {
   rcode  retU;
   RCD_SET_OK(retU);

   GLog.DoLog("SaveReport()");

   retU = AssembleEDID_main();
   if (!RCD_IS_OK(retU)) return retU;

   wxString* reps = new wxString();
   if (reps == NULL) {
      RCD_RETURN_FAULT(retU);
   }
   reps->Alloc(16*1024);

   *reps = "wxEDID v";
   *reps << ver << "\nEDID structure and data:\nSource file: \"";
   *reps << edid_file_name << "\"";

   if (EDID.getNumValidBlocks() == 0) {
      delete reps;
      RCD_RETURN_FAULT(retU);
   }

   for (u32_t blk=0; blk<EDID.getNumValidBlocks(); blk++ ) {
      tmps.Printf("\n\n----| EDID block [%u] |----\n", blk);
      *reps << tmps;
      wxArrGroup_cl* pBlockA = EDID.BlkGroupsAr[blk];
      u32_t      grpcnt  = EDID.BlkGroupsAr[blk]->GetCount();

      for (u32_t itg=0; itg<grpcnt; itg++) {
         edi_grp_cl *pgrp = pBlockA->Item(itg);
         if (pgrp == NULL) {
            RCD_SET_FAULT(retU); break;
         }

         //group fields / subgroups / subfields
         retU = SaveRep_SubGrps(pgrp, *reps);
         if (!RCD_IS_OK(retU)) break;
      }
      if (!RCD_IS_OK(retU)) break;
   }

   if (!RCD_IS_OK(retU)) return retU;

   *reps << "\n\n----| END |----\n";

   wxFileDialog dlg_open(this, "Save EDID report", "", "",
                         "text (*.txt)|*.txt|All (*.*)|*.*", wxFD_SAVE|wxFD_OVERWRITE_PROMPT);
   //set last used path
   if (config.b_have_last_fpath) {
      dlg_open.SetPath(config.last_used_fpath);
   }

   if (dlg_open.ShowModal() != wxID_OK) {
      GLog.DoLog("SaveReport() canceled by user.");
      delete reps;
      RCD_RETURN_OK(retU);
   }

   wxFile file;
   tmps = dlg_open.GetPath();
   if (! file.Open(tmps, wxFile::write) ) {
      delete reps;
      RCD_RETURN_FAULT(retU);
   }

   //save EDID - no extensions
   if ( reps->Len() != file.Write(reps->ToAscii(), reps->Len()) ) {
      RCD_SET_FAULT(retU);
   } else {
      GLog.slog = "File saved:\n ";
      GLog.slog << tmps;
      GLog.DoLog();
   }

   file.Close();

   delete reps;
   return retU;
}

rcode wxEDID_Frame::ExportEDID_hex() {
   rcode retU;
   RCD_SET_OK(retU);

   GLog.DoLog("ExportEDID_hex()");

   uint dtalen = (EDID.getNumValidBlocks() * EDI_BLK_SIZE);
   if ((dtalen == 0) || (dtalen > sizeof(edi_buf_t)) ) {
      RCD_RETURN_FAULT(retU);
   }

   retU = AssembleEDID_main();
   if (!RCD_IS_OK(retU)) return retU;

   wxFileDialog dlg_open(this, "Export EDID as text (hex)", "", "",
                         "text (*.txt)|*.txt|All (*.*)|*.*", wxFD_SAVE|wxFD_OVERWRITE_PROMPT);
   //set last used path
   if (config.b_have_last_fpath) {
      dlg_open.SetPath(config.last_used_fpath);
   }

   if (dlg_open.ShowModal() != wxID_OK) {
      GLog.DoLog("ExportEDID_hex() cancelled by user.");
      RCD_RETURN_OK(retU);
   }

   //gen hex string
   wxString* shex = new wxString();
   if (shex == NULL) RCD_RETURN_FAULT(retU);

   shex->Alloc(1024);
   tmps.Empty();
   u8_t *edi = reinterpret_cast <u8_t*> (EDID.getEDID());

   for (u32_t itb=0; itb<dtalen; itb++) {
      tmps.Printf("%02X", edi[itb]);
      *shex << tmps;
      if ((itb & 0x0F) == 0x0F) *shex << wxLF;
      if ((itb & 0x7F) == 0x7F) *shex << wxLF;
   }

   wxFile file;
   tmps = dlg_open.GetPath();
   if (! file.Open(tmps, wxFile::write) ) {
      delete shex;
      RCD_RETURN_FAULT(retU);
   }
   //save edid_t - no extensions
   if ( shex->Len() != file.Write(shex->ToAscii(), shex->Len()) ) {
      RCD_SET_FAULT(retU);
   }
   file.Close();
   if (RCD_IS_OK(retU)) {
      GLog.slog = "Export as Hex: File saved:\n ";
      GLog.slog << tmps;
      GLog.DoLog();
   }

   delete shex;
   return retU;
}

rcode wxEDID_Frame::ImportEDID_hex() {
   rcode    retU;
   char    *buff8 = NULL;
   u32_t    itb;
   ssize_t  itc, len;
   wxFile   file;

   RCD_SET_OK(retU);

   GLog.DoLog("ImportEDID_hex()");

   if (! config.b_cmd_txt_file) { //menu event: import EDID text file

      wxFileDialog dlg_open(this, "Import EDID from text (hex)", "", "",
                            "text (*.txt)|*.txt|All (*.*)|*", wxFD_DEFAULT_STYLE);
      //set last used path
      if (config.b_have_last_fpath) {
         dlg_open.SetPath(config.last_used_fpath);
      }

      if (dlg_open.ShowModal() != wxID_OK) {
         GLog.DoLog("ImportEDID_hex() canceled by user.");
         RCD_RETURN_OK(retU);
      }
      edid_file_name = dlg_open.GetFilename();
      tmps           = dlg_open.GetPath();

   } else {
      //open EDID text file provided as cmd line arg
      edid_file_name = config.last_used_fname;
      tmps           = config.cmd_open_file_path;
      config.b_cmd_txt_file = false; //single shot
   }

   if (! file.Open(tmps, wxFile::read) ) RCD_RETURN_FAULT(retU);

   //remember last used file path
   config.last_used_fpath   = tmps;
   config.b_have_last_fpath = true;

   len = file.Length();
   if ( (len < (128*2)) || (len > (1024*4)) ) {
      /* reject files which are too small
         but also prevent importing essays :)
      */
      GLog.DoLog("[E!] Bad file. Allowed size is 256...4096 bytes.");
      file.Close();
      RCD_RETURN_FAULT(retU);
   }

   buff8 = new char[len+1];
   if (buff8 == NULL) {
      file.Close();
      RCD_RETURN_FAULT(retU);
   }
   //read text
   if ( len != file.Read(buff8, len) ) {
      RCD_SET_FAULT(retU);
   }

   flags.bits.data_loadeed = 1; //allows parsing partially loaded data

   file.Close();
   if (!RCD_IS_OK(retU)) {
      delete [] buff8;
      return retU;
   }

   GLog.slog = "File imported:\n ";
   GLog.slog << tmps;
   GLog.DoLog();

   ClearAll();
   EDID.Clear();
   //hex to bin
   itb = 0;
   u8_t *edi = EDID.getEDID()->buff;
   for (itc=0; itc<len; itc++) {
      bool conv_ok;
      ulong chr = buff8[itc];
      if ( (chr < '0') || ((chr > '9') && (chr < 'A')) || \
          ((chr > 'F') && (chr < 'a')) || (chr > 'f') ) continue;
      //Not UTF8, just take 2 chars in one call;
      tmps    = wxString::FromUTF8(&buff8[itc], 2); itc++;
      conv_ok = tmps.ToULong(&chr, 16);

      if (! conv_ok) {
         static const char msg[] = "[E!] ImportEDID_hex() parsing file content failed @offs=%d";
         wxedid_RCD_SET_FAULT_VMSG(retU, msg, (int) itc);
         if (! EDID.Get_ERR_Ignore()) {
            delete [] buff8;
            return retU;
         } else {
            GLog.PrintRcode(retU);
            GLog.DoLog("Error ignored: trying to Reparse().");
            goto err_reparse;
         }
      }

      edi[itb] = static_cast <u8_t> (chr);

      if (itb == offsetof(edid_t, num_extblk)) {
         /*don't read extensions if they are not claimed,
           otherwise read until end of the buffer or file */
         if (chr == 0) len = (itc+1);
      }
      if (itb > sizeof(edi_buf_t)) break;
      itb++ ;
   }

err_reparse:
   delete [] buff8;

   retU = Reparse();

   return retU;
}

rcode wxEDID_Frame::AssembleEDID_main() {
   rcode         retU;
   u32_t         n_ext;
   u32_t         cksum;
   edi_grp_cl   *p_grp;
   edi_dynfld_t *p_fld;

   GLog.DoLog("Assemble EDID:");

   retU = EDID.AssembleEDID();
   if (!RCD_IS_OK(retU)) goto err;

   retU = CalcVerifyChksum(EDI_BASE_IDX);
   if (!RCD_IS_OK(retU)) goto err;

   //Update checksum in EDID.BASE.BED group instance ->
   //No need to reparse the whole buffer to get checksums refreshed.
   cksum = EDID.getEDID()->edi.base.chksum;
   p_grp = EDID.EDI_BaseGrpAr.Item(0); //BED
   p_fld = p_grp->FieldsAr.Item(9);    //BED.checksum field
   retU  = (EDID.*p_fld->field.handlerfn)(OP_WRINT, tmps, cksum, p_fld);
   if (!RCD_IS_OK(retU)) goto err;

   //if EDID.BASE.BED is currently selected, refresh the BlockData grid
   if (p_grp == edigrp_sel) {
      retU = UpdateDataGrid(p_grp);
      if (!RCD_IS_OK(retU)) {
         GLog.PrintRcode(retU);
         GLog.Show();
      }
   }

   n_ext = EDID.getEDID()->edi.base.num_extblk;
   if (n_ext > EDI_EXT2_IDX) {
      RCD_SET_FAULT(retU);
      goto err;
   }

   for (u32_t itb=1; itb<=n_ext; ++itb) {
      retU = CalcVerifyChksum(itb);
      if (!RCD_IS_OK(retU)) goto err;
   }

   if (n_ext >= EDI_EXT0_IDX) {
      //Update checksum in EDID.CEA.CHD group instance

      p_grp = EDID.EDI_Ext0GrpAr.Item(0);
      if (p_grp->getTypeID() != (ID_CHD | T_EDID_FIXED)) RCD_RETURN_OK(retU); //Not a CEA extension

      cksum = EDID.getEDID()->edi.ext0[EDI_BLK_SIZE-1];
      p_fld = p_grp->FieldsAr.Item(8);    //CHD.checksum field
      retU  = (EDID.*p_fld->field.handlerfn)(OP_WRINT, tmps, cksum, p_fld);
      if (!RCD_IS_OK(retU)) goto err;
   }

   //if EDID.CEA.CHD is currently selected, refresh the BlockData grid
   if (p_grp == edigrp_sel) {
      retU = UpdateDataGrid(p_grp);
      if (!RCD_IS_OK(retU)) {
         GLog.PrintRcode(retU);
         GLog.Show();
      }
   }

   return retU;

err:
   GLog.DoLog("[E!] Assemble EDID FAILED.");
   GLog.PrintRcode(retU);
   GLog.Show();
   return retU;
}

rcode wxEDID_Frame::UpdateBlockTree() {
   rcode        retU;
   wxTreeItemId trRoot, trBlock;

   BlockTree->DeleteAllItems();

   if (EDID.EDI_BaseGrpAr.GetCount() < 1) RCD_RETURN_FAULT(retU);

   trRoot  = BlockTree->AddRoot(edid_file_name);
   trBlock = BlockTree->AppendItem(trRoot, "EDID: Main", -1, -1, NULL);

   BlockTree->Expand(trRoot);

   for (u32_t itg=0; itg<EDID.EDI_BaseGrpAr.GetCount(); itg++) {
      wxTreeItemId  item;
      edi_grp_cl   *group;
      group = EDID.EDI_BaseGrpAr.Item(itg);
      GetFullGroupName(group, tmps);
      item  = BlockTree->AppendItem(trBlock, tmps, -1, -1, group);
   }
   BlockTree->Expand(trBlock);

   if (EDID.EDI_Ext0GrpAr.GetCount() < 1) {
      RCD_RETURN_OK(retU);
   }

   trBlock = BlockTree->AppendItem(trRoot, "EDID: CEA-861", -1, -1, NULL);
   for (u32_t itg=0; itg<EDID.EDI_Ext0GrpAr.GetCount(); itg++) {
      wxTreeItemId  item;
      edi_grp_cl   *group;
      group = EDID.EDI_Ext0GrpAr.Item(itg);
      GetFullGroupName(group, tmps);
      item  = BlockTree->AppendItem(trBlock, tmps, -1, -1, group);
      //sub groups
      if (group->getSubGrpCount() > 0) {
         for (u32_t idx=0; idx<group->getSubGrpCount(); idx++ ) {
            wxTreeItemId  subitem;
            edi_grp_cl   *subgroup;
            subgroup = group->getSubGroup(idx);
            GetFullGroupName(subgroup, tmps);
            subitem  = BlockTree->AppendItem(item, tmps, -1, -1, subgroup);
         }
      }
   }
   BlockTree->Expand(trBlock);

   RCD_RETURN_OK(retU);
}

wxTreeItemId wxEDID_Frame::BlkTreeInsGrp(wxTreeItemId trItemID, edi_grp_cl* pgrp, u32_t idx) {
   u32_t  n_subg;

   GetFullGroupName(pgrp, tmps);
   trItemID = BlockTree->InsertItem(trItemID, idx, tmps, -1, -1, pgrp);
   n_subg   = pgrp->getSubGrpCount();

   if (n_subg == 0) return trItemID;

   { //insert subgroups
      GroupAr_cl   *subg_ar;
      edi_grp_cl   *psubg;

      subg_ar = pgrp->getSubArray();

      for (u32_t idx=0; idx<n_subg; ++idx) {
         psubg = subg_ar->Item(idx);
         GetFullGroupName(psubg, tmps);
         BlockTree->AppendItem(trItemID, tmps, -1, -1, psubg);
      }
   }
   return trItemID;
}

void wxEDID_Frame::BlkTreeDelGrp(edi_grp_cl* pgrp) {
   //detach edi_grp_cl::wxTreeItemData before group deletion
   wxTreeItemId trItem;
   u32_t        n_subg;

   trItem = pgrp->GetId();
   n_subg = pgrp->getSubGrpCount();

   if (n_subg == 0) goto del_node;

   { //detach subgroups
      GroupAr_cl   *subg_ar;
      edi_grp_cl   *psubg;
      wxTreeItemId  subItem;

      subg_ar = pgrp->getSubArray();

      for (u32_t idx=0; idx<n_subg; ++idx) {
         psubg   = subg_ar->Item(idx);
         subItem = psubg->GetId();
         BlockTree->SetItemData(subItem, NULL);
         BlockTree->Delete(subItem);
      }
   }
del_node:
   BlockTree->SetItemData(trItem, NULL);
   BlockTree->Delete     (trItem);
}

rcode wxEDID_Frame::UpdateDataGridRow(int nrow, edi_dynfld_t *p_field) {
   rcode        retU;
   u32_t        tmpi;
   static const wxString fmsg = "[E!] UpdateDataGridRow(): failed to read field value.";

   //p_field name
   tmps = wxString::FromAscii(p_field->field.name);
   BlkDataGrid->SetCellValue(nrow, DATGR_COL_NAME, tmps);
   BlkDataGrid->SetReadOnly (nrow, DATGR_COL_NAME, true);
   //p_field value
   tmps.Empty();
   retU = (EDID.*p_field->field.handlerfn)(OP_READ, tmps, tmpi, p_field);
   if (!RCD_IS_OK(retU)) {
      GLog.DoLog(fmsg);
      GLog.PrintRcode(retU);
      //return retU;
      tmps = "<error!>";
   }
   BlkDataGrid->SetCellValue(nrow, DATGR_COL_VAL, tmps);
   //is read-only
   if (EDID.Get_RD_Ignore()) {
      BlkDataGrid->SetReadOnly(nrow, DATGR_COL_VAL, false);
   } else {
      BlkDataGrid->SetReadOnly(nrow, DATGR_COL_VAL, (p_field->field.flags & EF_RD));
   }
   //get types
   retU = EDID.getValTypeName(tmps, p_field->field.flags);
   if (!RCD_IS_OK(retU)) return retU;
   BlkDataGrid->SetCellValue(nrow, DATGR_COL_TYPE, tmps);
   BlkDataGrid->SetReadOnly (nrow, DATGR_COL_TYPE, true);
   //get units
   retU = EDID.getValUnitName(tmps, p_field->field.flags);
   if (!RCD_IS_OK(retU)) return retU;
   if (tmps.IsEmpty()) tmps = "--";
   BlkDataGrid->SetCellValue(nrow, DATGR_COL_UNIT, tmps);
   BlkDataGrid->SetReadOnly (nrow, DATGR_COL_UNIT, true);
   //get flags
   retU = EDID.getValFlagsAsString(tmps, p_field->field.flags);
   if (!RCD_IS_OK(retU)) return retU;
   BlkDataGrid->SetCellValue(nrow, DATGR_COL_FLG, tmps);
   BlkDataGrid->SetReadOnly (nrow, DATGR_COL_FLG, true);
   //show field details
   if (b_dta_grid_details) {
      u32_t  fval;
      //base offset
      fval  = p_field->field.offs;
      tmps.Empty(); tmps << fval;
      BlkDataGrid->SetCellValue(nrow, DATGR_COL_OFFS, tmps);
      BlkDataGrid->SetReadOnly (nrow, DATGR_COL_OFFS, true);
      //field offset in bits/bytes
      fval  = p_field->field.shift;
      tmps.Empty(); tmps << fval;
      BlkDataGrid->SetCellValue(nrow, DATGR_COL_SHIFT, tmps);
      BlkDataGrid->SetReadOnly (nrow, DATGR_COL_SHIFT, true);
      //field size in bits/bytes
      fval  = p_field->field.fldsize;
      tmps.Empty(); tmps << fval;
      BlkDataGrid->SetCellValue(nrow, DATGR_COL_FSZ, tmps);
      BlkDataGrid->SetReadOnly (nrow, DATGR_COL_FSZ, true);
   }

   //set row color for type
   RCD_SET_OK(retU);

   wxGridCellAttr *RowAttr = BlkDataGrid->GetOrCreateCellAttr(nrow, DATGR_COL_NAME);
   RowAttr->SetTextColour(*wxBLACK);

   {
      __label__  set_bgcol;
      u32_t      fflags;
      wxColour  *rowColour;

      fflags = p_field->field.flags;

      if (fflags & EF_FLT) {
         rowColour = &grid_color_float;
         goto set_bgcol;
      }
      if (fflags & EF_HEX) {
         rowColour = &grid_color_hex;
         goto set_bgcol;
      }
      if (fflags & EF_BIT) {
         rowColour = &grid_color_bit;
         goto set_bgcol;
      }
      goto def_color;

      set_bgcol:
         RowAttr->SetBackgroundColour(*rowColour);
         goto exit;
   }

def_color:
   RowAttr->SetBackgroundColour(*wxWHITE);
exit:
   BlkDataGrid->SetRowAttr(nrow, RowAttr);

   return retU;
}

rcode wxEDID_Frame::UpdateDataGrid(edi_grp_cl* pgrp) {
   rcode  retU;
   int    NumR;
   int    nGrpF;
   int    itf;

   if (pgrp == NULL) RCD_RETURN_FAULT(retU);

   NumR  = BlkDataGrid->GetNumberRows();
   nGrpF = pgrp->FieldsAr.GetCount();

   if (NumR < nGrpF) {
      BlkDataGrid->AppendRows(nGrpF-NumR);
   } else if (NumR > nGrpF) {
      NumR -= nGrpF;
      BlkDataGrid->DeleteRows(0, NumR);
   }

   for (itf=0; itf<nGrpF; itf++) {
      edi_dynfld_t *field = pgrp->FieldsAr.Item(itf);
      if (field == NULL) RCD_RETURN_FAULT(retU);
      retU = UpdateDataGridRow(itf, field);
   }

   BlkDataGrid->AutoSizeColumns(false);
   BlkDataGrid->AutoSizeRows(false);
   BlkDataGrid->SetRowLabelSize(wxGRID_AUTOSIZE);
   BlkDataGrid->FitInside();

   flags.bits.edigridblk_ok = 1;

   RCD_RETURN_OK(retU);
}

void wxEDID_Frame::DataGrid_ChangeView() {
   rcode  retU;
   u32_t  ncol;

   ncol = BlkDataGrid->GetNumberCols();

   if (b_dta_grid_details) {
      if (ncol == DATGR_NCOL_2) return;
      BlkDataGrid->AppendCols(DATGR_NCOL_DIFF);
      BlkDataGrid->SetColLabelValue(DATGR_COL_OFFS , "offs" );
      BlkDataGrid->SetColLabelValue(DATGR_COL_SHIFT, "shift");
      BlkDataGrid->SetColLabelValue(DATGR_COL_FSZ  , "size" );

      if (! BlkDataGrid->IsEnabled()) return;

      retU = UpdateDataGrid(edigrp_sel);
      if (!RCD_IS_OK(retU)) {
         GLog.PrintRcode(retU);
         GLog.Show();
      }
   } else {
      if (ncol == DATGR_NCOL_1) return;
      BlkDataGrid->DeleteCols(DATGR_NCOL_1, DATGR_NCOL_DIFF);
   }
}

rcode wxEDID_Frame::SetFieldDesc(int row) {
   rcode retU;
   RCD_SET_OK(retU);
   //show field describtion
   if (edigrp_sel != NULL) {
      edi_dynfld_t *p_field = edigrp_sel->FieldsAr.Item(row);
      if (p_field == NULL) {
         RCD_SET_FAULT(retU);
         GLog.DoLog("[E!] SetFieldDesc()");
         GLog.PrintRcode(retU);
         return retU;
      }
      if (p_field->field.flags & EF_GPD) {
         txc_edid_info->SetValue(edigrp_sel->GroupDesc);
      } else {
         txc_edid_info->SetValue(wxString::FromUTF8(p_field->field.desc));
      }
   }
   return retU;
}

rcode wxEDID_Frame::DTD_Ctor_ModeLine() {
   rcode  retU;
   int    tmpi;

   //pix clk tmp
   GLog.slog.Printf("%.02f", static_cast <double> (sct_pixclk->data / 100.0));

   tmpi  = sct_xres->data;
   tmps.Printf("\"%dx%dx", tmpi, sct_vres->data);
   tmps << txc_vrefresh->GetValue() << "\" ";
   tmps << GLog.slog;
   //xres
   tmps << wxSP << tmpi << wxSP;
   //hsoffs - front porch
   tmpi += sct_hsoffs->data; tmps << tmpi << wxSP;
   //hsync width
   tmpi += sct_hswidth->data; tmps << tmpi << wxSP;
   //hsync end - back porch
   tmps << txc_htotal->GetValue() << wxSP;
   //vres
   tmpi = sct_vres->data; tmps << tmpi << wxSP;
   //vsync - front porch
   tmpi += sct_vsoffs->data; tmps << tmpi << wxSP;
   //vsync width
   tmpi += sct_vswidth->data; tmps << tmpi << wxSP;
   //vsync - back porch
   tmps << txc_vtotal->GetValue();

   txc_modeline->SetValue(tmps);

   RCD_RETURN_OK(retU);
}

rcode wxEDID_Frame::DTD_Ctor_WriteInt(dtd_sct_cl& sct) {
   static const wxString fmsg  = "FAULT: DTD_Ctor_WriteInt()";
   static       wxString s_oldv;

   rcode         retU, retU2;
   uint          val;
   wxString      sval;
   edi_dynfld_t *p_field = sct.field;

   if (p_field == NULL) {
      RCD_SET_FAULT(retU);
      GLog.DoLog(fmsg);
      GLog.PrintRcode(retU);
      return retU;
   }

   //read old value
   s_oldv.Empty();
   retU = (EDID.*p_field->field.handlerfn)(OP_READ, s_oldv, val, p_field);
   if (! RCD_IS_OK(retU)) {
      GLog.PrintRcode(retU); GLog.Show(); return retU;
   }

   val = sct.data;
   GetFullGroupName(edigrp_sel, tmps);
   tmps << ", " << win_stat_bar->GetStatusText(SBAR_GRPOFFS) << wxLF;
   tmps << "field changed: " << wxString::FromAscii(p_field->field.name);
   tmps << ", old value: " << s_oldv;

   sval.Empty();
   retU = (EDID.*p_field->field.handlerfn)(OP_WRINT, sval, val, p_field);
   if (!RCD_IS_OK(retU)) {
      GLog.DoLog(fmsg);
      GLog.PrintRcode(retU);
   }
   tmps << "\nnew value: " << val;

   //re-read p_field -> immediately check the value/revert to last correct value
   sval.Empty();
   retU2 = (EDID.*p_field->field.handlerfn)(OP_READ, sval, val, p_field);
   if (!RCD_IS_OK(retU2)) {
      GLog.DoLog(fmsg);
      GLog.PrintRcode(retU2);
   }
   sct.SetValue(val);
   tmps << ", verified value: " << val << wxLF;
   GLog.DoLog(tmps);

   if (!RCD_IS_OK(retU)) {
      GLog.DoLog(fmsg);
      GLog.PrintRcode(retU);
   }

   return retU;
}

rcode wxEDID_Frame::DTD_Ctor_Recalc() {
   rcode retU;
   double linew, pixclk, tline, totlies, framet;

   linew   = (sct_hblank->data + sct_xres->data);
   pixclk  = (sct_pixclk->data * 10000);
   tline   = (linew / pixclk);
   totlies = (sct_vblank->data + sct_vres->data);
   framet  = (totlies * tline);
   tmps.Printf("%.02f", (1.0/framet)); //Hz
   txc_vrefresh->SetValue(tmps);
   tmps.Printf("%.02f", ((1.0/tline)/1000.0)); //kHz
   txc_hfreq->SetValue(tmps);

   RCD_RETURN_OK(retU);
}

rcode wxEDID_Frame::DTD_Ctor_read_field(dtd_sct_cl& sct, const edi_grp_cl& group, u32_t idx_field) {
   rcode         retU;
   uint          val   = 0;
   edi_dynfld_t *p_field = NULL;

   tmps.Empty();
   p_field = group.FieldsAr.Item(idx_field);
   if (p_field == NULL) RCD_RETURN_FAULT(retU);

   retU = (EDID.*p_field->field.handlerfn)(OP_READ, tmps, val, p_field);
   sct.SetValue(val);
   if (! RCD_IS_OK(retU)) {
      GLog.DoLog("DTD_Ctor_read_field() FAILED.");
      GLog.PrintRcode(retU);
   }
   sct.field = p_field;
   sct.data = val;
   return retU;
}

rcode wxEDID_Frame::DTD_Ctor_read_all() {
   rcode retU;

   if (edigrp_sel == NULL) RCD_RETURN_FAULT(retU);

   edi_grp_cl& group = *edigrp_sel;
   if (group.getTypeID() != ID_DTD) RCD_RETURN_FAULT(retU);

   if (! RCD_IS_OK( DTD_Ctor_read_field(*sct_pixclk , group, DTD_IDX_PIXCLK ) ) ) RCD_RETURN_FAULT(retU);
   if (! RCD_IS_OK( DTD_Ctor_read_field(*sct_xres   , group, DTD_IDX_HAPIX  ) ) ) RCD_RETURN_FAULT(retU);
   if (! RCD_IS_OK( DTD_Ctor_read_field(*sct_hblank , group, DTD_IDX_HBPIX  ) ) ) RCD_RETURN_FAULT(retU);
   if (! RCD_IS_OK( DTD_Ctor_read_field(*sct_vres   , group, DTD_IDX_VALIN  ) ) ) RCD_RETURN_FAULT(retU);
   if (! RCD_IS_OK( DTD_Ctor_read_field(*sct_vblank , group, DTD_IDX_VBLIN  ) ) ) RCD_RETURN_FAULT(retU);
   if (! RCD_IS_OK( DTD_Ctor_read_field(*sct_hsoffs , group, DTD_IDX_HSOFFS ) ) ) RCD_RETURN_FAULT(retU);
   if (! RCD_IS_OK( DTD_Ctor_read_field(*sct_hswidth, group, DTD_IDX_HSWIDTH) ) ) RCD_RETURN_FAULT(retU);
   if (! RCD_IS_OK( DTD_Ctor_read_field(*sct_vsoffs , group, DTD_IDX_VSOFFS ) ) ) RCD_RETURN_FAULT(retU);
   if (! RCD_IS_OK( DTD_Ctor_read_field(*sct_vswidth, group, DTD_IDX_VSWIDTH) ) ) RCD_RETURN_FAULT(retU);
   if (! RCD_IS_OK( DTD_Ctor_read_field(*sct_hborder, group, DTD_IDX_HBORD  ) ) ) RCD_RETURN_FAULT(retU);
   if (! RCD_IS_OK( DTD_Ctor_read_field(*sct_vborder, group, DTD_IDX_VBORD  ) ) ) RCD_RETURN_FAULT(retU);

   retU = DTD_Ctor_Recalc();
   return retU;
}

rcode wxEDID_Frame::Reparse() {
   rcode retU;
   rcode retU2;
   bool  err_ignore;
   u32_t n_extblk;

   GLog.DoLog("Reparse()");

   RCD_SET_OK(retU2);

   ntbook->SetSelection(0); //go to tree view

   if (b_srcgrp_orphaned) delete edigrp_src;
   edigrp_src = NULL;
   edigrp_sel = NULL;

   err_ignore = EDID.Get_ERR_Ignore();
   dtd_panel->Enable(false);
   BlkDataGrid->Enable(false);
   ClearAll(false); //do not clear BlockTree & BlkGrid

   retU = VerifyChksum(EDI_BASE_IDX);
   if (!RCD_IS_OK(retU)) {
      if (! err_ignore) return retU;
      GLog.PrintRcode(retU);
      retU2 = retU;
   }

   retU = EDID.ParseEDID_Base(n_extblk);
   if (!RCD_IS_OK(retU)) {
      if (! err_ignore) return retU;
      GLog.PrintRcode(retU);
      retU2 = retU;
   }

   if (n_extblk > 0) {
      retU = VerifyChksum(EDI_EXT0_IDX);
      if (!RCD_IS_OK(retU)) {
         if (! err_ignore) return retU;
         GLog.PrintRcode(retU);
         retU2 = retU;
      }

      retU = EDID.ParseEDID_CEA();
      if (!RCD_IS_OK(retU)) {
         if (err_ignore) {
            EDID.ForceNumValidBlocks(2);
            GLog.PrintRcode(retU);
            retU2 = retU;
         } else
         return retU;
      }
   }

   retU = UpdateBlockTree();
   if (!RCD_IS_OK(retU)) return retU;

   EnableControl(true);
   if (!RCD_IS_OK(retU2)) return retU2;

   RCD_RETURN_OK(retU);
}

void wxEDID_Frame::ClearAll(bool b_clrBlocks) {
   EDID.EDI_BaseGrpAr.Empty();
   EDID.EDI_Ext0GrpAr.Empty();
   EDID.EDI_Ext1GrpAr.Empty();
   EDID.EDI_Ext2GrpAr.Empty();

   if (b_srcgrp_orphaned) {delete edigrp_src;}

   edigrp_src        = NULL;
   b_srcgrp_orphaned = false;

   if (miInfoNOK != mnu_BlkTree->FindItemByPosition(0)) {
      miRemoved = mnu_BlkTree->Remove(id_mnu_info);
      mnu_BlkTree->Insert(0, miInfoNOK);
      mnu_BlkTree->SetLabel(id_mnu_info, "<void>");
   }

   if (b_clrBlocks) {
      BlkDataGrid->ClearGrid();
      BlockTree->DeleteAllItems();
   }
   BlkDataGrid->Enable(false);
   EnableControl      (false);
   txc_edid_info->Clear();
   tmps.Empty();
   win_stat_bar->SetStatusText(tmps, SBAR_GRPOFFS);
}

void wxEDID_Frame::EnableControl(bool enb) {
   mnu_save_edi ->Enable(enb);
   mnu_exphex   ->Enable(enb);
   mnu_save_text->Enable(enb);

   flags.bits.ctrl_enabled = enb;
}

rcode wxEDID_Frame::VerifyChksum(uint block) {
   rcode retU;
   wxString schksum;

   schksum.Printf("0x%02X", EDID.getEDID()->blk[block][offsetof(edid_t, chksum)]);
   tmps.Printf("EDID block %u checksum= ", block);
   tmps << schksum;
   if (EDID.VerifyChksum(block)) {
      tmps << " OK";
      RCD_SET_OK(retU);
   } else {
      tmps << " BAD!";
      RCD_SET_FAULT(retU);
   }
   GLog.DoLog(tmps);
   return retU;
}

rcode wxEDID_Frame::CalcVerifyChksum(uint block) {
   rcode retU;

   tmps.Printf("EDID block %u checksum= 0x%02X ", block, EDID.genChksum(block));
   if (EDID.VerifyChksum(block)) {
      tmps << "OK";
      RCD_SET_OK(retU);
   } else {
      tmps << "BAD!";
      RCD_SET_FAULT(retU);
   }
   GLog.DoLog(tmps);
   return retU;
}

rcode wxEDID_Frame::WriteField() {
   //Call field handler(write), then re-read the value to verify it
   static wxString sval;
   static wxString s_oldv;

   rcode  retU, retU2;
   u32_t  tmpi;
   u32_t  u_oldv;
   bool   b_len_chg = false;
   bool   b_tag_chg = false;

   if ( (row_op >= (int) edigrp_sel->FieldsAr.GetCount()) || (row_op < 0) ) {
      RCD_RETURN_FAULT(retU);
   }

   edi_dynfld_t *p_field = edigrp_sel->FieldsAr.Item(row_op);

   if (p_field == NULL) {
      RCD_RETURN_FAULT(retU);
   }
   //read old value
   s_oldv.Empty();
   retU = (EDID.*p_field->field.handlerfn)(OP_READ, s_oldv, u_oldv, p_field);
   if (! RCD_IS_OK(retU)) {
      GLog.PrintRcode(retU); GLog.Show(); return retU;
   }
   //write value
   GetFullGroupName(edigrp_sel, sval);
   sval << ", " << win_stat_bar->GetStatusText(SBAR_GRPOFFS) << wxLF;
   sval << "field changed: " << wxString::FromAscii(p_field->field.name);

   sval << ", old value: " << s_oldv;

   tmps  = BlkDataGrid->GetCellValue(row_op, DATGR_COL_VAL);
   sval << "\nnew value: " << tmps;

   retU2  = (EDID.*p_field->field.handlerfn)(OP_WRSTR, tmps, tmpi, p_field);
   if (! RCD_IS_OK(retU2)) {
      //print error code, but continue: re-reading will revert the field value
      GLog.PrintRcode(retU2);
   }
   //re-read field -> immediately check the value/revert to last correct value
   tmps.Empty();
   retU = (EDID.*p_field->field.handlerfn)(OP_READ, tmps, tmpi, p_field);
   if (!RCD_IS_OK(retU)) {
      GLog.PrintRcode(retU);
   }
   BlkDataGrid->SetCellValue(row_op, DATGR_COL_VAL, tmps);
   sval << ", verified value: " << tmps;
   GLog.DoLog(sval);

   //special case: DBC block Tag Code change:
   if (p_field->field.handlerfn == &EDID_cl::CEA_DBC_Tag) {
      b_tag_chg = true;
   } else
   //special case: DBC block Extended Tag Code change:
   if (p_field->field.handlerfn == &EDID_cl::CEA_DBC_ExTag) {
      b_tag_chg = true;
   } else
   //special case: DBC block length change:
   if (p_field->field.handlerfn == &EDID_cl::CEA_DBC_Len) {
      GroupAr_cl *grp_ar;
      i32_t       blk_free;
      long        new_len;

      grp_ar   = edigrp_sel->getParentAr();
      blk_free = grp_ar->getFreeSpace();
      tmps.ToLong(&new_len);

      new_len -= u_oldv;
      if (new_len > blk_free) {
         tmps.Empty();
         tmps << tmpi;
         BlkDataGrid->SetCellValue(row_op, DATGR_COL_VAL, s_oldv); //restore last value
         retU  = (EDID.*p_field->field.handlerfn)(OP_WRINT, s_oldv, u_oldv, p_field);
         if (!RCD_IS_OK(retU)) {
            GLog.PrintRcode(retU);
         }

         GLog.slog.Printf("[E!] DBC block length change: +%d, free: %d\n", (int) new_len, blk_free);
         GLog.DoLog();
         GLog.Show();
         RCD_RETURN_OK(retU);
      }
      b_len_chg = true;
   }
   //re-init the group
   if (RCD_IS_TRUE(retU2)) {
      if (b_tag_chg) {
         retU = BlkTreeChangeGrpType(); //spawn new group after tag code change
         if (!RCD_IS_OK(retU)) {
            //restore last value
            BlkDataGrid->SetCellValue(row_op, DATGR_COL_VAL, s_oldv);
            retU  = (EDID.*p_field->field.handlerfn)(OP_WRINT, s_oldv, u_oldv, p_field);
            RCD_RETURN_OK(retU);
         }
      }
      if (b_len_chg) {
         BlkTreeUpdateGrp(); //update after DBC block length change
      }
   }

   //changed DBC block length: group re-initialized by BlkTreeUpdateGrp()
   if (b_len_chg) return retU;

   //changing field value can lead to change in data structure,
   //check the EF_FGR flag (group refresh).
   if ((p_field->field.flags & EF_FGR) != 0) {
      flags.bits.edi_grp_rfsh = 1;
      //re-parse group data, possibly changing the layout
      retU = edigrp_sel->ForcedGroupRefresh();
   } else
   //EF_INIT: forced re-initialization of the group
   if ((p_field->field.flags & EF_INIT) != 0) {
      u32_t type_id;
      type_id = edigrp_sel->getTypeID();

      if (type_id & T_DBC_SUBGRP) {
         if (edigrp_src == edigrp_sel) edigrp_src = NULL;
         if (edigrp_sel->getParentGrp() != NULL) {
            //remember sub-group selection
            subg_idx = edigrp_sel->getParentArIdx();
            //select parent group
            edigrp_sel = edigrp_sel->getParentGrp();
            BT_Iparent = BlockTree->GetItemParent(edigrp_sel->GetId());
         }

         retU = edigrp_sel->AssembleGroup();
         if (!RCD_IS_OK(retU)) {
            GLog.PrintRcode(retU);
            return retU;
         }

         BlkTreeUpdateGrp();
      }
   }

   return retU;
}

void wxEDID_Frame::GetFullGroupName(edi_grp_cl* pgrp, wxString& grp_name) {
   grp_name  = pgrp->CodeName;
   grp_name << ": ";
   grp_name << pgrp->GroupName;
};

void wxEDID_Frame::LogGroupOP(edi_grp_cl* pgrp, const wxString& opName) {
   GLog.slog.Printf("%s: %s, Offs: Abs=%u, Rel=%u",
                    opName, pgrp->CodeName, pgrp->getAbsOffs(), pgrp->getRelOffs() );
   GLog.DoLog();
};

void wxEDID_Frame::InitBlkTreeMenu() {
   bmpUp  = wxArtProvider::GetBitmap(wxART_GO_UP     , wxART_MENU);
   bmpDn  = wxArtProvider::GetBitmap(wxART_GO_DOWN   , wxART_MENU);
   bmpIn  = wxArtProvider::GetBitmap(wxART_GO_FORWARD, wxART_MENU);
   bmpNOK = wxArtProvider::GetBitmap(wxART_WARNING   , wxART_MENU);

   mnu_BlkTree = new wxMenu();
   mnu_SubInfo = new wxMenu();
   {
    wxMenuItem *miDelete;
    wxMenuItem *miInsUp;
    wxMenuItem *miInsIn;
    wxMenuItem *miInsDn;
    wxMenuItem *miMovUp;
    wxMenuItem *miMovDn;

    accDelete = new wxAcceleratorEntry(wxACCEL_NORMAL, WXK_DELETE);
    accMoveUp = new wxAcceleratorEntry(wxACCEL_CTRL  , WXK_UP    );
    accMoveDn = new wxAcceleratorEntry(wxACCEL_CTRL  , WXK_DOWN  );

    miInfoNOK = new wxMenuItem(NULL, id_mnu_info  , " "           , wxEmptyString, wxITEM_NORMAL);
    miInfoOK  = new wxMenuItem(NULL, id_mnu_info  , " "           , wxEmptyString, wxITEM_NORMAL);
    miSubInfo = new wxMenuItem(NULL, id_mnu_info  , "sub info"    , wxEmptyString, wxITEM_NORMAL);
    miRemoved = miInfoOK;
    miInsUp   = new wxMenuItem(NULL, id_mnu_ins_up, "Insert above", wxEmptyString, wxITEM_NORMAL);
    miInsIn   = new wxMenuItem(NULL, id_mnu_ins_in, "Insert into" , wxEmptyString, wxITEM_NORMAL);
    miInsDn   = new wxMenuItem(NULL, id_mnu_ins_dn, "Insert below", wxEmptyString, wxITEM_NORMAL);

    miMovUp   = new wxMenuItem(NULL, wxID_UP      , "Move Up"     , wxEmptyString, wxITEM_NORMAL);
    miMovDn   = new wxMenuItem(NULL, wxID_DOWN    , "Move Down"   , wxEmptyString, wxITEM_NORMAL);
    miDelete  = new wxMenuItem(NULL, wxID_DELETE  , "Delete"      , wxEmptyString, wxITEM_NORMAL);

    miInfoNOK->SetBitmap(bmpNOK);
    miInsUp  ->SetBitmap(bmpUp );
    miInsIn  ->SetBitmap(bmpIn );
    miInsDn  ->SetBitmap(bmpDn );

    miDelete->SetAccel(accDelete);
    miMovUp ->SetAccel(accMoveUp);
    miMovDn ->SetAccel(accMoveDn);

    mnu_SubInfo->Append(miSubInfo);

    miInfoNOK  ->SetSubMenu(mnu_SubInfo);

    mnu_BlkTree->Append (miInfoNOK); //evt_blktree_rmb: group info
    mnu_BlkTree->InsertSeparator(1);
    mnu_BlkTree->Append (wxID_EXECUTE, "Reparse Group");
    mnu_BlkTree->Append (wxID_COPY   , "Copy"         );
    mnu_BlkTree->Append (wxID_PASTE  , "Paste"        );
    mnu_BlkTree->Append (miDelete);
    mnu_BlkTree->Append (wxID_CUT    , "Cut"          );
    mnu_BlkTree->Append (miInsUp);
    mnu_BlkTree->Append (miInsIn);
    mnu_BlkTree->Append (miInsDn);
    mnu_BlkTree->InsertSeparator(10);
    mnu_BlkTree->Append (miMovUp);
    mnu_BlkTree->Append (miMovDn);
   }
}

void wxEDID_Frame::AppLayout() {
   wxPoint wpos;
   wxSize  wsz;
   wxRect  dspsz;
   bool    b_AUI_ok;

   wpos  = config.win_pos;
   wsz   = config.win_size;
   dspsz = wxDisplay(wxDisplay::GetFromWindow(this)).GetClientArea();

   {
      wxSize wmin = GetMinSize();
      if (wsz.x < wmin.x) wsz.x = wmin.x;
      if (wsz.y < wmin.y) wsz.y = wmin.y;
   }

   if (wsz.x > dspsz.width ) wsz.x = dspsz.width;
   if (wsz.y > dspsz.height) wsz.y = dspsz.height;

   if ((wpos.x >= 0) && (wpos.y >= 0)) {

      if ((wpos.x + wsz.x) > dspsz.width ) wpos.x = dspsz.width  - wsz.x;
      if ((wpos.y + wsz.y) > dspsz.height) wpos.y = dspsz.height - wsz.y;

      SetPosition(wpos);
   }

   SetSize(wsz);

   //The wxAuiManager::LoadPerspective() can return true even if
   //the layout data is heavily malformed, generating broken UI.
   //Manual editing of the cfg file is a bad idea, because only
   //critical errors can be catch here.
   b_AUI_ok = AuiMgrEDID->LoadPerspective(config.aui_layout);

   if (! b_AUI_ok) {
      tmps = wxString::FromAscii(AUI_DefLayout);
      AuiMgrEDID->LoadPerspective(tmps);
   }
}


wxBEGIN_EVENT_TABLE(blktree_cl, wxTreeCtrl)
   EVT_KEY_UP   (blktree_cl::evt_key)
   EVT_KEY_DOWN (blktree_cl::evt_key)
wxEND_EVENT_TABLE()

void blktree_cl::evt_key(wxKeyEvent& evt) {
   wxEventType  evtype = evt.GetEventType();

   if (wxEVT_KEY_UP == evtype) {
      b_key_block = false;
      keycode     = 0;
      return;
   }
   { // block key-down evt repetition, except arrows
      bool b_arrow;
      int  newkcode = evt.GetKeyCode();

      b_arrow      = (newkcode == WXK_UP  );
      b_arrow     |= (newkcode == WXK_NUMPAD_UP);
      b_arrow     |= (newkcode == WXK_DOWN);
      b_arrow     |= (newkcode == WXK_NUMPAD_DOWN);
      b_arrow     &= !evt.ControlDown();
      b_key_block  = !b_arrow;
      b_key_block &= (newkcode == keycode );
      keycode      = newkcode;
      if (b_key_block) return;
   }

   evt.Skip(true);
}

wxBEGIN_EVENT_TABLE(fgrid_cl, wxGrid)
   EVT_MENU(wxID_ANY, fgrid_cl::evt_datagrid_vmnu)
wxEND_EVENT_TABLE()

void fgrid_cl::evt_datagrid_vmnu(wxCommandEvent& evt) {
   int row = GetGridCursorRow();

   HideCellEditControl();

   //MenuItem id contains selected value:
   tmps.Empty(); tmps << evt.GetId();
   SetCellValue(row, DATGR_COL_VAL, tmps);

   wxGridEvent evtgr(wxID_ANY, wxEVT_GRID_CELL_CHANGED, NULL, row, DATGR_COL_VAL);
   AddPendingEvent(evtgr);
}


wxBEGIN_EVENT_TABLE( dtd_screen_cl, wxPanel   )
    EVT_PAINT      ( dtd_screen_cl::evt_paint )
wxEND_EVENT_TABLE()

void dtd_screen_cl::scr_aspect_ratio(wxSize& dcsize, wxPaintDC& dc) {
   float  faspHV;
   bool   b_scaling_to_dc_Y;

   {
      wxSize HVtotal;
      float  faspDC;

      HVtotal.x = pwin->dtd_Htotal;
      HVtotal.y = pwin->dtd_Vtotal;

      //check DC aspect ratio
      faspDC  = dcsize.x;
      faspDC /= dcsize.y;
      faspHV  = HVtotal.x;
      faspHV /= HVtotal.y;

      b_scaling_to_dc_Y = (faspDC > faspHV);
   }

   {  //DC clipping & scaling
      wxSize  szClip;
      wxPoint dcXY(0, 0);

      if (b_scaling_to_dc_Y) {

         szClip.y  = dcsize.y;
         szClip.x  = dcsize.y;
         szClip.x *= faspHV;

         dcXY.x    = dcsize.x;
         dcXY.x   -= szClip.x;
         dcXY.x  >>= 1;

      } else {
         //scaling to dc X-axis
         szClip.x  = dcsize.x;
         szClip.y  = dcsize.x;
         szClip.y /= faspHV;

         dcXY.y    = dcsize.y;
         dcXY.y   -= szClip.y;
         dcXY.y  >>= 1;
      }

      dcsize = szClip;
      dc.SetDeviceOrigin(dcXY.x, dcXY.y);
   }
}

rcode dtd_screen_cl::calc_coords(wxSize& dcsize) {
   rcode  retU;
   float  ftmpA, ftmpB, fltDCsize, scale_base;

   if (pwin == NULL) RCD_RETURN_FAULT(retU);

   //V-sync pulse bar (top edge)
   rcVsync.x = 0; rcVsync.y = 0; rcVsync.width = dcsize.x;
   fltDCsize = dcsize.y;
   scale_base = (pwin->sct_vblank->data + pwin->sct_vres->data);
   if (scale_base == 0) RCD_RETURN_FAULT(retU);
   ftmpA = pwin->sct_vswidth->data;
   ftmpB = (ftmpA*fltDCsize)/scale_base;
   rcVsync.height = ftmpB;
   if (rcVsync.height == 0) rcVsync.height = 1;

   //Screen Area Y-coords rcScreen
   // Y
   ftmpA = (pwin->sct_vblank->data - pwin->sct_vsoffs->data);
   ftmpB = (ftmpA*fltDCsize)/scale_base;
   rcScreen.y = ftmpB;
   // Height
   ftmpA = pwin->sct_vres->data;
   ftmpB = (ftmpA*fltDCsize)/scale_base;
   rcScreen.height = ftmpB;

   //borders
   szVborder.y = 0;
   if (pwin->sct_vborder->data != 0) {
      ftmpA = pwin->sct_vborder->data;
      ftmpB = (ftmpA*fltDCsize)/scale_base;
      szVborder.y = ftmpB;
      if (szVborder.y == 0) szVborder.y = 1;
   }
   szHborder.y = rcScreen.height;

   //H-sync pulse bar (vertical, left edge)
   rcHsync.x = 0; rcHsync.y = rcVsync.height;
   rcHsync.height = (dcsize.y-rcVsync.height);
   fltDCsize = dcsize.x;
   scale_base = (pwin->sct_hblank->data + pwin->sct_xres->data);
   if (scale_base == 0) RCD_RETURN_FAULT(retU);
   ftmpA = pwin->sct_hswidth->data;
   ftmpB = (ftmpA*fltDCsize)/scale_base;
   rcHsync.width = ftmpB;
   if (rcHsync.width == 0) rcHsync.width = 1;

   //Screen Area X-coords rcScreen
   // X
   ftmpA = (pwin->sct_hblank->data - pwin->sct_hsoffs->data);
   ftmpB = (ftmpA*fltDCsize)/scale_base;
   rcScreen.x = ftmpB;
   // Width
   ftmpA = pwin->sct_xres->data;
   ftmpB = (ftmpA*fltDCsize)/scale_base;
   rcScreen.width = ftmpB;

   //borders
   szHborder.x = 0;
   if (pwin->sct_hborder->data != 0) {
      ftmpA = pwin->sct_hborder->data;
      ftmpB = (ftmpA*fltDCsize)/scale_base;
      szHborder.x = ftmpB;
      if (szHborder.x == 0) szHborder.x = 1;
   }
   szVborder.x = rcScreen.width;

   RCD_RETURN_OK(retU);
}

void dtd_screen_cl::scr_area_str(wxPaintDC& dc) {

   {  //active area size to string
      int  Xres, Yres, tmpi;

      tmpi   = pwin->sct_hborder->data;
      tmpi <<= 1; //exclude borders on both screen edges
      Xres   = pwin->sct_xres->data;
      Xres  -= tmpi;

      tmpi   = pwin->sct_vborder->data;
      tmpi <<= 1;
      Yres   = pwin->sct_vres->data;
      Yres  -= tmpi;

      tmps.Printf("%u x %u", Xres, Yres);
   }

   {  //set the DC font
      wxFont font;

      font = dc.GetFont();
      font.MakeBold();

      dc.SetFont(font);
   }

   {  //draw the string
      wxPoint str_pos;
      wxSize  str_sz;

      str_sz = dc.GetTextExtent(tmps);

      //skip if active area is too small
      if ((str_sz.x >= rcScreen.width ) ||
          (str_sz.y >= rcScreen.height) ) return;

      str_sz.x >>= 1;
      str_sz.y >>= 1;

      str_pos.x   = rcScreen.width;
      str_pos.x >>= 1;
      str_pos.x  += rcScreen.x;
      str_pos.x  -= str_sz.x;

      str_pos.y   = rcScreen.height;
      str_pos.y >>= 1;
      str_pos.y  += rcScreen.y;
      str_pos.y  -= str_sz.y;

      dc.SetTextForeground(cResStr);
      dc.DrawText(tmps, str_pos);
   }

}

#pragma GCC diagnostic ignored "-Wunused-parameter"
void dtd_screen_cl::evt_paint(wxPaintEvent& evt){
   rcode     retU;
   wxPaintDC dc(this);
   wxSize    dcsize = dc.GetSize();
   wxBrush   bgbrush(*wxMEDIUM_GREY_BRUSH);

   if (!IsEnabled()) {
      dc.SetBrush(bgbrush);
      dc.DrawRectangle(0, 0, dcsize.x, dcsize.y);
      return;
   }

   if (pwin->b_dtd_keep_aspect) {
      scr_aspect_ratio(dcsize, dc);
   }

   retU = calc_coords(dcsize);
   if (! RCD_IS_OK(retU)) {
      pwin->GLog.DoLog("dtd_screen:evt_paint():calc_coords() FAILED.");
      pwin->GLog.PrintRcode(retU);
      return;
   }

   //background
   dc.SetBrush(*wxBLACK);
   dc.DrawRectangle(0, 0, dcsize.x, dcsize.y);

   dc.SetPen(*wxTRANSPARENT_PEN);

   //vsync bar
   bgbrush.SetColour(cVsync);
   dc.SetBrush(bgbrush);
   dc.DrawRectangle(rcVsync);

   //hsync bar
   bgbrush.SetColour(cHsync);
   dc.SetBrush(bgbrush);
   dc.DrawRectangle(rcHsync);

   //sandcastle area (hsync + vsync)
   bgbrush.SetColour(cSandC);
   dc.SetBrush(bgbrush);
   dc.DrawRectangle(0, 0, rcHsync.width, rcVsync.height);

   //screen area
   bgbrush.SetColour(cScrArea);
   dc.SetBrush(bgbrush);
   dc.DrawRectangle(rcScreen);

   //print effective resolution on the active screen area.
   scr_area_str(dc);

   //h-border
   if (szHborder.x != 0) {
      bgbrush.SetColour(cSandC);
      dc.SetBrush(bgbrush);
      dc.DrawRectangle(rcScreen.x, rcScreen.y, szHborder.x, szHborder.y);
      dc.DrawRectangle((rcScreen.x + rcScreen.width) - szHborder.x,
                       rcScreen.y,
                       szHborder.x,
                       szHborder.y);
   }
   //v-border
   if (szVborder.y != 0) {
      bgbrush.SetColour(cSandC);
      dc.SetBrush(bgbrush);
      dc.DrawRectangle(rcScreen.x, rcScreen.y, szVborder.x, szVborder.y);
      dc.DrawRectangle(rcScreen.x,
                       (rcScreen.y + rcScreen.height) - szVborder.y,
                       szVborder.x,
                       szVborder.y);
   }
}
#pragma GCC diagnostic warning "-Wunused-parameter"

