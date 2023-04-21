/***************************************************************
 * Name:      EDID.h
 * Purpose:   EDID base structure definitions
 * Author:    Tomasz Pawlak (tomasz.pawlak@wp.eu)
 * Created:   2014-03-18
 * Copyright: Tomasz Pawlak (C) 2014-2022
 * License:   GPLv3+
 **************************************************************/

#ifndef EDID_H
#define EDID_H 1

#include "def_types.h"

/* NOTE:
   EDID uses little-endian byte order* for multi-byte fields.
   The structures below are for use with little-endian architectures
   (f.e. x86, x86_64, LE-mode ARM).
   For big-endian arch, many of these structures may not work correctly. (!)

   (*) with one exception: mfc_id field (!) : Big-endian PNP ID by MSFT
*/

typedef unsigned int uint;

//0-7 : std header
typedef union __attribute__ ((packed)) { //hdr_s
   u8_t   hdr_bytes[8];
   u32_t  hdr_uint [2];
} hdr_t;

//8-9 : Manufacturer ID
//LE mfcid: requires byte swapping BE->LE
typedef struct __attribute__ ((packed)) { //mfc_id_s
    uint ltr3     :5;
    uint ltr2     :5;
    uint ltr1     :5;
    uint reserved :1;
} mfc_id_t;

typedef union __attribute__ ((packed)) { //mfc_id_u
   u8_t     ar8[2];
   u16_t    u16;
   mfc_id_t mfc;
} mfc_id_u;

//20–24 : Basic display parameters.
//20: Video Input
typedef union { //vid_in_u
   struct __attribute__ ((packed)) digital_s { //Bit 7=1 : in_type
/*    uint vesa_compat :1;    bit 0    : EDID v1.3: compatibility with VESA DFP 1.x TMDS CRGB,
                                         1 pixel per clock, up to 8 bits per color, MSB aligned */
//    uint reserved    :6;    bits 1-6 : mandatory zeros in EDID v1.3
//    EDID v1.4:
      uint if_type     :4;  //bits 0-3 : digital interface type
      uint color_depth :3;  //bits 4-6 : color bit depth
      uint input_type  :1;  //bit 7    : 1=digital
   } digital;

   struct __attribute__ ((packed)) analog_s {
      uint vsync       :1;  //bit 0 : VSync pulse must be serrated when composite/sync-on-green is used.
      uint sync_green  :1;  //bit 1 : Sync on green supported
      uint comp_sync   :1;  //bit 2 : Composite sync (on HSync) supported
      uint sep_sync    :1;  //bit 3 : Separate sync supported
      uint blank_black :1;  //bit 4 : Blank-to-black setup (pedestal) expected
      uint sync_wh_lvl :2;  //bit 6-5 : Video white and sync levels, relative to blank
      uint input_type  :1;  //bit 7 : 0=analog
   } analog;
} vid_in_t;

//BDD: basic display descriptor
typedef struct __attribute__ ((packed)) { //bdd_s
   u8_t       max_hsize;    //21 : Maximum horizontal image size, in centimetres, 0=undefined
   u8_t       max_vsize;    //22 : Maximum vertical image size, in centimetres, 0=undefined
   u8_t       gamma;        //23 : Display Transfer Characteristics (GAMMA) value = (gamma*100)-100 (range 1.00–3.54)
} bdd_t;

//24 : Supported features: display/signal type/format
typedef struct __attribute__ ((packed)) { //disp_feat_s
   uint gtf_support :1; //bit 0 : GTF supported with default parameter values.
   uint db1_fnative :1; //bit 1 : descriptor block 1: use native pixel format and refresh rate.
   uint std_srbg    :1; //bit 2 : Standard sRGB colour space. Bytes 25–34 must contain sRGB standard values.
   uint vsig_format :2; /* bit 4-3 :
                        Display type (analog): 00 = Monochrome or Grayscale; 01 = RGB color;
                        10 = Non-RGB color; 11 = Undefined
                        Display type (digital): 00 = RGB 4:4:4; 01 = RGB 4:4:4 + YCrCb 4:4:4;
                        10 = RGB 4:4:4 + YCrCb 4:2:2; 11 = RGB 4:4:4 + YCrCb 4:4:4 + YCrCb 4:2:2 */
   uint dpms_off    :1; //bit 5 : DPMS active-off supported
   uint dpms_susp   :1; //bit 6 : DPMS suspend supported
   uint dpms_stby   :1; //bit 7 : DPMS standby supported
} dsp_feat_t;

//25-34 : Chromaticity coordinates. 10-bit CIE xy coords for R,G,B and white. [0–1023/1024].
//25 : Red and green least-significant bits
typedef struct __attribute__ ((packed)) { //rgxy_lsb_s
   uint green_y  :2; //bit 1-0 : Green y value least-significant 2 bits
   uint green_x  :2; //bit 3-2
   uint red_y    :2; //bit 5-4
   uint red_x    :2; //bit 7-6
} rgxy_lsb_t;

//26 : Blue and white least-significant bits
typedef struct __attribute__ ((packed)) { //bwxy_lsb_s
   uint white_y  :2; //bit 1-0 : White y value least-significant 2 bits
   uint white_x  :2; //bit 3-2
   uint blue_y   :2; //bit 5-4
   uint blue_x   :2; //bit 7-6
} bwxy_lsb_t;

//Chromaticity coordinates : base struct
typedef struct __attribute__ ((packed)) { //chromxy_s
   rgxy_lsb_t rgxy_lsbits;  //25 : Red and green xy lsbits (chromacity coords)
   bwxy_lsb_t bwxy_lsbits;  //26 : Blue and white ls 2 bits
   u8_t       red8h_x;      //27 : Red x, 8ms+2ls bits. 0–255 -> 0–0.996 (255/256); 0–0.999 -> (1023/1024) with lsbits
   u8_t       red8h_y;      //28 : Red y
   u8_t       green8h_x;    //29 : Green x
   u8_t       green8h_y;    //30 : Green y
   u8_t       blue8h_x;     //31 : Blue x
   u8_t       blue8h_y;     //32 : Blue y
   u8_t       white8h_x;    //33 : Default white point x value most significant 8 bits
   u8_t       white8h_y;    //34 : Default white point y
} chromxy_t;


//35-37 : Established timing bitmap. Supported bitmap for (formerly) very common timing modes.
typedef struct __attribute__ ((packed)) { //est_map_s
   //byte 35:
   uint m800x600x60   :1; //bit 0
   uint m800x600x56   :1;
   uint m640x480x75   :1;
   uint m640x480x72   :1;
   uint m640x480x67   :1;
   uint m640x480x60   :1;
   uint m720x400x88   :1;
   uint m720x400x70   :1;
   //byte 36:
   uint m1280x1024x75 :1; //bit 0
   uint m1024x768x75  :1;
   uint m1024x768x72  :1;
   uint m1024x768x60  :1;
   uint m1024x768x87i :1; //interlaced
   uint m832x624x75   :1;
   uint m800x600x75   :1;
   uint m800x600x72   :1;
   //byte 37:
   uint m_mfc_spec    :7; //bit 6-0 : manufacturer-specific display modes
   uint m1152x870x75  :1; //bit 7
} est_map_t;

//38–53 : Standard timing information. Up to 8 2-byte fields describing standard display modes.
//        Unused fields are filled with 01 01
//38-39, ...
typedef struct __attribute__ ((packed)) { //std_timg_s
   u8_t      x_res8;       //byte0: Xres= (HActive_pix / 8)–31 (256–2288 pixels, value 00 is reserved.
   uint      v_freq    :6; //byte1: V-freq, less 60 (60–123 Hz)
   uint      pix_ratio :2; /* X:Y pixel ratio: 00=16:10; 01=4:3; 10=5:4; 11=16:9.
                            (Versions prior to 1.3 defined 00 as 1:1.) */
} std_timg_t;

//54-125 : 4x18bytes : EDID Detailed Timing Descriptor(s)

//54+17 : DTD Features flags:
typedef struct __attribute__ ((packed)) { //dtd_feat_s
   uint il2w_stereo    :1; //bit0 : 2-way line-interleaved stereo, if bits 6–5 are not 00.
   uint Hsync_type     :1; /*bit1 : analog sync: 1: Sync on all 3 RGB lines /0: sync_on_green
                                    digital: HSync polarity (1=positive) */
   uint Vsync_type     :1; /*bit2 : separated digital sync: Vsync polarity (1=positive),
                                    Other types: composite VSync (HSync during VSync) */
   uint sync_type      :2; /*bit4-3 : Sync type: 00=Analog composite; 01=Bipolar analog composite;
                                      10=Digital composite (on HSync); 11=Digital separate */
   uint stereo_mode    :2; /*bit6-5 :
                             Stereo mode: 00=No stereo; other values depend on bit 0:
                             Bit 0=0: 01=Field sequential, sync=1 during right;
                             10=similar, sync=1 during left; 11=4-way interleaved stereo
                             Bit 0=1 2-way interleaved stereo: 01=Right image on even lines;
                             10=Left image on even lines; 11=side-by-side */
   uint interlaced     :1; //bit7
} dtd_feat_t;

//54-71, ...
// Detailed Timing Descriptor : DTD
typedef struct __attribute__ ((packed)) { //dtd_s
   u16_t   pix_clk;      /*0-1 : Pixel clock in 10 kHz units (0.01–655.35 MHz, LE).
                                 pix_clk=0 -> not a DTD */

   u8_t    HApix_8lsb;     //2 : Horizontal active pixels 8 lsbits (0–4095)
   u8_t    HBpix_8lsb;     //3 : H-blank pixels 8 lsbits (0–4095) End of active to start of next active.
   uint    HBpix_4msb  :4; //4.3-0 : H-blank pixels 4 msbits
   uint    HApix_4msb  :4; //4.7-4 : Horizontal active pixels 4 msbits

   u8_t    VAlin_8lsb;     //5 : Vertical active lines 8 lsbits (0–4095)
   u8_t    VBlin_8lsb;     //6 : V-blank lines 8 lsbits (0–4095)
   uint    VBlin_4msb  :4; //7.3-0 : V-blank lines 4 msbits
   uint    VAlin_4msb  :4; //7.7-4 : Vertical active lines 4 msbits

   u8_t    HOsync_8lsb;    //8 : H-sync offset in pix 8 lsbits (0–1023) From blanking start
   u8_t    HsyncW_8lsb;    //9 : H-sync pulse width in pix 8 lsbits (0–1023)
   uint    VsyncW_4lsb :4; //10.3-0 : V-sync pulse width in lines 4 lsbits (0–63)
   uint    VOsync_4lsb :4; //10.7-4 : V-sync offset in lines 4 lsbits (0–63)

   uint    VsyncW_2msb :2; //11.1-0 : V-sync pulse width in lines 2 msbits
   uint    VOsync_2msb :2; //11.3-2 : V-sync offset in lines 2 msbits
   uint    HsyncW_2msb :2; //11.5-4 : H-sync pulse width in pix 2 msbits
   uint    HOsync_2msb :2; //11.7-6 : H-sync offset in pix 2 msbits

   u8_t    Hsize_8lsb;     //12 : Horizontal display size, mm, 8 lsbits (0–4095)
   u8_t    Vsize_8lsb;     //13 : Vertical display size, mm, 8 lsbits (0–4095)
   uint    Vsize_4msb  :4; //14.3-0 Vertical display size, mm, 4 lsbits
   uint    Hsize_4msb  :4; //14.7-4 Horizontal display size, mm, 4 msbits
   u8_t    Hborder_pix;    //15 : Horizontal border pixels (each side; total is twice this)
   u8_t    Vborder_lin;    //16 : Vertical border lines (each side; total is twice this)

   dtd_feat_t features;    //17 : Features flags (interlace, sync type, audio format)
} dtd_t;

//DTD variation : Monitor Descriptor

/* Defined descriptor types:
   0xFF: Monitor serial number (text), code page 437
   0xFE: Unspecified text (text), code page 437
   0xFD: Monitor range limits. 6- or 13-byte binary descriptor.
   0xFC: Monitor name (text), padded with 0A 20 20 (LF, SP). code page 437.
   0xFB: Additional white point data. 2× 5-byte descriptors, padded with 0A 20 20.
   0xFA: Additional standard timings identifiers. 6× 2-byte descriptors, padded with 0A.
*/

// Monitor Range Limits Descriptor (type 0xFD) - data
typedef struct __attribute__ ((packed)) { //mrl_s
   u8_t     zero_hdr[3];  //0-2 : *magic* pix_clk=0, HApix_8lsb=0 -> not a DTD
   u8_t     desc_type;    //3 : 0xFA-0xFF defined, 0x00-0x0F for vendors.
   u8_t     zero_req1;    //4 : mandatory zero
   u8_t     min_Vfreq;    //5 : 1-255Hz
   u8_t     max_Vfreq;    //6 : 1-255Hz
   u8_t     min_Hfreq;    //7 : 1-255kHz
   u8_t     max_Hfreq;    //8 : 1-255kHz
   u8_t     max_pixclk;   //9 : Max pixel clock rate, rounded up to 10 MHz multiple (10–2550 MHz)
   u8_t     extd_timg;    /*10 : 00: No information, padded with 0A 20 20 20 20 20 20.
                                 02: Secondary GTF supported, parameters in 12-17 */
   u8_t     resvd;        //11 : Reserved, mandatory zero.
   u8_t     sfreq_sec;    //12 : Start frequency for secondary curve, divided by 2 kHz (0–510 kHz)
   u8_t     gtf_c;        //13 : GTF C value, multiplied by 2 (0–127.5)
   u16_t gtf_m;        //14-15 : GTF M value (0–65535, LE)
   u8_t     gtf_k;        //16 : GTF K value (0–255)
   u8_t     gtf_j;        //17 : GTF J value, multiplied by 2 (0–127.5)
} mrl_t;

// additional White Point Descriptor (type 0xFB) - data
typedef struct __attribute__ ((packed)) { //wpd_s
   u8_t     zero_hdr[3];  //0-2 : *magic* pix_clk=0, HApix_8lsb=0 -> not a DTD
   u8_t     desc_type;    //3 : 0xFA-0xFF defined, 0x00-0x0F for vendors.
   u8_t     zero_req1;    //4 : mandatory zero

   u8_t     wp1_idx;      //5 : White point index number (1–255), 0 -> descriptor not used.
   uint     wp1y_2lsb :2; //6.1-0 White point y value, 2 lsbits
   uint     wp1x_2lsb :2; //6.3-2 White point x value, 2 lsbits
   uint     wp1xy_nu  :4; //6.7-4 unused, mandatory zero.
   u8_t     wp1x_8msb;    //7
   u8_t     wp1y_8msb;    //8
   u8_t     wp1_gamma;    //9 : (gamma*100)-100 (1.0 ... 3.54)

   u8_t     wp2_idx;      //10 : White point index number (1–255), 0 -> descriptor not used.
   uint     wp2y_2lsb :2; //11.1-0 White point y value, 2 lsbits
   uint     wp2x_2lsb :2; //11.3-2 White point x value, 2 lsbits
   uint     wp2xy_nu  :4; //11.7-4 unused, mandatory zero.
   u8_t     wp2x_8msb;    //12
   u8_t     wp2y_8msb;    //13
   u8_t     wp2_gamma;    //14 : (gamma*100)-100 (1.0 ... 3.54)

   u8_t     pad[3];       //15-17 : padded with 0A 20 20. (LF, SP)
} wpd_t;

//MND: Monitor Name Descriptor (type 0xFC) - data
typedef struct __attribute__ ((packed)) { //mnd_s
   u8_t     zero_hdr[3];  //0-2 : *magic* pix_clk=0, HApix_8lsb=0 -> not a DTD
   u8_t     desc_type;    //3 : 0xFA-0xFF defined, 0x00-0x0F for vendors.
   u8_t     zero_req1;    //4 : mandatory zero
   u8_t     text[13];     //15-17 : Monitor Name padded with 0A 20 20. (LF, SP, SP)
} mnd_t;

//AST: Additional Standard Timings identifiers (type 0xFA) - data
typedef struct __attribute__ ((packed)) { //ast_s
   u8_t     zero_hdr[3];  //0-2 : *magic* pix_clk=0, HApix_8lsb=0 -> not a DTD
   u8_t     desc_type;    //3 : 0xFA-0xFF defined, 0x00-0x0F for vendors.
   u8_t     zero_req1;    //4 : mandatory zero
   //STI 0
   u8_t     x0_res8;       //5: X res div by 8, less 31 (256–2288 pixels, value 00 is reserved.
   uint     v0_freq    :6; //6: V-freq, less 60 (60–123 Hz)
   uint     pix0_ratio :2; /* X:Y pixel ratio: 00=16:10; 01=4:3; 10=5:4; 11=16:9.
                            (Versions prior to 1.3 defined 00 as 1:1.) */
   //STI 1
   u8_t     x1_res8;       //7:
   uint     v1_freq    :6; //8:
   uint     pix1_ratio :2; //
   //STI 2
   u8_t     x2_res8;       //9:
   uint     v2_freq    :6; //10:
   uint     pix2_ratio :2; //
   //STI 3
   u8_t     x3_res8;       //11:
   uint     v3_freq    :6; //12:
   uint     pix3_ratio :2; //
   //STI 4
   u8_t     x4_res8;       //13:
   uint     v4_freq    :6; //14:
   uint     pix4_ratio :2; //
   //STI 5
   u8_t     x5_res8;       //15:
   uint     v5_freq    :6; //16:
   uint     pix5_ratio :2; //
   u8_t     pad;           //17: pad: const 0x0A
} ast_t;

//UNK: Unknown Descriptor (type != 0xFA-0xFF)
typedef struct __attribute__ ((packed)) { //unk_s
   u8_t     zero_hdr[3];  //0-2 : *magic* pix_clk=0, HApix_8lsb=0 -> not a DTD
   u8_t     desc_type;    //3 : 0xFA-0xFF defined, 0x00-0x0F for vendors.
   u8_t     zero_req1;    //4 : mandatory zero
   u8_t     unkb[13];     //15-17 : unknown byte values
} unk_t;

//final descriptor type / union
typedef union dsctor_s {
   u8_t     bytes[18]; //
   dtd_t    dtd;       //
   mrl_t    mrl;       // Monitor Range Limits Descriptor (type 0xFD)
   wpd_t    wpd;       // White Point Descriptor (type 0xFB)
   mnd_t    mnd;       // Monitor Name Descriptor (type 0xFC)
   mnd_t    msn;       // MSN: Monitor Serial Number Descriptor (type 0xFF)
   mnd_t    ust;       // UST: UnSpecified Text (type 0xFE)
   ast_t    ast;       // AST: Additional Standard Timing identifiers (type 0xFA)
   unk_t    unk;       // UNK: Unknown Descriptor (type != 0xFA-0xFF)
} dsctor_u;

//-------------------------------------------------------------------------------------------------
//EDID block0: base structure (128 bytes)
typedef struct __attribute__ ((packed)) { //edid_s
   //header - generic info
   hdr_t      hdr;          //0–7 : Fixed header pattern: 0x00_FF_FF_FF_FF_FF_FF_00
   mfc_id_u   mfc_id;       //8-9
   u16_t      prod_id;      //10–11 : product code
   u32_t      serial;       //12–15
   u8_t       prodweek;     //16
   u8_t       year;         //17 (1990–2245). If week=255, it is the model year instead.
   //version info
   u8_t       edid_ver;     //18
   u8_t       edid_rev;     //19
   //video input
   vid_in_t   vinput_dsc;   //20
   //basic display descriptor
   bdd_t      bdd;          //21-23

   dsp_feat_t features;     //24 : Supported features
   //chromacity coords
   chromxy_t  chromxy;      //25-34 : Chromaticity coordinates. 10-bit CIE xy coords for R,G,B and white.
   //resolution map
   est_map_t  res_map;      //35-37 : Supported resolutions flags
   //std timing descriptors
   std_timg_t std_timg0;    //38-39 : 38-53 Standard (old) timing info blocks, 0x0101 if unused
   std_timg_t std_timg1;    //40-41
   std_timg_t std_timg2;    //42-43
   std_timg_t std_timg3;    //44-45
   std_timg_t std_timg4;    //46-47
   std_timg_t std_timg5;    //48-49
   std_timg_t std_timg6;    //50-51
   std_timg_t std_timg7;    //52-53
   //detailed descriptors
   dsctor_u   descriptor0;  //54-71 : DTD, MRL, WPD, ...
   dsctor_u   descriptor1;  //72-89
   dsctor_u   descriptor2;  //90-107
   dsctor_u   descriptor3;  //108-125

   u8_t       num_extblk;   //126 : number of extension data blocks
   u8_t       chksum;       //127 : Sum of all 128 bytes should equal 0 (mod 256).
} edid_t;

/* Descriptor blocks 0-3:
   Detailed timing descriptors, in decreasing preference order.
   After all detailed timing descriptors, additional descriptors are permitted:
     Monitor range limits (required)
     ASCII text (monitor name (required), monitor serial number or unstructured text)
     6 Additional standard timing information blocks
     Color point data
*/

#endif /* EDID_H */
