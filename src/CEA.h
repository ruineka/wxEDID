/***************************************************************
 * Name:      CEA.h
 * Purpose:   EDID Extension structures: CEA/CTA-861-G
 * Author:    Tomasz Pawlak (tomasz.pawlak@wp.eu)
 * Created:   2014-03-28
 * Copyright: Tomasz Pawlak (C) 2014-2022
 * License:   GPLv3+
 **************************************************************/

#ifndef CEA_H
#define CEA_H 1

#include "def_types.h"

typedef unsigned int uint;

enum { //Block Header : type tags (TagCode)
   DBC_RSVD0 = 0, //reserved
   DBC_T_ADB = 1, //Audio Data Block
   DBC_T_VDB = 2, //Video Data Block
   DBC_T_VSD = 3, //Vendor Specific Data Block
   DBC_T_SAB = 4, //Speaker Allocation Data Block
   DBC_T_VTC = 5, //VESA Display Transfer Characteristic Data Block (gamma)
   DBC_RSVD6 = 6, //reserved
   DBC_T_EXT = 7  //Use Extended Tag -> CEA_EXT.h
};


//Std Block Header
typedef struct __attribute__ ((packed)) { //tag_s
   uint blk_len  :5;    //bit 4-0: Total number of bytes in the block excluding this byte
   uint tag_code :3;    //bit 7-5: block type, TagCode: 0,6 -> reserved
} tag_t;

typedef union __attribute__ ((packed)) { //bhdr_s
   u8_t   bval;
   tag_t  tag;
} bhdr_t;

//Block Header with Extended Tag Code
typedef union __attribute__ ((packed)) { //ethdr_s
   u16_t  w16;
   struct __attribute__ ((packed)) {
      bhdr_t hdr;  //std header
      u8_t   extag; //Extended Tag Code
   } ehdr;
} ethdr_t;

//ADB: Audio Data Block ->
//SAD: Short Audio Descriptor
 /* AFC: Audio format code
0 = reserved
1 = Linear Pulse Code Modulation (LPCM)
2 = AC-3
3 = MPEG1 (Layers 1 and 2)
4 = MP3
5 = MPEG2
6 = AAC
7 = DTS
8 = ATRAC
9 = One-bit audio aka SACD
10 = DD+
11 = DTS-HD
12 = MLP/Dolby TrueHD
13 = DST Audio
14 = Microsoft WMA Pro
15 = SAD byte 2: Audio Coding Extension Type Code
*/

typedef union __attribute__ ((packed)) { //sad0_s
   // SAD byte 0: Audio Format Code and number of channels
   //AF-code = 1...14
   //AF-code = 15 && ACE = 4-6,8,10
   struct __attribute__ ((packed)) {
      uint  num_ch    :3;   //bit 2-0: Num_of_channels -1 (7->8 channels, 1-> 2ch)
      uint  audio_fmt :4;   //bit 6-3: Audio Format Code, AFC
      uint  resvd7    :1;   //bit 7:   reserved.
   } afc1_14;
   //AF-code = 15 && ACE = 11,12 (MPEG-H 3D Audio, AC-4)
   struct __attribute__ ((packed)) {
      uint  resvd02   :3;   //bit 2-0: reserved.
      uint  audio_fmt :4;   //bit 6-3: Audio Format Code, AFC
      uint  resvd7    :1;   //bit 7:   reserved.
   } afc15_ace11_12;
   //AF-code = 15 && ACE = 13 (L-PCM 3D Audio)
   struct __attribute__ ((packed)) {
      uint  MC0       :1;   //bit 0: bit 0 of LPCM_MC (Max_Number_of_Channels -1) for L-PCM 3D Audio
      uint  MC1       :1;   //bit 1: bit 1 of LPCM_MC
      uint  MC2       :1;   //bit 2: bit 2 of LPCM_MC
      uint  audio_fmt :4;   //bit 6-3: Audio format code
      uint  MC3       :1;   //bit 7: bit 3 of LPCM_MC
   } afc15_ace13;
} sad0_t;

typedef union __attribute__ ((packed)) { //sad1_s
   //SAD byte 1: sampling frequencies supported
   //AF-code = 1...14 || AF-code = 15 && ACE = 11 (MPEG-H 3D Audio)
   struct __attribute__ ((packed)) {
      uint  sf32k     :1;   //bit 0: smp. freq. 32kHz
      uint  sf44k     :1;   //bit 1: smp. freq. 44.1kHz
      uint  sf48k     :1;   //bit 2: smp. freq. 48kHz
      uint  sf88k     :1;   //bit 3: smp. freq. 88.2kHz
      uint  sf96k     :1;   //bit 4: smp. freq. 96kHz
      uint  sf176k    :1;   //bit 5: smp. freq. 176.4kHz
      uint  sf192k    :1;   //bit 6: smp. freq. 192kHz
      uint  resvd7    :1;   //bit 7: reserved
   } fsmp_afc1_14_ace11;
   //AF-code = 15 && ACE = 4,5,6,8,10
   struct __attribute__ ((packed)) {
      uint  sf32k     :1;   //bit 0: smp. freq. 32kHz
      uint  sf44k     :1;   //bit 1: smp. freq. 44.1kHz
      uint  sf48k     :1;   //bit 2: smp. freq. 48kHz
      uint  sf88k     :1;   //bit 3: smp. freq. 88.2kHz
      uint  sf96k     :1;   //bit 4: smp. freq. 96kHz
      uint  resvd5    :1;   //bit 5: reserved
      uint  resvd6    :1;   //bit 6: reserved
      uint  resvd7    :1;   //bit 7: reserved
   } fsmp_afc15_ace456810;
   //AF-code = 15 && ACE = 12 (AC-4)
   struct __attribute__ ((packed)) {
      uint  resvd0    :1;   //bit 0: reserved
      uint  sf44k     :1;   //bit 1: smp. freq. 44.1kHz
      uint  sf48k     :1;   //bit 2: smp. freq. 48kHz
      uint  resvd3    :1;   //bit 3: reserved
      uint  sf96k     :1;   //bit 4: smp. freq. 96kHz
      uint  resvd5    :1;   //bit 5: reserved
      uint  sf192k    :1;   //bit 6: smp. freq. 192kHz
      uint  resvd7    :1;   //bit 7: reserved
   } fsmp_afc15_ace12;
   //AF-code = 15 && ACE = 13 (L-PCM 3D Audio)
   struct __attribute__ ((packed)) {
      uint  sf32k     :1;   //bit 0: smp. freq. 32kHz
      uint  sf44k     :1;   //bit 1: smp. freq. 44.1kHz
      uint  sf48k     :1;   //bit 2: smp. freq. 48kHz
      uint  sf88k     :1;   //bit 3: smp. freq. 88.2kHz
      uint  sf96k     :1;   //bit 4: smp. freq. 96kHz
      uint  sf176k    :1;   //bit 5: smp. freq. 176.4kHz
      uint  sf192k    :1;   //bit 6: smp. freq. 192kHz
      uint  MC4       :1;   //bit 7: bit4 of LPCM_MC (Max_Number_of_Channels -1) for L-PCM 3D Audio
   } fsmp_afc15_ace13;
} sad1_t;

//SAD byte 2: AF-code = 15: structure depends on Audio Coding Extension Type Code (ACE)
typedef union __attribute__ ((packed)) {
   //ACE = 4, 5, 6
   struct __attribute__ ((packed)) {
      uint resvd0      :1; //reserved
      uint frm960s     :1; //960_TL: total frame length 960 samples
      uint frm1024s    :1; //1024_TL: total frame length 1024 samples
      uint ace_tc      :5; //ACE: Audio Coding Extension Type Code
   } ace_456;
   //ACE = 8 or 10
   struct __attribute__ ((packed)) {
      uint mps_l       :1; //MPS_L: MPEG Surround
      uint frm960s     :1; //960_TL: total frame length 960 samples
      uint frm1024s    :1; //1024_TL: total frame length 1024 samples
      uint ace_tc      :5; //ACE: Audio Coding Extension Type Code
   } ace_8_10;
   //ACE = 11 (MPEG-H 3D Audio) or 12 (AC-4)
   struct __attribute__ ((packed)) {
      uint ace_afc     :3; //Audio Format Code dependent value
      uint ace_tc      :5; //ACE: Audio Coding Extension Type Code
   } ace_11_12;
   //ACE = 13 (L-PCM 3D Audio)
   struct __attribute__ ((packed)) {
      uint s16bit      :1; //16bit sample
      uint s20bit      :1; //20bit sample
      uint s24bit      :1; //24bit sample
      uint ace_tc      :5; //ACE: Audio Coding Extension Type Code
   } ace_13;
} sad2_afc15_t;

//SAD byte 2:
typedef union __attribute__ ((packed)) { //sad2_s
   //AF-code = 1: LPCM: bits 7:3 are reserved, bits 2:0 define bit depth
   struct __attribute__ ((packed)) {
      uint sample16b   :1; //LPCM: 16bit sample
      uint sample20b   :1; //LPCM: 20bit sample
      uint sample24b   :1; //LPCM: 24bit sample
      uint resvd       :5;
   } afc1_lpcm;
   //AF-code = 2...8 : bitrate div by 8kbit/s
   u8_t afc2_8_bitrate8k;
   //AF-code = 9...13: AF-code dependent value
   u8_t afc9_13_dep_v ;
   //AF-code = 14: WMA-Pro: bits 7:3 are reserved, bits 2:0 define profile
   struct __attribute__ ((packed)) {
      uint profile     :3; //profile
      uint resvd       :5; //reserved
   } afc14_wmapro;
   //AF-code = 15: ACE: Audio Coding Extension Type Code
   sad2_afc15_t afc15_ace;
} sad2_t;

//SAD unknown byte:
typedef struct __attribute__ ((packed)) {
   u8_t unknown; //unknown or invalid data
} sad_unk_t;

//SAD: base struct
typedef struct __attribute__ ((packed)) { //sad_s
   sad0_t sad0; //1: SAD byte 0: format and number of channels
   sad1_t sad1; //2: SAD byte 1: sampling frequencies supported
   sad2_t sad2; //3: SAD byte 2: codec, bitrate, sample size
} sad_t;

//VDB: Video Data Block ->
//SVD: Short Video Descriptor base struct
typedef struct __attribute__ ((packed)) { //svd_s
   uint  vidmd_idx :7;  //1: bit 6-0: video mode idx: 0, 64-127 reserved
   uint  native    :1;  //1: bit 7  : native mode @ idx
} svd_t;

//VSD: Vendor Specific Data Block
//VSD: Sink supported features
typedef struct __attribute__ ((packed)) { //vsd_feat_s
   uint   dvi_dual :1;  //bit0 : DVI Dual Link Operation
   uint   resvd0   :1;  //bit1 : reserved
   uint   resvd1   :1;  //bit2 : reserved
   uint   dc_y444  :1;  //bit3 : DC_Y444  (supports 4:4:4 in deep color modes)
   uint   dc_30bit :1;  //bit4 : DC_30bit (supports 10-bit-per-channel deep color)
   uint   dc_36bit :1;  //bit5 : DC_36bit (supports 10-bit-per-channel deep color)
   uint   dc_48bit :1;  //bit6 : DC_48bit (supports 10-bit-per-channel deep color)
   uint   ai_supp  :1;  //bit7 : Supports_AI (needs info from ACP or ISRC packets)
} vsd_feat_t;

//VSD: Latency
typedef struct __attribute__ ((packed)) { //vsd_ltncy_s
   uint   resvd    :6;  //bit0-5 : reserved
   uint   i_ltncy  :1;  /*bit6 : i_latency_fields (set if interlaced latency fields are present; if set
                                  four latency fields will be present, 0 if bit 7 is 0) */
   uint   ltncy_f  :1;  //bit7 : latency_fields (set if latency fields are present)
} vsd_ltncy_t;

//VSD:
typedef struct __attribute__ ((packed)) { //vsd_s
   u8_t        ieee_id[3]; //1-3: IEEE Registration Id LE, 00-0C-03 for HDMI Licensing, LLC
   u16_t       src_phy;    //4-5: Source Physical Address (section 8.7 of HDMI 1.3a)
   vsd_feat_t  sink_feat;  //6: sink supported features
   u8_t        max_tmds;   //7: (opt) If non-zero (Max_TMDS_Frequency / 5mhz)
   vsd_ltncy_t ltncy_hdr;  //8: (opt) latency fields indicators

   u8_t        vid_lat;    //9: (opt) Video Latency value=1+ms/2 with a max of 251 meaning 500ms
   u8_t        aud_lat;    /*10: (opt) Audio Latency value=1+ms/2 with a max of 251 meaning 500ms
                                 (video delay for progressive sources */
   u8_t        vid_ilat;   //11: (opt) Interlaced Video Latency value=1+ms/2 with a max of 251 meaning 500ms
   u8_t        aud_ilat;   /*12: (opt) Interlaced Audio Latency value=1+ms/2 with a max of 251 meaning 500ms
                                 (video delay for interlaced sources) */
   //Additional bytes may be present, but the HDMI spec says they shall be zero.
} vsd_hdmi14_t;

//SAB: Speaker Allocation Block: bit val. 1 means "present"
//     Field naming convention used after CTA-861-G
typedef struct __attribute__ ((packed)) { //spm0_s
   uint    FL_FR      :1;  //bit0: Front Left/Right
   uint    LFE1       :1;  //bit1: LFE Low Frequency Effects 1
   uint    FC         :1;  //bit2: Front Center
   uint    BL_BR      :1;  //bit3: Back Left/Right
   uint    BC         :1;  //bit4: Back Center
   uint    FLC_FRC    :1;  //bit5: Front Left/Right of Center
   uint    RLC_RRC    :1;  //bit6: Rear Left/Right of Center
   uint    FLW_FRW    :1;  //bit7: Front Left/Right Wide
} spm0_t;

typedef struct __attribute__ ((packed)) { //spm1_s
   uint    TpFL_TpFR  :1;  //bit0: Top Front Left/Right (bug in the CTA-861-G specs: Table 69: TpFH? - undefined, but see table 91)
   uint    TpC        :1;  //bit1: Top Center
   uint    TpFC       :1;  //bit2: Top Front Center
   uint    LS_RS      :1;  //bit3: Left/Right Surround
   uint    LFE2       :1;  //bit4: LFE Low Frequency Effects 2
   uint    TpBC       :1;  //bit5: Top Back Center
   uint    SiL_SiR    :1;  //bit6: Side Left/Right
   uint    TpSiL_TpSiR:1;  //bit7: Top Side Left/Right
} spm1_t;

typedef struct __attribute__ ((packed)) { //spm2_s
   uint    TpBL_TpBR  :1;  //bit0: Top Back Left/Right
   uint    BtFC       :1;  //bit1: Bottom Front Center
   uint    BtFL_BtFR  :1;  //bit2: Bottom Front Left/Right
   uint    TpLS_TpRS  :1;  //bit3: Top Left/Right ? Surround ? - a bug in CTA-861-G ?
   uint    resvd4     :1;  //bit4: reserved (0)
   uint    resvd5     :1;  //bit5: reserved (0)
   uint    resvd6     :1;  //bit6: reserved (0)
   uint    resvd7     :1;  //bit7: reserved (0)
} spm2_t;

typedef struct __attribute__ ((packed)) { //sab_s
   spm0_t  spm0;   //1: Speaker Presence Mask, byte0
   spm1_t  spm1;   //2: Speaker Presence Mask, byte1
   spm2_t  spm2;   //3: Speaker Presence Mask, byte2
} sab_t;

//VTC: VESA Display Transfer Characteristic Data Block.
//     This is the same as in EDID block 0, byte 23 (gamma)
typedef struct __attribute__ ((packed)) { //tag_s
   u8_t   gamma; //value = (gamma*100)-100 (range 1.00–3.54)
} vtc_t;

// CEA/CTA-861 header
//extension block info
typedef struct __attribute__ ((packed)) { //infoblk_s
   uint    num_dtd     :4;  //bit 3-0: number of *native* DTDs included in this block (num_dtd <= num of DTD included)
   uint    ycbcr422    :1;  //bit4:    1 = supports YCbCr 4:2:2
   uint    ycbcr444    :1;  //bit5:    1 = supports YCbCr 4:4:4
   uint    basic_audio :1;  //bit6:    1 = supports basic audio
   uint    underscan   :1;  //bit7:    1 = supports underscan
} infoblk_t;

//CEA header
typedef struct __attribute__ ((packed)) { //cea_hdr_s
   u8_t      ext_tag;   //0 : extension id 0x02 for CEA Timing Extension (CEA-EDID)
   u8_t      rev;       //1 : extension block revision
   u8_t      dtd_offs;  //2 : offset to first DTD, if dtd_offs=0 -> No DTD sections
   infoblk_t info_blk;  //3 : Extension block info: number of DTDs present, other Version 2+ information
   u8_t      dta_start; //4 : First data byte
/* Content: (start at 4th byte)
   if dtd_offs == 0 no other data exists (no DBC and no DTD)
   if dtd_offs  > 4 DBC starts here (Data Block Collection).
   if dtd_offs == 4 no DBC data exists, only DTDs.
   if infoblk_t.num_dtd > 0, then at least num_dtd DTD structures must be present.
   Besides num_dtd DTDs, additional non-mandatory DTDs can be included.
   The last DTD is discovered by checking that Next->DTD.pixel_clock value is zero.
   The remaining padding bytes after last DTD (excluding last byte which contains block checksum)
   should be zeroed.
*/
} cea_hdr_t;


#endif /* CEA_H */
