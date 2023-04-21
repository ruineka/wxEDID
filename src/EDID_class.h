/***************************************************************
 * Name:      EDID_class.h
 * Purpose:   EDID classes and field handlers
 * Author:    Tomasz Pawlak (tomasz.pawlak@wp.eu)
 * Created:   2014-03-18
 * Copyright: Tomasz Pawlak (C) 2014-2022
 * License:   GPLv3+
 **************************************************************/

#ifndef EDID_CLASS_H
#define EDID_CLASS_H 1

#include "EDID_shared.h"

//----------------- EDID base block

//BED: Base EDID data
class edibase_cl : public edi_grp_cl {
   private:
      static const edi_field_t fields[];

      static const char  CodN[];
      static const char  Name[];
      static const char  Desc[];

   public:
      rcode  init(const u8_t* inst, u32_t orflags, edi_grp_cl* parent);
      void   SpawnInstance(u8_t *pinst);
};
//VID: Video Input Descriptor
class vindsc_cl : public edi_grp_cl {
   private:
      enum {
         in_analog_fcnt  = 7,
         in_digital_fcnt = 4,
         max_fcnt        = in_analog_fcnt //for pre-allocating array of fields
      };

      //block data layouts
      static const edi_field_t in_analog[];
      static const edi_field_t in_digital[];

      static const char  CodN[];
      static const char  Name[];
      static const char  Desc[];

      rcode  gen_data_layout(const u8_t* inst);

   public:
      rcode  init(const u8_t* inst, u32_t orflags, edi_grp_cl* parent);
      rcode  ForcedGroupRefresh();
};
//BDD: basic display descriptior
class bddcs_cl : public edi_grp_cl {
   private:
      static const edi_field_t fields[];

      static const char  CodN[];
      static const char  Name[];
      static const char  Desc[];

   public:
      rcode  init(const u8_t* inst, u32_t orflags, edi_grp_cl* parent);
};
//SPF: Supported features
class spft_cl : public edi_grp_cl {
   private:
      static const edi_field_t fields[];

      static const char  CodN[];
      static const char  Name[];
      static const char  Desc[];

   public:
      rcode  init(const u8_t* inst, u32_t orflags, edi_grp_cl* parent);
};
//Chromacity coords
class chromxy_cl : public edi_grp_cl {
   private:
      static const edi_field_t fields[];

      static const char  CodN[];
      static const char  Name[];
      static const char  Desc[];

   public:
      rcode  init(const u8_t* inst, u32_t orflags, edi_grp_cl* parent);
};
//ETM: Estabilished Timings Map
class resmap_cl : public edi_grp_cl {
   private:
      static const edi_field_t fields[];

      static const char  CodN[];
      static const char  Name[];
      static const char  Desc[];

   public:
      rcode  init(const u8_t* inst, u32_t orflags, edi_grp_cl* parent);
};
//STI: Std Timing Information Descriptor
class sttd_cl : public edi_grp_cl {
   private:
      static const edi_field_t fields[];

   protected:
      static const char  CodN[];
      static const char  Name[];
      static const char  Desc[];

   public:
      rcode  init(const u8_t* inst, u32_t orflags, edi_grp_cl* parent);
};
//DTD fields indexes in array
enum {
   DTD_IDX_PIXCLK = 0,
   DTD_IDX_HAPIX,
   DTD_IDX_HBPIX,
   DTD_IDX_VALIN,
   DTD_IDX_VBLIN,
   DTD_IDX_HSOFFS,
   DTD_IDX_HSWIDTH,
   DTD_IDX_VSOFFS,
   DTD_IDX_VSWIDTH,
   DTD_IDX_HSIZE,
   DTD_IDX_VSIZE,
   DTD_IDX_HBORD,
   DTD_IDX_VBORD,
   DTD_IDX_IL2W,
   DTD_IDX_HSTYPE,
   DTD_IDX_VSTYPE,
   DTD_IDX_SYNCTYP,
   DTD_IDX_STEREOMD,
   DTD_IDX_ILACE
};
//DTD : detailed timing descriptor
class dtd_cl : public edi_grp_cl {
   private:
      static const edi_field_t fields[];

      static const char  CodN[];
      static const char  Name[];
      static const char  Desc[];

   public:
      rcode       init(const u8_t* inst, u32_t orflags, edi_grp_cl* parent);
      edi_grp_cl* Clone(rcode& rcd, u32_t flags) {return base_clone(rcd, new dtd_cl(), inst_data, flags); };
};
//MRL : Monitor Range Limits Descriptor
class mrl_cl : public edi_grp_cl {
   private:
      static const edi_field_t fields[];

      static const char  CodN[];
      static const char  Name[];
      static const char *Desc;

   public:
      rcode       init(const u8_t* inst, u32_t orflags, edi_grp_cl* parent);
      edi_grp_cl* Clone(rcode& rcd, u32_t flags) {return base_clone(rcd, new dtd_cl(), inst_data, flags); };
};
//WPD : White Point Descriptor
class wpt_cl : public edi_grp_cl {
   private:
      static const edi_field_t fields[];

      static const char  CodN[];
      static const char  Name[];
      static const char *Desc;

   public:
      rcode       init(const u8_t* inst, u32_t orflags, edi_grp_cl* parent);
      edi_grp_cl* Clone(rcode& rcd, u32_t flags) {return base_clone(rcd, new wpt_cl(), inst_data, flags); };
};
//MND: Monitor Name Descriptor
class mnd_cl : public edi_grp_cl {
   private:
      static const edi_field_t fields[];

      static const char  CodN[];
      static const char  Name[];
      static const char *Desc;

   public:
      rcode       init(const u8_t* inst, u32_t orflags, edi_grp_cl* parent);
      edi_grp_cl* Clone(rcode& rcd, u32_t flags) {return base_clone(rcd, new mnd_cl(), inst_data, flags); };
};
//MSN: Monitor Serial Number Descriptor
class msn_cl : public edi_grp_cl {
   private:
      static const edi_field_t fields[];

      static const char  CodN[];
      static const char  Name[];
      static const char *Desc;

   public:
      rcode       init(const u8_t* inst, u32_t orflags, edi_grp_cl* parent);
      edi_grp_cl* Clone(rcode& rcd, u32_t flags) {return base_clone(rcd, new msn_cl(), inst_data, flags); };
};
//UTX: UnSpecified Text
class utx_cl : public edi_grp_cl {
   private:
      static const edi_field_t fields[];

      static const char  CodN[];
      static const char  Name[];
      static const char *Desc;

   public:
      rcode       init(const u8_t* inst, u32_t orflags, edi_grp_cl* parent);
      edi_grp_cl* Clone(rcode& rcd, u32_t flags) {return base_clone(rcd, new utx_cl(), inst_data, flags); };
};
//AST: Additional Standard Timing identifiers
class ast_cl : public edi_grp_cl {
   private:
      static const edi_field_t fields[];

      static const char  CodN[];
      static const char  Name[];
      static const char *Desc;

   public:
      rcode       init(const u8_t* inst, u32_t orflags, edi_grp_cl* parent);
      edi_grp_cl* Clone(rcode& rcd, u32_t flags) {return base_clone(rcd, new ast_cl(), inst_data, flags); };
};
//UNK: Unknown Descriptor (type != 0xFA-0xFF)
class unk_cl : public edi_grp_cl {
   private:
      static const edi_field_t fields[];

      static const char  CodN[];
      static const char  Name[];
      static const char *Desc;

   public:
      rcode       init(const u8_t* inst, u32_t orflags, edi_grp_cl* parent);
      edi_grp_cl* Clone(rcode& rcd, u32_t flags) {return base_clone(rcd, new unk_cl(), inst_data, flags); };
};


#endif /* EDID_CLASS_H */
