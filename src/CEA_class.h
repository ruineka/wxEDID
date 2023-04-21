/***************************************************************
 * Name:      CEA_class.h
 * Purpose:   EDID::CEA-DBC descriptor classes
 * Author:    Tomasz Pawlak (tomasz.pawlak@wp.eu)
 * Created:   2021-03-14
 * Copyright: Tomasz Pawlak (C) 2021-2022
 * License:   GPLv3+
 **************************************************************/

#ifndef CEA_CLASS_H
#define CEA_CLASS_H 1

#include "EDID_shared.h"

//----------------- CEA/CTA-861 extension

//CHD: CEA Header
class cea_hdr_cl : public edi_grp_cl {
   private:
      static const edi_field_t fields[];

      static const char  CodN[];
      static const char  Name[];
      static const char  Desc[];

   public:
      rcode       init(const u8_t* inst, u32_t orflags, edi_grp_cl* parent);
      void        SpawnInstance(u8_t *pinst);
};
//ADB: Audio Data Block
class cea_adb_cl : public dbc_grp_cl {
   private:
      static const char          Desc[];
      static const gproot_dsc_t ADB_grp;

   public:
      rcode       init(const u8_t* inst, u32_t orflags, edi_grp_cl* parent);
      edi_grp_cl* Clone(rcode& rcd, u32_t flags) {return base_clone(rcd, new cea_adb_cl(), inst_data, flags); };
};
//ADB: Audio Data Block ->
//SAD: Short Audio Descriptor
class cea_sad_cl : public edi_grp_cl {
   private:
      enum {
         //SAD byte 0: AFC = 1...14
         byte0_afc1_14_fcnt          = 3,
         byte0_afc15_ace11_12_fcnt   = 3,
         byte0_afc15_ace13_fcnt      = 2,
         //SAD byte 1: AFC = 1...14 || AFC = 15 && ACE = 11 (MPEG-H 3D Audio)
         byte1_afc1_14_ace11_fcnt    = 8,
         byte1_afc15_ace456810_fcnt  = 8,
         byte1_afc15_ace12_fsmp_fcnt = 8,
         byte1_afc15_ace13_fcnt      = 8,
         //SAD byte 2: AFC = 1: LPCM
         byte2_afc1_fcnt             = 4,
         byte2_afc2_8_fcnt           = 1,
         byte2_afc9_13_fcnt          = 1,
         byte2_afc14_fcnt            = 2,
         byte2_afc15_ace456_fcnt     = 4,
         byte2_afc15_ace8_10_fcnt    = 4,
         byte2_afc15_ace11_12_fcnt   = 2,
         byte2_afc15_ace13_fcnt      = 4,
      };

      //block data layouts
      static const edi_field_t byte0_afc1_14[];
      static const edi_field_t byte0_afc15_ace11_12[];
      static const edi_field_t byte0_afc15_ace13[];

      static const edi_field_t byte1_afc1_14_ace11[];
      static const edi_field_t byte1_afc15_ace456810[];
      static const edi_field_t byte1_afc15_ace12_fsmp[];
      static const edi_field_t byte1_afc15_ace13[];

      static const edi_field_t byte2_afc1[];
      static const edi_field_t byte2_afc2_8[];
      static const edi_field_t byte2_afc9_13[];
      static const edi_field_t byte2_afc14[];
      static const edi_field_t byte2_afc15_ace456[];
      static const edi_field_t byte2_afc15_ace8_10[];
      static const edi_field_t byte2_afc15_ace11_12[];
      static const edi_field_t byte2_afc15_ace13[];

      //SAD unknown/invalid byte:
      static const edi_field_t byte_unk[];

      static const char        Desc[];

      //SAD needs custom constructor for field array
      rcode  gen_data_layout(const u8_t* inst);

   public:
      static const subgrp_dsc_t SAD_subg;

      rcode       init(const u8_t* inst, u32_t orflags, edi_grp_cl* parent);
      rcode       ForcedGroupRefresh();
      edi_grp_cl* Clone(rcode& rcd, u32_t flags) {return base_clone(rcd, new cea_sad_cl(), inst_data, flags); };

      static edi_grp_cl* group_new() {return new cea_sad_cl();};
};
//VDB: Video Data Block
class cea_vdb_cl : public dbc_grp_cl {
   private:
      static const gproot_dsc_t VDB_grp;

      static const char  CodN[];
      static const char  Name[];
      static const char  Desc[];

   public:
      rcode       init(const u8_t* inst, u32_t orflags, edi_grp_cl* parent);
      edi_grp_cl* Clone(rcode& rcd, u32_t flags) {return base_clone(rcd, new cea_vdb_cl(), inst_data, flags); };
};
//VDB: Video Data Block ->
//SVD: Short Video Descriptor
class cea_svd_cl : public edi_grp_cl {
   private:
      static const char         Desc   [];
      static const edi_field_t  fld_dsc[];

   public:
      static const subgrp_dsc_t SVD_subg;

      rcode       init(const u8_t* inst, u32_t orflags, edi_grp_cl* parent);
      edi_grp_cl* Clone(rcode& rcd, u32_t flags) {return base_clone(rcd, new cea_svd_cl(), inst_data, flags); };

      static edi_grp_cl* group_new() {return new cea_svd_cl();};
};
//VSD: Vendor Specific Data Block
class cea_vsd_cl : public edi_grp_cl {
   private:
      static const char          Desc[];
      static const edi_field_t   hdr_fld_dsc[];
      static const edi_field_t   sink_feat_fld_dsc[];
      static const edi_field_t   max_tmds_fld_dsc[];
      static const edi_field_t   latency_fld_dsc[];
      static const edi_field_t   av_latency_fld_dsc[];
      static const gpfld_dsc_t   sub_fld_grp[];
      static const gpflat_dsc_t  VSD_grp;

   public:
      rcode       init(const u8_t* inst, u32_t orflags, edi_grp_cl* parent);
      edi_grp_cl* Clone(rcode& rcd, u32_t flags) {return base_clone(rcd, new cea_vsd_cl(), inst_data, flags); };
};
//SAB: Speaker Allocation Data Block
class cea_sab_cl : public edi_grp_cl {
   private:
      static const gpflat_dsc_t SAB_grp;

   public:
      rcode       init(const u8_t* inst, u32_t orflags, edi_grp_cl* parent);
      edi_grp_cl* Clone(rcode& rcd, u32_t flags) {return base_clone(rcd, new cea_sab_cl(), inst_data, flags); };
};
//VDTC: VESA Display Transfer Characteristic Data Block (gamma)
class cea_vdtc_cl : public edi_grp_cl {
   private:
      static const gpfld_dsc_t fields[];

      static const char         Desc[];
      static const edi_field_t  fld_dsc[];
      static const gpflat_dsc_t VDTC_grp;

   public:
      rcode       init(const u8_t* inst, u32_t orflags, edi_grp_cl* parent);
      edi_grp_cl* Clone(rcode& rcd, u32_t flags) {return base_clone(rcd, new cea_vdtc_cl(), inst_data, flags); };
};
//UNK-TC: Unknown Data Block (Tag Code)
class cea_unktc_cl : public edi_grp_cl {
   private:
      static const gpflat_dsc_t UNK_TC_grp;

   public:
      rcode       init(const u8_t* inst, u32_t orflags, edi_grp_cl* parent);
      edi_grp_cl* Clone(rcode& rcd, u32_t flags) {return base_clone(rcd, new cea_unktc_cl(), inst_data, flags); };
};

#endif /* CEA_CLASS_H */
