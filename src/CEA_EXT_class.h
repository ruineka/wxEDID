/***************************************************************
 * Name:      CEA_EXT_class.h
 * Purpose:   EDID::CEA-DBC-EXT descriptor classes
 * Author:    Tomasz Pawlak (tomasz.pawlak@wp.eu)
 * Created:   2021-03-14
 * Copyright: Tomasz Pawlak (C) 2021-2022
 * License:   GPLv3+
 **************************************************************/

#ifndef CEA_EXT_CLASS_H
#define CEA_EXT_CLASS_H 1

#include "EDID_shared.h"

//----------------- DBC Extended Tag Codes

//VCDB: Video Capability Data Block (DBC_EXT_VCDB = 0)
class cea_vcdb_cl : public edi_grp_cl {
   private:
      static const char         Desc[];
      static const edi_field_t  fld_dsc[];
      static const gpfld_dsc_t  fields;
      static const gpflat_dsc_t VCDB_grp;

   public:
      rcode       init(const u8_t* inst, u32_t orflags, edi_grp_cl* parent);
      edi_grp_cl* Clone(rcode& rcd, u32_t flags) {return base_clone(rcd, new cea_vcdb_cl(), inst_data, flags); };
};
//VSVD: Vendor-Specific Video Data Block (DBC_EXT_VSVD = 1)
class cea_vsvd_cl : public edi_grp_cl {
   private:
      static const char         Desc[];
      //fields shared with VSAD
      static const gpflat_dsc_t VSVD_grp;

   public:
      rcode       init(const u8_t* inst, u32_t orflags, edi_grp_cl* parent);
      edi_grp_cl* Clone(rcode& rcd, u32_t flags) {return base_clone(rcd, new cea_vsvd_cl(), inst_data, flags); };
};

//VDDD: VESA Display Device Data Block (DBC_EXT_VDDD = 2)
class cea_vddd_cl : public dbc_grp_cl {
   private:
      //subgroups;

      static const char  CodN[];
      static const char  Name[];
      static const char  Desc[];

   public:
      rcode       init(const u8_t* inst, u32_t orflags, edi_grp_cl* parent);
      edi_grp_cl* Clone(rcode& rcd, u32_t flags) {return base_clone(rcd, new cea_vddd_cl(), inst_data, flags); };
};
//VDDD: IFP: Interface
class vddd_iface_cl : public edi_grp_cl {
   private:
      static const edi_field_t fields[];

   protected:
      static const char  CodN[];
      static const char  Name[];
      static const char  Desc[];

   public:
      rcode  init(const u8_t* inst, u32_t orflags, edi_grp_cl* parent);
};
//VDDD: CPT: Content Protection
class vddd_cprot_cl : public edi_grp_cl {
   private:
      static const edi_field_t fields[];

   protected:
      static const char  CodN[];
      static const char  Name[];
      static const char  Desc[];

   public:
      rcode  init(const u8_t* inst, u32_t orflags, edi_grp_cl* parent);
};
//VDDD: AUPR: Audio properties
class vddd_audio_cl : public edi_grp_cl {
   private:
      static const edi_field_t fields[];

   protected:
      static const char  CodN[];
      static const char  Name[];
      static const char  Desc[];

   public:
      rcode  init(const u8_t* inst, u32_t orflags, edi_grp_cl* parent);
};
//VDDD: DPR: Display properties
class vddd_disp_cl : public edi_grp_cl {
   private:
      static const edi_field_t fields[];

   protected:
      static const char  CodN[];
      static const char  Name[];
      static const char  Desc[];

   public:
      rcode  init(const u8_t* inst, u32_t orflags, edi_grp_cl* parent);
};
//VDDD: CXY: Additional Chromaticity Coordinates
class vddd_cxy_cl : public edi_grp_cl {
   private:
      static const edi_field_t fields[];

   protected:
      static const char  CodN[];
      static const char  Name[];
      static const char  Desc[];

   public:
      rcode  init(const u8_t* inst, u32_t orflags, edi_grp_cl* parent);
};

//VESA Video Timing Block Extension (DBC_EXT_VVTB = 3) BUG!
//     UNK-ET is used for ExtTagCode == 3

//CLDB: Colorimetry Data Block (DBC_EXT_CLDB = 5)
class cea_cldb_cl : public edi_grp_cl {
   private:
      static const char         Desc[];
      static const edi_field_t  fld_dsc[];
      static const gpfld_dsc_t  fields;
      static const gpflat_dsc_t CLDB_grp;

   public:
      rcode       init(const u8_t* inst, u32_t orflags, edi_grp_cl* parent);
      edi_grp_cl* Clone(rcode& rcd, u32_t flags) {return base_clone(rcd, new cea_cldb_cl(), inst_data, flags); };
};
//HDRS: HDR Static Metadata Data Block (DBC_EXT_HDRS = 6)
class cea_hdrs_cl : public edi_grp_cl {
   private:
      static const char         Desc[];
      static const edi_field_t  fld_byte_2_3_dsc[];
      static const edi_field_t  fld_opt_byte_4_5_6_dsc[];
      static const gpfld_dsc_t  fields[];
      static const gpflat_dsc_t HDRS_grp;

   public:
      rcode       init(const u8_t* inst, u32_t orflags, edi_grp_cl* parent);
      edi_grp_cl* Clone(rcode& rcd, u32_t flags) {return base_clone(rcd, new cea_hdrs_cl(), inst_data, flags); };
};

//HDRD: HDR Dynamic Metadata Data Block (DBC_EXT_HDRD = 7)
class cea_hdrd_cl : public dbc_grp_cl {
   private:
      static const char         Desc[];
      static const gproot_dsc_t HDRD_grp;

   public:
      rcode       init(const u8_t* inst, u32_t orflags, edi_grp_cl* parent);
      edi_grp_cl* Clone(rcode& rcd, u32_t flags) {return base_clone(rcd, new cea_hdrd_cl(), inst_data, flags); };
};
//HDRD: HDR Dynamic Metadata sub-group
class hdrd_mtd_cl : public edi_grp_cl {
   private:
      static const char         Desc   [];
      static const edi_field_t  fld_dsc[];

   public:
      static const subgrp_dsc_t MTD_hdr;

      rcode  init(const u8_t* inst, u32_t orflags, edi_grp_cl* parent);

      static edi_grp_cl* group_new() {return new hdrd_mtd_cl();};
};

//VFPD: Video Format Preference Data Block (DBC_EXT_VFPD = 13)
class cea_vfpd_cl : public dbc_grp_cl {
   private:
      static const gproot_dsc_t VFPD_grp;

      static const char  CodN[];
      static const char  Name[];
      static const char  Desc[];

   public:
      rcode       init(const u8_t* inst, u32_t orflags, edi_grp_cl* parent);
      edi_grp_cl* Clone(rcode& rcd, u32_t flags) {return base_clone(rcd, new cea_vfpd_cl(), inst_data, flags); };
};
//VFPD: Video Format Preference Data Block ->
//SVR: Short Video Reference
class cea_svr_cl : public edi_grp_cl {
   private:
      static const char         Desc   [];
      static const edi_field_t  fld_dsc[];

   public:
      static const subgrp_dsc_t SVR_subg;

      rcode       init(const u8_t* inst, u32_t orflags, edi_grp_cl* parent);
      edi_grp_cl* Clone(rcode& rcd, u32_t flags) {return base_clone(rcd, new cea_svr_cl(), inst_data, flags); };

      static edi_grp_cl* group_new() {return new cea_svr_cl();};
};
//Y42V: YCBCR 4:2:0 Video Data Block (DBC_EXT_Y42V = 14)
class cea_y42v_cl : public dbc_grp_cl {
   private:
      static const char         Desc[];
      static const gproot_dsc_t Y42V_grp;

   public:
      rcode       init(const u8_t* inst, u32_t orflags, edi_grp_cl* parent);
      edi_grp_cl* Clone(rcode& rcd, u32_t flags) {return base_clone(rcd, new cea_y42v_cl(), inst_data, flags); };
};
//Y42C: YCBCR 4:2:0 Capability Map Data Block (DBC_EXT_Y42C = 15)
class cea_y42c_cl : public edi_grp_cl {
   private:
      static const edi_field_t bitmap_fld;

      static const char  CodN[];
      static const char  Name[];
      static const char  Desc[];

      //array of strings with SVD nr
      y42c_svdn_t *svdn_ar;

   public:
      rcode       init(const u8_t* inst, u32_t orflags, edi_grp_cl* parent);
      edi_grp_cl* Clone(rcode& rcd, u32_t flags) {return base_clone(rcd, new cea_y42c_cl(), inst_data, flags); };

       cea_y42c_cl() : svdn_ar(NULL) {};
      ~cea_y42c_cl() { if (svdn_ar != NULL) free(svdn_ar); };
};
//VSAD: Vendor-Specific Audio Data Block (DBC_EXT_VSAD = 17)
class cea_vsad_cl : public edi_grp_cl {
   private:

   protected:
      static const char       Desc[];
      //fields shared with VSVD
      static const gpflat_dsc_t VSAD_grp;

   public:
      rcode       init(const u8_t* inst, u32_t orflags, edi_grp_cl* parent);
      edi_grp_cl* Clone(rcode& rcd, u32_t flags) {return base_clone(rcd, new cea_vsad_cl(), inst_data, flags); };
};

//RMCD: Room Configuration Data Block (DBC_EXT_RMCD = 19)
class cea_rmcd_cl : public dbc_grp_cl {
   private:
      static const char          Desc[];
      static const subgrp_dsc_t* fields[];
      static const gproot_dsc_t  RMCD_grp;

   public:
      rcode       init(const u8_t* inst, u32_t orflags, edi_grp_cl* parent);
      edi_grp_cl* Clone(rcode& rcd, u32_t flags) {return base_clone(rcd, new cea_rmcd_cl(), inst_data, flags); };
};
//RMCD: DHDR: Data Header
class rmcd_hdr_cl : public edi_grp_cl {
   private:
      static const char         Desc   [];
      static const edi_field_t  fld_dsc[];

   public:
      static const subgrp_dsc_t DHDR_subg;

      rcode  init(const u8_t* inst, u32_t orflags, edi_grp_cl* parent);

      static edi_grp_cl* group_new() {return new rmcd_hdr_cl();};
};
//RMCD: SPM: Speaker Mask
class rmcd_spm_cl : public edi_grp_cl {
   private:
      //field descriptors and group description are shared with SAB

   public:
      static const subgrp_dsc_t SPM_subg;

      rcode  init(const u8_t* inst, u32_t orflags, edi_grp_cl* parent);

      static edi_grp_cl* group_new() {return new rmcd_spm_cl();};
};
//RMCD: SPKD: Speaker Distance
class rmcd_spkd_cl : public edi_grp_cl {
   private:
      static const char         Desc   [];
      static const edi_field_t  fld_dsc[];

   public:
      static const subgrp_dsc_t SPKD_subg;

      rcode  init(const u8_t* inst, u32_t orflags, edi_grp_cl* parent);

      static edi_grp_cl* group_new() {return new rmcd_spkd_cl();};
};
//RMCD: DSPC: Display Coordinates
class rmcd_dspc_cl : public edi_grp_cl {
   private:
      static const char         Desc   [];
      static const edi_field_t  fld_dsc[];

   public:
      static const subgrp_dsc_t DSPC_subg;

      rcode  init(const u8_t* inst, u32_t orflags, edi_grp_cl* parent);

      static edi_grp_cl* group_new() {return new rmcd_dspc_cl();};
};

//SLDB: Speaker Location Data Block (DBC_EXT_SLDB = 20)
class cea_sldb_cl : public dbc_grp_cl {
   private:
      static const char         Desc[];
      static const gproot_dsc_t SLDB_grp;

   public:
      rcode       init(const u8_t* inst, u32_t orflags, edi_grp_cl* parent);
      edi_grp_cl* Clone(rcode& rcd, u32_t flags) {return base_clone(rcd, new cea_sldb_cl(), inst_data, flags); };
};
//SLDB: SLOCD: Speaker Location Descriptor
class slocd_cl : public edi_grp_cl {
   private:

   public:
      static const char         Desc[];
      static const subgrp_dsc_t SLOCD_subg;
      static const edi_field_t  fld_dsc[];

      rcode  init(const u8_t* inst, u32_t orflags, edi_grp_cl* parent);

      static edi_grp_cl* group_new() {return new slocd_cl();};
};
//IFDB: InfoFrame Data Block (DBC_EXT_IFDB = 32)
class cea_ifdb_cl : public dbc_grp_cl {
   private:
      static const char  CodN[];
      static const char  Name[];
      static const char  Desc[];

   public:
      rcode       init(const u8_t* inst, u32_t orflags, edi_grp_cl* parent);
      edi_grp_cl* Clone(rcode& rcd, u32_t flags) {return base_clone(rcd, new cea_ifdb_cl(), inst_data, flags); };
};
//IFDB: IFPD: InfoFrame Processing Descriptor
class ifdb_ifpdh_cl : public edi_grp_cl {
   private:
      static const char         Desc   [];
      static const edi_field_t  fld_dsc[];

   public:
      static const subgrp_dsc_t IFPD_subg;

      rcode  init(const u8_t* inst, u32_t orflags, edi_grp_cl* parent);
};
//IFDB: SIFD: Short InfoFrame Descriptor, InfoFrame Type Code != 0x00, 0x01
class ifdb_sifd_cl : public edi_grp_cl {
   private:
      static const char         Desc[];
      static const edi_field_t  fld_dsc[];
      static const subgrp_dsc_t SIFD_subg;

   public:
      rcode  init(const u8_t* inst, u32_t orflags, edi_grp_cl* parent);
};
//IFDB: VSIFD: Short Vendor-Specific InfoFrame Descriptor, InfoFrame Type Code = 0x01
class ifdb_vsifd_cl : public edi_grp_cl {
   private:
      static const char         Desc[];
      static const edi_field_t  fld_dsc[];
      static const subgrp_dsc_t VSIFD_subg;

   public:
      rcode  init(const u8_t* inst, u32_t orflags, edi_grp_cl* parent);
};

//UNK-ET: Unknown Data Block (Extended Tag Code)
class cea_unket_cl : public edi_grp_cl {
   private:
      static const char  CodN[];
      static const char  Name[];
      static const char  Desc[];

   public:
      rcode       init(const u8_t* inst, u32_t orflags, edi_grp_cl* parent);
      edi_grp_cl* Clone(rcode& rcd, u32_t flags) {return base_clone(rcd, new cea_unket_cl(), inst_data, flags); };
};


#endif /* CEA_EXT_CLASS_H */
