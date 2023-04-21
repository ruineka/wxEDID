/***************************************************************
 * Name:      EDID_shared.h
 * Purpose:   EDID shared declarations
 * Author:    Tomasz Pawlak (tomasz.pawlak@wp.eu)
 * Created:   2021-03-14
 * Copyright: Tomasz Pawlak (C) 2021-2022
 * License:   GPLv3+
 **************************************************************/

#ifndef EDID_SHARED_H
#define EDID_SHARED_H 1

#include <wx/string.h>
#include <wx/treectrl.h>
#include <wx/menu.h>
#include <wx/dynarray.h>
#include <wx/arrimpl.cpp>

#include "EDID.h"
#include "CEA_EXT.h"
#include "CEA.h"
#include "def_types.h"

#include "rcode/rcode.h"

#include "guilog.h"

#define EDI_FIELD_SZ sizeof(edi_field_t)
enum {
   CEA_DBCHDR_FCNT = 2,
   CEA_EXTHDR_FCNT = 3
};

typedef unsigned int  uint;
typedef unsigned long ulong;

enum { //block and groups IDs & flags
   //EDID base:
   ID_BED          = 0x00000001,
   ID_VID          = 0x00000002,
   ID_BDD          = 0x00000003,
   ID_SPF          = 0x00000004,
   ID_CXY          = 0x00000005,
   ID_ETM          = 0x00000006,
   ID_STI          = 0x00000007,
   ID_DTD          = 0x00000008,
   ID_MRL          = 0x00000009,
   ID_MSN          = 0x0000000A,
   ID_MND          = 0x0000000B,
   ID_WPD          = 0x0000000C,
   ID_AST          = 0x0000000D,
   ID_UST          = 0x0000000E,
   ID_UNK          = 0x0000000F,
   ID_EDID_MASK    = 0x0000000F, //base EDID type mask
   //CEA-DBC:
   ID_CHD          = 0x00000010,
   ID_ADB          = 0x00000020,
   ID_SAD          = 0x00010000,
   ID_VDB          = 0x00000030,
   ID_SVD          = 0x00020000,
   ID_VSD          = 0x00000040,
   ID_SAB          = 0x00000050,
   ID_VDTC         = 0x00000060,
   ID_CEA_UTC      = 0x00000070, //unknown Tag Code
   ID_CEA_MASK     = 0x000000F0, //mask for Tag Code type
   //CEA-DBC-EXT:
   ID_VCDB         = 0x00000100,
   ID_VSVD         = 0x00000200,
   ID_VSAD         = 0x00000300,
   ID_VDDD         = 0x00000400,
   ID_VDDD_IPF     = 0x00030000,
   ID_VDDD_CPT     = 0x00040000,
   ID_VDDD_AUD     = 0x00050000,
   ID_VDDD_DPR     = 0x00060000,
   ID_VDDD_CXY     = 0x00070000,
   ID_RSV4         = 0x00000500,
   ID_CLDB         = 0x00000600,
   ID_HDRS         = 0x00000700,
   ID_HDRD         = 0x00000800,
   ID_HDRD_MTD     = 0x00080000,
   ID_VFPD         = 0x00000900,
   ID_VFPD_SVR     = 0x00080000,
   ID_Y42V         = 0x00000A00,
   ID_Y42C         = 0x00000B00,
   ID_RMCD         = 0x00000C00,
   ID_RMCD_HDR     = 0x00090000,
   ID_RMCD_SPM     = 0x000A0000,
   ID_RMCD_SPKD    = 0x000B0000,
   ID_RMCD_DPC     = 0x000C0000,
   ID_SLDB         = 0x00000D00,
   ID_SLDB_SLOCD   = 0x000D0000,
   ID_IFDB         = 0x00000E00,
   ID_IFDB_IFPD    = 0x000E0000,
   ID_IFDB_SIFD    = 0x000F0000,
   ID_IFDB_VSD     = 0x00100000,
   ID_CEA_UETC     = 0x00000F00, //unknown Extended Tag Code
   ID_CEA_UDAT     = 0x00FF0000, //unknown data sub-group
   ID_CEA_EXT_MASK = 0x0000FF00, //mask for Extended Tag type ID

   ID_SUBGRP_MASK  = 0x00FF0000, //mask for subgroups
   ID_PARENT_MASK  = (ID_EDID_MASK|ID_CEA_MASK|ID_CEA_EXT_MASK), //mask for parent groups
   //Block type:
   T_EDID_FIXED    = 0x80000000, //EDID fixed block: can't be moved or copied.
   T_DBC_FIXED     = 0x40000000, //DBC fixed group: can't be moved or copied within DBC data range.
   T_DBC_SUBGRP    = 0x20000000, //DBC sub-group
   T_MODE_EDIT     = 0x10000000, //skip some checks on <re>init()
   T_FLAG_MASK     = 0xF0000000,
   //Group init flags:
   TG_FLEX_LAYOUT  = 0x01000000, // use field data lengths
   TG_FLEX_LEN     = 0x02000000, // !TG_FLEX_LEN -> fail if group length is to small
   TG_CUSTOM_INIT  = 0x04000000, // Custom init function for the group
   TG_FLEX_OFFS    = 0x08000000, // update field's offsets
   TG_FLAG_MASK    = 0x0F000000
};

enum { //EDID field flags
   //byte0: property flags
   EF_PRCNT  = 7,
   EF_PRSHFT = 0,
   EF_PRMASK = 0x7F,
   EF_RD     = 0x00000001, //read-only
   EF_NI     = 0x00000002, //field value can't be provided as int (for write)
   EF_NU     = 0x00000004, //field is not used i.e. it is marked as unused in the EDID
   EF_FGR    = 0x00000008, //force group refresh after editing a field with this flag set
   EF_INIT   = 0x00000010, //group requires re-initialization on field change
   EF_GPD    = 0x00000020, //display group description for this field
   EF_VS     = 0x00000040, //use value selector / values map present
   //byte1: val type
   EF_TPCNT  = 8,
   EF_TPSHFT = 8,
   EF_TPMASK = 0xFF,
   EF_BIT    = 0x00000100, //flag field, single bit
   EF_BFLD   = 0x00000200, //bitfield
   EF_BYTE   = 0x00000400, //
   EF_INT    = 0x00000800, //uint
   EF_FLT    = 0x00001000, //float
   EF_HEX    = 0x00002000, //display as hex
   EF_STR    = 0x00004000, //text/byte string
   EF_LE     = 0x00008000, //Little Endian byte order : reverse the byte r/w order for strings
   //byte2: val unit
   EF_UNCNT  = 9,
   EF_UNSHFT = 16,
   EF_UNMASK = 0x1FF,
   EF_PIX    = 0x00010000, //pixels
   EF_MM     = 0x00020000, //
   EF_CM     = 0x00040000, //
   EF_DM     = 0x00080000, //decimeters
   EF_HZ     = 0x00100000, //
   EF_KHZ    = 0x00200000, //
   EF_MHZ    = 0x00400000, //
   EF_MLS    = 0x00800000, //milliseconds
   EF_PCT    = 0x01000000  //percent
};

enum { //handler operating modes
   OP_READ  = 0x01,
   OP_WRSTR = 0x02,
   OP_WRINT = 0x04
};

enum { //EDID blocks
   EDI_BLK_SIZE = 128,
   EDI_BASE_IDX = 0,
   EDI_EXT0_IDX,
   EDI_EXT1_IDX,
   EDI_EXT2_IDX
};

typedef u8_t ediblk_t[EDI_BLK_SIZE];

typedef struct __attribute__ ((packed)) { //edi_s
   edid_t   base;
   ediblk_t ext0;
   ediblk_t ext1;
   ediblk_t ext2;
} edi_t;

typedef union __attribute__ ((packed)) { //edi_buff_u
   u8_t     buff[4* sizeof(edi_t)];
   ediblk_t blk[4];
   edi_t    edi;
} edi_buf_t;

//value map : single entry
typedef struct { // vname_map_t
   const u32_t  val;
   const char  *name; //NULL -> reserved value, not included in value selector menu
} vname_map_t;

typedef struct { // vmap_t
   const u32_t        resvd; //reserved 0
   const u32_t        nval;
   const vname_map_t *vmap;
} vmap_t;

class  EDID_cl;
struct edi_dynfld_s;
//field handler fn ptr
typedef rcode (EDID_cl::*field_fn)(u32_t, wxString&, u32_t&, edi_dynfld_s*);

typedef struct edi_field_s {
         field_fn  handlerfn;
   const vmap_t   *vmap;
         u32_t     offs;    //offset in data struct - generic/multi-instance functions
         u32_t     shift;   //offset in bits
         u32_t     fldsize; //len in bytes/bits -> depends on flags
         u32_t     flags;
         u32_t     minv;
         u32_t     maxv;
   const char     *name;
   const char     *desc;
} edi_field_t;

typedef struct __attribute__ ((packed)) edi_dynfld_s {
   edi_field_t  field;
   u8_t        *base;     //data pointer (instance) - generic handlers
   wxMenu      *selector;
} edi_dynfld_t;

class edi_grp_cl;
typedef edi_grp_cl* (*psub_ctor)();

//Field group descriptor
typedef struct gpfld_dsc_s {
         u32_t        flags;
         u32_t        fcount;
         u32_t        dat_sz;
         i32_t        inst_cnt; //instance count, (-1) == until data end
   const edi_field_t *fields;
} gpfld_dsc_t;
//Base group descriptor: no sub-groups
typedef struct gpflat_dsc_s {
   const char         *CodN;
   const char         *Name;
   const char         *Desc;
         u32_t         type_id;
         u32_t         flags;
         u32_t         min_len;
         u32_t         max_len;
         u32_t         max_fld;
         u32_t         hdr_fcnt; //CEA_DBCHDR_FCNT | CEA_EXTHDR_FCNT
         u32_t         hdr_sz;   //sizeof(bhdr_t) | //sizeof(ethdr_t)
         u32_t         fld_arsz;
   const gpfld_dsc_t  *fld_ar;
} gpflat_dsc_t;
//Field group descriptor
typedef struct subgrp_dsc_s {
   psub_ctor          s_ctor;
   const char        *CodN;
   const char        *Name;
   const char        *Desc;
         u32_t        type_id;
         u32_t        min_len;
         u32_t        fcount;
         i32_t        inst_cnt; //instance count, (-1) == until data end
   const edi_field_t *fields;
} subgrp_dsc_t;
//Base group descriptor: sub-groups
typedef struct grptree_dsc_s {
   const char         *CodN;
   const char         *Name;
   const char         *Desc;
         u32_t         type_id;
         u32_t         flags;
         u32_t         min_len;
         u32_t         max_len;
         u32_t         hdr_fcnt; //CEA_DBCHDR_FCNT | CEA_EXTHDR_FCNT
         u32_t         hdr_sz;   //sizeof(bhdr_t) | sizeof(ethdr_t)
         u32_t         grp_arsz;
   const subgrp_dsc_t *grp_ar;
} gproot_dsc_t;

WX_DECLARE_OBJARRAY(edi_dynfld_t*, wxArrGrpField);

#include "grpar.h"

class edi_grp_cl : public wxTreeItemData {
   protected:

      u8_t    inst_data[32]; //local copy of instance data, including sub-groups
      u32_t   dat_sz;        //total instance data size
      u32_t   hdr_sz;        //hdr data size in inst_data: SpawnInstance() uses this when number of sub-groups is variable
      u32_t   subg_sz;       //subgroup init: free space left for subgroup
      u32_t   type_id;
      u32_t   abs_offs;
      u32_t   rel_offs;

      u32_t       grp_idx;    //index within the array
      GroupAr_cl *grp_ar;     //array containing *this* group
      edi_grp_cl *parent_grp; //NULL if not a sub-group

      //dynamic field array, depends on data layout
      u32_t        dyn_fcnt;
      edi_field_t *dyn_fldar;

      rcode       init_fields(const edi_field_t* field_ar, const u8_t* inst, u32_t fcount, u32_t orflags,
                              const char *pname = NULL, const char *pdesc = NULL, const char *pcodn = NULL);
      rcode       create_selector(edi_dynfld_t *pfld);
      void        clear_fields();
      edi_grp_cl* base_clone(rcode& rcd, edi_grp_cl* grp, u8_t* inst, u32_t orflags);

      rcode       base_DBC_Init_FlatGrp(const u8_t* inst, const gpflat_dsc_t *pgdsc, u32_t orflags, edi_grp_cl* parent);
      rcode       base_DBC_Init_GrpFields(const gpfld_dsc_t *pgfld, edi_field_t **pp_fld, u32_t *pdlen, u32_t *poffs);

      rcode       base_DBC_Init_RootGrp(const u8_t* inst, const gproot_dsc_t *pGDsc, u32_t orflags, edi_grp_cl* parent);

                  //IFDB: InfoFrame Data Block: common init fn for all sub-groups
      rcode       IFDB_Init_SubGrp(const u8_t* inst, const subgrp_dsc_t* pSGDsc, u32_t orflags, edi_grp_cl* parent);

   public:
      wxArrGrpField  FieldsAr;

      wxString       CodeName;
      wxString       GroupName;
      wxString       GroupDesc;

      inline  void   CopyInstData (const u8_t *pinst, u32_t datsz);
      virtual void   SpawnInstance(u8_t *pinst); //copy local data back to EDID buffer
              rcode  AssembleGroup(); //copy sub-group data to parent' group local data buffer

      inline  u32_t  getTypeID    () {return type_id;};
      inline  u32_t  getAbsOffs   () {return abs_offs;};
      inline  u32_t  getRelOffs   () {return rel_offs;};
      inline  u32_t  getDataSize  () {return (hdr_sz == 0) ? dat_sz : hdr_sz ;};
      inline  u32_t  getHeaderSize() {return hdr_sz;};
      inline  u32_t  getTotalSize () {return dat_sz;};
      inline  u8_t*  getInsPtr    () {return inst_data;};
      inline  u32_t  getFreeSubgSZ() {return subg_sz;};

      inline  GroupAr_cl* getParentAr   () {return grp_ar ;};
      inline  u32_t       getParentArIdx() {return grp_idx;};

      inline  void   setAbsOffs (u32_t offs) {abs_offs = offs;};
      inline  void   setRelOffs (u32_t offs) {rel_offs = offs;};
      inline  void   setIndex   (u32_t idx ) {grp_idx  = idx;};
      inline  void   setArray   (GroupAr_cl *p_ar) {grp_ar = p_ar;};
      inline  void   setArrayIdx(GroupAr_cl *p_ar, u32_t idx) {grp_ar = p_ar; grp_idx = idx;};
      virtual void   setParentAr(GroupAr_cl *) {return;};
      virtual void   setDataSize(u32_t dsz) {dat_sz = dsz;};

      virtual rcode  init              (const u8_t*, u32_t, edi_grp_cl*) =0;
      virtual rcode  ForcedGroupRefresh() {rcode rcd; rcd.value=0; return rcd;};
      virtual u32_t  getSubGrpCount    () {return 0;};

      inline  void        setParentGrp(edi_grp_cl *parent) {parent_grp = parent;};
      inline  edi_grp_cl* getParentGrp() {return parent_grp;};
      virtual edi_grp_cl* getSubGroup (u32_t ) {return NULL;};
      virtual GroupAr_cl* getSubArray () {return NULL;};

      virtual rcode       Append_UNK_DAT(const u8_t*, u32_t, u32_t, u32_t, u32_t, edi_grp_cl*)
                                        {rcode rcd; rcd.value=0; return rcd;};

      virtual edi_grp_cl* Clone(rcode&, u32_t) {return NULL;};

      edi_grp_cl() : dat_sz(0), hdr_sz(0), subg_sz(0), type_id(0), abs_offs(0), rel_offs(0),
                     grp_idx(0), grp_ar(NULL), parent_grp(NULL), dyn_fcnt(0), dyn_fldar(NULL)
                   { memset(inst_data, 0, 32);};

      ~edi_grp_cl() {
         if (dyn_fldar != NULL) { free(dyn_fldar); }
         clear_fields();
         FieldsAr.Clear();
      };
};

//copy EDID buffer data to a local buffer
void edi_grp_cl::CopyInstData(const u8_t *pinst, u32_t datsz) {
   memcpy(inst_data, pinst, datsz);
   dat_sz = datsz;
}

class dbc_grp_cl : public edi_grp_cl {
   protected:
      DBC_GrpAr_cl  subgroups;

   public:
      void setDataSize(u32_t blklen) {
         bhdr_t *phdr;

         phdr = reinterpret_cast<bhdr_t*> (inst_data);
         phdr->tag.blk_len = blklen;

         dat_sz  = hdr_sz;
         dat_sz += blklen;
      };

      rcode Append_UNK_DAT(const u8_t* inst, u32_t dlen, u32_t orflags, u32_t abs_offs, u32_t rel_offs, edi_grp_cl* parent_grp);

      inline void setParentAr(GroupAr_cl *parent_ar) {
         subgroups.setParentArray(parent_ar);
      };

      inline DBC_GrpAr_cl* getSubArray   ()          {return &subgroups;};

      inline edi_grp_cl*   getSubGroup   (u32_t idx) {return subgroups.Item(idx);};
      inline u32_t         getSubGrpCount()          {return subgroups.GetCount();};
};

class EDID_cl {
   private:
      edi_buf_t    EDID_buff;

      guilog_cl   *pGLog;

      u32_t        num_valid_blocks;
      bool         b_RD_Ignore;
      bool         b_ERR_Ignore;
      wxString     tmps;

   protected:
      static const wxString val_unit_name [];
      static const wxString val_type_name [];
      static const wxString prop_flag_name[];

      edi_grp_cl*  ParseDetDtor(u32_t blkidx, dsctor_u* pdsc, rcode& retU);

      //Common handlers: helpers
      inline u32_t calcGroupOffs (void *inst) {return (reinterpret_cast <u8_t*> (inst) - EDID_buff.buff);};
      inline u8_t* getInstancePtr(edi_dynfld_t* p_field) {return p_field->base;};
      inline u8_t* getValPtr     (edi_dynfld_t* p_field);

      rcode getStrUint  (wxString& sval, int base, u32_t minv, u32_t maxv, ulong& val);
      rcode getStrDouble(wxString& sval, const double minv, const double maxv, double& val);
      rcode rdByteStr   (wxString& sval, u8_t* pstrb, u32_t slen);
      rcode rdByteStrLE (wxString& sval, u8_t* pstrb, u32_t slen);
      rcode wrByteStr   (wxString& sval, u8_t* pstrb, u32_t slen);
      rcode wrByteStrLE (wxString& sval, u8_t* pstrb, u32_t slen);

   public:
      GroupAr_cl*    BlkGroupsAr[4];
      EDID_GrpAr_cl  EDI_BaseGrpAr;
      CEA_GrpAr_cl   EDI_Ext0GrpAr;
      GroupAr_cl     EDI_Ext1GrpAr;
      GroupAr_cl     EDI_Ext2GrpAr;

      inline  edi_buf_t* getEDID() {return &EDID_buff;};
      inline  void       SetGuiLogPtr(guilog_cl *p_glog) {pGLog = p_glog;};
      inline  void       Set_ERR_Ignore(bool errign) {b_ERR_Ignore = errign;};
      inline  bool       Get_ERR_Ignore() {return b_ERR_Ignore;};
      inline  void       Set_RD_Ignore(bool rd) {b_RD_Ignore = rd;};
      inline  bool       Get_RD_Ignore() {return b_RD_Ignore;};
      inline  u32_t      getNumValidBlocks() {return num_valid_blocks;};
      inline  void       ForceNumValidBlocks(u32_t nblk) {num_valid_blocks = (nblk%5);}; //max 4
      inline  void       CEA_Set_DTD_Offset(u8_t *pbuf, GroupAr_cl *p_grp_ar);

      u32_t genChksum(u32_t block);
      bool  VerifyChksum(u32_t block);
      void  Clear();

      rcode ParseEDID_Base(u32_t& n_extblk);
      rcode ParseEDID_CEA();
      rcode ParseCEA_DBC(u8_t *pinst);
      rcode ParseDBC_TAG(u8_t *pinst, edi_grp_cl** pp_grp);
      rcode AssembleEDID();

      //field flags
      rcode getValUnitName     (wxString& sval, const u32_t flags);
      rcode getValTypeName     (wxString& sval, const u32_t flags);
      rcode getValFlagsAsString(wxString& sval, const u32_t flags);
      void  getVDesc           (wxString &sval, edi_dynfld_t* p_field, u32_t idx);

      //Common handlers
      rcode FldPadString(u32_t op, wxString& sval, u32_t& ival, edi_dynfld_t* p_field);
      rcode ByteStr     (u32_t op, wxString& sval, u32_t& ival, edi_dynfld_t* p_field);
      rcode ByteVal     (u32_t op, wxString& sval, u32_t& ival, edi_dynfld_t* p_field);
      rcode BitF8Val    (u32_t op, wxString& sval, u32_t& ival, edi_dynfld_t* p_field);
      rcode BitVal      (u32_t op, wxString& sval, u32_t& ival, edi_dynfld_t* p_field);
      rcode Gamma       (u32_t op, wxString& sval, u32_t& ival, edi_dynfld_t* p_field);
      //BED handlers
      rcode MfcId (u32_t op, wxString& sval, u32_t& ival, edi_dynfld_t* p_field);
      rcode ProdSN(u32_t op, wxString& sval, u32_t& ival, edi_dynfld_t* p_field);
      rcode ProdWk(u32_t op, wxString& sval, u32_t& ival, edi_dynfld_t* p_field);
      rcode ProdYr(u32_t op, wxString& sval, u32_t& ival, edi_dynfld_t* p_field);
      //input type : no dedicated handlers
      //basic display descriptor (old) : no dedicated handlers
      //Supported features : no dedicated handlers
      //Chromacity coords
      rcode ChrXY_getWriteVal(u32_t op, wxString& sval, u32_t& ival);
      rcode CHredX    (u32_t op, wxString& sval, u32_t& ival, edi_dynfld_t* p_field);
      rcode CHredY    (u32_t op, wxString& sval, u32_t& ival, edi_dynfld_t* p_field);
      rcode CHgrnX    (u32_t op, wxString& sval, u32_t& ival, edi_dynfld_t* p_field);
      rcode CHgrnY    (u32_t op, wxString& sval, u32_t& ival, edi_dynfld_t* p_field);
      rcode CHbluX    (u32_t op, wxString& sval, u32_t& ival, edi_dynfld_t* p_field);
      rcode CHbluY    (u32_t op, wxString& sval, u32_t& ival, edi_dynfld_t* p_field);
      rcode CHwhtX    (u32_t op, wxString& sval, u32_t& ival, edi_dynfld_t* p_field);
      rcode CHwhtY    (u32_t op, wxString& sval, u32_t& ival, edi_dynfld_t* p_field);
      //Resolution map : no dedicated handlers
      //std timing descriptors
      rcode StdTxres8 (u32_t op, wxString& sval, u32_t& ival, edi_dynfld_t* p_field);
      rcode StdTvsync (u32_t op, wxString& sval, u32_t& ival, edi_dynfld_t* p_field);
      //DTD: Detailed Timing Descriptor
      rcode DTD_PixClk(u32_t op, wxString& sval, u32_t& ival, edi_dynfld_t* p_field);
      rcode DTD_HApix (u32_t op, wxString& sval, u32_t& ival, edi_dynfld_t* p_field);
      rcode DTD_HBpix (u32_t op, wxString& sval, u32_t& ival, edi_dynfld_t* p_field);
      rcode DTD_VAlin (u32_t op, wxString& sval, u32_t& ival, edi_dynfld_t* p_field);
      rcode DTD_VBlin (u32_t op, wxString& sval, u32_t& ival, edi_dynfld_t* p_field);
      rcode DTD_HOsync(u32_t op, wxString& sval, u32_t& ival, edi_dynfld_t* p_field);
      rcode DTD_VOsync(u32_t op, wxString& sval, u32_t& ival, edi_dynfld_t* p_field);
      rcode DTD_VsyncW(u32_t op, wxString& sval, u32_t& ival, edi_dynfld_t* p_field);
      rcode DTD_HsyncW(u32_t op, wxString& sval, u32_t& ival, edi_dynfld_t* p_field);
      rcode DTD_Hsize (u32_t op, wxString& sval, u32_t& ival, edi_dynfld_t* p_field);
      rcode DTD_Vsize (u32_t op, wxString& sval, u32_t& ival, edi_dynfld_t* p_field);
      //MRL handlers
      rcode MRL_Hdr      (u32_t op, wxString& sval, u32_t& ival, edi_dynfld_t* p_field);
      rcode MRL_GTFM     (u32_t op, wxString& sval, u32_t& ival, edi_dynfld_t* p_field);
      rcode MRL_MaxPixClk(u32_t op, wxString& sval, u32_t& ival, edi_dynfld_t* p_field);
      //WPD handlers
      rcode WPD_pad      (u32_t op, wxString& sval, u32_t& ival, edi_dynfld_t* p_field);
      //CEA:DBC Block Header
      rcode CEA_DBC_Len  (u32_t op, wxString& sval, u32_t& ival, edi_dynfld_t* p_field);
      rcode CEA_DBC_Tag  (u32_t op, wxString& sval, u32_t& ival, edi_dynfld_t* p_field);
      rcode CEA_DBC_ExTag(u32_t op, wxString& sval, u32_t& ival, edi_dynfld_t* p_field);
      //CEA:VDB
      u32_t CEA_VDB_SVD_decode(u32_t vic, u32_t &native);
      //CEA:ADB:SAD
      rcode SAD_LPCM_MC  (u32_t op, wxString& sval, u32_t& ival, edi_dynfld_t* p_field);
      rcode SAD_BitRate  (u32_t op, wxString& sval, u32_t& ival, edi_dynfld_t* p_field);
      //CEA:VSD
      rcode VSD_ltncy    (u32_t op, wxString& sval, u32_t& ival, edi_dynfld_t* p_field);
      rcode VSD_MaxTMDS  (u32_t op, wxString& sval, u32_t& ival, edi_dynfld_t* p_field);
      //CEA-Ext: VDDD
      rcode VDDD_IF_MaxF   (u32_t op, wxString& sval, u32_t& ival, edi_dynfld_t* p_field);
      rcode VDDD_HVpix_cnt (u32_t op, wxString& sval, u32_t& ival, edi_dynfld_t* p_field);
      rcode VDDD_AspRatio  (u32_t op, wxString& sval, u32_t& ival, edi_dynfld_t* p_field);
      rcode VDDD_HVpx_pitch(u32_t op, wxString& sval, u32_t& ival, edi_dynfld_t* p_field);
      rcode VDDD_AudioDelay(u32_t op, wxString& sval, u32_t& ival, edi_dynfld_t* p_field);
      //CEA-Ext: VFDB
      u32_t CEA_VFPD_SVR_decode(u32_t svr, u32_t &ndtd);
      //CEA-Ext: HDRD
      rcode HDRD_mtd_type  (u32_t op, wxString& sval, u32_t& ival, edi_dynfld_t* p_field);


      EDID_cl() : num_valid_blocks(0), b_RD_Ignore(false), b_ERR_Ignore(false) {
         BlkGroupsAr[0] = &EDI_BaseGrpAr;
         BlkGroupsAr[1] = &EDI_Ext0GrpAr;
         BlkGroupsAr[2] = &EDI_Ext1GrpAr;
         BlkGroupsAr[3] = &EDI_Ext2GrpAr;
      };

      ~EDID_cl() {
         EDI_BaseGrpAr.Clear();
         EDI_Ext0GrpAr.Clear();
         EDI_Ext1GrpAr.Clear();
         EDI_Ext2GrpAr.Clear();
      };
};

u8_t* EDID_cl::getValPtr(edi_dynfld_t* p_field) {
   u8_t* ptr;

   ptr  = getInstancePtr(p_field);
   ptr += p_field->field.offs;
   return ptr;
}


//UNK-DAT Unknown Data bytes, subgroup.
class cea_unkdat_cl : public edi_grp_cl {
   private:
      static const char  CodN[];
      static const char  Name[];
      static const char  Desc[];

   public:
      rcode       init(const u8_t* inst, u32_t orflags, edi_grp_cl* parent);
      edi_grp_cl* Clone(rcode& rcd, u32_t flags) {return base_clone(rcd, new cea_unkdat_cl(), inst_data, flags); };
};

#endif /* EDID_SHARED_H */
