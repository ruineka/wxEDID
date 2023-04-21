/***************************************************************
 * Name:      CEA_EXT_class.cpp
 * Purpose:   CEA/CTA-861-G Extended Tag Codes
 * Author:    Tomasz Pawlak (tomasz.pawlak@wp.eu)
 * Created:   2019-12-20
 * Copyright: Tomasz Pawlak (C) 2019-2022
 * License:   GPLv3+
 **************************************************************/

#include "debug.h"
#include "rcdunits.h"
#ifndef idCEA_EXT
   #error "CEA_EXT_class.cpp: missing unit ID"
#endif
#define RCD_UNIT idCEA_EXT
#include "rcode/rcode.h"

#include "wxedid_rcd_scope.h"

RCD_AUTOGEN_DEFINE_UNIT

#include <stddef.h>

#include "CEA_class.h"
#include "CEA_EXT_class.h"

//CEA bad block length msg
extern const char DBC_BAD_LEN_MSG[];

//CEA header field descriptions (defined in CEA_class.cpp)
extern const char CEA_BlkHdr_dscF0[];
extern const char CEA_BlkHdr_dscF1[];
extern const char CEA_BlkHdr_dscF2[];
extern const edi_field_t CEA_BlkHdr_fields[];

//CEA VDB video mode map (svd_vidfmt.h)
extern const vmap_t CEA_vidm_map;

//unknown/invalid byte field (defined in CEA_class.cpp)
extern const edi_field_t unknown_byte_fld;
extern void insert_unk_byte(edi_field_t *p_fld, u32_t len, u32_t s_offs);

//SPM Speaker Presence Mask: shared description (defined in CEA_class.cpp)
extern const char SAB_SPM_Desc[];
//SPM Speaker Presence Mask: shared field descriptors (SAB: defined in CEA_class.cpp)
extern const gpfld_dsc_t SAB_SPM_fields;

//-------------------------------------------------------------------------------------------------
//CEA-DBC Extended Tag Codes

//VCDB: Video Capability Data Block (DBC_EXT_VCDB = 0)
const char  cea_vcdb_cl::Desc[] =
"VCDB is used to declare display capability to overscan/underscan and the quantization range.";

const gpfld_dsc_t cea_vcdb_cl::fields = {
   .flags    = 0,
   .fcount   = 5,
   .dat_sz   = 1,
   .inst_cnt = 1,
   .fields   = cea_vcdb_cl::fld_dsc
};

const edi_field_t cea_vcdb_cl::fld_dsc[] = {
   //VCDB data
   {&EDID_cl::BitF8Val, NULL, 2, 0, 2, EF_BFLD|EF_INT, 0, 3, "S_CE01",
   "CE overscan/underscan:\n0= not supported\n1= always overscan\n2= always underscan\n3= both supported" },
   {&EDID_cl::BitF8Val, NULL, 2, 2, 2, EF_BFLD|EF_INT, 0, 3, "S_IT01",
   "IT overscan/underscan:\n0= not supported\n1= always overscan\n2= always underscan\n3= both supported" },
   {&EDID_cl::BitF8Val, NULL, 2, 4, 2, EF_BFLD|EF_INT, 0, 3, "S_PT01",
   "PT overscan/underscan:\n0= not supported\n1= always overscan\n2= always underscan\n3= both supported" },
   {&EDID_cl::BitVal, NULL, 2, 6, 1, EF_BIT, 0, 3, "QS",
   "Quantization Range Selectable:\n1= selectable via AVI Q (RGB only), 0= no data" },
   {&EDID_cl::BitVal, NULL, 2, 7, 1, EF_BIT, 0, 3, "QY",
   "Quantization Range:\n1= selectable via AVI YQ (YCC only), 0= no data" }
};

const gpflat_dsc_t cea_vcdb_cl::VCDB_grp = {
   .CodN     = "VCDB",
   .Name     = "Video Capability Data Block",
   .Desc     = Desc,
   .type_id  = ID_VCDB,
   .flags    = 0,
   .min_len  = 2,
   .max_len  = 2,
   .max_fld  = (32 - sizeof(vcdb_t) - sizeof(ethdr_t) + 5),
   .hdr_fcnt = CEA_EXTHDR_FCNT,
   .hdr_sz   = sizeof(ethdr_t),
   .fld_arsz = 1,
   .fld_ar   = &fields
};

rcode cea_vcdb_cl::init(const u8_t* inst, u32_t orflags, edi_grp_cl* parent) {
   rcode retU;

   retU = base_DBC_Init_FlatGrp(inst, &VCDB_grp, orflags, parent);
   return retU;
}

//VSVD: Vendor-Specific Video Data Block (DBC_EXT_VSVD =  1)
const char cea_vsvd_cl::Desc[] = "Vendor-Specific Video Data Block: undefined data.";

//fields shared with VSAD
static const edi_field_t fld_dsc[] = {
   //VSVD data
   {&EDID_cl::ByteStr, NULL, 2, 0, 3, EF_STR|EF_HEX|EF_LE|EF_RD, 0, 0xFFFFFF, "IEEE-OUI",
   "IEEE OUI (Organizationally Unique Identifier)" }
};

static const gpfld_dsc_t VSVD_VSAD_fields = {
   .flags    = 0,
   .fcount   = 1,
   .dat_sz   = 3,
   .inst_cnt = 1,
   .fields   = fld_dsc
};

const gpflat_dsc_t cea_vsvd_cl::VSVD_grp = {
   .CodN     = "VSVD",
   .Name     = "Vendor-Specific Video Data Block",
   .Desc     = Desc,
   .type_id  = ID_VSVD,
   .flags    = 0,
   .min_len  = (sizeof(vs_vadb_t) +1),
   .max_len  = 31,
   .max_fld  = (32 - sizeof(vs_vadb_t) - sizeof(ethdr_t) + 1),
   .hdr_fcnt = CEA_EXTHDR_FCNT,
   .hdr_sz   = sizeof(ethdr_t),
   .fld_arsz = 1,
   .fld_ar   = &VSVD_VSAD_fields
};

rcode cea_vsvd_cl::init(const u8_t* inst, u32_t orflags, edi_grp_cl* parent) {
   rcode retU;

   retU = base_DBC_Init_FlatGrp(inst, &VSVD_grp, orflags, parent);
   return retU;
}

//VDDD: VESA Display Device Data Block (DBC_EXT_VDDD = 2)
//VDDD: IFP: Interface
const char  vddd_iface_cl::CodN[] = "IFP";
const char  vddd_iface_cl::Name[] = "Interface properties";
const char  vddd_iface_cl::Desc[] = "Interface properties";

const edi_field_t vddd_iface_cl::fields[] = {
   //if_type
   {&EDID_cl::BitF8Val, NULL, offsetof(vdddb_t, if_type), 4, 4, EF_BFLD|EF_INT, 0, 0xC, "if_type",
   "Interface Type Code: (num of channels/sub-type)\n"
   "0x0= analog (sub-type in 'n_lanes' field)\n"
   "0x1= LVDS (any)\n"
   "0x2= RSDS (any)\n"
   "0x3= DVI-D (1 or 2)\n"
   "0x4= DVI-I, analog section (0)\n"
   "0x5= DVI-I, digital section (1 or 2)\n"
   "0x6= HDMI-A (1)\n"
   "0x7= HDMI-B (2)\n"
   "0x8= MDDI (1 or 2)\n"
   "0x9= DisplayPort (1,2 or 4)\n"
   "0xA= IEEE-1394 (0)\n"
   "0xB= M1, analog (0)\n"
   "0xC= M1, digital (1 or 2)\n" },
   {&EDID_cl::BitF8Val, NULL, offsetof(vdddb_t, if_type), 0, 4, EF_BFLD|EF_INT, 0, 4, "n_lanes",
   "Number of lanes/channels provided by the interface.\n"
   "If interface type is 'analog' (0), then 'n_lanes' is interpreted as follows:\n"
   "0= 15HD/VGA (VESA EDDC std. pinout)\n"
   "1= VESA NAVI-V (15HD)\n"
   "2= VESA NAVI-D" },
   //iface standard version and release
   {&EDID_cl::BitF8Val, NULL, offsetof(vdddb_t, if_version), 4, 4, EF_BFLD|EF_INT, 0, 0xF, "version",
   "Interface version number" },
   {&EDID_cl::BitF8Val, NULL, offsetof(vdddb_t, if_version), 0, 4, EF_BFLD|EF_INT, 0, 0xF, "release",
   "Interface release number" },
   //min/max clock frequency for each interface link/channel
   {&EDID_cl::BitF8Val, NULL, offsetof(vdddb_t, clkfrng), 2, 6, EF_BFLD|EF_INT|EF_MHZ, 0, 63, "if_fmin",
   "Min clock frequency for each interface link/channel" },
   {&EDID_cl::VDDD_IF_MaxF, NULL, offsetof(vdddb_t, clkfrng), 0, 2, EF_BFLD|EF_INT|EF_MHZ, 0, 63, "if_fmax",
   "Max clock frequency for each interface link/channel" }
};

rcode vddd_iface_cl::init(const u8_t* inst, u32_t orflags, edi_grp_cl* parent) {
   static const u32_t fcount = 6;

   rcode  retU;

   parent_grp = parent;
   type_id    = ID_VDDD_IPF | orflags;

   retU = init_fields(&fields[0], inst, fcount, 0, Name, Desc, CodN);
   return retU;
}

//VDDD: CPT: Content Protection
const char  vddd_cprot_cl::CodN[] = "CPT";
const char  vddd_cprot_cl::Name[] = "Content Protection";
const char  vddd_cprot_cl::Desc[] = "Content Protection";

const edi_field_t vddd_cprot_cl::fields[] = {
   //Content Protection support method
   {&EDID_cl::ByteVal, NULL, offsetof(vdddb_t, cont_prot), 0, 1, EF_BYTE|EF_INT, 0, 3, "ContProt",
   "Content Protection supported method:\n"
   "0= none supported\n"
   "1= HDCP\n"
   "2= DTCP\n"
   "3= DPCP (DisplayPort)" }
};

rcode vddd_cprot_cl::init(const u8_t* inst, u32_t orflags, edi_grp_cl* parent) {
   static const u32_t fcount = 1;

   rcode retU;

   parent_grp = parent;
   type_id    = ID_VDDD_CPT | orflags;

   retU = init_fields(&fields[0], inst, fcount, 0, Name, Desc, CodN);
   return retU;
}

//VDDD: AUPR: Audio properties
const char  vddd_audio_cl::CodN[] = "AUPR";
const char  vddd_audio_cl::Name[] = "Audio properties";
const char  vddd_audio_cl::Desc[] = "Audio properties";

const edi_field_t vddd_audio_cl::fields[] = {
   //Audio flags
   {&EDID_cl::BitF8Val, NULL, offsetof(vdddb_t, audioflg), 0, 5, EF_BFLD|EF_RD, 0, 0x7, "resvd04",
   "reserved (0)" },
   {&EDID_cl::BitVal, NULL, offsetof(vdddb_t, audioflg), 5, 1, EF_BIT, 0, 1, "audio_ovr",
   "audio input override, 1= automatically override other audio inputs" },
   {&EDID_cl::BitVal, NULL, offsetof(vdddb_t, audioflg), 6, 1, EF_BIT, 0, 1, "sep_achn",
   "separate audio channel inputs: not via video interface" },
   {&EDID_cl::BitVal, NULL, offsetof(vdddb_t, audioflg), 7, 1, EF_BIT, 0, 1, "vid_achn",
   "1= audio support on video interface" },
   //Audio delay
   {&EDID_cl::VDDD_AudioDelay, NULL, offsetof(vdddb_t, audiodly), 0, 7, EF_BFLD|EF_INT|EF_MLS, 0, 254, "delay",
   "Audio delay in 2ms resolution.\n"
   "If dly> 254ms, the stored value is 254/2=127, i.e. 254ms is max,\n"
   "0= no delay compensation" },
   {&EDID_cl::BitVal, NULL, offsetof(vdddb_t, audiodly), 7, 1, EF_BIT, 0, 1, "dly_sign",
   "Audio delay sign: 1= '+', 0= '-'" }
};

rcode vddd_audio_cl::init(const u8_t* inst, u32_t orflags, edi_grp_cl* parent) {
   const u32_t fcount = 6;

   rcode retU;

   parent_grp = parent;
   type_id    = ID_VDDD_AUD | orflags;

   retU = init_fields(&fields[0], inst, fcount, 0, Name, Desc, CodN);
   return retU;
}

//VDDD: DPR: Display properties
//VDDD: DPR: Display properties
const char  vddd_disp_cl::CodN[] = "DPR";
const char  vddd_disp_cl::Name[] = "Display properties";
const char  vddd_disp_cl::Desc[] = "Display properties";

const edi_field_t vddd_disp_cl::fields[] = {
   //H/V pixel count
   {&EDID_cl::VDDD_HVpix_cnt, NULL, offsetof(vdddb_t, Hpix_cnt), 2, 6, EF_INT|EF_PIX, 1, 0x10000, "Hpix_cnt",
   "Count of pixels in horizontal direction: 1-65536" },
   {&EDID_cl::VDDD_HVpix_cnt, NULL, offsetof(vdddb_t, Vpix_cnt), 0, 2, EF_INT|EF_PIX, 1, 0x10000, "Vpix_cnt",
   "Count of pixels in vertical direction: 1-65536" },
   //Aspect Ratio
   {&EDID_cl::VDDD_AspRatio, NULL, offsetof(vdddb_t, aspect), 0, 1, EF_FLT|EF_NI, 0, 0xFF, "AspRatio",
   "Aspect ratio: stored value: 100*((long_axis/short_axis)-1)" },
   //Scan Direction, Position of "zero" pixel, Display Rotation, Orientation
   {&EDID_cl::BitF8Val, NULL, offsetof(vdddb_t, scanrot), 0, 2, EF_BFLD|EF_INT, 0, 3, "scan_dir",
   "Scan Direction:\n"
   "0= undefined\n"
   "1= 'fast' along long axis, 'slow' along short axis\n"
   "2= 'fast' along short axis, 'slow' along long axis\n"
   "3= reserved" },
   {&EDID_cl::BitF8Val, NULL, offsetof(vdddb_t, scanrot), 2, 2, EF_BFLD|EF_INT, 0, 3, "pix0_pos",
   "Position of 'zero' pixel:\n"
   "0= upper left corner\n"
   "1= upper right corner\n"
   "2= lower left corner\n"
   "3= lower right corner" },
   {&EDID_cl::BitF8Val, NULL, offsetof(vdddb_t, scanrot), 4, 2, EF_BFLD|EF_INT, 0, 3, "rotation",
   "Possible display rotation:\n"
   "0= not possible\n"
   "1= 90 degrees clockwise\n"
   "2= 90 degrees counterclockwise\n"
   "3= both 1 & 2 possible" },
   {&EDID_cl::BitF8Val, NULL, offsetof(vdddb_t, scanrot), 6, 2, EF_BFLD|EF_INT, 0, 3, "orientation",
   "Display Orientation:\n"
   "0= landscape\n"
   "1= portrait\n"
   "2= not fixed (can be rotated)\n"
   "3= undefined" },
   //sub-pixel layout code
   {&EDID_cl::ByteVal, NULL, offsetof(vdddb_t, cont_prot), 0, 1, EF_BYTE|EF_INT, 0, 0xC, "subpixlc",
   "sub-pixel layout code:\n"
   "0x0= undefined\n"
   "0x1= RGB V-stripes\n"
   "0x2= RGB H-stripes\n"
   "0x3= RGB V-stripes, primary ordering matches chromaticity info in base EDID\n"
   "0x4= RGB H-stripes, primary ordering matches chromaticity info in base EDID\n"
   "0x5= 2x2 quad: red@top-left, blue@btm-right, remaining 2pix are green\n"
   "0x6= 2x2 quad: red@btm-left, blue@top-right, remaining 2pix are green\n"
   "0x7= delta (triad)\n"
   "0x8= mosaic\n"
   "0x9= 2x2 quad: 3 RGB + 1 extra color (*)\n"
   "0xA= 3 RGB + 2 extra colors below or above the RGB set (*)\n"
   "0xB= 3 RGB + 3 extra colors below or above the RGB set (*)\n"
   "0xC= Clairvoyante, Inc. PenTile Matrix(TM)\n\n"
   "(*) Additional sub-pixel's colors should be described in Additional Chromaticity Coordinates"
   "srtucture of this block (DDDB Std, section 2.16)" },
   //horizontal/vertical pixel pitch
   {&EDID_cl::VDDD_HVpx_pitch, NULL, offsetof(vdddb_t, Hpix_pitch), 0, 1, EF_FLT|EF_NI|EF_MM, 0, 0xFF, "Hpix_pitch",
   "horizontal pixel pitch in 0.01mm (0.00 - 2.55)" },
   {&EDID_cl::VDDD_HVpx_pitch, NULL, offsetof(vdddb_t, Vpix_pitch), 0, 1, EF_FLT|EF_NI|EF_MM, 0, 0xFF, "Vpix_pitch",
   "vertical pixel pitch in 0.01mm (0.00 - 2.55)" },
   //miscellaneous display caps.
   {&EDID_cl::BitF8Val, NULL, offsetof(vdddb_t, misccaps), 0, 3, EF_BFLD|EF_RD, 0, 0x7, "resvd02",
   "reserved (0)" },
   {&EDID_cl::BitVal, NULL, offsetof(vdddb_t, misccaps), 3, 1, EF_BIT, 0, 1, "deinterlacing",
   "deinterlacing, 1= possible" },
   {&EDID_cl::BitVal, NULL, offsetof(vdddb_t, misccaps), 4, 1, EF_BIT, 0, 1, "ovrdrive",
   "overdrive not recommended, 1= src 'should' not overdrive the signal by default" },
   {&EDID_cl::BitVal, NULL, offsetof(vdddb_t, misccaps), 5, 1, EF_BIT, 0, 1, "d_drive",
   "direct-drive, 0= no direct-drive" },
   {&EDID_cl::BitF8Val, NULL, offsetof(vdddb_t, misccaps), 6, 2, EF_BFLD, 0, 0x7, "dither",
   "dithering, 0=no, 1=spatial, 2=temporal, 3=both" },
   //Frame rate and mode conversion
   {&EDID_cl::BitF8Val, NULL, offsetof(vdddb_t, frm_rcnv), 0, 6, EF_BFLD|EF_INT, 0, 0x3F, "frm_delta",
   "Frame rate delta range:\n"
   "max FPS deviation from default frame rate (+/- 63 FPS)" },
   {&EDID_cl::BitF8Val, NULL, offsetof(vdddb_t, frm_rcnv), 6, 2, EF_BFLD|EF_INT, 0, 3, "conv_mode",
   "available conversion mode:\n"
   "0= none\n"
   "1= single buffer\n"
   "2= double buffer\n"
   "3= other, f.e. interframe interpolation" },
   {&EDID_cl::ByteVal, NULL, offsetof(vdddb_t, frm_rcnv), 1, 1, EF_BYTE|EF_INT, 0, 0xFF, "frm_rate",
   "default frame rate (FPS)" },
   //Color bit depth
   {&EDID_cl::BitF8Val, NULL, offsetof(vdddb_t, colbitdpth), 0, 4, EF_BFLD|EF_INT, 0, 0xF, "cbd_dev",
   "color bit depth: display device: value=cbd-1 (1-16)" },
   {&EDID_cl::BitF8Val, NULL, offsetof(vdddb_t, colbitdpth), 4, 4, EF_BFLD|EF_INT, 0, 0xF, "cbd_iface",
   "color bit depth: interface: value=cbd-1 (1-16)" },
   //Display response time
   {&EDID_cl::BitF8Val, NULL, offsetof(vdddb_t, resp_time), 0, 7, EF_BFLD|EF_INT|EF_MLS, 0, 0x7F, "resp_time",
   "Display response time in milliseconds, max 126. 127 means greater than 126, but unspecified" },
   {&EDID_cl::BitVal, NULL, offsetof(vdddb_t, resp_time), 7, 1, EF_BIT, 0, 1, "resp_type",
   "Display response time type:\n"
   "1=black-to-white\n"
   "0=white-to-black" },
   //Overscan
   {&EDID_cl::BitF8Val, NULL, offsetof(vdddb_t, overscan), 0, 4, EF_BFLD|EF_INT|EF_PCT, 0, 0xF, "V_overscan",
   "Vertical overscan, 0-15%, 0= no overscan (but doesn't mean 100% image match)" },
   {&EDID_cl::BitF8Val, NULL, offsetof(vdddb_t, overscan), 4, 4, EF_BFLD|EF_INT|EF_PCT, 0, 0xF, "H_overscan",
   "Horizontal overscan, 0-15%, 0= no overscan (but doesn't mean 100% image match)" }
};

rcode vddd_disp_cl::init(const u8_t* inst, u32_t orflags, edi_grp_cl* parent) {
   const u32_t fcount = 24;

   rcode retU;

   parent_grp = parent;
   type_id    = ID_VDDD_DPR | orflags;

   retU = init_fields(&fields[0], inst, fcount, 0, Name, Desc, CodN);
   return retU;
}

//VDDD: CXY: Additional Chromaticity coords
const char  vddd_cxy_cl::CodN[] = "CXY";
const char  vddd_cxy_cl::Name[] = "Additional Chromaticity coords";
const char  vddd_cxy_cl::Desc[] = "Chromaticity coords for additional primary color sub-pixels";

const edi_field_t vddd_cxy_cl::fields[] = {
   //Additional Chromaticity Coordinates
   {&EDID_cl::BitF8Val, NULL, offsetof(vdddb_t, addchrxy)+1, 0, 2, EF_BFLD|EF_INT, 0, 0xF, "n_colors",
   "Number of additional chromaticity coordinates for primary colors 4-6\n"
   "Standard RGB primary colors are numbered 1-3 and described in base EDID structure" },
   {&EDID_cl::BitF8Val, NULL, offsetof(vdddb_t, addchrxy)+1, 2, 2, EF_BFLD|EF_INT, 0, 0xF, "resvd32",
   "reserved (0)" },
   {&EDID_cl::CHredX, NULL, offsetof(vdddb_t, addchrxy), 0, 10, EF_BFLD|EF_FLT|EF_NI, 0, 1, "col4_x", "" },
   {&EDID_cl::CHredY, NULL, offsetof(vdddb_t, addchrxy), 0, 10, EF_BFLD|EF_FLT|EF_NI, 0, 1, "col4_y", "" },
   {&EDID_cl::CHgrnX, NULL, offsetof(vdddb_t, addchrxy), 0, 10, EF_BFLD|EF_FLT|EF_NI, 0, 1, "col5_x", "" },
   {&EDID_cl::CHgrnY, NULL, offsetof(vdddb_t, addchrxy), 0, 10, EF_BFLD|EF_FLT|EF_NI, 0, 1, "col5_y", "" },
   {&EDID_cl::CHbluX, NULL, offsetof(vdddb_t, addchrxy), 0, 10, EF_BFLD|EF_FLT|EF_NI, 0, 1, "col6_x", "" },
   {&EDID_cl::CHbluY, NULL, offsetof(vdddb_t, addchrxy), 0, 10, EF_BFLD|EF_FLT|EF_NI, 0, 1, "col6_y", "" }
};

rcode vddd_cxy_cl::init(const u8_t* inst, u32_t orflags, edi_grp_cl* parent) {
   const u32_t fcount = 8;

   rcode retU;

   parent_grp = parent;
   type_id    = ID_VDDD_CXY | orflags;

   retU = init_fields(&fields[0], inst, fcount, 0, Name, Desc, CodN);
   return retU;
}

//VDDD: VESA Display Device Data Block: base class
//NOTE: subgroups data are not continuous within DBC.
const char  cea_vddd_cl::CodN[] = "VDDD";
const char  cea_vddd_cl::Name[] = "VESA Display Device Data Block";
const char  cea_vddd_cl::Desc[] = "Additional information about display device";

rcode cea_vddd_cl::init(const u8_t* inst, u32_t orflags, edi_grp_cl* parent) {
   rcode       retU, retU2;
   u32_t       dlen;
   edi_grp_cl* pgrp;
   bool        b_edit_mode;

   RCD_SET_OK(retU2);

   parent_grp = parent;
   type_id    = ID_VDDD;

   CopyInstData(inst, sizeof(vdddb_t)); //32

   //create and init sub-groups: 5: IFP, CPT, AUD, DPR, CXY

   b_edit_mode  = (0 != (T_MODE_EDIT & orflags));

   dlen = reinterpret_cast <const ethdr_t*> (inst)->ehdr.hdr.tag.blk_len;
   if (dlen != 31) {
      wxedid_RCD_SET_FAULT_VMSG(retU2, DBC_BAD_LEN_MSG, CodN);
      if (! b_edit_mode) return retU2;
      goto unk;
   }

   //NOTE: vdddb_t contains the block header, so the inst ptr is the same for all sub-groups
   //      Local instance data are shared for all sub-groups.

   //IFP
   pgrp = new vddd_iface_cl;
   if (pgrp == NULL) RCD_RETURN_FAULT(retU);
   retU = pgrp->init(inst_data, (ID_VDDD | T_DBC_SUBGRP | T_DBC_FIXED), this );
   if (! RCD_IS_OK(retU)) return retU;
   pgrp->setAbsOffs(abs_offs);
   subgroups.Append(pgrp);
   //CPT
   pgrp = new vddd_cprot_cl;
   if (pgrp == NULL) RCD_RETURN_FAULT(retU);
   retU = pgrp->init(inst_data, (ID_VDDD | T_DBC_SUBGRP | T_DBC_FIXED), this );
   if (! RCD_IS_OK(retU)) return retU;
   pgrp->setAbsOffs(abs_offs);
   subgroups.Append(pgrp);
   //AUD
   pgrp = new vddd_audio_cl;
   if (pgrp == NULL) RCD_RETURN_FAULT(retU);
   retU = pgrp->init(inst_data, (ID_VDDD | T_DBC_SUBGRP | T_DBC_FIXED), this );
   if (! RCD_IS_OK(retU)) return retU;
   pgrp->setAbsOffs(abs_offs);
   subgroups.Append(pgrp);
   //DPR
   pgrp = new vddd_disp_cl;
   if (pgrp == NULL) RCD_RETURN_FAULT(retU);
   retU = pgrp->init(inst_data, (ID_VDDD | T_DBC_SUBGRP | T_DBC_FIXED), this );
   if (! RCD_IS_OK(retU)) return retU;
   pgrp->setAbsOffs(abs_offs);
   subgroups.Append(pgrp);
   //CXY
   pgrp = new vddd_cxy_cl;
   if (pgrp == NULL) RCD_RETURN_FAULT(retU);
   retU = pgrp->init(inst_data, (ID_VDDD | T_DBC_SUBGRP | T_DBC_FIXED), this );
   if (! RCD_IS_OK(retU)) return retU;
   pgrp->setAbsOffs(abs_offs);
   subgroups.Append(pgrp);

unk:
   retU = init_fields(&CEA_BlkHdr_fields[0], inst_data, CEA_EXTHDR_FCNT, orflags, Name, Desc, CodN);

   if (! RCD_IS_OK(retU2)) {
      u8_t *p_inst = inst_data;

      if (dlen < sizeof(ethdr_t)) return retU2;

      p_inst += sizeof(ethdr_t);
      dlen   -= sizeof(ethdr_t);

      retU = Append_UNK_DAT(p_inst, dlen, type_id, abs_offs, 0, this);
      if ( (! RCD_IS_OK(retU)) && !b_edit_mode ) return retU;
   }

   subgroups.CalcDataSZ(this);

   if (! RCD_IS_OK(retU)) return retU2;
   return retU;

}

rcode EDID_cl::VDDD_IF_MaxF(u32_t op, wxString& sval, u32_t& ival, edi_dynfld_t* p_field) {
   rcode   retU;
   u8_t   *inst;

   inst = getValPtr(p_field);

   if (op == OP_READ) {
      u32_t  tmpv;
      tmpv   = reinterpret_cast<clkfrng_t*> (inst)->fmax_2msb;
      ival   = reinterpret_cast<clkfrng_t*> (inst)->fmax_8lsb;
      tmpv <<= 8;
      ival  |= tmpv; //2msb

      sval << ival;
      RCD_SET_OK(retU);
   } else {
      ulong  utmp;
      RCD_SET_FAULT(retU);

      if (op == OP_WRSTR) {
         retU = getStrUint(sval, 10, p_field->field.minv, p_field->field.maxv, utmp);
         if (! RCD_IS_OK(retU)) return retU;
      } else if (op == OP_WRINT) {
         utmp = ival;
         RCD_SET_OK(retU);
      } else {
         RCD_RETURN_FAULT(retU); //wrong op code
      }
      {
         u32_t  tmpv;
         utmp &= 0x3FF;

         tmpv  = utmp >> 8; //2msb
         utmp &= 0xFF;      //8lsb

         reinterpret_cast<clkfrng_t*> (inst)->fmax_2msb = tmpv;
         reinterpret_cast<clkfrng_t*> (inst)->fmax_8lsb = utmp;
      }
   }
   return retU;
}

rcode EDID_cl::VDDD_HVpix_cnt(u32_t op, wxString& sval, u32_t& ival, edi_dynfld_t* p_field) {
   rcode   retU;
   u16_t  *inst;

   inst = (u16_t*) getValPtr(p_field);

   if (op == OP_READ) {
      ival   = *inst;
      ival  += 1; //stored value is pix_count-1

      sval << ival;
      RCD_SET_OK(retU);
   } else {
      ulong  utmp;
      RCD_SET_FAULT(retU);

      if (op == OP_WRSTR) {
         retU = getStrUint(sval, 10, p_field->field.minv, p_field->field.maxv, utmp);
         if (! RCD_IS_OK(retU)) return retU;
      } else if (op == OP_WRINT) {
         utmp = ival;
         RCD_SET_OK(retU);
      } else {
         RCD_RETURN_FAULT(retU); //wrong op code
      }

      utmp -= 1;
      if (utmp > 0xFFFF) RCD_RETURN_FAULT(retU); //from ival
      *inst = utmp;
   }
   return retU;
}

rcode EDID_cl::VDDD_AspRatio(u32_t op, wxString& sval, u32_t& ival, edi_dynfld_t* p_field) {
   rcode  retU;
   u8_t  *inst;

   inst = getValPtr(p_field);

   if (op == OP_READ) {
      double  dval;
      ival  = *inst;
      dval  = ival;
      dval /= 100.0;
      dval += 1.0;

      if (sval.Printf("%.03f", dval) < 0) {
         RCD_SET_FAULT(retU);
      } else {
         RCD_SET_OK(retU);
      }
   } else {
      u32_t  utmp = 0;
      RCD_SET_FAULT(retU);

      if (op == OP_WRSTR) {
         double  dval;
         retU = getStrDouble(sval, 1.0, 3.55, dval);
         if (! RCD_IS_OK(retU)) return retU;
         dval -= 1.0;
         dval *= 100.0;
         ival = dval;
      } else if (op == OP_WRINT) {
         ival &= 0xFF;
         utmp  = ival;
         RCD_SET_OK(retU);
      } else {
         RCD_RETURN_FAULT(retU); //wrong op code
      }

      *inst = utmp;
   }
   return retU;
}

rcode EDID_cl::VDDD_HVpx_pitch(u32_t op, wxString& sval, u32_t& ival, edi_dynfld_t* p_field) {
   rcode  retU;
   u8_t  *inst;

   inst = getValPtr(p_field);

   if (op == OP_READ) {
      double  dval;
      ival  = *inst;
      dval  = ival;
      dval /= 100.0;

      if (sval.Printf("%.03f", dval) < 0) {
         RCD_SET_FAULT(retU);
      } else {
         RCD_SET_OK(retU);
      }
   } else {
      u32_t  utmp = 0;
      RCD_SET_FAULT(retU);

      if (op == OP_WRSTR) {
         double  dval;
         retU = getStrDouble(sval, 0.0, 2.55, dval);
         if (! RCD_IS_OK(retU)) return retU;
         dval *= 100.0;
         ival = dval;
      } else if (op == OP_WRINT) {
         ival &= 0xFF;
         utmp  = ival;
         RCD_SET_OK(retU);
      } else {
         RCD_RETURN_FAULT(retU); //wrong op code
      }

      *inst = utmp;
   }
   return retU;
}

rcode EDID_cl::VDDD_AudioDelay(u32_t op, wxString& sval, u32_t& ival, edi_dynfld_t* p_field) {
   rcode   retU;
   u8_t   *inst;

   inst = getValPtr(p_field);

   if (op == OP_READ) {
      ival   = *inst;
      ival  &= 0x7F;
      ival <<= 1;   //2ms resolution

      sval << ival;
      RCD_SET_OK(retU);
   } else {
      ulong  utmp;
      RCD_SET_FAULT(retU);

      if (op == OP_WRSTR) {
         retU = getStrUint(sval, 10, p_field->field.minv, p_field->field.maxv, utmp);
         if (! RCD_IS_OK(retU)) return retU;
      } else if (op == OP_WRINT) {
         utmp = ival;
         RCD_SET_OK(retU);
      } else {
         RCD_RETURN_FAULT(retU); //wrong op code
      }

      utmp >>= 1;
      utmp  &= 0x7F;
      *inst  = utmp;
   }
   return retU;
}

//VVTB: VESA Video Timing Block Extension (DBC_EXT_VVTB = 3) BUG!
//      UNK-ET is used for ExtTagCode == 3

//CLDB: Colorimetry Data Block (DBC_EXT_CLDB = 5)
const char  cea_cldb_cl::Desc[] =
"The Colorimetry Data Block indicates support of specific extended colorimetry standards";

const gpfld_dsc_t cea_cldb_cl::fields = {
   .flags    = 0,
   .fcount   = 11,
   .dat_sz   = sizeof(cldb_t),
   .inst_cnt = 1,
   .fields   = cea_cldb_cl::fld_dsc
};

const edi_field_t cea_cldb_cl::fld_dsc[] = {
    //byte2
   {&EDID_cl::BitVal, NULL, 2, 0, 1, EF_BIT, 0, 1, "xvYCC601",
   "Standard Definition Colorimetry based on IEC 61966-2-4" },
   {&EDID_cl::BitVal, NULL, 2, 1, 1, EF_BIT, 0, 1, "xvYCC709",
   "High Definition Colorimetry based on IEC 61966-2-4" },
   {&EDID_cl::BitVal, NULL, 2, 2, 1, EF_BIT, 0, 1, "sYCC601",
   "Colorimetry based on IEC 61966-2-1/Amendment 1" },
   {&EDID_cl::BitVal, NULL, 2, 3, 1, EF_BIT, 0, 1, "opYCC601",
   "Colorimetry based on IEC 61966-2-5, Annex A" },
   {&EDID_cl::BitVal, NULL, 2, 4, 1, EF_BIT, 0, 1, "opRGB",
   "Colorimetry based on IEC 61966-2-5" },
   {&EDID_cl::BitVal, NULL, 2, 5, 1, EF_BIT, 0, 1, "BT2020cYCC",
   "Colorimetry based on ITU-R BT.2020, Y’cC’BCC’RC" },
   {&EDID_cl::BitVal, NULL, 2, 6, 1, EF_BIT, 0, 1, "BT2020YCC",
   "Colorimetry based on ITU-R BT.2020, Y’C’BC’R" },
   {&EDID_cl::BitVal, NULL, 2, 7, 1, EF_BIT, 0, 1, "BT2020RGB",
   "Colorimetry based on ITU-R BT.2020, R’G’B’" },
   //byte3
   {&EDID_cl::BitF8Val, NULL, 3, 0, 4, EF_BFLD|EF_INT|EF_RD, 0, 0x07, "MD03",
    "MD0-3: Designated for future gamut-related metadata. "
    "As yet undefined, this metadata is carried in an interface-specific way." },
   {&EDID_cl::BitF8Val, NULL, 3, 4, 3, EF_BFLD|EF_INT|EF_RD, 0, 0x07, "resvd46",
    "reserved (0)" },
   {&EDID_cl::BitVal, NULL, 3, 7, 1, EF_BIT, 0, 1, "DCI-P3",
   "Colorimetry based on DCI-P3" }
};

const gpflat_dsc_t cea_cldb_cl::CLDB_grp = {
   .CodN     = "CLDB",
   .Name     = "Colorimetry Data Block",
   .Desc     = Desc,
   .type_id  = ID_CLDB,
   .flags    = 0,
   .min_len  = (sizeof(cldb_t) +1),
   .max_len  = (sizeof(cldb_t) +1),
   .max_fld  = (32 - sizeof(cldb_t) - sizeof(ethdr_t) + 11),
   .hdr_fcnt = CEA_EXTHDR_FCNT,
   .hdr_sz   = sizeof(ethdr_t),
   .fld_arsz = 1,
   .fld_ar   = &fields
};

rcode cea_cldb_cl::init(const u8_t* inst, u32_t orflags, edi_grp_cl* parent) {
   rcode retU;

   retU = base_DBC_Init_FlatGrp(inst, &CLDB_grp, orflags, parent);
   return retU;
}

//HDRS: HDR Static Metadata Data Block (DBC_EXT_HDRS = 6)
const char  cea_hdrs_cl::Desc[] = "Indicates the HDR capabilities of the Sink.";

const edi_field_t cea_hdrs_cl::fld_byte_2_3_dsc[] = {
    //byte2
   {&EDID_cl::BitVal, NULL, 2, 0, 1, EF_BIT, 0, 1, "SDR",
   "Traditional gamma - SDR Luminance Range" },
   {&EDID_cl::BitVal, NULL, 2, 1, 1, EF_BIT, 0, 1, "HDR",
   "Traditional gamma - HDR Luminance Range" },
   {&EDID_cl::BitVal, NULL, 2, 2, 1, EF_BIT, 0, 1, "SMPTE",
   "SMPTE ST 2084" },
   {&EDID_cl::BitVal, NULL, 2, 3, 1, EF_BIT, 0, 1, "HLG",
   "Hybrid Log-Gamma (HLG) based on Recommendation ITU-R BT.2100-0" },
   {&EDID_cl::BitVal, NULL, 2, 4, 1, EF_BIT|EF_RD, 0, 1, "ET_4",
   "reserved (0)" },
   {&EDID_cl::BitVal, NULL, 2, 5, 1, EF_BIT|EF_RD, 0, 1, "ET_5",
   "reserved (0)" },
   {&EDID_cl::BitF8Val, NULL, 2, 6, 2, EF_BFLD|EF_RD, 0, 1, "ET6-7",
   "reserved (0)" },
   //byte3
   {&EDID_cl::BitVal, NULL, 3, 0, 1, EF_BIT|EF_RD, 0, 1, "SM_0",
   "Static Metadata Type 1" },
   {&EDID_cl::BitF8Val, NULL, 3, 1, 7, EF_BFLD|EF_INT|EF_RD, 0, 1, "SM1-7",
   "reserved (0)" }
};

const edi_field_t cea_hdrs_cl::fld_opt_byte_4_5_6_dsc[] = {
   //byte 4,5,6
   {&EDID_cl::ByteVal, NULL, 4, 0, 1, EF_BYTE|EF_INT|EF_RD, 0, 0xFF, "max_lum",
   "(Optional) Desired Content Max Luminance data (8 bits)" },
   {&EDID_cl::ByteVal, NULL, 5, 0, 1, EF_BYTE|EF_INT|EF_RD, 0, 0xFF, "avg_lum",
   "(Optional) Desired Content Max Frame-average Luminance data (8 bits)" },
   {&EDID_cl::ByteVal, NULL, 6, 0, 1, EF_BYTE|EF_INT|EF_RD, 0, 0xFF, "min_lum",
   "(Optional) Desired Content Min Luminance data (8 bits)" }
};

const gpflat_dsc_t cea_hdrs_cl::HDRS_grp = {
   .CodN     = "HDRS",
   .Name     = "HDR Static Metadata Data Block",
   .Desc     = Desc,
   .type_id  = ID_HDRS,
   .flags    = 0,
   .min_len  = 3,
   .max_len  = (sizeof(hdrs_t) +1),
   .max_fld  = (32 - sizeof(hdrs_t) - sizeof(ethdr_t) + 12),
   .hdr_fcnt = CEA_EXTHDR_FCNT,
   .hdr_sz   = sizeof(ethdr_t),
   .fld_arsz = 2,
   .fld_ar   = cea_hdrs_cl::fields
};

const gpfld_dsc_t cea_hdrs_cl::fields[] = {
   { //fld_byte_2_3
      .flags    = 0,
      .fcount   = 9,
      .dat_sz   = 2,
      .inst_cnt = 1,
      .fields   = cea_hdrs_cl::fld_byte_2_3_dsc
   },
   { //fld_opt_byte_4_5_6
      .flags    = TG_FLEX_LAYOUT|TG_FLEX_LEN,
      .fcount   = 3,
      .dat_sz   = 3,
      .inst_cnt = 1,
      .fields   = cea_hdrs_cl::fld_opt_byte_4_5_6_dsc
   }
};

rcode cea_hdrs_cl::init(const u8_t* inst, u32_t orflags, edi_grp_cl* parent) {
   rcode retU;

   retU = base_DBC_Init_FlatGrp(inst, &HDRS_grp, orflags, parent);
   return retU;
}

//HDRD: HDR Dynamic Metadata Data Block (DBC_EXT_HDRD = 7)

//HDRD: HDR Dynamic Metadata sub-group
const char  hdrd_mtd_cl::Desc[] = "HDR Dynamic Metadata";

const subgrp_dsc_t hdrd_mtd_cl::MTD_hdr = {
   .s_ctor   = &hdrd_mtd_cl::group_new,
   .CodN     = "MTD",
   .Name     = "HDR Dynamic Metadata",
   .Desc     = Desc,
   .type_id  = ID_HDRD_MTD | T_DBC_SUBGRP,
   .min_len  = sizeof(hdrd_mtd_t)-1, //min len for HDR_dmtd3 (no Support Flags byte)
   .fcount   = 4,
   .inst_cnt = -1,
   .fields   = hdrd_mtd_cl::fld_dsc
};

const edi_field_t hdrd_mtd_cl::fld_dsc[] = {
   {&EDID_cl::ByteVal, NULL, offsetof(hdrd_mtd_t, mtd_len), 0, 1, EF_BYTE|EF_INT|EF_RD|EF_INIT, 0, (30-1), "length",
   "Length of the metadata block\n" },
   {&EDID_cl::HDRD_mtd_type, NULL, offsetof(hdrd_mtd_t, mtd_type), 0, 2, EF_HEX|EF_RD|EF_INIT, 0x1, 0x4, "mtd_type",
   "Type of metadata:\n"
   "0x0000: Reserved\n"
   "0x0001: Metadata specified in Annex R\n"
   "0x0002: Supplemental Enhancement Information (SEI) messages, according to ETSI TS 103 433\n"
   "0x0003: Color Remapping Information SEI message according to ITU-T H.265\n"
   "0x0004: Metadata pecified in Annex S\n"
   "0x0005 .. 0xFFFF: Reserved" },
   {&EDID_cl::BitF8Val, NULL, offsetof(hdrd_mtd_t, supp_flg), 0, 4, EF_BFLD|EF_INT|EF_RD, 0, 0xF, "mtd_ver",
   "Support Flags: HDR metadata version for metadata types 1,2,4\n"
   "For metadata type = 0x0003 this field does not exist (and also the resvd47)" },
   {&EDID_cl::BitF8Val, NULL, offsetof(hdrd_mtd_t, supp_flg), 4, 4, EF_BFLD|EF_INT|EF_RD, 0, 0xF, "resvd47",
   "reserved (0)" }
};

rcode hdrd_mtd_cl::init(const u8_t* inst, u32_t orflags, edi_grp_cl* parent) {
   const hdrd_mtd_t *mtdata;

   rcode  retU, retU2;
   u32_t  dlen;
   u32_t  blk_len;
   u32_t  m_type;
   u32_t  unk_offs;
   u32_t  unk_byte; //payload bytes: not interpreted

   RCD_SET_OK(retU2);

   mtdata     = reinterpret_cast<const hdrd_mtd_t*> (inst);
   blk_len    = mtdata->mtd_len;
   m_type     = mtdata->mtd_type;
   dyn_fcnt   = MTD_hdr.fcount -2; //mtd_ver, resvd47
   dlen       = parent->getFreeSubgSZ();

   parent_grp = parent;
   type_id    = MTD_hdr.type_id | orflags;

   if (m_type != HDR_dmtd3) {
      if (blk_len < 4) {
         wxedid_RCD_SET_FAULT_VMSG(retU2, DBC_BAD_LEN_MSG, MTD_hdr.CodN);
      }
   } else {
      if (blk_len != 2) {
         wxedid_RCD_SET_FAULT_VMSG(retU2, DBC_BAD_LEN_MSG, MTD_hdr.CodN);
      }
   }

   if (blk_len < 2) {
      wxedid_RCD_SET_FAULT_VMSG(retU2, DBC_BAD_LEN_MSG, MTD_hdr.CodN);
      blk_len = 2; //use trash data from local buffer
   }

   blk_len  += 1; // +metadata_length byte
   blk_len   = (blk_len <= dlen  ) ? blk_len : dlen;
   blk_len   = (blk_len <= (30-1)) ? blk_len : (30-1);

   unk_byte  = blk_len;
   unk_byte -= MTD_hdr.min_len;
   unk_offs  = MTD_hdr.min_len;

   if (blk_len > MTD_hdr.min_len) { //only HDR_dmtd3 can be represented in full
      if (m_type != HDR_dmtd3) {
         dyn_fcnt += 2; //mtd_ver, resvd47
         unk_offs += 1;
         unk_byte -= 1;

      } else { //DBC length ?> max length for HDR_dmtd3
         wxedid_RCD_SET_FAULT_VMSG(retU2, DBC_BAD_LEN_MSG, MTD_hdr.CodN);
      }
   }

   CopyInstData(inst, blk_len);

   dyn_fldar = (edi_field_t*) malloc((dyn_fcnt + unk_byte) * EDI_FIELD_SZ);
   if (NULL == dyn_fldar) RCD_RETURN_FAULT(retU);

   {
      edi_field_t *p_fld;

      p_fld = dyn_fldar;
      memcpy( p_fld, &MTD_hdr.fields[0], (dyn_fcnt * EDI_FIELD_SZ) );
      p_fld    += dyn_fcnt;
      dyn_fcnt += unk_byte;

      insert_unk_byte(p_fld, unk_byte, unk_offs);

      retU = init_fields(dyn_fldar, inst_data, dyn_fcnt, 0,
                         MTD_hdr.Name, MTD_hdr.Desc, MTD_hdr.CodN);
   }

   if (RCD_IS_OK(retU)) return retU2;
   return retU;
}

//HDRD: HDR Dynamic Metadata Data Block: base class
const char  cea_hdrd_cl::Desc[] = "HDR Dynamic Metadata Data Block";

const gproot_dsc_t cea_hdrd_cl::HDRD_grp = {
   .CodN     = "HDRD",
   .Name     = "HDR Dynamic Metadata Data Block",
   .Desc     = Desc,
   .type_id  = ID_HDRD,
   .flags    = 0,
   .min_len  = sizeof(hdrd_mtd_t)+1,
   .max_len  = 31,
   .hdr_fcnt = CEA_EXTHDR_FCNT,
   .hdr_sz   = sizeof(ethdr_t),
   .grp_arsz = 1,
   .grp_ar   = &hdrd_mtd_cl::MTD_hdr
};

rcode cea_hdrd_cl::init(const u8_t* inst, u32_t orflags, edi_grp_cl* parent) {
   rcode  retU;

   retU = base_DBC_Init_RootGrp(inst, &HDRD_grp, orflags, parent);
   return retU;
}

rcode EDID_cl::HDRD_mtd_type(u32_t op, wxString& sval, u32_t& ival, edi_dynfld_t* p_field) {
   rcode   retU;
   u16_t  *inst;

   inst = (u16_t*) getValPtr(p_field);

   if (op == OP_READ) {
      ival   = *inst;

      sval.Printf("0x%04X", ival);
      RCD_SET_OK(retU);
   } else {
      ulong  utmp;
      RCD_SET_FAULT(retU);

      if (op == OP_WRSTR) {
         retU = getStrUint(sval, 16, p_field->field.minv, p_field->field.maxv, utmp);
         if (! RCD_IS_OK(retU)) return retU;
      } else if (op == OP_WRINT) {
         utmp = ival;
         RCD_SET_OK(retU);
      } else {
         RCD_RETURN_FAULT(retU); //wrong op code
      }
      *inst = utmp;
   }
   return retU;
}

//VFPD: Video Format Preference Data Block (DBC_EXT_VFPD = 13)
const char  cea_vfpd_cl::Desc[] =
"VFPD defines order of preference for selected video formats.\n"
"Preferred formats can be referenced as an index of DTD or SVD already present in the EDID or directly, as a Video ID Code (VIC)."
"The order defined here takes precedence over all other rules defined elsewhere in the standard."
"Each byte in VFPD block contains Short Video Reference (SVR) code, first SVR is the most-preferred video format.";

const gproot_dsc_t cea_vfpd_cl::VFPD_grp = {
   .CodN     = "VFPD",
   .Name     = "Video Format Preference Data",
   .Desc     = Desc,
   .type_id  = ID_VFPD,
   .flags    = 0,
   .min_len  = 2,
   .max_len  = 31,
   .hdr_fcnt = CEA_EXTHDR_FCNT,
   .hdr_sz   = sizeof(ethdr_t),
   .grp_arsz = 1,
   .grp_ar   = &cea_svr_cl::SVR_subg
};

rcode cea_vfpd_cl::init(const u8_t* inst, u32_t orflags, edi_grp_cl* parent) {
   rcode  retU;

   retU = base_DBC_Init_RootGrp(inst, &VFPD_grp, orflags, parent);
   return retU;
}

u32_t EDID_cl::CEA_VFPD_SVR_decode(u32_t svr, u32_t &ndtd) {
   u32_t dtd  = 0;
   u32_t mode = (svr & 0xFF);

   switch (svr) {
      case 0 ... 128: //0, 128 reserved, 1..127 -> VIC code
         break;
      case 129 ... 144: //DTD 1..16
         dtd  = (mode - 128);
         break;
      case 145 ... 192: //reserved
         break;
      default: //193 ... 255: svd_vidfmt.h: max VIC is 219, natv=0, 220..255 are reserved in G spec.
         break;
   }

   ndtd = dtd;
   return mode;
}

//VFPD: Video Format Preference Data Block ->
//SVR: Short Video Reference
const char  cea_svr_cl::Desc[] =
"Short Video Reference: "
"contains index number of preferred resolution defined in CEA/CTA-861 standard "
"or a DTD index";

const subgrp_dsc_t cea_svr_cl::SVR_subg = {
   .s_ctor   = &cea_svr_cl::group_new,
   .CodN     = "SVR",
   .Name     = "Short Video Reference",
   .Desc     = Desc,
   .type_id  = ID_VFPD_SVR | T_DBC_SUBGRP,
   .min_len  = 1,
   .fcount   = 1,
   .inst_cnt = -1,
   .fields   = cea_svr_cl::fld_dsc
};

const edi_field_t cea_svr_cl::fld_dsc[] = {
   //NOTE: no EF_VS flag: no value selector menu
   //CEA_vidm_map used only in MAIN::SaveRep_SubGrps()
   {&EDID_cl::ByteVal, &CEA_vidm_map, 0, 0, 1, EF_BYTE|EF_INT, 0, 0xFF, "SVR",
   "The SVR codes are interpreted as follows:\n"
   "0\t\t: reserved\n"
   "1-127\t: VIC\n"
   "128\t\t: reserved\n"
   "129-144\t: Nth DTD block, N=SVR-128 (1..16)\n"
   "145-192\t: reserved\n"
   "193-219\t: VIC\n"
   "220-255\t: reserved" }
};

rcode cea_svr_cl::init(const u8_t* inst, u32_t orflags, edi_grp_cl* parent) {
   rcode retU;

   parent_grp = parent;
   type_id    = SVR_subg.type_id | orflags; //cea_y42v_cl: set T_DBC_FIXED flag

   CopyInstData(inst, 1);

   retU = init_fields(&SVR_subg.fields[0], inst_data, SVR_subg.fcount, 0, SVR_subg.Name, SVR_subg.Desc, SVR_subg.CodN);
   return retU;
}


//Y42V: YCBCR 4:2:0 Video Data Block (DBC_EXT_Y42V = 14)
const char  cea_y42v_cl::Desc[] =
"List of Short Video Descriptors (SVD), for which the ONLY supported sampling mode is YCBCR 4:2:0 "
"(all other sampling modes are NOT supported: RGB, YCBCR 4:4:4, or YCBCR 4:2:2)\n"
"NOTE:\nBy default, SVDs in this block shall be less preferred than all regular "
"SVDs - this can be changed using Video Format Preference Data Block (VFPD)";

const gproot_dsc_t cea_y42v_cl::Y42V_grp = {
   .CodN     = "Y42V",
   .Name     = "YCBCR 4:2:0 Video Data Block",
   .Desc     = Desc,
   .type_id  = ID_Y42V,
   .flags    = 0,
   .min_len  = sizeof(svd_t)+1,
   .max_len  = 31,
   .hdr_fcnt = CEA_EXTHDR_FCNT,
   .hdr_sz   = sizeof(ethdr_t),
   .grp_arsz = 1,
   .grp_ar   = &cea_svd_cl::SVD_subg
};

rcode cea_y42v_cl::init(const u8_t* inst, u32_t orflags, edi_grp_cl* parent) {
   rcode  retU;

   retU = base_DBC_Init_RootGrp(inst, &Y42V_grp, orflags, parent);
   return retU;
}

//Y42C: YCBCR 4:2:0 Capability Map Data Block (DBC_EXT_Y42C = 15)
const char  cea_y42c_cl::CodN[] = "Y42C";
const char  cea_y42c_cl::Name[] = "YCBCR 4:2:0 Capability Map Data Block";
const char  cea_y42c_cl::Desc[] =
"Bytes in this block are forming a bitmap, in which a single bit indicates whether a format defined in VDB::SVD "
"supports YCBCR 4:2:0 sampling mode (in addition to other modes: RGB, YCBCR 4:4:4, or YCBCR 4:2:2)\n"
"0= SVD does NOT support YCBCR 4:2:0\n"
"1= SVD supports YCBCR 4:2:0 in addition to other modes\n\n"
"Bit 0 in 1st byte refers to 1st SVD in VDB, bit 1 refers to 2nd SVD, and so on, "
"and 9th SVD is referenced by bit 0 in 2nd byte.";

const edi_field_t cea_y42c_cl::bitmap_fld =
   {&EDID_cl::BitVal, NULL, 0, 0, 1, EF_BIT|EF_GPD, 0, 1, NULL, "" };

rcode cea_y42c_cl::init(const u8_t* inst, u32_t orflags, edi_grp_cl* parent) {
   rcode        retU, retU2;
   u32_t        dlen;
   u32_t        offs;
   u32_t        svdn;
   edi_field_t *p_fld;
   y42c_svdn_t *p_svdn;
   bool         b_edit_mode;

   RCD_SET_OK(retU2);
   b_edit_mode = (0 != (T_MODE_EDIT & orflags));

   dlen = reinterpret_cast<const ethdr_t*> (inst)->ehdr.hdr.tag.blk_len;
   if (dlen < 2) { //min 1 bitmap byte
      wxedid_RCD_SET_FAULT_VMSG(retU2, DBC_BAD_LEN_MSG, CodN);
      if (! b_edit_mode) return retU2;
   }

   parent_grp = parent;
   type_id    = ID_Y42C | orflags;

   offs = dlen;
   if (dlen < 2) offs = 2;
   offs ++ ; //hdr
   CopyInstData(inst, offs);
   if (dlen >= 2) {
      dlen  -= 1;
   } else {
      dlen = 0;
   }

   //max 30 bytes * 8 bits = 240
   //+ hdr + ext_tag -> max = 243 fields.
   //buffer for array of fields is allocated dynamically

   dyn_fcnt  = (dlen << 3); //max 30 bytes * 8 bits
   dyn_fcnt += CEA_EXTHDR_FCNT;

   //alloc fields array
   dyn_fldar = (edi_field_t*) malloc( dyn_fcnt * EDI_FIELD_SZ );
   if (NULL == dyn_fldar) RCD_RETURN_FAULT(retU);

   p_fld  = dyn_fldar;

   //CEA-DBC-EXT header fields
   memcpy( p_fld, CEA_BlkHdr_fields, (CEA_EXTHDR_FCNT * EDI_FIELD_SZ) );

   if (dlen == 0) {
      wxedid_RCD_SET_FAULT_VMSG(retU2, DBC_BAD_LEN_MSG, CodN);
      goto grp_empty; //manual editing of blk_len
   }

   //alloc array of y42c_svdn_t: dynamic field names
   svdn_ar = (y42c_svdn_t*) malloc( dyn_fcnt * sizeof(y42c_svdn_t) );
   if (NULL == svdn_ar) RCD_RETURN_FAULT(retU);

   p_fld  += CEA_EXTHDR_FCNT;
   offs    = sizeof(ethdr_t); //1st bitmap byte
   p_svdn  = svdn_ar;
   svdn    = 0;

   //SVD map data: 1 field per bit
   for (u32_t itb=0; itb<dlen; ++itb) {
      for (u32_t bitn=0; bitn<8; ++bitn) {

         memcpy( p_fld, &bitmap_fld, EDI_FIELD_SZ );
         //dynamic field offset:
         p_fld->offs  = offs;
         p_fld->shift = bitn;

         //dynamic field name:
         snprintf(p_svdn->svd_num, sizeof(y42c_svdn_t), "SVD_%u", svdn);
         p_svdn->svd_num[7] = 0;
         p_fld->name        = p_svdn->svd_num;

         p_fld  += 1;
         p_svdn += 1;
         svdn   += 1;
      }
      offs += 1;
   }

grp_empty:
   retU = init_fields(&dyn_fldar[0], inst_data, dyn_fcnt, 0, Name, Desc, CodN);

   if (RCD_IS_OK(retU)) return  retU2;
   return retU;
}

//VSAD: Vendor-Specific Audio Data Block (DBC_EXT_VSAD = 17)
const char  cea_vsad_cl::Desc[] = "Vendor-Specific Audio Data Block: undefined data.";

const gpflat_dsc_t cea_vsad_cl::VSAD_grp = {
   .CodN     = "VSAD",
   .Name     = "Vendor-Specific Audio Data Block",
   .Desc     = Desc,
   .type_id  = ID_VSAD,
   .flags    = 0,
   .min_len  = (sizeof(vs_vadb_t) +1),
   .max_len  = 31,
   .max_fld  = (32 - sizeof(vs_vadb_t) - sizeof(ethdr_t) + 1),
   .hdr_fcnt = CEA_EXTHDR_FCNT,
   .hdr_sz   = sizeof(ethdr_t),
   .fld_arsz = 1,
   .fld_ar   = &VSVD_VSAD_fields //fields shared with VSVD
};

rcode cea_vsad_cl::init(const u8_t* inst, u32_t orflags, edi_grp_cl* parent) {
   rcode retU;

   retU = base_DBC_Init_FlatGrp(inst, &VSAD_grp, orflags, parent);
   return retU;
}

//RMCD: Room Configuration Data Block (DBC_EXT_RMCD = 19)
//RMCD: DHDR: Data Header
const char  rmcd_hdr_cl::Desc[] = "RMCD Data Header";

const subgrp_dsc_t rmcd_hdr_cl::DHDR_subg = {
   .s_ctor   = &rmcd_hdr_cl::group_new,
   .CodN     = "DHDR",
   .Name     = "RMCD Data Header",
   .Desc     = Desc,
   .type_id  = ID_RMCD_HDR | T_DBC_SUBGRP,
   .min_len  = sizeof(rmcd_hdr_t),
   .fcount   = 4,
   .inst_cnt = 1,
   .fields   = rmcd_hdr_cl::fld_dsc
};

const edi_field_t rmcd_hdr_cl::fld_dsc[] = {
   {&EDID_cl::BitVal, NULL, 0, 5, 1, EF_BIT, 0, 1, "SLD",
   "SLD: Speaker Location Descriptor, 1= fields in SPKD (Speaker Distance) are valid (used)" },
   {&EDID_cl::BitVal, NULL, 0, 6, 1, EF_BIT, 0, 1, "Speaker",
   "Speaker: 1= spk_cnt is valid, 0= spk_cnt is undefined" },
   {&EDID_cl::BitVal, NULL, 0, 7, 1, EF_BIT, 0, 1, "Display",
   "1= fields in DSPC (Display Coordinates) are valid (used)" },
   {&EDID_cl::BitF8Val, NULL, 0, 0, 5, EF_BFLD|EF_INT, 0, 0x7, "spk_cnt",
   "Speaker Count: number of LPCM_channels -1, required for SLD=1" }
};

rcode rmcd_hdr_cl::init(const u8_t* inst, u32_t orflags, edi_grp_cl* parent) {
   rcode  retU;

   parent_grp = parent;
   type_id    = DHDR_subg.type_id | orflags;

   CopyInstData(inst, sizeof(rmcd_hdr_t));

   retU = init_fields(&DHDR_subg.fields[0], inst_data, DHDR_subg.fcount, 0, DHDR_subg.Name, DHDR_subg.Desc, DHDR_subg.CodN);
   return retU;
}

//RMCD: SPM: Speaker Presence Mask
//NOTE: Desc is shared with SAB: CEA_class.cpp
const subgrp_dsc_t rmcd_spm_cl::SPM_subg = {
   .s_ctor   = &rmcd_spm_cl::group_new,
   .CodN     = "SPM",
   .Name     = "Speaker Presence Mask",
   .Desc     = SAB_SPM_Desc,
   .type_id  = ID_RMCD_SPM | T_DBC_SUBGRP,
   .min_len  = sizeof(spk_mask_t),
   .fcount   = 0,
   .inst_cnt = 1,
   .fields   = {}
};

rcode rmcd_spm_cl::init(const u8_t* inst, u32_t orflags, edi_grp_cl* parent) {
   rcode  retU;
   u32_t  fcount;

   fcount     = SAB_SPM_fields.fcount;
   parent_grp = parent;
   type_id    = SPM_subg.type_id | orflags;

   CopyInstData(inst, sizeof(spk_mask_t));

   //SAB_SPM_fields

   //pre-alloc buffer for array of fields (max=32 including hdr + ext_tag)
   dyn_fldar = (edi_field_t*) malloc( fcount * EDI_FIELD_SZ );
   if (NULL == dyn_fldar) RCD_RETURN_FAULT(retU);

   { //copy field descriptors & update offsets
      u32_t        fidx;
      u32_t        f_offs;
      edi_field_t *p_fld;
      p_fld  = const_cast<edi_field_t*> (SAB_SPM_fields.fields);

      //copy field descriptors 2..26 (SPM part)
      memcpy( dyn_fldar, p_fld, (fcount * EDI_FIELD_SZ) );

      //SAB field descriptors: the change the byte offsets (relative to local data)
      p_fld  = dyn_fldar;
      fidx   = 0;
      f_offs = 0; //local data offset

      for (; fidx<8; ++fidx) { //SPM byte0
         p_fld->offs  = f_offs;
         p_fld       += 1;
      }
      f_offs += 1;

      for (; fidx<16; ++fidx) { //SPM byte1
         p_fld->offs  = f_offs;
         p_fld       += 1;
      }
      f_offs += 1;

      for (; fidx<fcount; ++fidx) { //SPM byte2, fcount=24
         p_fld->offs  = f_offs;
         p_fld       += 1;
      }
   }

   retU = init_fields(&dyn_fldar[0], inst_data, fcount, 0, SPM_subg.Name, SPM_subg.Desc, SPM_subg.CodN);
   return retU;
}

//RMCD: SPKD: Speaker Distance
const char  rmcd_spkd_cl::Desc[] = "Speaker Distance";

const subgrp_dsc_t rmcd_spkd_cl::SPKD_subg = {
   .s_ctor   = &rmcd_spkd_cl::group_new,
   .CodN     = "SPKD",
   .Name     = "Speaker Distance",
   .Desc     = Desc,
   .type_id  = ID_RMCD_SPKD | T_DBC_SUBGRP,
   .min_len  = sizeof(spk_plpd_t),
   .fcount   = 3,
   .inst_cnt = 1,
   .fields   = rmcd_spkd_cl::fld_dsc
};

const edi_field_t rmcd_spkd_cl::fld_dsc[] = {
   {&EDID_cl::ByteVal, NULL, 0, 0, 1, EF_BYTE|EF_INT|EF_DM, 0, 255, "Xmax",
   "X-axis distance from Primary Listening Position (PLP) in decimeters [dm]" },
   {&EDID_cl::ByteVal, NULL, 1, 0, 1, EF_BYTE|EF_INT|EF_DM, 0, 255, "Ymax",
   "Y-axis distance from Primary Listening Position (PLP) in decimeters [dm]" },
   {&EDID_cl::ByteVal, NULL, 2, 0, 1, EF_BYTE|EF_INT|EF_DM, 0, 255, "Zmax",
   "Z-axis distance from Primary Listening Position (PLP) in decimeters [dm]" }
};

rcode rmcd_spkd_cl::init(const u8_t* inst, u32_t orflags, edi_grp_cl* parent) {
   rcode retU;

   parent_grp = parent;
   type_id    = SPKD_subg.type_id | orflags;

   CopyInstData(inst, sizeof(spk_plpd_t));

   retU = init_fields(&SPKD_subg.fields[0], inst_data, SPKD_subg.fcount, 0, SPKD_subg.Name, SPKD_subg.Desc, SPKD_subg.CodN);
   return retU;
}

//RMCD: DSPC: Display Coordinates
const char  rmcd_dspc_cl::Desc[] = "Display Coordinates";

const subgrp_dsc_t rmcd_dspc_cl::DSPC_subg = {
   .s_ctor   = &rmcd_dspc_cl::group_new,
   .CodN     = "DSPC",
   .Name     = "Display Coordinates",
   .Desc     = Desc,
   .type_id  = ID_RMCD_DPC | T_DBC_SUBGRP,
   .min_len  = sizeof(disp_xyz_t),
   .fcount   = 3,
   .inst_cnt = 1,
   .fields   = rmcd_dspc_cl::fld_dsc
};

const edi_field_t rmcd_dspc_cl::fld_dsc[] = {
   {&EDID_cl::ByteVal, NULL, 0, 0, 1, EF_BYTE|EF_INT, 0, 255, "DisplayX",
   "X-coordinate value normalized to Xmax in SPKD (Speaker Distance)" },
   {&EDID_cl::ByteVal, NULL, 1, 0, 1, EF_BYTE|EF_INT, 0, 255, "DisplayY",
   "Y-coordinate value normalized to Ymax in SPKD (Speaker Distance)" },
   {&EDID_cl::ByteVal, NULL, 2, 0, 1, EF_BYTE|EF_INT, 0, 255, "DisplayZ",
   "Z-coordinate value normalized to Zmax in SPKD (Speaker Distance)" }
};

rcode rmcd_dspc_cl::init(const u8_t* inst, u32_t orflags, edi_grp_cl* parent) {
   rcode  retU;

   parent_grp = parent;
   type_id    = DSPC_subg.type_id | orflags;

   CopyInstData(inst, sizeof(disp_xyz_t));

   retU = init_fields(&DSPC_subg.fields[0], inst_data, DSPC_subg.fcount, 0, DSPC_subg.Name, DSPC_subg.Desc, DSPC_subg.CodN);
   return retU;
}

//RMCD: Room Configuration Data Block: base class
const char  cea_rmcd_cl::Desc[] =
"Room Configuration Data Block (RMCD) describes playback environment, using room coordinate system.";

const gproot_dsc_t cea_rmcd_cl::RMCD_grp = {
   .CodN     = "RMCD",
   .Name     = "Room Configuration Data Block",
   .Desc     = Desc,
   .type_id  = ID_RMCD,
   .flags    = 0,
   .min_len  = sizeof(rmcd_t)+1,
   .max_len  = sizeof(rmcd_t)+1,
   .hdr_fcnt = CEA_EXTHDR_FCNT,
   .hdr_sz   = sizeof(ethdr_t),
   .grp_arsz = 4,
   .grp_ar   = cea_rmcd_cl::fields[0]
};

const subgrp_dsc_t* cea_rmcd_cl::fields[] = {
   &rmcd_hdr_cl ::DHDR_subg,
   &rmcd_spm_cl ::SPM_subg,
   &rmcd_spkd_cl::SPKD_subg,
   &rmcd_dspc_cl::DSPC_subg
};

rcode cea_rmcd_cl::init(const u8_t* inst, u32_t orflags, edi_grp_cl* parent) {
   rcode  retU;

   orflags |= T_DBC_FIXED;

   retU = base_DBC_Init_RootGrp(inst, &RMCD_grp, orflags, parent);
   return retU;
}

//SLDB: Speaker Location Data Block (DBC_EXT_SLDB = 20)
//SLDB: SLOCD: Speaker Location Descriptor
const char  slocd_cl::Desc[] = "Speaker Location Descriptor";

static const char SLDB_SPK_index[] =
"Speaker index 0..31, CTA-861-G: Table 34:\n"
"0x00 FL\t: Front Left\n"
"0x01 FR\t: Front Right\n"
"0x02 FC\t: Front Center\n"
"0x03 LFE1\t: LFE Low Frequency Effects 1 (subwoofer1)\n"
"0x04 BL\t: Back Left\n"
"0x05 BR\t: Back Right\n"
"0x06 FLC\t: Front Left of Center\n"
"0x07 FRC\t: Front Right of Center\n"
"0x08 BC\t: Back Center\n"
"0x09 LFE2\t: LFE Low Frequency Effects 2 (subwoofer2)\n"
"0x0A SiL\t: Side Left\n"
"0x0B SiR\t: Side Right\n"
"0x0C TpFL\t: Top Front Left\n"
"0x0D TpFR\t: Top Front Right\n"
"0x0E TpFC\t: Top Front Center\n"
"0x0F TpC\t: Top Center\n"
"0x10 TpBL\t: Top Back Left\n"
"0x11 TpBR\t: Top Back Right\n"
"0x12 TpSiL\t: Top Side Left\n"
"0x13 TpSiR\t: Top Side Right\n"
"0x14 TpBC\t: Top Back Center\n"
"0x15 BtFC\t: Bottom Front Center\n"
"0x16 BtFL\t: Bottom Front Left\n"
"0x17 BtFR\t: Bottom Front Right\n"
"0x18 FLW\t: Front Left Wide\n"
"0x19 FRW\t: Front Right Wide\n"
"0x1A LS\t: Left Surround\n"
"0x1B RS\t: Right Surround\n"
"0x1C reserved\n"
"0x1D reserved\n"
"0x1E reserved\n"
"0x1F reserved\n";

const subgrp_dsc_t slocd_cl::SLOCD_subg = {
   .s_ctor   = &slocd_cl::group_new,
   .CodN     = "SLOCD",
   .Name     = "Speaker Location Descriptor",
   .Desc     = Desc,
   .type_id  = ID_SLDB_SLOCD | T_DBC_SUBGRP,
   .min_len  = offsetof(slocd_t, CoordX),
   .fcount   = 11,
   .inst_cnt = -1,
   .fields   = slocd_cl::fld_dsc
};

const edi_field_t slocd_cl::fld_dsc[] = {
   //channel index, byte0
   {&EDID_cl::BitF8Val, NULL, offsetof(slocd_t, channel), 0, 5, EF_BFLD|EF_INT, 0, 0x1F, "Cahnnel_IDX",
   "Channel index 0..31" },
   {&EDID_cl::BitVal, NULL, offsetof(slocd_t, channel), 5, 1, EF_BIT, 0, 3, "Active",
   "1= channel active, 0= channel unused" },
   {&EDID_cl::BitVal, NULL, offsetof(slocd_t, channel), 6, 1, EF_BIT|EF_INIT, 0, 3, "COORD",
   "1= CoordX/Y/Z fields are used, 0= CoordX/Y/Z fields are not present in SLDB" },
   {&EDID_cl::BitVal, NULL, offsetof(slocd_t, channel), 7, 1, EF_BIT, 0, 3, "resvd7",
   "reserved (0)" },
   //speaker index, byte1
   {&EDID_cl::BitF8Val, NULL, offsetof(slocd_t, spk_id), 0, 5, EF_BFLD|EF_INT, 0, 0x1B, "Speaker_ID",
    SLDB_SPK_index },
   {&EDID_cl::BitVal, NULL, offsetof(slocd_t, spk_id), 5, 1, EF_BIT, 0, 3, "resvd5",
   "reserved (0)" },
   {&EDID_cl::BitVal, NULL, offsetof(slocd_t, spk_id), 6, 1, EF_BIT, 0, 3, "resvd6",
   "reserved (0)" },
   {&EDID_cl::BitVal, NULL, offsetof(slocd_t, spk_id), 7, 1, EF_BIT, 0, 3, "resvd7",
   "reserved (0)" },
   //speaker position, bytes 2-4
   {&EDID_cl::ByteVal, NULL, offsetof(slocd_t, CoordX), 0, 1, EF_BYTE|EF_INT, 0, 3, "CoordX",
   "X-axis position value normalized to Xmax in RMCD: SPKD: Speaker Distance (?)" },
   {&EDID_cl::ByteVal, NULL, offsetof(slocd_t, CoordY), 1, 1, EF_BYTE|EF_INT, 0, 3, "CoordY",
   "Y-axis position value normalized to Ymax in RMCD: SPKD: Speaker Distance (?)" },
   {&EDID_cl::ByteVal, NULL, offsetof(slocd_t, CoordZ), 2, 1, EF_BYTE|EF_INT, 0, 3, "CoordZ",
   "Z-axis position value normalized to Zmax in RMCD: SPKD: Speaker Distance (?)" }
};

rcode slocd_cl::init(const u8_t* inst, u32_t orflags, edi_grp_cl* parent) {
   const slocd_t *slocd;

   rcode    retU, retU2;
   u32_t    dlen;
   u32_t    fcount;
   u32_t    unk_byte;

   RCD_SET_OK(retU2);

   slocd      = reinterpret_cast<const slocd_t*> (inst);
   fcount     = SLOCD_subg.fcount;
   dlen       = parent->getFreeSubgSZ();
   unk_byte   = 0;
   parent_grp = parent;
   type_id    = SLOCD_subg.type_id | orflags;

   if (slocd->channel.coord != 0) { //speaker coordinates data present
      if (dlen < sizeof(slocd_t)) {
         wxedid_RCD_SET_FAULT_VMSG(retU2, DBC_BAD_LEN_MSG, SLOCD_subg.CodN);
         unk_byte  = dlen;
         unk_byte -= SLOCD_subg.min_len;
         fcount   -= 3;
      } else {
         dlen = sizeof(slocd_t);
      }
   } else {
      fcount -= 3;
      dlen    = SLOCD_subg.min_len;
   }

   CopyInstData(inst, dlen);

   if (0 == unk_byte) {
      retU = init_fields(&SLOCD_subg.fields[0], inst_data, fcount, 0,
                          SLOCD_subg.Name, SLOCD_subg.Desc, SLOCD_subg.CodN);
   } else {
      edi_field_t *p_fld;

      dyn_fcnt  = fcount;
      dyn_fldar = (edi_field_t*) malloc((dyn_fcnt + unk_byte) * EDI_FIELD_SZ);
      if (NULL == dyn_fldar) RCD_RETURN_FAULT(retU);

      p_fld = dyn_fldar;
      memcpy( p_fld, &SLOCD_subg.fields[0], (dyn_fcnt * EDI_FIELD_SZ) );
      p_fld    += dyn_fcnt;
      dyn_fcnt += unk_byte;

      insert_unk_byte(p_fld, unk_byte, offsetof(slocd_t, CoordX) );

      retU = init_fields(dyn_fldar, inst_data, dyn_fcnt, 0,
                         SLOCD_subg.Name, SLOCD_subg.Desc, SLOCD_subg.CodN);
   }

   if (RCD_IS_OK(retU)) return retU2;
   return retU;
}

//SLDB: Speaker Location Data Block: base class
const char  cea_sldb_cl::Desc[] = "Speaker Location Data Block";

const gproot_dsc_t cea_sldb_cl::SLDB_grp = {
   .CodN     = "SLDB",
   .Name     = "Speaker Location Data Block",
   .Desc     = Desc,
   .type_id  = ID_SLDB,
   .flags    = 0,
   .min_len  = offsetof(slocd_t, CoordX)+1,
   .max_len  = 31,
   .hdr_fcnt = CEA_EXTHDR_FCNT,
   .hdr_sz   = sizeof(ethdr_t),
   .grp_arsz = 1,
   .grp_ar   = &slocd_cl::SLOCD_subg
};

rcode cea_sldb_cl::init(const u8_t* inst, u32_t orflags, edi_grp_cl* parent) {
   rcode  retU;

   retU = base_DBC_Init_RootGrp(inst, &SLDB_grp, orflags, parent);
   return retU;
}

//IFDB: InfoFrame Data Block (DBC_EXT_IFDB = 32)
//IFDB: IFPD: InfoFrame Processing Descriptor Header
const char  ifdb_ifpdh_cl::Desc[] = "InfoFrame Processing Descriptor";

const subgrp_dsc_t ifdb_ifpdh_cl::IFPD_subg = {
   .s_ctor   = NULL,
   .CodN     = "IFPD",
   .Name     = "InfoFrame Processing Descriptor",
   .Desc     = Desc,
   .type_id  = ID_IFDB_IFPD,
   .min_len  = sizeof(ifpdh_t),
   .fcount   = 3,
   .inst_cnt = 1,
   .fields   = ifdb_ifpdh_cl::fld_dsc
};

const edi_field_t ifdb_ifpdh_cl::fld_dsc[] = {
   {&EDID_cl::BitF8Val, NULL, 0, 0, 5, EF_BFLD|EF_INT|EF_RD, 0, 0x1F, "resvd04",
    "reserved (0)" },
   {&EDID_cl::BitF8Val, NULL, 0, 5, 3, EF_BFLD|EF_INT|EF_INIT, 0, 7, "Blk length",
    "Block length: num of bytes following the 'n_VSIFs' byte.\n"
    "For CTA-861-G the payload length should be zero." },
   {&EDID_cl::ByteVal, NULL, offsetof(ifpdh_t, n_VSIFs), 0, 1, EF_BYTE|EF_INT, 0, 0xFF, "n_VSIFs",
   "num of additional Vendor-Specific InfoFrames (VSIFs) that can be received simultaneously."
   "The number is encoded as (n_VSIFs - 1), so 0 means 1." }
};

rcode ifdb_ifpdh_cl::init(const u8_t* inst, u32_t orflags, edi_grp_cl* parent) {
   rcode  retU;

   retU = IFDB_Init_SubGrp(inst, &IFPD_subg, orflags, parent);
   return retU;
}

//IFDB: SIFD: Short InfoFrame Descriptor, InfoFrame Type Code != 0x00, 0x01
const char  ifdb_sifd_cl::Desc[] = "Short InfoFrame Descriptor";

const subgrp_dsc_t ifdb_sifd_cl::SIFD_subg = {
   .s_ctor   = NULL,
   .CodN     = "SIFD",
   .Name     = "Short InfoFrame Descriptor",
   .Desc     = Desc,
   .type_id  = ID_IFDB_SIFD,
   .min_len  = sizeof(sifdh_t),
   .fcount   = 2,
   .inst_cnt = 1,
   .fields   = ifdb_sifd_cl::fld_dsc
};

const edi_field_t ifdb_sifd_cl::fld_dsc[] = {
   {&EDID_cl::BitF8Val, NULL, 0, 0, 5, EF_BFLD|EF_INT|EF_INIT, 0, 0x1F, "IFT Code",
    "InfoFrame Type Code, values 0x00, 0x01 are reserved for other types of descriptors." },
   {&EDID_cl::BitF8Val, NULL, 0, 5, 3, EF_BFLD|EF_INT|EF_INIT, 0, 7, "Blk length",
    "Block length: num of bytes following the header byte." }
};

rcode ifdb_sifd_cl::init(const u8_t* inst, u32_t orflags, edi_grp_cl* parent) {
   rcode  retU;

   retU = IFDB_Init_SubGrp(inst, &SIFD_subg, orflags, parent);
   return retU;
}

//IFDB: VSIFD: Short Vendor-Specific InfoFrame Descriptor, InfoFrame Type Code = 0x01
const char  ifdb_vsifd_cl::Desc[] = "Short Vendor-Specific InfoFrame Descriptor";

const subgrp_dsc_t ifdb_vsifd_cl::VSIFD_subg = {
   .s_ctor   = NULL,
   .CodN     = "VSIFD",
   .Name     = "Short Vendor-Specific InfoFrame Descriptor",
   .Desc     = Desc,
   .type_id  = ID_IFDB_VSD,
   .min_len  = sizeof(svsifd_t),
   .fcount   = 3,
   .inst_cnt = 1,
   .fields   = ifdb_vsifd_cl::fld_dsc
};

const edi_field_t ifdb_vsifd_cl::fld_dsc[] = {
   {&EDID_cl::BitF8Val, NULL, offsetof(svsifd_t, ifhdr), 0, 5, EF_BFLD|EF_INT|EF_INIT, 0, 0x1F, "IFT Code",
    "Vendor-Specific InfoFrame Type Code == 0x01." },
   {&EDID_cl::BitF8Val, NULL, offsetof(svsifd_t, ifhdr), 5, 3, EF_BFLD|EF_INT|EF_INIT, 0, 7, "Blk length",
    "Block length: num of bytes following the header byte." },
   //IEEE OUI
   {&EDID_cl::ByteStr, NULL, offsetof(svsifd_t, ieee_id), 0, 3, EF_STR|EF_HEX|EF_LE|EF_RD, 0, 0xFFFFFF, "IEEE-OUI",
   "IEEE OUI (Organizationally Unique Identifier)" }
};

rcode ifdb_vsifd_cl::init(const u8_t* inst, u32_t orflags, edi_grp_cl* parent) {
   rcode  retU;

   retU = IFDB_Init_SubGrp(inst, &VSIFD_subg, orflags, parent);
   return retU;
}

//IFDB: InfoFrame Data Block: base class
const char  cea_ifdb_cl::CodN[] = "IFDB";
const char  cea_ifdb_cl::Name[] = "InfoFrame Data Block";
const char  cea_ifdb_cl::Desc[] = "InfoFrame Data Block";

rcode cea_ifdb_cl::init(const u8_t* inst, u32_t orflags, edi_grp_cl* parent) {
   rcode        retU, retU2;
   u32_t        dlen;
   u32_t        g_type;
   u32_t        grp_sz;
   u32_t        abso;
   u32_t        relo;
   u8_t        *g_inst;
   edi_grp_cl  *pgrp;
   bool         b_edit_mode;

   RCD_SET_OK(retU2);

   b_edit_mode = (0 != (T_MODE_EDIT & orflags));
   parent_grp  = parent;
   type_id     = ID_IFDB;

   dlen    = reinterpret_cast <const ifdb_t*> (inst)->ethdr.ehdr.hdr.tag.blk_len;
   dlen   ++ ; //hdr
   grp_sz  = dlen;
   //sizeof(ethdr_t) + 2 bytes IFPDH
   if (dlen < 4) {
      if (! b_edit_mode) RCD_RETURN_FAULT(retU);
      wxedid_RCD_SET_FAULT_VMSG(retU2, DBC_BAD_LEN_MSG, CodN);
      grp_sz = 32;
   }

   CopyInstData(inst, grp_sz);
   hdr_sz  = sizeof(ethdr_t);

   retU = init_fields(&CEA_BlkHdr_fields[0], inst_data, CEA_EXTHDR_FCNT, orflags, Name, Desc, CodN);
   if (! RCD_IS_OK(retU)) return retU;

   subg_sz = dlen;
   if (subg_sz  > hdr_sz) {
      subg_sz -= hdr_sz;
   } else {
      subg_sz  = 0;
   }
   if (subg_sz == 0) goto grp_empty;
   g_inst  = inst_data;
   abso    = abs_offs;
   abso   += hdr_sz;
   relo    = hdr_sz;
   g_inst += hdr_sz;

   if (subg_sz < ifdb_ifpdh_cl::IFPD_subg.min_len) {
      wxedid_RCD_SET_FAULT_VMSG(retU2, DBC_BAD_LEN_MSG, CodN);
      goto unk;
   }

   //create and init sub-groups: 2: IFPDH + 1 short descriptor
   //IFPDH
   pgrp = new ifdb_ifpdh_cl;
   if (pgrp == NULL) RCD_RETURN_FAULT(retU);
   retU2 = pgrp->init(g_inst, (ID_IFDB | T_DBC_SUBGRP | T_DBC_FIXED | orflags), this );
   if (! RCD_IS_OK(retU2)) goto unk;

   pgrp->setAbsOffs(abso);
   pgrp->setRelOffs(relo);
   subgroups.Append(pgrp);

   grp_sz  = pgrp->getTotalSize();
   relo   += grp_sz;

   while (relo < dlen) {

      g_inst  += grp_sz;
      abso    += grp_sz;
      subg_sz -= grp_sz;
      g_type   = reinterpret_cast <sifdh_t*> (g_inst)->ift_code;

      if (g_type == 0x01) { //Short Vendor-Specific InfoFrame Descriptor
         pgrp = new ifdb_vsifd_cl;
         if (pgrp == NULL) RCD_RETURN_FAULT(retU);
         retU2 = pgrp->init(g_inst, (ID_IFDB | T_DBC_SUBGRP | orflags), this );

      } else { //Check InfoFrame Type Code
         if (g_type == 0x00) {
            wxedid_RCD_SET_FAULT_VMSG(retU2, "[E!] %s: Bad InfoFrame Type Code", CodN);
            if (! b_edit_mode) return retU2;
            goto unk;
         }
         //Short InfoFrame Descriptor
         pgrp = new ifdb_sifd_cl;
         if (pgrp == NULL) RCD_RETURN_FAULT(retU);
         retU2 = pgrp->init(g_inst, (ID_IFDB | T_DBC_SUBGRP | orflags), this );
      }
      if (! RCD_IS_OK(retU2)) goto unk;

      pgrp->setAbsOffs(abso);
      pgrp->setRelOffs(relo);
      subgroups.Append(pgrp);

      grp_sz  = pgrp->getTotalSize();
      relo   += grp_sz;
   }

   if (relo > dlen) wxedid_RCD_SET_FAULT_VMSG(retU2, DBC_BAD_LEN_MSG, CodN);

unk:
   if (! RCD_IS_OK(retU2)) { //blk_len too small or bad InfoFrame Type Code
      retU = Append_UNK_DAT(g_inst, subg_sz, orflags, abso, relo, this);
   }

grp_empty:
   subgroups.CalcDataSZ(this);

   if (RCD_IS_OK(retU)) return retU2;
   return retU;
}

//common init fn for IFDB sub-groups
rcode edi_grp_cl::IFDB_Init_SubGrp(const u8_t* inst, const subgrp_dsc_t* pSGDsc, u32_t orflags, edi_grp_cl* parent) {
   rcode        retU, retU2;
   u32_t        gplen;
   u32_t        blklen;
   u32_t        fcount;
   edi_field_t *p_fld;
   bool         b_edit_mode;

   RCD_SET_OK(retU2);

   b_edit_mode = (0 != (T_MODE_EDIT & orflags));
   parent_grp  = parent;
   type_id     = pSGDsc->type_id | orflags;
   fcount      = pSGDsc->fcount;

   //block length:
   //sifdh_t, svsifd_t, ifpdh_t have the block length encoded in the same way,
   //so it doesn't matter which of those will be used here.
   blklen = reinterpret_cast <const sifdh_t*> (inst)->blk_len;
   gplen  = blklen;
   gplen += pSGDsc->min_len;

   if (gplen > parent->getFreeSubgSZ()) {
      if (! b_edit_mode) RCD_RETURN_FAULT(retU);
      wxedid_RCD_SET_FAULT_VMSG(retU2, DBC_BAD_LEN_MSG, pSGDsc->CodN);
      if (blklen > pSGDsc->min_len) return retU2; //insert UNK-DAT group
      gplen = parent->getFreeSubgSZ();
   }

   CopyInstData(inst, gplen);

   if (gplen > pSGDsc->min_len) {
      gplen -= pSGDsc->min_len; //payload size
   } else {
      gplen = 0;
   }

   //pre-alloc buffer for array of fields: fcount + gplen
   dyn_fldar = (edi_field_t*) malloc( (gplen + fcount) * EDI_FIELD_SZ );
   if (NULL == dyn_fldar) RCD_RETURN_FAULT(retU);

   p_fld = dyn_fldar;

   memcpy( p_fld, &pSGDsc->fields[0], (fcount * EDI_FIELD_SZ) );
   p_fld    += fcount;
   dyn_fcnt  = fcount;
   dyn_fcnt += gplen;

   //payload data interpreted as unknown
   insert_unk_byte(p_fld, gplen, pSGDsc->min_len);

   retU = init_fields(&dyn_fldar[0], inst_data, dyn_fcnt, 0, pSGDsc->Name, pSGDsc->Desc, pSGDsc->CodN);

   if (RCD_IS_OK(retU)) return retU2;
   return retU;
}


//UNK-ET: Unknown Data Block (Extended Tag Code)
const char  cea_unket_cl::CodN[] = "UNK-ET";
const char  cea_unket_cl::Name[] = "Unknown Data Block";
const char  cea_unket_cl::Desc[] = "Unknown Extended Tag Code";

#pragma GCC diagnostic ignored "-Wunused-parameter"
rcode cea_unket_cl::init(const u8_t* inst, u32_t orflags, edi_grp_cl* parent) {
   rcode        retU;
   u32_t        dlen;
   edi_field_t *p_fld;

   //block length
   dlen   = reinterpret_cast <const ethdr_t*> (inst)->ehdr.hdr.tag.blk_len;

   parent_grp = parent;
   type_id    = ID_CEA_UETC; //unknown Extended Tag Code
   dlen      ++ ; //hdr

   CopyInstData(inst, dlen);

   dlen     -= 2; // -hdr, -ext_tag
   dyn_fcnt  = CEA_EXTHDR_FCNT;

   //pre-alloc buffer for array of fields: hdr_fcnt + dlen
   dyn_fldar = (edi_field_t*) malloc( (dlen + dyn_fcnt) * EDI_FIELD_SZ );
   if (NULL == dyn_fldar) RCD_RETURN_FAULT(retU);

   p_fld   = dyn_fldar;

   //DBC-EXT header
   memcpy( p_fld, CEA_BlkHdr_fields, (dyn_fcnt * EDI_FIELD_SZ) );
   p_fld  += dyn_fcnt;

   dyn_fcnt += dlen;

   //payload data interpreted as unknown
   insert_unk_byte(p_fld, dlen, sizeof(ethdr_t) );

   retU = init_fields(&dyn_fldar[0], inst_data, dyn_fcnt, 0, Name, Desc, CodN);
   return retU;
}
#pragma GCC diagnostic warning "-Wunused-parameter"

