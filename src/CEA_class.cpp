/***************************************************************
 * Name:      EDID_CEA_class.cpp
 * Purpose:   EDID CEA-861 extension
 * Author:    Tomasz Pawlak (tomasz.pawlak@wp.eu)
 * Created:   2014-05-08
 * Copyright: Tomasz Pawlak (C) 2014-2022
 * License:   GPLv3+
 **************************************************************/

#include "debug.h"
#include "rcdunits.h"
#ifndef idCEA
   #error "CEA_class.cpp: missing unit ID"
#endif
#define RCD_UNIT idCEA
#include "rcode/rcode.h"

#include "wxedid_rcd_scope.h"

RCD_AUTOGEN_DEFINE_UNIT

#include <stddef.h>

#include "CEA_class.h"

//CHD: CEA Header
const char  cea_hdr_cl::CodN[] = "CHD";
const char  cea_hdr_cl::Name[] = "CEA-861 header";
const char  cea_hdr_cl::Desc[] = "CEA/CTA-861 header";

const edi_field_t cea_hdr_cl::fields[] = {
   {&EDID_cl::ByteVal, NULL, offsetof(cea_hdr_t, ext_tag), 0, 1, EF_BYTE|EF_HEX|EF_RD, 0, 0xFF, "Extension tag",
   "Extension type: 0x02 for CEA-861" },
   {&EDID_cl::ByteVal, NULL, offsetof(cea_hdr_t, rev), 0, 1, EF_BYTE|EF_INT|EF_RD, 0, 0xFF, "Revision",
   "Extension revision" },
   {&EDID_cl::ByteVal, NULL, offsetof(cea_hdr_t, dtd_offs), 0, 1, EF_BYTE|EF_HEX|EF_RD, 0, 0xFF, "DTD offset",
   "Offset to first DTD:\n"
   "dtd_offs = 0 -> No DBC data and no DTD sections\n"
   "dtd_offs = 4 -> No DBC data, DTD sections available\n"
   "dtd_offs > 4 -> Both DBC data and DTD sections available\n"
    },
   {&EDID_cl::BitF8Val, NULL, offsetof(cea_hdr_t, info_blk), 0, 4, EF_BFLD|EF_INT|EF_RD, 0, 0x0F, "num_dtd",
   "Byte3, bits 3-0: number of native DTD sections in CEA extension.\n"
   "Note: there can be additional non-native DTDs present at the end of the block." },
   {&EDID_cl::BitVal, NULL, offsetof(cea_hdr_t, info_blk), 4, 1, EF_BIT, 0, 1, "YCbCr 4:2:2",
   "1 = supported." },
   {&EDID_cl::BitVal, NULL, offsetof(cea_hdr_t, info_blk), 5, 1, EF_BIT, 0, 1, "YCbCr 4:4:4",
   "1 = supported." },
   {&EDID_cl::BitVal, NULL, offsetof(cea_hdr_t, info_blk), 6, 1, EF_BIT, 0, 1, "Basic Audio",
   "1 = supports basic audio." },
   {&EDID_cl::BitVal, NULL, offsetof(cea_hdr_t, info_blk), 7, 1, EF_BIT, 0, 1, "Underscan",
   "1 = supports underscan." },
   {&EDID_cl::ByteVal, NULL, offsetof(cea_hdr_t, info_blk)+1, 0, 1, EF_BYTE|EF_HEX|EF_RD, 0, 0xFF, "checksum",
   "Block checksum: sum of all bytes mod 256 must equal zero." }
};

#pragma GCC diagnostic ignored "-Wunused-parameter"
rcode cea_hdr_cl::init(const u8_t* inst, u32_t orflags, edi_grp_cl* parent) {
   static const u32_t fcount = 9;

   rcode  retU;
   u32_t  ext_tag;
   u32_t  rev;

   ext_tag = reinterpret_cast <const cea_hdr_t*> (inst)->ext_tag;
   rev     = reinterpret_cast <const cea_hdr_t*> (inst)->rev;

   if (ext_tag != 0x02) { //CEA tag
      wxedid_RCD_RETURN_FAULT_VMSG(retU, "[E!] CEA/CTA-861: unknown header tag=0x%02X", ext_tag);
   }

   if ((rev > 3) || (rev < 1)) { //CEA revision
      wxedid_RCD_RETURN_FAULT_VMSG(retU, "[E!] CEA/CTA-861: unknown revision=%u", rev);
   }

   parent_grp = parent;
   type_id    = ID_CHD | T_EDID_FIXED;

   CopyInstData(inst, sizeof(cea_hdr_t)-1 ); //-1 for cea_hdr_t.dta_start
   hdr_sz              = dat_sz;
   inst_data[dat_sz++] = inst[127]; //chksum

   retU = init_fields(&fields[0], inst_data, fcount, 0, Name, Desc, CodN);
   return retU;
}
#pragma GCC diagnostic warning "-Wunused-parameter"

void cea_hdr_cl::SpawnInstance(u8_t *pinst) {
   u32_t   dsz;

   dsz    = dat_sz;
   dsz   -= 1; //chksum

   memcpy(pinst, inst_data, dsz);

   pinst[127] = inst_data[dsz];
}

//CEA bad block length msg
extern const char DBC_BAD_LEN_MSG[];
       const char DBC_BAD_LEN_MSG[] = "[E!] %s: bad block length";

//CEA header field descriptions
extern const char CEA_BlkHdr_dscF0[];
extern const char CEA_BlkHdr_dscF1[];
extern const char CEA_BlkHdr_dscF2[];
extern const edi_field_t CEA_BlkHdr_fields[];

extern const edi_field_t unknown_byte_fld;
extern void insert_unk_byte(edi_field_t *p_fld, u32_t len, u32_t s_offs);

const char CEA_BlkHdr_dscF0[] =
"Total number of bytes in the block excluding 1st header byte. "
"If Block Type Tag = 7 (extended type tag), then the header takes 2 bytes. "
"The 2nd header byte contains the extended tag value.";
const char CEA_BlkHdr_dscF1[] =
"Block Type, Tag Code: \n"
"0: reserved\n"
"1: ADB: Audio Data Block\n"
"2: VDB: Video Data Block\n"
"3: VSD: Vendor Specific Data Block\n"
"4: SAB: Speaker Allocation Data Block\n"
"5: VDT: VESA Display Transfer Characteristic Data Block\n"
"6: reserved\n"
"7: EXT: Use Extended Tag\n";
const char CEA_BlkHdr_dscF2[] =
"Extended Tag Codes:\n"
"00: Video Capability Data Block\n"
"01: Vendor-Specific Video Data Block\n"
"02: VESA Display Device Data Block\n"
"03: VESA Video Timing Block Extension\n"
"04: ! Reserved for HDMI Video Data Block\n"
"05: Colorimetry Data Block\n"
"06: HDR Static Metadata Data Block\n"
"07: HDR Dynamic Metadata Data Block\n"
"08..12 ! Reserved for video-related blocks\n"
"13: Video Format Preference Data Block\n"
"14: YCBCR 4:2:0 Video Data Block\n"
"15: YCBCR 4:2:0 Capability Map Data Block\n"
"16: ! Reserved for CTA Miscellaneous Audio Fields\n"
"17: Vendor-Specific Audio Data Block\n"
"18: Reserved for HDMI Audio Data Block\n"
"19: Room Configuration Data Block\n"
"20: Speaker Location Data Block\n"
"21..31 ! Reserved for audio-related blocks\n"
"32: InfoFrame Data Block (includes one or more Short InfoFrame Descriptors)\n"
"33..255 ! Reserved";

//CEA-DBC Tag Code selector
const vname_map_t dbc_tag_map[] = {
   {0, NULL},
   {1, "ADB: Audio Data Block"},
   {2, "VDB: Video Data Block"},
   {3, "VSD: Vendor Specific Data Block"},
   {4, "SAB: Speaker Allocation Data Block"},
   {5, "VDTC: VESA Display Transfer Characteristic Data Block"},
   {6, NULL},
   {7, "EXT: Extended Tag Code"},
};

const vmap_t DBC_Tag_sel = {
   0, 8,
   dbc_tag_map
};

//CEA-DBC Extended Tag Code selector
const vname_map_t dbc_ext_tag_map[] = {
   { 0, "VCDB: Video Capability Data Block"},
   { 1, "VSVD: Vendor-Specific Video Data Block"},
   { 2, "VDDD: VESA Display Device Data Block"},
   { 3, "VVTB: VESA Video Timing Block Extension (BUG!)"},
   { 4, NULL},
   { 5, "CLDB: Colorimetry Data Block"},
   { 6, "HDRS: HDR Static Metadata Data Block"},
   { 7, "HDRD: HDR Dynamic Metadata Data Block"},
   { 8, NULL},
   { 9, NULL},
   {10, NULL},
   {11, NULL},
   {12, NULL},
   {13, "VFPD: Video Format Preference Data Block"},
   {14, "Y42V: YCBCR 4:2:0 Video Data Block"},
   {15, "Y42C: YCBCR 4:2:0 Capability Map Data Block"},
   {16, NULL},
   {17, "VSAD: Vendor-Specific Audio Data Block"},
   {18, NULL},
   {19, "RMCD: Room Configuration Data Block"},
   {20, "SLDB: Speaker Location Data Block"},
   {21, NULL},
   {22, NULL},
   {23, NULL},
   {24, NULL},
   {25, NULL},
   {26, NULL},
   {27, NULL},
   {28, NULL},
   {29, NULL},
   {30, NULL},
   {31, NULL},
   {32, "IFDB: InfoFrame Data Block"},
};

const vmap_t DBC_ExtTag_sel = {
   0, 33,
   dbc_ext_tag_map
};

//CEA-DBC Header fields:
const edi_field_t CEA_BlkHdr_fields[] = {
   {&EDID_cl::CEA_DBC_Len, NULL, 0, 0, 5, EF_BFLD|EF_INT|EF_RD|EF_FGR, 0, 0x1F, "Blk length",
    CEA_BlkHdr_dscF0 },
   {&EDID_cl::CEA_DBC_Tag, &DBC_Tag_sel, 0, 5, 3, EF_BFLD|EF_INT|EF_RD|EF_FGR|EF_VS, 0, 0x07, "Tag Code",
    CEA_BlkHdr_dscF1 },
   //Extended Tag Code field is included conditionally, depending on Tag Code
   {&EDID_cl::CEA_DBC_ExTag, &DBC_ExtTag_sel, 1, 0, 1, EF_BYTE|EF_INT|EF_RD|EF_FGR|EF_VS, 0, 0xFF, "Ext Tag Code",
    CEA_BlkHdr_dscF2 }
};

//unknown/invalid byte: (offset has to be modified accordingly)
const edi_field_t unknown_byte_fld =
   {&EDID_cl::ByteVal, NULL, 0, 0, 1, EF_BYTE|EF_HEX|EF_RD, 0, 0xFF, "unknown", "Unknown data" };

rcode EDID_cl::CEA_DBC_Tag(u32_t op, wxString& sval, u32_t& ival, edi_dynfld_t* p_field) {
   rcode   retU;

   if (op == OP_READ) {
      retU = BitF8Val(op, sval, ival, p_field);

   } else {
      u32_t   blkTag;
      bhdr_t *phdr;

      phdr   = reinterpret_cast<bhdr_t*> ( getValPtr(p_field) );
      blkTag = phdr->tag.tag_code;
      retU   = BitF8Val(op, sval, ival, p_field);

      if (RCD_IS_OK(retU)) {
         if (blkTag != (u32_t) phdr->tag.tag_code) RCD_RETURN_TRUE(retU);
         //True means block type changed -> the BlockTree needs to be updated.
      }
   }
   return retU;
}

rcode EDID_cl::CEA_DBC_ExTag(u32_t op, wxString& sval, u32_t& ival, edi_dynfld_t* p_field) {
   rcode  retU;
   if (op == OP_READ) {
      retU = ByteVal(op, sval, ival, p_field);
   } else {
      u8_t   blkExTag;
      u8_t  *ptag;

      ptag     = getValPtr(p_field);
      blkExTag = *ptag;
      retU     = ByteVal(op, sval, ival, p_field);

      if (RCD_IS_OK(retU)) {
         if (blkExTag != *ptag) RCD_RETURN_TRUE(retU);
      }
   }
   return retU;
}

rcode EDID_cl::CEA_DBC_Len(u32_t op, wxString& sval, u32_t& ival, edi_dynfld_t* p_field) {
   rcode   retU;
   bhdr_t *phdr;

   phdr = reinterpret_cast<bhdr_t*> ( getValPtr(p_field) );

   if (op == OP_READ) {
      retU = BitF8Val(op, sval, ival, p_field);
   } else {
      u32_t  blklen;

      blklen = phdr->tag.blk_len;
      retU   = BitF8Val(op, sval, ival, p_field);

      if (RCD_IS_OK(retU)) {
         if (blklen != (u32_t) phdr->tag.blk_len) RCD_RETURN_TRUE(retU);
      }
   }
   return retU;
}

//fallback: insert "unknown byte" field: display unknown data instead of error
void insert_unk_byte(edi_field_t *p_fld, u32_t len, u32_t s_offs) {
   for (u32_t cnt=0; cnt<len; ++cnt) {

      memcpy( p_fld, &unknown_byte_fld, EDI_FIELD_SZ );

      p_fld->offs = s_offs;

      s_offs ++ ;
      p_fld  ++ ;
   }
}


//ADB: Audio Data Block
//ADB: Audio Data Block : audio format codes : selector
const vname_map_t adb_audiofmt_map[] = {
   { 0, "Reserved"},
   { 1, "LPCM"},
   { 2, "AC-3"},
   { 3, "MPEG1 (Layers 1 and 2)"},
   { 4, "MP3"},
   { 5, "MPEG2"},
   { 6, "AAC"},
   { 7, "DTS"},
   { 8, "ATRAC"},
   { 9, "SACD"},
   {10, "DD+"},
   {11, "DTS-HD"},
   {12, "MLP/Dolby TrueHD"},
   {13, "DST Audio"},
   {14, "Microsoft WMA Pro"},
   {15, "ACE: Audio Coding Extension Type Code"}
};

const vmap_t ADB_audiofmt = {
   0, 16,
   adb_audiofmt_map
};

const char  cea_adb_cl::Desc[] =
"Audio Data Block contains one or more 3-byte "
"Short Audio Descriptors (SADs). Each SAD defines audio format, channel number, "
"and bitrate/resolution capabilities of the display";

const gproot_dsc_t cea_adb_cl::ADB_grp = {
   .CodN     = "ADB",
   .Name     = "Audio Data Block",
   .Desc     = Desc,
   .type_id  = ID_ADB,
   .flags    = 0,
   .min_len  = sizeof(sad_t),
   .max_len  = 31,
   .hdr_fcnt = CEA_DBCHDR_FCNT,
   .hdr_sz   = sizeof(bhdr_t),
   .grp_arsz = 1,
   .grp_ar   = &cea_sad_cl::SAD_subg
};

rcode cea_adb_cl::init(const u8_t* inst, u32_t orflags, edi_grp_cl* parent) {
   rcode  retU;

   retU = base_DBC_Init_RootGrp(inst, &ADB_grp, orflags, parent);
   return retU;
}

//ADB: Audio Data Block ->
//SAD: Short Audio Descriptor
const char  cea_sad_cl::Desc[] =
"SAD describes audio format, channel number, "
"and sample bitrate/resolution capabilities of the display";

static const char freqSuppDsc[] = "SAD byte 1, bit flag =1: sampling frequency is supported.";
static const char sadAFC_Desc[] =
   "Audio Format Codes:\n"
   "00: reserved\n"
   "01: LPCM (Linear Pulse Code Modulation)\n"
   "02: AC-3\n"
   "03: MPEG1 (Layers 1 and 2)\n"
   "04: MP3\n"
   "05: MPEG2\n"
   "06: AAC\n"
   "07: DTS\n"
   "08: ATRAC\n"
   "09: SACD\n"
   "10: DD+\n"
   "11: DTS-HD\n"
   "12: MLP/Dolby TrueHD\n"
   "13: DST Audio\n"
   "14: Microsoft WMA Pro\n"
   "15: Audio Coding Extension Type Code is used (ACE)";
//const u32_t cea_sad_cl::fcount = 16;

/* SAD (Short Audio Descriptor) uses different data layouts, depending
   on Audio Format Code (AFC) and Audio Coding Extension Type Code (ACE).
   All the possible data layouts are defined here, and the final SAD is
   constructed by cea_sad_cl::gen_data_layout(), which is also setting the
   cea_sad_cl::fcount variable.
*/

const subgrp_dsc_t cea_sad_cl::SAD_subg = {
   .s_ctor   = &cea_sad_cl::group_new,
   .CodN     = "SAD",
   .Name     = "Short Audio Descriptor",
   .Desc     = Desc,
   .type_id  = ID_SAD | T_DBC_SUBGRP,
   .min_len  = sizeof(sad_t),
   .fcount   = 0,
   .inst_cnt = -1,
   .fields   = {}
};

//SAD byte 0: AFC = 1...14
const edi_field_t cea_sad_cl::byte0_afc1_14[] {
   {&EDID_cl::BitF8Val, NULL, offsetof(sad_t, sad0), 0, 3, EF_BFLD|EF_INT, 0, 8, "num_chn",
   "Bitfield of 3 bits: number of channels minus 1 -> f.e. 0 = 1 channel; 7 = 8 channels." },
   {&EDID_cl::BitF8Val, &ADB_audiofmt, offsetof(sad_t, sad0), 3, 4, EF_BFLD|EF_INT|EF_VS|EF_FGR, 0, 0x0F, "AFC",
    sadAFC_Desc },
   {&EDID_cl::BitVal, NULL, offsetof(sad_t, sad0), 7, 1, EF_BIT|EF_RD, 0, 1, "reserved",
   "reserved (0)" }
};
//SAD byte 0: AFC = 15 && ACE = 11,12 (MPEG-H 3D Audio, AC-4)
const edi_field_t cea_sad_cl::byte0_afc15_ace11_12[] {
   {&EDID_cl::BitF8Val, NULL, offsetof(sad_t, sad0), 0, 3, EF_BFLD|EF_INT, 0, 8, "reserved",
   "reserved (0)" },
   {&EDID_cl::BitF8Val, &ADB_audiofmt, offsetof(sad_t, sad0), 3, 4, EF_BFLD|EF_INT|EF_VS|EF_FGR, 0, 0x0F, "AFC",
    sadAFC_Desc },
   {&EDID_cl::BitVal, NULL, offsetof(sad_t, sad0), 7, 1, EF_BIT|EF_RD, 0, 1, "reserved",
   "reserved (0)" }
};
//SAD byte 0: AFC = 15 && ACE = 13 (L-PCM 3D Audio)
const edi_field_t cea_sad_cl::byte0_afc15_ace13[] {
   {&EDID_cl::SAD_LPCM_MC, NULL, offsetof(sad_t, sad0), 0, 3, EF_BFLD|EF_INT, 0, 32, "num_chn",
   "Set of 5 bits (scattered - not a bitfield), number of channels minus 1 - i.e. 0 -> 1 channel; "
   "7 -> 8 channels.\n"
   "Used only with L-PCM 3D Audio." },
   {&EDID_cl::BitF8Val, &ADB_audiofmt, offsetof(sad_t, sad0), 3, 4, EF_BFLD|EF_INT|EF_VS|EF_FGR, 0, 0x0F, "AFC",
    sadAFC_Desc },
};

//SAD byte 1: AFC = 1...14 || AFC = 15 && ACE = 11 (MPEG-H 3D Audio)
const edi_field_t cea_sad_cl::byte1_afc1_14_ace11[] = {
   {&EDID_cl::BitVal, NULL, offsetof(sad_t, sad1), 0, 1, EF_BIT, 0, 1, "sf_32kHz",
    freqSuppDsc },
   {&EDID_cl::BitVal, NULL, offsetof(sad_t, sad1), 1, 1, EF_BIT, 0, 1, "sf_44.1kHz",
    freqSuppDsc },
   {&EDID_cl::BitVal, NULL, offsetof(sad_t, sad1), 2, 1, EF_BIT, 0, 1, "sf_48kHz",
    freqSuppDsc },
   {&EDID_cl::BitVal, NULL, offsetof(sad_t, sad1), 3, 1, EF_BIT, 0, 1, "sf_88.2kHz",
    freqSuppDsc },
   {&EDID_cl::BitVal, NULL, offsetof(sad_t, sad1), 4, 1, EF_BIT, 0, 1, "sf_96kHz",
    freqSuppDsc },
   {&EDID_cl::BitVal, NULL, offsetof(sad_t, sad1), 5, 1, EF_BIT, 0, 1, "sf_176.4kHz",
    freqSuppDsc },
   {&EDID_cl::BitVal, NULL, offsetof(sad_t, sad1), 6, 1, EF_BIT, 0, 1, "sf_192kHz",
    freqSuppDsc },
   {&EDID_cl::BitVal, NULL, offsetof(sad_t, sad1), 7, 1, EF_BIT|EF_RD, 0, 1, "reserved",
   "reserved (0)" }
};
//SAD byte 1: AFC = 15 && ACE = 4,5,6,8,10
const edi_field_t cea_sad_cl::byte1_afc15_ace456810[] = {
   {&EDID_cl::BitVal, NULL, offsetof(sad_t, sad1), 0, 1, EF_BIT, 0, 1, "sf_32kHz",
    freqSuppDsc },
   {&EDID_cl::BitVal, NULL, offsetof(sad_t, sad1), 1, 1, EF_BIT, 0, 1, "sf_44.1kHz",
    freqSuppDsc },
   {&EDID_cl::BitVal, NULL, offsetof(sad_t, sad1), 2, 1, EF_BIT, 0, 1, "sf_48kHz",
    freqSuppDsc },
   {&EDID_cl::BitVal, NULL, offsetof(sad_t, sad1), 3, 1, EF_BIT, 0, 1, "sf_88.2kHz",
    freqSuppDsc },
   {&EDID_cl::BitVal, NULL, offsetof(sad_t, sad1), 4, 1, EF_BIT, 0, 1, "sf_96kHz",
    freqSuppDsc },
   {&EDID_cl::BitVal, NULL, offsetof(sad_t, sad1), 5, 1, EF_BIT|EF_RD, 0, 1, "reserved",
   "reserved (0)" },
   {&EDID_cl::BitVal, NULL, offsetof(sad_t, sad1), 6, 1, EF_BIT|EF_RD, 0, 1, "reserved",
   "reserved (0)" },
   {&EDID_cl::BitVal, NULL, offsetof(sad_t, sad1), 7, 1, EF_BIT|EF_RD, 0, 1, "reserved",
   "reserved (0)" }
};
//SAD byte 1: //AFC = 15 && ACE = 12 (AC-4)
const edi_field_t cea_sad_cl::byte1_afc15_ace12_fsmp[] = {
   {&EDID_cl::BitVal, NULL, offsetof(sad_t, sad1), 0, 1, EF_BIT|EF_RD, 0, 1, "reserved",
   "reserved (0)" },
   {&EDID_cl::BitVal, NULL, offsetof(sad_t, sad1), 1, 1, EF_BIT, 0, 1, "sf_44.1kHz",
    freqSuppDsc },
   {&EDID_cl::BitVal, NULL, offsetof(sad_t, sad1), 2, 1, EF_BIT, 0, 1, "sf_48kHz",
    freqSuppDsc },
   {&EDID_cl::BitVal, NULL, offsetof(sad_t, sad1), 3, 1, EF_BIT|EF_RD, 0, 1, "reserved",
   "reserved (0)" },
   {&EDID_cl::BitVal, NULL, offsetof(sad_t, sad1), 4, 1, EF_BIT, 0, 1, "sf_96kHz",
    freqSuppDsc },
   {&EDID_cl::BitVal, NULL, offsetof(sad_t, sad1), 5, 1, EF_BIT|EF_RD, 0, 1, "reserved",
   "reserved (0)" },
   {&EDID_cl::BitVal, NULL, offsetof(sad_t, sad1), 6, 1, EF_BIT, 0, 1, "sf_192kHz",
    freqSuppDsc },
   {&EDID_cl::BitVal, NULL, offsetof(sad_t, sad1), 7, 1, EF_BIT|EF_RD, 0, 1, "reserved",
   "reserved (0)" }
};
//SAD byte 1: AFC = 15 && ACE = 13 (L-PCM 3D Audio)
const edi_field_t cea_sad_cl::byte1_afc15_ace13[] = {
   {&EDID_cl::BitVal, NULL, offsetof(sad_t, sad1), 0, 1, EF_BIT, 0, 1, "sf_32kHz",
    freqSuppDsc },
   {&EDID_cl::BitVal, NULL, offsetof(sad_t, sad1), 1, 1, EF_BIT, 0, 1, "sf_44.1kHz",
    freqSuppDsc },
   {&EDID_cl::BitVal, NULL, offsetof(sad_t, sad1), 2, 1, EF_BIT, 0, 1, "sf_48kHz",
    freqSuppDsc },
   {&EDID_cl::BitVal, NULL, offsetof(sad_t, sad1), 3, 1, EF_BIT, 0, 1, "sf_88.2kHz",
    freqSuppDsc },
   {&EDID_cl::BitVal, NULL, offsetof(sad_t, sad1), 4, 1, EF_BIT, 0, 1, "sf_96kHz",
    freqSuppDsc },
   {&EDID_cl::BitVal, NULL, offsetof(sad_t, sad1), 5, 1, EF_BIT, 0, 1, "sf_176.4kHz",
    freqSuppDsc },
   {&EDID_cl::BitVal, NULL, offsetof(sad_t, sad1), 6, 1, EF_BIT, 0, 1, "sf_192kHz",
    freqSuppDsc },
   {&EDID_cl::BitVal, NULL, offsetof(sad_t, sad1), 7, 1, EF_BIT|EF_RD, 0, 1, "MC4",
   "This bit is part of 'num_chn' value for L-PCM 3D Audio, do not change it directly!." }
};

//SAD byte 2: AFC = 1: LPCM
const edi_field_t cea_sad_cl::byte2_afc1[] = {
   {&EDID_cl::BitVal, NULL, offsetof(sad_t, sad2), 0, 1, EF_BIT, 0, 1, "sample16b",
   "LPCM sample size in bits: 1=supported." },
   {&EDID_cl::BitVal, NULL, offsetof(sad_t, sad2), 1, 1, EF_BIT, 0, 1, "sample20b",
   "LPCM sample size in bits: 1=supported." },
   {&EDID_cl::BitVal, NULL, offsetof(sad_t, sad2), 2, 1, EF_BIT, 0, 1, "sample24b",
   "LPCM sample size in bits: 1=supported." },
   {&EDID_cl::BitF8Val, NULL, offsetof(sad_t, sad2), 3, 5, EF_BFLD|EF_RD, 0, 0, "reserved",
   "reserved (0)" }
};
//SAD byte 2: AFC = 2...8 : bitrate
const edi_field_t cea_sad_cl::byte2_afc2_8[] = {
   {&EDID_cl::SAD_BitRate, NULL, offsetof(sad_t, sad2), 0, 1, EF_BYTE|EF_INT|EF_KHZ, 0, (0xFF*8), "Max bitrate",
   "Maximum bitrate, stored value is the bitrate divided by 8kHz" }
};
//SAD byte 2: AFC = 9...13: AFC dependent value
const edi_field_t cea_sad_cl::byte2_afc9_13[] = {
   {&EDID_cl::ByteVal, NULL, offsetof(sad_t, sad2), 0, 1, EF_BYTE|EF_HEX, 0, 0xFF, "byte2",
   "AFC dependent value" }
};
//SAD byte 2: AFC = 14: WMA-Pro
const edi_field_t cea_sad_cl::byte2_afc14[] = {
   {&EDID_cl::BitF8Val, NULL, offsetof(sad_t, sad2), 0, 3, EF_BFLD|EF_RD, 0, 0, "profile",
   "WMA-Pro profile." },
   {&EDID_cl::BitF8Val, NULL, offsetof(sad_t, sad2), 3, 5, EF_BFLD|EF_RD, 0, 0, "reserved",
   "reserved (0)" }
};
//SAD byte 2: AFC = 15, ACE = 4, 5, 6
const edi_field_t cea_sad_cl::byte2_afc15_ace456[] = {
   {&EDID_cl::BitVal, NULL, offsetof(sad_t, sad2), 0, 1, EF_BIT|EF_RD, 0, 1, "reserved",
   "reserved (0)" },
   {&EDID_cl::BitVal, NULL, offsetof(sad_t, sad2), 1, 1, EF_BIT, 0, 1, "960_TL",
   "Total frame length 960 samples" },
   {&EDID_cl::BitVal, NULL, offsetof(sad_t, sad2), 2, 1, EF_BIT, 0, 1, "1024_TL",
   "Total frame length 1024 samples" },
   {&EDID_cl::BitF8Val, NULL, offsetof(sad_t, sad2), 3, 5, EF_INT|EF_FGR, 1, 13, "ACE_TC",
   "Audio Coding Extension Type Code." }
};
//SAD byte 2: AFC = 15, ACE = 8 or 10
const edi_field_t cea_sad_cl::byte2_afc15_ace8_10[] = {
   {&EDID_cl::BitVal, NULL, offsetof(sad_t, sad2), 0, 1, EF_BIT, 0, 1, "MPS_L",
   "MPEG Surround" },
   {&EDID_cl::BitVal, NULL, offsetof(sad_t, sad2), 1, 1, EF_BIT, 0, 1, "960_TL",
   "Total frame length 960 samples" },
   {&EDID_cl::BitVal, NULL, offsetof(sad_t, sad2), 2, 1, EF_BIT, 0, 1, "1024_TL",
   "Total frame length 1024 samples" },
   {&EDID_cl::BitF8Val, NULL, offsetof(sad_t, sad2), 3, 5, EF_INT|EF_FGR, 1, 13, "ACE_TC",
   "Audio Coding Extension Type Code." }
};
//SAD byte 2: AFC = 15, ACE = 11 (MPEG-H 3D Audio) or 12 (AC-4)
const edi_field_t cea_sad_cl::byte2_afc15_ace11_12[] = {
   {&EDID_cl::BitF8Val, NULL, offsetof(sad_t, sad2), 0, 3, EF_BFLD, 0, 0, "AFC_dep_val",
   "Audio Format Code dependent value" },
   {&EDID_cl::BitF8Val, NULL, offsetof(sad_t, sad2), 3, 5, EF_INT|EF_FGR, 1, 13, "ACE_TC",
   "Audio Coding Extension Type Code." }
};
//SAD byte 2: AFC = 15, ACE = 13 (L-PCM 3D Audio)
const edi_field_t cea_sad_cl::byte2_afc15_ace13[] = {
   {&EDID_cl::BitVal, NULL, offsetof(sad_t, sad2), 0, 1, EF_BIT, 0, 1, "s16bit",
   "16bit sample" },
   {&EDID_cl::BitVal, NULL, offsetof(sad_t, sad2), 1, 1, EF_BIT, 0, 1, "s20bit",
   "20bit sample" },
   {&EDID_cl::BitVal, NULL, offsetof(sad_t, sad2), 2, 1, EF_BIT, 0, 1, "s24bit",
   "24bit sample" },
   {&EDID_cl::BitF8Val, NULL, offsetof(sad_t, sad2), 3, 5, EF_INT|EF_FGR, 1, 13, "ACE_TC",
   "Audio Coding Extension Type Code." }
};

rcode cea_sad_cl::init(const u8_t* inst, u32_t orflags, edi_grp_cl* parent) {
   rcode retU;
   rcode retU2;

   parent_grp = parent;
   type_id    = ID_SAD | T_DBC_SUBGRP | orflags;

   //pre-alloc buffer for array of fields
   dyn_fldar = (edi_field_t*) malloc( 31 * EDI_FIELD_SZ );
   if (NULL == dyn_fldar) RCD_RETURN_FAULT(retU);

   CopyInstData(inst, sizeof(sad_t));

   retU = gen_data_layout(inst_data);

   retU2 = init_fields(dyn_fldar, inst_data, dyn_fcnt, 0, SAD_subg.Name, SAD_subg.Desc, SAD_subg.CodN);

   if (! RCD_IS_OK(retU)) return retU;
   return retU2;
}

rcode cea_sad_cl::ForcedGroupRefresh() {
   rcode retU;
   rcode retU2;

   //clear fields
   memset( (void*) dyn_fldar, 0, ( 31 * EDI_FIELD_SZ ) );

   //on error, "unknown data" layout is generated, init_fields is always called.
   retU  = gen_data_layout(inst_data);

   retU2 = init_fields(dyn_fldar, inst_data, dyn_fcnt, 0);

   if (! RCD_IS_OK(retU)) return retU;
   return retU2;
}

rcode cea_sad_cl::gen_data_layout(const u8_t* inst) {

   rcode        retU;
   edi_field_t *p_fld;
   int          AFC;
   int          ACE;

   p_fld = dyn_fldar;

   AFC = reinterpret_cast <const sad0_t*> (inst  )->afc1_14.audio_fmt;
   ACE = reinterpret_cast <const sad2_t*> (inst+2)->afc15_ace.ace_456.ace_tc;

   { // SAD byte 0
      if (AFC < 1 ) {
         insert_unk_byte(p_fld, 3, 0);
         dyn_fcnt = 3;
         RCD_RETURN_FAULT_MSG(retU, "[E!] SAD: bad Audio Format Code (AFC)");
      }
      if (AFC < 15) { //AFC = 1...14
         memcpy( p_fld, byte0_afc1_14, (byte0_afc1_14_fcnt * EDI_FIELD_SZ) );
         p_fld    += byte0_afc1_14_fcnt;
         dyn_fcnt  = byte0_afc1_14_fcnt;
      } else
      {  //AFC = 15 && ACE = 4-6,8,10: same as for AFC = 1...14
         if ((ACE >= 4 && 6 >= ACE) || (ACE = 8) || (ACE = 10)) {
            memcpy( p_fld, byte0_afc1_14, (byte0_afc1_14_fcnt * EDI_FIELD_SZ) );
            p_fld    += byte0_afc1_14_fcnt;
            dyn_fcnt  = byte0_afc1_14_fcnt;

         } else
         //AFC = 15 && ACE = 11,12 (MPEG-H 3D Audio, AC-4)
         if ((ACE = 11) || (ACE = 12)) {
            memcpy( p_fld, byte0_afc15_ace11_12, (byte0_afc15_ace11_12_fcnt * EDI_FIELD_SZ) );
            p_fld    += byte0_afc15_ace11_12_fcnt;
            dyn_fcnt  = byte0_afc15_ace11_12_fcnt;

         } else
         // AFC == 15, ACE = 13 (L-PCM 3D Audio)
         if (ACE == 13) {
            memcpy( p_fld, byte0_afc15_ace13, (byte0_afc15_ace13_fcnt * EDI_FIELD_SZ) );
            p_fld    += byte0_afc15_ace13_fcnt;
            dyn_fcnt  = byte0_afc15_ace13_fcnt;
         } else {
         // AFC == 15, ACE > 13 || ACE = 9 || ACE = 7 || ACE < 4 : unknown
            insert_unk_byte(p_fld, 3, 0);
            dyn_fcnt = 3;
            RCD_RETURN_FAULT_MSG(retU, "[E!] SAD: bad Audio Coding Extension Type Code (ACE)");
         }
      }
   }
   { // SAD byte 1
      //AFC = 1...14 || AFC = 15 && ACE = 11 (MPEG-H 3D Audio)
      if ( (AFC < 15) || ( AFC == 15 && ACE == 11 ) ) {
         memcpy( p_fld, byte1_afc1_14_ace11, (byte1_afc1_14_ace11_fcnt * EDI_FIELD_SZ) );
         p_fld    += byte1_afc1_14_ace11_fcnt;
         dyn_fcnt += byte1_afc1_14_ace11_fcnt;
      } else {
         //AFC == 15
         if ( (ACE >= 4 && 6 >= ACE) || ACE==8 || ACE==10 ) { //ACE = 4,5,6,8,10
            memcpy( p_fld, byte1_afc15_ace456810, (byte1_afc15_ace456810_fcnt * EDI_FIELD_SZ) );
            p_fld    += byte1_afc15_ace456810_fcnt;
            dyn_fcnt += byte1_afc15_ace456810_fcnt;
         } else
         if ( ACE==12 ) { //ACE = 12 (AC-4)
            memcpy( p_fld, byte1_afc15_ace12_fsmp, (byte1_afc15_ace12_fsmp_fcnt * EDI_FIELD_SZ) );
            p_fld    += byte1_afc15_ace12_fsmp_fcnt;
            dyn_fcnt += byte1_afc15_ace12_fsmp_fcnt;
         } else
         if ( ACE==13 ) { //ACE = 13 (L-PCM 3D Audio)
            memcpy( p_fld, byte1_afc15_ace13, (byte1_afc15_ace13_fcnt * EDI_FIELD_SZ) );
            p_fld    += byte1_afc15_ace13_fcnt;
            dyn_fcnt += byte1_afc15_ace13_fcnt;
         } else {
         // AFC == 15, ACE > 13 || ACE = 9 || ACE = 7 || ACE < 4 :
         // insert "unknown byte" on error
            insert_unk_byte(p_fld, 2, 1);
            dyn_fcnt += 2;
            RCD_RETURN_FAULT_MSG(retU, "[E!] SAD: bad Audio Coding Extension Type Code (ACE)");
         }
      }
   }
   { // SAD byte 2
      if (AFC == 1 ) { //AFC = 1: LPCM
         memcpy( p_fld, byte2_afc1, (byte2_afc1_fcnt * EDI_FIELD_SZ) );
         p_fld    += byte2_afc1_fcnt;
         dyn_fcnt += byte2_afc1_fcnt;
         RCD_RETURN_OK(retU);
      } else
      if (AFC >= 2 && 8 >= AFC ) { //AFC = 2...8 : bitrate
         memcpy( p_fld, byte2_afc2_8, (byte2_afc2_8_fcnt * EDI_FIELD_SZ) );
         p_fld    += byte2_afc2_8_fcnt;
         dyn_fcnt += byte2_afc2_8_fcnt;
         RCD_RETURN_OK(retU);
      } else
      if (AFC >= 9 && 13 >= AFC ) { //AFC = 9...13: AFC dependent value
         memcpy( p_fld, byte2_afc9_13, (byte2_afc9_13_fcnt * EDI_FIELD_SZ) );
         p_fld    += byte2_afc9_13_fcnt;
         dyn_fcnt += byte2_afc9_13_fcnt;
         RCD_RETURN_OK(retU);
      } else
      if (AFC == 14) { //AFC = 14: WMA-Pro
         memcpy( p_fld, byte2_afc14, (byte2_afc14_fcnt * EDI_FIELD_SZ) );
         p_fld    += byte2_afc14_fcnt;
         dyn_fcnt += byte2_afc14_fcnt;
         RCD_RETURN_OK(retU);
      } else
      if (AFC == 15) {
         if ( ACE >= 4 && 6 >= ACE ) { //AFC = 15, ACE = 4, 5, 6
            memcpy( p_fld, byte2_afc15_ace456, (byte2_afc15_ace456_fcnt * EDI_FIELD_SZ) );
            p_fld    += byte2_afc15_ace456_fcnt;
            dyn_fcnt += byte2_afc15_ace456_fcnt;
            RCD_RETURN_OK(retU);
         } else
         if ( ACE==8 || ACE==10 ) { //AFC = 15, ACE = 8 or 10
            memcpy( p_fld, byte2_afc15_ace8_10, (byte2_afc15_ace8_10_fcnt * EDI_FIELD_SZ) );
            p_fld    += byte2_afc15_ace8_10_fcnt;
            dyn_fcnt += byte2_afc15_ace8_10_fcnt;
            RCD_RETURN_OK(retU);
         } else
         if ( ACE==11 || ACE==12 ) { //AFC = 15, ACE = 11 (MPEG-H 3D Audio) or 12 (AC-4)
            memcpy( p_fld, byte2_afc15_ace11_12, (byte2_afc15_ace11_12_fcnt * EDI_FIELD_SZ) );
            p_fld    += byte2_afc15_ace11_12_fcnt;
            dyn_fcnt += byte2_afc15_ace11_12_fcnt;
            RCD_RETURN_OK(retU);
         } else
         if ( ACE==13 ) { //AFC = 15, ACE = 13 (L-PCM 3D Audio)
            memcpy( p_fld, byte2_afc15_ace13, (byte2_afc15_ace13_fcnt * EDI_FIELD_SZ) );
            p_fld    += byte2_afc15_ace13_fcnt;
            dyn_fcnt += byte2_afc15_ace13_fcnt;
            RCD_RETURN_OK(retU);
         }
         // AFC == 15, ACE > 13 || ACE = 9 || ACE = 7 || ACE < 4 : unknown
      }
   }
   //unknown byte format:
   insert_unk_byte(p_fld, 1, 2);
   dyn_fcnt += 1;
   RCD_RETURN_FAULT_MSG(retU, "[E!] SAD: bad Audio Coding Extension Type Code (ACE)");
}

//ADB: Audio Data Block: handlers
//ADB:SAD byte 1, AFC=2..8, Max bitrate
rcode EDID_cl::SAD_BitRate(u32_t op, wxString& sval, u32_t& ival, edi_dynfld_t* p_field) {
   rcode   retU;
   sad2_t *sad2;

   sad2 = reinterpret_cast<sad2_t*> ( getValPtr(p_field) );

   if (op == OP_READ) {
      ival   = sad2->afc2_8_bitrate8k;
      ival <<= 3; //*8kHz
      sval <<  ival;
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
         uint  tmpv;
         tmpv   = utmp;
         tmpv >>= 3; // div by 8kHz
         tmpv  &= 0xFF;
         sad2->afc2_8_bitrate8k = tmpv;
      }
   }
   return retU;
}
//ADB:SAD byte0 & 1, AFC = 15 && ACE = 13 (L-PCM 3D Audio): number of channels, bits MC4..MC0
rcode EDID_cl::SAD_LPCM_MC(u32_t op, wxString& sval, u32_t& ival, edi_dynfld_t* p_field) {
   rcode  retU;
   u8_t  *inst;

   inst = getValPtr(p_field);

   if (op == OP_READ) {
      uint  tmpv;
      tmpv   = ((sad0_t*)  inst   )->afc15_ace13.MC3;
      ival   = ((sad1_t*) (inst+1))->fsmp_afc15_ace13.MC4;
      ival <<= 1;
      ival  |= tmpv; //bits MC4,MC3
      tmpv   = inst[0] & 0x07; //bits MC2 .. MC0
      ival <<= 3;
      ival  |= tmpv; //bits MC4 .. MC0

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
         uint  tmpv;
         utmp   &= 0xFF;

         tmpv    = utmp >> 3; //bit MC3
         tmpv  <<= 7;
         ival    = inst[0]; //SAD byte 0
         ival   &= 0x78;    //clr M3, MC2-0
         ival   |= tmpv;    //bit MC3

         tmpv    = utmp & 0x07; //bits MC2 .. MC0
         ival   |= tmpv;
         inst[0] = ival; //write bits M3, MC2-0

         tmpv    = utmp >> 4; //bit MC4
         ((sad1_t*) inst+1)->fsmp_afc15_ace13.MC4 = tmpv;
      }
   }
   return retU;
}

//VDB: Video Data Block : video modes
#include "svd_vidfmt.h"

//VDB: Video Data Block
const char  cea_vdb_cl::Desc[] =
"Video Data Block contains one or more 1-byte "
"Short Video Descriptors (SVD). Each SVD contains index number from supported "
"resolutions set defined in CEA/EIA standard, and an optional \"native\" resolution flag";

const gproot_dsc_t cea_vdb_cl::VDB_grp = {
   .CodN     = "VDB",
   .Name     = "Video Data Block",
   .Desc     = Desc,
   .type_id  = ID_VDB,
   .flags    = 0,
   .min_len  = sizeof(svd_t),
   .max_len  = 31,
   .hdr_fcnt = CEA_DBCHDR_FCNT,
   .hdr_sz   = sizeof(bhdr_t),
   .grp_arsz = 1,
   .grp_ar   = &cea_svd_cl::SVD_subg
};

rcode cea_vdb_cl::init(const u8_t* inst, u32_t orflags, edi_grp_cl* parent) {
   rcode  retU;

   retU = base_DBC_Init_RootGrp(inst, &VDB_grp, orflags, parent);
   return retU;
}

//VDB: Video Data Block ->
//SVD: Short Video Descriptor
const char  cea_svd_cl::Desc[] =
"Short Video Descriptor: "
"contains index number of supported resolution defined in CEA/CTA-861 standard, "
"and an optional \"native\" resolution flag";

const subgrp_dsc_t cea_svd_cl::SVD_subg = {
   .s_ctor   = &cea_svd_cl::group_new,
   .CodN     = "SVD",
   .Name     = "Short Video Descriptor",
   .Desc     = Desc,
   .type_id  = ID_SVD | T_DBC_SUBGRP,
   .min_len  = sizeof(svd_t),
   .fcount   = 2,
   .inst_cnt = -1,
   .fields   = cea_svd_cl::fld_dsc
};

const edi_field_t cea_svd_cl::fld_dsc[] = {
   {&EDID_cl::ByteVal, &CEA_vidm_map, 0, 0, 1, EF_BYTE|EF_INT|EF_VS|EF_FGR, 0, 0xFF, "VIC",
   "Video ID Code: an index referencing the Table 3 in CEA/CTA-861, containing standard screen resolutions. "
   "For CEA-861-F and above, VIC value can take 8bits, interpretation:\n"
   "0\t\t: reserved\n"
   "1-127\t\t: 7bit VIC,\n"
   "128\t\t: reserved\n"
   "129-192\t: 7bit VIC, modes 1-64 with \"native\" bit set to 1 (128+1..128+64)\n"
   "193-219\t: 8bit VIC, no \"native\" bit\n"
   "220-255\t: reserved" },
   {&EDID_cl::BitVal, NULL, 0, 7, 1, EF_BIT|EF_RD|EF_FGR, 0, 0, "Native",
   "\"native\" mode flag\n\n"
   "See the VIC field description." }
};

rcode cea_svd_cl::init(const u8_t* inst, u32_t orflags, edi_grp_cl* parent) {
   rcode retU;

   parent_grp = parent;
   type_id    = SVD_subg.type_id | orflags; //cea_y42v_cl: set T_DBC_FIXED flag

   CopyInstData(inst, sizeof(svd_t));

   retU = init_fields(&SVD_subg.fields[0], inst_data, SVD_subg.fcount, 0, SVD_subg.Name, SVD_subg.Desc, SVD_subg.CodN);
   return retU;
}

u32_t EDID_cl::CEA_VDB_SVD_decode(u32_t vic, u32_t &native) {
   u32_t natv = 0;
   u32_t mode = (vic & 0xFF);

   switch (vic) {
      case 0 ... 128: //0, 128 reserved, 1..127 -> VIC code
         break;
      case 129 ... 192:
         natv = 1;
         mode = (vic & 0x7F);
         break;
      default: //193 ... 255: svd_vidfmt.h: max VIC is 219, natv=0, 220..255 are reserved in G spec.
         break;
   }

   native = natv;
   return mode;
}

//VSD: Vendor Specific Data Block: handlers
rcode EDID_cl::VSD_ltncy(u32_t op, wxString& sval, u32_t& ival, edi_dynfld_t* p_field) {
   rcode  retU;
   u8_t  *inst;

   inst = getValPtr(p_field);

   if (op == OP_READ) {
      ival   = inst[0];
      if (ival > 0) {
         ival  -= 1;
         ival <<= 1;
      } else {
         //value not provided
         p_field->field.flags |= EF_NU;
      }

      //ival = (inst[0] - 1) << 1; //(val-1)*2
      sval << ival;
      RCD_SET_OK(retU);
   } else {
      ulong  tmpv;
      RCD_SET_FAULT(retU);

      if (op == OP_WRSTR) {
         retU = getStrUint(sval, 10, p_field->field.minv, p_field->field.maxv, tmpv);
         if (! RCD_IS_OK(retU)) return retU;
      } else if (op == OP_WRINT) {
         tmpv = ival;
         RCD_SET_OK(retU);
      } else {
         RCD_RETURN_FAULT(retU); //wrong op code
      }

      tmpv >>= 1; //(val/2)+1
      tmpv ++ ;
      inst[0] = (tmpv & 0xFF);
      p_field->field.flags &= ~EF_NU;
   }
   return retU;
}

rcode EDID_cl::VSD_MaxTMDS(u32_t op, wxString& sval, u32_t& ival, edi_dynfld_t* p_field) {
   rcode  retU;
   u8_t  *inst;

   inst = getValPtr(p_field);

   if (op == OP_READ) {
      ival = (inst[0] * 5);
      sval << ival;
      RCD_SET_OK(retU);
   } else {
      ulong       tmpv;
      RCD_SET_FAULT(retU);

      if (op == OP_WRSTR) {
         retU = getStrUint(sval, 10, p_field->field.minv, p_field->field.maxv, tmpv);
         if (! RCD_IS_OK(retU)) return retU;
      } else if (op == OP_WRINT) {
         tmpv = ival;
         RCD_SET_OK(retU);
      } else {
         RCD_RETURN_FAULT(retU); //wrong op code
      }

      tmpv = (tmpv / 5);
      inst[0] = (tmpv & 0xFF);
   }
   return retU;
}

//VSD: Vendor Specific Data Block
const char  cea_vsd_cl::Desc[] =
"Vendor Specific Data Block is required to contain the following fields:\n"
"1. Vendor's IEEE-OUI 24-bit LE registration number\n"
"2. Source Physical Address (16bit LE). The source physical address provides the CEC physical address for upstream CEC devices.\n"
"The rest of fields can be anything the vendor considers worthy of inclusion in the VSD.\n"
"Known IEEE-OUI codes:\n"
"- 00-0C-03 \"HDMI Licensing, LLC\" -> provides HDMI 1.4 payload\n"
"- C4-5D-D8 \"HDMI Forum\" -> provides HDMI 2.0 payload\n"
"- 00-D0-46 \"DOLBY LABORATORIES, INC.\" -> provides Dolby Vision payload\n"
"- 90-84-8b \"HDR10+ Technologies, LLC\" -> provides HDR10+ payload as part of HDMI 2.1 Amendment A1 standard\n\n"
"NOTE: Currently, wxEDID interprets the payload as for 00-0C-03 \"HDMI Licensing, LLC\".\n";

const edi_field_t cea_vsd_cl::hdr_fld_dsc[] = {
   {&EDID_cl::ByteStr, NULL, offsetof(vsd_hdmi14_t, ieee_id)+1, 0, 3, EF_STR|EF_HEX|EF_LE|EF_RD|EF_FGR, 0, 0xFFFFFF, "IEEE-OUI",
   "IEEE Registration Id LE" },
   {&EDID_cl::ByteStr, NULL, offsetof(vsd_hdmi14_t, src_phy)+1, 0, 2, EF_STR|EF_HEX|EF_LE|EF_RD, 0, 0xFFFF, "src phy",
   "Source Physical Address (section 8.7 of HDMI 1.3a)." }
};

const edi_field_t cea_vsd_cl::sink_feat_fld_dsc[] = {
   {&EDID_cl::BitVal, NULL, offsetof(vsd_hdmi14_t, sink_feat)+1, 0, 1, EF_BIT, 0, 1, "DVI_dual",
   "DVI Dual Link Operation support." },
   {&EDID_cl::BitVal, NULL, offsetof(vsd_hdmi14_t, sink_feat)+1, 1, 1, EF_BIT, 0, 1, "reserved",
   "reserved (0)" },
   {&EDID_cl::BitVal, NULL, offsetof(vsd_hdmi14_t, sink_feat)+1, 2, 1, EF_BIT, 0, 1, "reserved",
   "reserved (0)" },
   {&EDID_cl::BitVal, NULL, offsetof(vsd_hdmi14_t, sink_feat)+1, 3, 1, EF_BIT, 0, 1, "DC_Y444",
   "4:4:4 support in deep color modes." },
   {&EDID_cl::BitVal, NULL, offsetof(vsd_hdmi14_t, sink_feat)+1, 4, 1, EF_BIT, 0, 1, "DC_30bit",
   "10-bit-per-channel deep color support." },
   {&EDID_cl::BitVal, NULL, offsetof(vsd_hdmi14_t, sink_feat)+1, 5, 1, EF_BIT, 0, 1, "DC_36bit",
   "12-bit-per-channel deep color support." },
   {&EDID_cl::BitVal, NULL, offsetof(vsd_hdmi14_t, sink_feat)+1, 6, 1, EF_BIT, 0, 1, "DC_48bit",
   "16-bit-per-channel deep color support." },
   {&EDID_cl::BitVal, NULL, offsetof(vsd_hdmi14_t, sink_feat)+1, 7, 1, EF_BIT, 0, 1, "AI",
   "Supports_AI (needs info from ACP or ISRC packets)." }
};

const edi_field_t cea_vsd_cl::max_tmds_fld_dsc[] = {
   {&EDID_cl::VSD_MaxTMDS, NULL, offsetof(vsd_hdmi14_t, max_tmds)+1, 0, 1, EF_INT|EF_MHZ, 0, 0xFF, "Max_TMDS",
   "Optional: If non-zero: Max_TMDS_Frequency / 5mhz." },
};

const edi_field_t cea_vsd_cl::latency_fld_dsc[] = {
   {&EDID_cl::BitF8Val, NULL, offsetof(vsd_hdmi14_t, ltncy_hdr)+1, 0, 6, EF_BFLD, 0, 0, "reserved",
   "reserved (0)" },
   {&EDID_cl::BitVal, NULL, offsetof(vsd_hdmi14_t, ltncy_hdr)+1, 6, 1, EF_BIT, 0, 0, "i_latency",
   "If set, 4 interlaced latency fields are present, must be 0 if latency_f (bit 7) is 0" },
   {&EDID_cl::BitVal, NULL, offsetof(vsd_hdmi14_t, ltncy_hdr)+1, 7, 1, EF_BIT, 0, 0, "latency_f",
   "If set, latency fields are present" }
};

const edi_field_t cea_vsd_cl::av_latency_fld_dsc[] = {
   {&EDID_cl::VSD_ltncy, NULL, offsetof(vsd_hdmi14_t, vid_lat)+1, 0, 1, EF_INT|EF_MLS|EF_FGR, 0, 500, "Video Latency",
   "Optional: Video Latency value=1+ms/2 with a max of 251 meaning 500ms, 2ms granularity." },
   {&EDID_cl::VSD_ltncy, NULL, offsetof(vsd_hdmi14_t, aud_lat)+1, 0, 1, EF_INT|EF_MLS|EF_FGR, 0, 500, "Audio Latency",
   "Optional: Audio Latency value=1+ms/2 with a max of 251 meaning 500ms, 2ms granularity." },
   {&EDID_cl::VSD_ltncy, NULL, offsetof(vsd_hdmi14_t, vid_ilat)+1, 0, 1, EF_INT|EF_MLS|EF_FGR, 0, 500, "Video iLatency",
   "Optional: Interlaced Video Latency value=1+ms/2 with a max of 251 meaning 500ms, 2ms granularity." },
   {&EDID_cl::VSD_ltncy, NULL, offsetof(vsd_hdmi14_t, aud_ilat)+1, 0, 1, EF_INT|EF_MLS|EF_FGR, 0, 500, "Audio iLatency",
   "Optional: Interlaced Audio Latency value=1+ms/2 with a max of 251 meaning 500ms, 2ms granularity." }
};

const gpfld_dsc_t cea_vsd_cl::sub_fld_grp[] = {
   { //vsd_hdr_idphy
      .flags    = TG_FLEX_LAYOUT,
      .fcount   = 2,
      .dat_sz   = 5,
      .inst_cnt = 1,
      .fields   = cea_vsd_cl::hdr_fld_dsc
   },
   { //vsd_sink_feat
      .flags    = 0,
      .fcount   = 8,
      .dat_sz   = 1,
      .inst_cnt = 1,
      .fields   = cea_vsd_cl::sink_feat_fld_dsc
   },
   { //vsd_max_tmds
      .flags    = 0,
      .fcount   = 1,
      .dat_sz   = 1,
      .inst_cnt = 1,
      .fields   = cea_vsd_cl::max_tmds_fld_dsc
   },
   { //vsd_latency
      .flags    = 0,
      .fcount   = 3,
      .dat_sz   = 1,
      .inst_cnt = 1,
      .fields   = cea_vsd_cl::latency_fld_dsc
   },
   { //vsd_av_latency
      .flags    = TG_FLEX_LAYOUT|TG_FLEX_LEN,
      .fcount   = 4,
      .dat_sz   = 4,
      .inst_cnt = 1,
      .fields   = cea_vsd_cl::av_latency_fld_dsc
   }
};

const gpflat_dsc_t cea_vsd_cl::VSD_grp = {
   .CodN     = "VSD",
   .Name     = "Vendor Specific Data Block",
   .Desc     = Desc,
   .type_id  = ID_VSD,
   .flags    = TG_FLEX_LAYOUT,
   .min_len  = 3,
   .max_len  = 31,
   .max_fld  = (32 - sizeof(vsd_hdmi14_t) - sizeof(bhdr_t) + 18),
   .hdr_fcnt = CEA_DBCHDR_FCNT,
   .hdr_sz   = sizeof(bhdr_t),
   .fld_arsz = 5,
   .fld_ar   = cea_vsd_cl::sub_fld_grp
};

rcode cea_vsd_cl::init(const u8_t* inst, u32_t orflags, edi_grp_cl* parent) {
   rcode  retU;

   retU = base_DBC_Init_FlatGrp(inst, &VSD_grp, orflags, parent);
   return retU;
}

//SAB: Speaker Allocation Data Block
//NOTE: the structure of SAB is basically an SPM: Speaker Presence Mask
//      The Desc and fields are shared with RMCD: Room Configuration Data Block: SPM

extern const char SAB_SPM_Desc[];
       const char SAB_SPM_Desc[] =
"Speaker Presence Mask (3 bytes).\n"
"Contains information about which speakers are present in the display device.\n"
"Byte1:\n"
"FL_FR\t\t: Front Left/Right\n"
"LFE1\t\t: LFE Low Frequency Effects 1 (subwoofer1)\n"
"FC\t\t\t: Front Center\n"
"BL_BR\t\t: Back Left/Right\n"
"BC\t\t\t: Back Center\n"
"FLC_FRC\t\t: Front Left/Right of Center\n"
"RLC_RRC\t\t: Rear Left/Right of Center\n"
"FLW_FRW\t: Front Left/Right Wide\n"
"Byte2:\n"
"TpFL_TpFR\t: Top Front Left/Right\n"
"TpC\t\t\t: Top Center\n"
"TpFC\t\t: Top Front Center\n"
"LS_RS\t\t: Left/Right Surround\n"
"LFE2\t\t: LFE Low Frequency Effects 2 (subwoofer2)\n"
"TpBC\t\t: Top Back Center\n"
"SiL_SiR\t\t: Side Left/Right\n"
"TpSiL_TpSiR\t: Top Side Left/Right\n"
"Byte3:\n"
"TpBL_TpBR\t: Top Back Left/Right\n"
"BtFC\t\t: Bottom Front Center\n"
"BtFL_BtFR\t: Bottom Front Left/Right\n"
"TpLS_TpRS\t: Top Left/Right ? Surround ? - a bug in CTA-861-G ?\n"
"bit4-7\t\t: reserved\n";

//NOTE: The SAB fields definitions are shared with RMCD: Room Configuration Data Block: SPM
extern const gpfld_dsc_t SAB_SPM_fields[];
extern const edi_field_t SAB_SPM_fld_dsc[];

const gpfld_dsc_t SAB_SPM_fields[] = {
   {
      .flags    = 0,
      .fcount   = 24,
      .dat_sz   = sizeof(sab_t),
      .inst_cnt = 1,
      .fields   = SAB_SPM_fld_dsc
   }
};

const edi_field_t SAB_SPM_fld_dsc[] = {
   //SAB byte 0
   {&EDID_cl::BitVal, NULL, offsetof(sab_t, spm0)+1, 0, 1, EF_BIT, 0, 1, "FL_FR",
   "Front Left/Right " },
   {&EDID_cl::BitVal, NULL, offsetof(sab_t, spm0)+1, 1, 1, EF_BIT, 0, 1, "LFE1",
   "LFE Low Frequency Effects 1 (subwoofer1)" },
   {&EDID_cl::BitVal, NULL, offsetof(sab_t, spm0)+1, 2, 1, EF_BIT, 0, 1, "FC",
   "Front Center" },
   {&EDID_cl::BitVal, NULL, offsetof(sab_t, spm0)+1, 3, 1, EF_BIT, 0, 1, "BL_BR",
   "Back Left/Right" },
   {&EDID_cl::BitVal, NULL, offsetof(sab_t, spm0)+1, 4, 1, EF_BIT, 0, 1, "BC",
   "Back Center" },
   {&EDID_cl::BitVal, NULL, offsetof(sab_t, spm0)+1, 5, 1, EF_BIT, 0, 1, "FLC_FRC",
   "Front Left/Right of Center" },
   {&EDID_cl::BitVal, NULL, offsetof(sab_t, spm0)+1, 6, 1, EF_BIT, 0, 1, "RLC_RRC",
   "Rear (Back) Left/Right of Center" },
   {&EDID_cl::BitVal, NULL, offsetof(sab_t, spm0)+1, 7, 1, EF_BIT, 0, 1, "FLW_FRW",
   "Front Left/Right Wide" },
   //SAB byte 1
   {&EDID_cl::BitVal, NULL, offsetof(sab_t, spm1)+1, 0, 1, EF_BIT, 0, 1, "TpFL_TpFR",
   "Top Front Left/Right" },
   {&EDID_cl::BitVal, NULL, offsetof(sab_t, spm1)+1, 1, 1, EF_BIT, 0, 1, "TpC",
   "Top Center" },
   {&EDID_cl::BitVal, NULL, offsetof(sab_t, spm1)+1, 2, 1, EF_BIT, 0, 1, "TpFC",
   "Top Front Center" },
   {&EDID_cl::BitVal, NULL, offsetof(sab_t, spm1)+1, 3, 1, EF_BIT, 0, 1, "LS_RS",
   "Left/Right Surround" },
   {&EDID_cl::BitVal, NULL, offsetof(sab_t, spm1)+1, 4, 1, EF_BIT, 0, 1, "LFE2",
   "LFE Low Frequency Effects 2 (subwoofer2)" },
   {&EDID_cl::BitVal, NULL, offsetof(sab_t, spm1)+1, 5, 1, EF_BIT, 0, 1, "TpBC",
   "Top Back Center" },
   {&EDID_cl::BitVal, NULL, offsetof(sab_t, spm1)+1, 6, 1, EF_BIT, 0, 1, "SiL_SiR",
   "Side Left/Right" },
   {&EDID_cl::BitVal, NULL, offsetof(sab_t, spm1)+1, 7, 1, EF_BIT, 0, 1, "TpSiL_TpSiR",
   "Top Side Left/Right" },
   //SAB byte 2
   {&EDID_cl::BitVal, NULL, offsetof(sab_t, spm2)+1, 0, 1, EF_BIT, 0, 1, "TpBL_TpBR",
   "Top Back Left/Right" },
   {&EDID_cl::BitVal, NULL, offsetof(sab_t, spm2)+1, 1, 1, EF_BIT, 0, 1, "BtFC",
   "Bottom Front Center" },
   {&EDID_cl::BitVal, NULL, offsetof(sab_t, spm2)+1, 2, 1, EF_BIT, 0, 1, "BtFL_BtFR",
   "Bottom Front Left/Right" },
   {&EDID_cl::BitVal, NULL, offsetof(sab_t, spm2)+1, 3, 1, EF_BIT, 0, 1, "TpLS_TpRS",
   "Top Left/Right ? Surround ? - a bug in CTA-861-G ?" },
   {&EDID_cl::BitVal, NULL, offsetof(sab_t, spm2)+1, 4, 1, EF_BIT|EF_RD, 0, 1, "resvd4",
   "reserved (0)" },
   {&EDID_cl::BitVal, NULL, offsetof(sab_t, spm2)+1, 5, 1, EF_BIT|EF_RD, 0, 1, "resvd5",
   "reserved (0)" },
   {&EDID_cl::BitVal, NULL, offsetof(sab_t, spm2)+1, 6, 1, EF_BIT|EF_RD, 0, 1, "resvd6",
   "reserved (0)" },
   {&EDID_cl::BitVal, NULL, offsetof(sab_t, spm2)+1, 7, 1, EF_BIT|EF_RD, 0, 1, "resvd7",
   "reserved (0)" }
};

const gpflat_dsc_t cea_sab_cl::SAB_grp = {
   .CodN     = "SAB",
   .Name     = "Speaker Allocation Block",
   .Desc     = SAB_SPM_Desc,
   .type_id  = ID_SAB,
   .flags    = 0,
   .min_len  = sizeof(sab_t),
   .max_len  = sizeof(sab_t),
   .max_fld  = (32 - sizeof(sab_t) - sizeof(bhdr_t) + 24),
   .hdr_fcnt = CEA_DBCHDR_FCNT,
   .hdr_sz   = sizeof(bhdr_t),
   .fld_arsz = 1,
   .fld_ar   = SAB_SPM_fields
};

rcode cea_sab_cl::init(const u8_t* inst, u32_t orflags, edi_grp_cl* parent) {
   rcode retU;

   retU = base_DBC_Init_FlatGrp(inst, &SAB_grp, orflags, parent);
   return retU;
}

//VDTC: VESA Display Transfer Characteristic Data Block (gamma)
const char  cea_vdtc_cl::Desc[] =
"Gamma value for the display device.\n"
"The format is exactly the same as in EDID block 0, byte 23";

const gpfld_dsc_t cea_vdtc_cl::fields[] = {
   {
      .flags    = 0,
      .fcount   = 1,
      .dat_sz   = sizeof(vtc_t),
      .inst_cnt = 1,
      .fields   = cea_vdtc_cl::fld_dsc
   }
};

const edi_field_t cea_vdtc_cl::fld_dsc[] = {
   {&EDID_cl::Gamma, NULL, offsetof(vtc_t, gamma)+1, 0, 1, EF_FLT|EF_NI, 0, 255, "gamma",
   "Byte value = (gamma*100)-100 (range 1.00–3.54)" }
};

const gpflat_dsc_t cea_vdtc_cl::VDTC_grp = {
   .CodN     = "VDTC",
   .Name     = "VESA Display Transfer Characteristic",
   .Desc     = Desc,
   .type_id  = ID_VDTC,
   .flags    = 0,
   .min_len  = sizeof(vtc_t),
   .max_len  = sizeof(vtc_t),
   .max_fld  = (31 + CEA_DBCHDR_FCNT),
   .hdr_fcnt = CEA_DBCHDR_FCNT,
   .hdr_sz   = sizeof(bhdr_t),
   .fld_arsz = 1,
   .fld_ar   = cea_vdtc_cl::fields
};

rcode cea_vdtc_cl::init(const u8_t* inst, u32_t orflags, edi_grp_cl* parent) {
   rcode retU;

   retU = base_DBC_Init_FlatGrp(inst, &VDTC_grp, orflags, parent);
   return retU;
}

//UNK-TC: Unknown Data Block (Tag Code)
const gpflat_dsc_t cea_unktc_cl::UNK_TC_grp = {
   .CodN     = "UNK-TC",
   .Name     = "Unknown Data Block",
   .Desc     = "Unknown Tag Code",
   .type_id  = ID_CEA_UTC,
   .flags    = 0,
   .min_len  = 0,
   .max_len  = 31,
   .max_fld  = (31 + CEA_DBCHDR_FCNT),
   .hdr_fcnt = CEA_DBCHDR_FCNT,
   .hdr_sz   = sizeof(bhdr_t),
   .fld_arsz = 0,
   .fld_ar   = NULL
};

rcode cea_unktc_cl::init(const u8_t* inst, u32_t orflags, edi_grp_cl* parent) {
   rcode retU;

   retU = base_DBC_Init_FlatGrp(inst, &UNK_TC_grp, orflags, parent);
   return retU;
}

//UNK-DAT Unknown data bytes, subgroup.
const char  cea_unkdat_cl::CodN[] = "UNK-DAT";
const char  cea_unkdat_cl::Name[] = "Unknown data bytes";
const char  cea_unkdat_cl::Desc[] = "Unknown data bytes";

#pragma GCC diagnostic ignored "-Wunused-parameter"
rcode cea_unkdat_cl::init(const u8_t* inst, u32_t orflags, edi_grp_cl* parent) {
   rcode        retU;
   u32_t        dlen;

   //variable size: dat_sz must be set before init() !!
   dlen       = dat_sz;
   parent_grp = parent;
   type_id    = ID_CEA_UDAT | T_DBC_SUBGRP | orflags; //unknown data sub-group + parent group ID

   CopyInstData(inst, dlen);

   //pre-alloc buffer for array of fields: hdr_fcnt + dlen
   dyn_fldar = (edi_field_t*) malloc( dlen * EDI_FIELD_SZ );
   if (NULL == dyn_fldar) RCD_RETURN_FAULT(retU);

   dyn_fcnt  = dlen;

   //payload data interpreted as unknown
   insert_unk_byte(dyn_fldar, dyn_fcnt, 0);

   retU = init_fields(&dyn_fldar[0], inst_data, dyn_fcnt, 0, Name, Desc, CodN);
   return retU;
}
#pragma GCC diagnostic warning "-Wunused-parameter"

//Universal Init function for DBC blocks with subgroups
rcode edi_grp_cl::base_DBC_Init_RootGrp(const u8_t* inst, const gproot_dsc_t *pGDsc, u32_t orflags, edi_grp_cl* parent) {
   rcode        retU, retU2;
   u32_t        dlen;
   u32_t        dlen2;
   u32_t        offs;
   u32_t        buflen;
   u32_t        gr_idx;
   u32_t        gr_inst;
   const u8_t  *pgrp_inst;
   bool         b_edit_mode;
   GroupAr_cl  *subgrp_ar;

   //NOTE: All the fault messages are logged, but ignored -> edi_grp_cl::base_clone()

   RCD_SET_OK(retU);
   RCD_SET_OK(retU2);
   b_edit_mode  = (0 != (T_MODE_EDIT & orflags));

   dlen  = reinterpret_cast <const bhdr_t*> (inst)->tag.blk_len;
   dlen2 = dlen;
   if (dlen < pGDsc->min_len) {
      wxedid_RCD_SET_FAULT_VMSG(retU2, DBC_BAD_LEN_MSG, pGDsc->CodN);
      if (! b_edit_mode) return retU2;
   }

   subgrp_ar  = getSubArray();
   parent_grp = parent;
   type_id    = pGDsc->type_id;
   pgrp_inst  = inst;

   dlen   += 1; //DBC header
   buflen  = (b_edit_mode) ? 32 : dlen;
   memcpy(inst_data, inst, buflen);
   dat_sz  = dlen;
   if (dlen >= pGDsc->hdr_sz) {
      dlen  -= pGDsc->hdr_sz;
   } else {
      dlen  -= 1;
   }
   hdr_sz     = pGDsc->hdr_sz;
   offs       = pGDsc->hdr_sz;
   pgrp_inst += offs;

   for (gr_idx=0; gr_idx<pGDsc->grp_arsz; ++gr_idx) {
      const subgrp_dsc_t *pSubGDsc;
      edi_grp_cl         *pgrp;
      u32_t               dsz;

      pSubGDsc = &pGDsc->grp_ar[gr_idx];
      //num of sub-group instances to spawn, if inst_cnt==(-1) -> until data end
      for (gr_inst=0; gr_inst<(u32_t)pSubGDsc->inst_cnt; ++gr_inst) {

         if (dlen < pSubGDsc->min_len) {
            if (dlen > 0) {
               //no space left for next sub-group
               wxedid_RCD_SET_FAULT_VMSG(retU2, DBC_BAD_LEN_MSG, pGDsc->CodN);
            }
            goto payload;
         }

         subg_sz = dlen; //free space left for subgroup->init()
         pgrp    = pSubGDsc->s_ctor();
         if (NULL == pgrp) RCD_RETURN_FAULT(retU);

         retU2 = pgrp->init(pgrp_inst, type_id, this);
         if (! RCD_IS_OK(retU2)) {
            if (retU2.detail.rcode > RCD_FVMSG) {
               wxedid_RCD_SET_FAULT_VMSG(retU2, DBC_BAD_LEN_MSG, pGDsc->CodN);
               delete pgrp;
               goto payload;
            }
            //else VMSG from sub-group
         }

         pgrp->setRelOffs(offs);
         pgrp->setAbsOffs(offs + abs_offs);
         subgrp_ar->Append(pgrp);

         dsz        = pgrp->getTotalSize();
         offs      += dsz;
         pgrp_inst += dsz;
         dlen      -= dsz;

         if (dsz < 1) RCD_RETURN_FAULT(retU); //only for TESTING group defs/init
      }
   }

payload:
   if (dlen2 > pGDsc->max_len) {
      wxedid_RCD_SET_FAULT_VMSG(retU2, DBC_BAD_LEN_MSG, pGDsc->CodN);
   }

   //unspecified payload: interpreted as unknown sub-group.
   if (dlen > 0) {
      retU = Append_UNK_DAT(pgrp_inst, dlen, type_id, (offs + abs_offs), offs, this);
      if ( (! RCD_IS_OK(retU)) && !b_edit_mode ) return retU;
   }

   subgrp_ar->CalcDataSZ(this);

   retU = init_fields(&CEA_BlkHdr_fields[0], inst_data, pGDsc->hdr_fcnt, 0,
                      pGDsc->Name, pGDsc->Desc, pGDsc->CodN);

   if (RCD_IS_OK(retU)) return retU2;
   return retU;
}

//Universal Init function for DBC blocks (no sub-groups)
rcode edi_grp_cl::base_DBC_Init_FlatGrp(const u8_t* inst, const gpflat_dsc_t *pGDsc, u32_t orflags, edi_grp_cl* parent) {
   rcode        retU, retU2;
   u32_t        dlen;
   u32_t        dlen2;
   u32_t        offs;
   u32_t        buflen;
   u32_t        fldgr_idx;
   u32_t        gr_inst;
   edi_field_t *p_fld;
   bool         b_edit_mode;

   //NOTE: All the fault messages are logged, but ignored -> edi_grp_cl::base_clone()

   RCD_SET_OK(retU);
   RCD_SET_OK(retU2);
   b_edit_mode  = (0 != (T_MODE_EDIT & orflags));

   dlen  = reinterpret_cast <const bhdr_t*> (inst)->tag.blk_len;
   dlen2 = dlen;
   if (dlen < pGDsc->min_len) {
      wxedid_RCD_SET_FAULT_VMSG(retU2, DBC_BAD_LEN_MSG, pGDsc->CodN);
      if (! b_edit_mode) return retU2;
   }

   parent_grp = parent;
   type_id    = pGDsc->type_id;

   dlen   += 1; //DBC header
   buflen  = (b_edit_mode) ? 32 : dlen;
   memcpy(inst_data, inst, buflen); //Local data copy
   dat_sz  = dlen;
   if (dlen >= pGDsc->hdr_sz) {
      dlen  -= pGDsc->hdr_sz;
   } else {
      dlen  -= 1;
   }

   //max fields:
   dyn_fldar = (edi_field_t*) malloc( pGDsc->max_fld * EDI_FIELD_SZ );
   if (NULL == dyn_fldar) RCD_RETURN_FAULT(retU);
   dyn_fcnt  = pGDsc->hdr_fcnt;
   p_fld     = dyn_fldar;

   memcpy( p_fld, CEA_BlkHdr_fields, (pGDsc->hdr_fcnt * EDI_FIELD_SZ) );
   p_fld += pGDsc->hdr_fcnt;
   offs   = pGDsc->hdr_sz;

   for (fldgr_idx=0; fldgr_idx<pGDsc->fld_arsz; ++fldgr_idx) {
      const gpfld_dsc_t *pgFld;
      u32_t fcnt;
      u32_t dsz;

      pgFld = &pGDsc->fld_ar[fldgr_idx];

      if ( (TG_FLEX_LAYOUT & pgFld->flags) != 0) {
         retU2 = base_DBC_Init_GrpFields(pgFld, &p_fld, &dlen, &offs);
         if (! RCD_IS_OK(retU2)) {
            wxedid_RCD_SET_FAULT_VMSG(retU2, DBC_BAD_LEN_MSG, pGDsc->CodN);
            goto payload;
         }
         if (RCD_IS_TRUE(retU2)) {
            continue;
         }
      }

      fcnt = pgFld->fcount;
      dsz  = pgFld->dat_sz;

      //num of sub-group instances to spawn, if inst_cnt==(-1) -> until data end
      for (gr_inst=0; gr_inst<(u32_t)pgFld->inst_cnt; ++gr_inst) {
         bool  b_flex;

         if (dlen < dsz) {
            b_flex  = ((TG_FLEX_LAYOUT & pGDsc->flags)  != 0);
            b_flex |= (                  pgFld->inst_cnt < 0);
            if (! b_flex) {
               wxedid_RCD_SET_FAULT_VMSG(retU2, DBC_BAD_LEN_MSG, pGDsc->CodN);
            }
            goto payload;
         }

         b_flex = ( (TG_FLEX_OFFS & pgFld->flags) != 0);

         memcpy( p_fld, pgFld->fields, (fcnt * EDI_FIELD_SZ) );

         if (b_flex) { //update field offsets: all fields have to be of size ==1
            edi_field_t *p_dstf;

            p_dstf = p_fld;
            for (u32_t fidx=0; fidx<fcnt; ++fidx) {
               p_dstf->offs = (offs + fidx);
               p_dstf      ++ ;
            }
         }

         p_fld    += fcnt;
         dyn_fcnt += fcnt;
         dlen     -= dsz;
         offs     += dsz;
      }
   }

payload:
   if (dlen2 > pGDsc->max_len) {
      wxedid_RCD_SET_FAULT_VMSG(retU2, DBC_BAD_LEN_MSG, pGDsc->CodN);
   }

   //unspecified payload: interpreted as unknown bytes.
   if (dlen > 0) {
      dyn_fcnt += dlen;
      insert_unk_byte(p_fld, dlen, offs);
   }
   retU = init_fields(&dyn_fldar[0], inst_data, dyn_fcnt, 0, pGDsc->Name, pGDsc->Desc, pGDsc->CodN);

   if (RCD_IS_OK(retU)) return retU2;
   return retU;
}

rcode edi_grp_cl::base_DBC_Init_GrpFields(const gpfld_dsc_t *pgFld, edi_field_t **pp_fld, u32_t *pdlen, u32_t *poffs) {
   rcode  retU;
   u32_t  dlen   = *pdlen;
   u32_t  offs   = *poffs;
   u32_t  nfld   = 0;
   u32_t  grlen  = 0;
   bool   b_offs = ( (TG_FLEX_OFFS & pgFld->flags) != 0);

         edi_field_t *p_fld  = *pp_fld;
   const edi_field_t *p_srcf = &pgFld->fields[0];

   p_srcf -- ;

   for (; nfld<pgFld->fcount; ++nfld) {
      u32_t  fldlen;

      p_srcf ++ ;
      fldlen  = p_srcf->fldsize;

      if (dlen < fldlen) break;

      memcpy( p_fld, p_srcf, EDI_FIELD_SZ );
      if (b_offs) { //update fields offsets:
         p_fld->offs = offs;
      }
      dlen  -= fldlen;
      grlen += fldlen;
      offs  += fldlen;
      p_fld ++ ;
   }

   retU = RCD_SET_VAL(RCD_UNIT, RCD_TRUE, __LINE__);

   if (grlen != pgFld->dat_sz) {
      bool  b_flex;
      b_flex  = ((TG_FLEX_LEN & pgFld->flags) != 0);
      if (! b_flex) {
         RCD_SET_FAULT(retU); //Bad length
      }
   }

   *pp_fld   = p_fld;
   *poffs    = offs;
   *pdlen    = dlen;
   dyn_fcnt += nfld;

   return retU;
}

