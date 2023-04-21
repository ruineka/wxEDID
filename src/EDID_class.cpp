/***************************************************************
 * Name:      EDID_class.cpp
 * Purpose:   EDID classes and field handlers
 * Author:    Tomasz Pawlak (tomasz.pawlak@wp.eu)
 * Created:   2014-03-18
 * Copyright: Tomasz Pawlak (C) 2014-2022
 * License:   GPLv3+
 **************************************************************/

#include "debug.h"
#include "rcdunits.h"
#ifndef idEDID
   #error "EDID_class.cpp: missing unit ID"
#endif
#define RCD_UNIT idEDID
#include "rcode/rcode.h"

#include "wxedid_rcd_scope.h"

RCD_AUTOGEN_DEFINE_UNIT

#include <stddef.h>

#include "EDID_class.h"
#include "CEA_class.h"
#include "CEA_EXT_class.h"

#include <wx/arrimpl.cpp>
WX_DEFINE_OBJARRAY(wxArrGrpField);

//unknown/invalid byte field (defined in CEA_class.cpp)
extern const edi_field_t unknown_byte_fld;
extern void insert_unk_byte(edi_field_t *p_fld, u32_t len, u32_t s_offs);

const wxString EDID_cl::prop_flag_name[] = {
   "RD",  //EF_RD
   "NI",  //EF_NI
   "NU",  //EF_NU
   "FR",  //EF_FGR
   "FR",  //EF_INIT, same as Forced Refresh
   "GD",  //EF_GPD
   "VS"   //EF_VS
};

const wxString EDID_cl::val_type_name[] = {
   "Bit",    //EF_BIT
   "BitFld", //EF_BFLD
   "Byte",   //EF_BYTE
   "Int",    //EF_INT
   "Float",  //EF_FLT
   "Hex",    //EF_HEX
   "String", //EF_STR
   "LE"      //EF_LE
};

const wxString EDID_cl::val_unit_name[] = {
   "pix", //EF_PIX
   "mm",  //EF_MM
   "cm",  //EF_CM
   "dm",  //EF_DM
   "Hz",  //EF_HZ
   "kHz", //EF_KHZ
   "MHz", //EF_MHZ
   "ms",  //EF_MLS
   "%"    //EF_PCT
};

rcode EDID_cl::getValUnitName(wxString& sval, const u32_t flags) {
   rcode retU;
   RCD_SET_OK(retU);

   u32_t flg = (flags >> EF_UNSHFT) & EF_UNMASK;

   if (flg == 0) {
      sval.Empty(); //sval = "--"; -> leave it to client
      return retU;
   }

   u32_t mask = 1;

   for (u32_t it=0; it<EF_UNCNT; it++) {
      if ((mask & flg) != 0) {
         sval = val_unit_name[it];
         break;
      }
      mask = mask << 1;
   }

   return retU;
}

rcode EDID_cl::getValTypeName(wxString& sval, const u32_t flags) {
   rcode retU;
   RCD_SET_OK(retU);

   u32_t flg = (flags >> EF_TPSHFT) & EF_TPMASK;

   if (flg == 0) {
      sval = "--";
      return retU;
   }

   u32_t mask = 1;
   sval.Empty();
   for (u32_t it=0; it<EF_TPCNT; it++) {
      if ((mask & flg) != 0) {
         if (sval.Len() > 0) sval << "|";
         sval << val_type_name[it];
      }
      mask = mask << 1;
   }
   return retU;
}

rcode EDID_cl::getValFlagsAsString(wxString& sval, const u32_t flags) {
   rcode retU;
   RCD_SET_OK(retU);

   u32_t flg = (flags >> EF_PRSHFT) & EF_PRMASK;

   if (flg == 0) {
      sval = "--";
      return retU;
   }

   u32_t mask = 1;
   sval.Empty();
   for (u32_t it=0; it<EF_PRCNT; it++) {
      if ((mask & flg) != 0) {
         if (sval.Len() > 0) sval << "|";
         sval << prop_flag_name[it];
      }
      mask = mask << 1;
   }
   return retU;
}

void EDID_cl::getVDesc(wxString &sval, edi_dynfld_t* p_field, u32_t idx) {
   const char* vname;
   //reserved values have NULL vname ptr, idx is checked by the caller
   vname = p_field->field.vmap->vmap[idx].name;
   if (vname != NULL) {
      sval = wxString::FromAscii(vname);
   } else {
      sval = "<reserved>";
   }
}

rcode EDID_cl::ParseDBC_TAG(u8_t *pinst, edi_grp_cl** pp_grp) {
   rcode       retU;
   ethdr_t     ethdr;
   int         tagcode;
   edi_grp_cl *pgrp = NULL;

   RCD_SET_OK(retU);

   ethdr.w16 = reinterpret_cast<u16_t*> (pinst)[0];
   tagcode   = ethdr.ehdr.hdr.tag.tag_code;

   switch (tagcode) {
      case DBC_T_ADB: //1: Audio Data Block
         pgrp = new cea_adb_cl;
         break;
      case DBC_T_VDB: //2: Video Data Block
         pgrp = new cea_vdb_cl;
         break;
      case DBC_T_VSD: //3: Vendor Specific Data Block
         pgrp = new cea_vsd_cl;
         break;
      case DBC_T_SAB: //4: Speaker Allocation Data Block
         pgrp = new cea_sab_cl;
         break;
      case DBC_T_VTC: //5: VESA Display Transfer Characteristic Data Block (gamma)
         pgrp = new cea_vdtc_cl;
         break;
      case DBC_T_EXT: //7: Extended Tag Codes
         {
            u32_t  etag;
            etag = ethdr.ehdr.extag;

            switch (etag) {
               case DBC_EXT_VCDB: //0: Video Capability Data Block
                  pgrp = new cea_vcdb_cl;
                  break;
               case DBC_EXT_VSVD: //1: Vendor-Specific Video Data Block
                  pgrp = new cea_vsvd_cl;
                  break;
               case DBC_EXT_VDDD: //2: VESA Display Device Data Block
                  pgrp = new cea_vddd_cl;
                  break;
               case DBC_EXT_VVTB: //3: VESA Video Timing Block Extension: ? VTB-EXT ? -> BUG in the CTA-861-F/G
                  pgrp = new cea_unket_cl;
                  RCD_SET_FAULT_MSG(retU,"[E!] CTA-861 BUG: VESA Video Timing Block Extension, ExtTagCode=3");
                  break;
               case DBC_EXT_RSV4: //4: Reserved for HDMI Video Data Block
                  pgrp = NULL;
                  break;
               case DBC_EXT_CLDB: //5: Colorimetry Data Block
                  pgrp = new cea_cldb_cl;
                  break;
               case DBC_EXT_HDRS: //6: HDR Static Metadata Data Block
                  pgrp = new cea_hdrs_cl;
                  break;
               case DBC_EXT_HDRD: //7: HDR Dynamic Metadata Data Block
                  pgrp = new cea_hdrd_cl;
                  break;
               case DBC_EXT_VFPD: //13: Video Format Preference Data Block
                  pgrp = new cea_vfpd_cl;
                  break;
               case DBC_EXT_Y42V: //14: YCBCR 4:2:0 Video Data Block
                  pgrp = new cea_y42v_cl;
                  break;
               case DBC_EXT_Y42C: //15: YCBCR 4:2:0 Capability Map Data Block
                  pgrp = new cea_y42c_cl;
                  break;
               case DBC_EXT_RS16: //16: Reserved for CTA Miscellaneous Audio Fields
                  pgrp = NULL;
                  break;
               case DBC_EXT_VSAD: //17: Vendor-Specific Audio Data Block
                  pgrp = new cea_vsad_cl;
                  break;
               case DBC_EXT_RS18: //18: Reserved for HDMI Audio Data Block
                  pgrp = NULL;
                  break;
               case DBC_EXT_RMCD: //19: Room Configuration Data Block
                  pgrp = new cea_rmcd_cl;
                  break;
               case DBC_EXT_SLDB: //20: Speaker Location Data Block
                  pgrp = new cea_sldb_cl;
                  break;
               case DBC_EXT_IFDB: //32  InfoFrame Data Block
                  pgrp = new cea_ifdb_cl;
                  break;
               default:
                  //CTA-861-G: reserved Extended Tag Codes: 8-12, 21-31, 33-255
                  //UNK-ET: Unknown Data Block (Extended Tag Code)
                  pgrp = new cea_unket_cl;
                  wxedid_RCD_SET_FAULT_VMSG(retU,
                                            "[E!] CTA-861 DBC: invalid Extended Tag Code=%u",
                                            etag);
            }
         }
         break;
      default:
         //CTA-861-G: reserved Tag Codes: 0,6
         //UNK-TC: Unknown Data Block (Tag Code)
         pgrp = new cea_unktc_cl;
         wxedid_RCD_SET_FAULT_VMSG(retU,
                                   "[E!] CTA-861 DBC: invalid Tag Code=%u", tagcode);
   }

  *pp_grp = pgrp;
   return retU;
}

rcode EDID_cl::ParseCEA_DBC(u8_t *pinst) {
   rcode       retU;
   rcode       retU2;
   u32_t       orflags;
   edi_grp_cl *pgrp = NULL;

   RCD_SET_OK(retU2);

   retU2 = ParseDBC_TAG(pinst, &pgrp);

   if (pgrp == NULL) {RCD_RETURN_FAULT(retU); }

   pgrp->setAbsOffs(calcGroupOffs(pinst));
   pgrp->setRelOffs(pgrp->getAbsOffs() % sizeof(edid_t));

   orflags = 0;
   if (b_ERR_Ignore) {
      orflags = T_MODE_EDIT;
   }

   retU = pgrp->init(pinst, orflags, NULL);
   if (!RCD_IS_OK(retU)) {
      //the fault message is logged, but ignored
      if (retU.detail.rcode > RCD_FVMSG) {
         delete pgrp;
         return retU;
      }
   }
   pgrp->setParentAr(&EDI_Ext0GrpAr);
   EDI_Ext0GrpAr.Append(pgrp);

   if (! RCD_IS_OK(retU2)) {return retU2;}
   if (! RCD_IS_OK(retU )) {return retU ;}

   RCD_RETURN_OK(retU);
}

rcode EDID_cl::ParseEDID_CEA() {
   rcode       retU;
   edi_grp_cl *pgrp;
   u8_t       *p8_dtd;
   u8_t       *pend;
   u8_t       *pinst;
   u32_t       dtd_offs;
   i32_t       num_dtd;

   EDI_Ext0GrpAr.Empty();

   u8_t *pext = EDID_buff.edi.ext0;

   // CEA/CTA-861 header
   pgrp = new cea_hdr_cl;
   if (pgrp == NULL) RCD_RETURN_FAULT(retU);

   retU = pgrp->init(pext, 0, NULL);
   if (!RCD_IS_OK(retU)) return retU;

   pgrp->setAbsOffs(calcGroupOffs(pext));
   EDI_Ext0GrpAr.Append(pgrp);

   dtd_offs = reinterpret_cast <cea_hdr_t*> (pext)->dtd_offs;
   num_dtd  = reinterpret_cast <cea_hdr_t*> (pext)->info_blk.num_dtd;

   //No DTD, no DBC
   if (dtd_offs == 0) {num_valid_blocks++ ; RCD_RETURN_OK(retU);}

   if ((dtd_offs >= 1) && (3 >= dtd_offs)) {
      wxedid_RCD_SET_FAULT_VMSG(retU, "[E!] CTA-861 Header: invalid DTD offset=%u", dtd_offs);
      return retU; //can't be ignored
   }

   //(dtd_offs > (EDI_BLK_SIZE - sizeof(dsctor_u) -3))

   p8_dtd = (pext + dtd_offs);
   pend   = (pext + EDI_BLK_SIZE);
   pinst  =  pext + offsetof(cea_hdr_t, dta_start);

   //DBC
   if (dtd_offs > 0x04) {
      ethdr_t  ethdr;
      u32_t    blklen;

      do {
         //Parse Data Block Collection (DBC)
         ethdr.w16 = reinterpret_cast<u16_t*> (pinst)[0];
         blklen    = ethdr.ehdr.hdr.tag.blk_len;

         if (p8_dtd < (pinst + blklen)) {
            wxedid_RCD_SET_FAULT_VMSG(retU,
                                      "[E!] CTA-861: Collision: DTD offset=%u and DBC@offset=%u, len=%u",
                                      dtd_offs, (u32_t) (calcGroupOffs(pinst) % sizeof(edid_t)), blklen);
            if (! b_ERR_Ignore) return retU;
            pGLog->PrintRcode(retU);
            RCD_SET_OK(retU);
         }

         retU = ParseCEA_DBC(pinst);
         if (!RCD_IS_OK(retU)) {
            if (! b_ERR_Ignore) return retU;
            RCD_SET_OK(retU);
         }

         pinst ++ ; //DBC hdr byte
         pinst += blklen;
         if (pinst >= p8_dtd) break;

      } while (pinst < pend);

      if (pinst != p8_dtd) {
         wxedid_RCD_SET_FAULT_VMSG(retU,
                                   "[E!] CTA-861: DTD offset=%u != DBC_end=%u",
                                   dtd_offs, (u32_t) (calcGroupOffs(pinst) % sizeof(edid_t)) );
         if (! b_ERR_Ignore) return retU;
         pGLog->PrintRcode(retU);
         RCD_SET_OK(retU);
      }
   }

   {  //DTD
      dtd_t      *pdtd;
      edi_grp_cl *pgrp;
      i32_t       space_left;
      i32_t       max_dtd;
      u32_t       offs;

      //search DTDs after DBC end, not at declared DTD offset
      pdtd        = reinterpret_cast<dtd_t*> (pinst);
      space_left  = (pend - pinst);
      space_left -- ; //last byte contains checksum
      max_dtd     = space_left / sizeof(dtd_t); //18

      //mandatory DTDs for *native* mode
      if (num_dtd > 0) {
         if (max_dtd < num_dtd) {
            wxedid_RCD_SET_FAULT_VMSG(retU,
                                      "[E!] CTA-861: insufficient space for declared number of native DTDs: %u, max: %u",
                                      num_dtd, max_dtd);
            if (! b_ERR_Ignore) return retU;
            pGLog->PrintRcode(retU);
            num_dtd = max_dtd;
            RCD_SET_OK(retU);
         }

         for (i32_t itd=0; itd<num_dtd; itd++) {

            if (pdtd->pix_clk == 0) {
               wxedid_RCD_SET_FAULT_VMSG(retU,
                                         "[E!] CTA-861: missing mandatory DTD @ offset %u",
                                         (u32_t) (reinterpret_cast <u8_t*> (pdtd) - EDID_buff.buff) );
               if (! b_ERR_Ignore) return retU;
               pGLog->PrintRcode(retU);
               RCD_SET_OK(retU);
               break;
            }

            pgrp = new dtd_cl;
            if (pgrp == NULL) {
               RCD_SET_FAULT(retU);
               pGLog->PrintRcode(retU);
               return retU;
            }

            pgrp->init(reinterpret_cast <u8_t*> (pdtd), 0, NULL);

            offs = (reinterpret_cast <u8_t*> (pdtd) - EDID_buff.buff);

            //append the group
            pgrp->setAbsOffs(offs);                  //offset in buffer
            pgrp->setRelOffs(offs % sizeof(edid_t)); //offset in extension block
            EDI_Ext0GrpAr.Append(pgrp);

            pdtd       ++ ;
            max_dtd    -- ;
            space_left -= sizeof(dtd_t);
         }
      }

      //Check for additional DTDs (non-mandatory)
      for (i32_t itd=0; itd<max_dtd; itd++) {
         if (pdtd->pix_clk == 0) break; //not a DTD

         pgrp = new dtd_cl;
         if (pgrp == NULL) {
            RCD_SET_FAULT(retU);
            pGLog->PrintRcode(retU);
            return retU;
         }

         pgrp->init(reinterpret_cast <u8_t*> (pdtd), 0, NULL);

         offs = (reinterpret_cast <u8_t*> (pdtd) - EDID_buff.buff);

         //append the group
         pgrp->setAbsOffs(offs);
         pgrp->setRelOffs(offs % sizeof(edid_t));
         EDI_Ext0GrpAr.Append(pgrp);

         pdtd       ++ ;
         space_left -= sizeof(dtd_t);
      }

      //Check padding bytes
      p8_dtd = reinterpret_cast <u8_t*> (pdtd);

      for (i32_t itb=0; itb<space_left; itb++) {
         if (*p8_dtd != 0) {
            wxedid_RCD_SET_FAULT_VMSG(retU,
                                      "[E!] CTA-861: padding byte != 0 @ offset %u",
                                      (u32_t) (p8_dtd - EDID_buff.buff) );
            if (! b_ERR_Ignore) return retU;
            pGLog->PrintRcode(retU);
            RCD_SET_OK(retU);
         }

         p8_dtd ++ ;
      }
   }

   num_valid_blocks++ ;

   //free/used used bytes in the block
   EDI_Ext0GrpAr.CalcDataSZ();

   RCD_RETURN_OK(retU);
}

rcode EDID_cl::ParseEDID_Base(u32_t& n_extblk) {
   rcode       retU;
   edi_grp_cl *pgrp;

   EDI_BaseGrpAr.Empty();
   num_valid_blocks = 0;

   //check header
   if (!b_ERR_Ignore && ((EDID_buff.edi.base.hdr.hdr_uint[0] != 0xFFFFFF00) ||
                       (EDID_buff.edi.base.hdr.hdr_uint[1] != 0x00FFFFFF)) )
   {
      wxedid_RCD_SET_FAULT_VMSG(retU,
                                "[E!] EDID block0: invalid header=0x%08X_%08X",
                                EDID_buff.edi.base.hdr.hdr_uint[0],
                                EDID_buff.edi.base.hdr.hdr_uint[1] );
      return retU;
   }
   //BED: Base EDID data
   pgrp = new edibase_cl;
   if (pgrp == NULL) RCD_RETURN_FAULT(retU);
   retU = pgrp->init(reinterpret_cast <u8_t*> (&EDID_buff.edi.base.hdr), 0, NULL);
   if (!RCD_IS_OK(retU)) return retU;
   EDI_BaseGrpAr.Append(pgrp);
   //VID: Video Input Descriptor
   pgrp = new vindsc_cl;
   if (pgrp == NULL) RCD_RETURN_FAULT(retU);
   retU = pgrp->init(reinterpret_cast <u8_t*> (&EDID_buff.edi.base.vinput_dsc), 0, NULL);
   if (!RCD_IS_OK(retU)) return retU;
   EDI_BaseGrpAr.Append(pgrp);
   //BDD: basic display descriptior
   pgrp = new bddcs_cl;
   if (pgrp == NULL) RCD_RETURN_FAULT(retU);
   retU = pgrp->init( reinterpret_cast <u8_t*> (&EDID_buff.edi.base.bdd), 0, NULL);
   if (!RCD_IS_OK(retU)) return retU;
   EDI_BaseGrpAr.Append(pgrp);
   //SPF: Supported features class
   pgrp = new spft_cl;
   if (pgrp == NULL) RCD_RETURN_FAULT(retU);
   retU = pgrp->init(reinterpret_cast <u8_t*> (&EDID_buff.edi.base.features), 0, NULL);
   if (!RCD_IS_OK(retU)) return retU;
   EDI_BaseGrpAr.Append(pgrp);
   //CXY: CIE Chromacity coords class
   pgrp = new chromxy_cl;
   if (pgrp == NULL) RCD_RETURN_FAULT(retU);
   retU = pgrp->init(reinterpret_cast <u8_t*> (&EDID_buff.edi.base.chromxy), 0, NULL);
   if (!RCD_IS_OK(retU)) return retU;
   EDI_BaseGrpAr.Append(pgrp);
   //Resolution map class
   pgrp = new resmap_cl;
   if (pgrp == NULL) RCD_RETURN_FAULT(retU);
   retU = pgrp->init(reinterpret_cast <u8_t*> (&EDID_buff.edi.base.res_map), 0, NULL);
   if (!RCD_IS_OK(retU)) return retU;
   EDI_BaseGrpAr.Append(pgrp);
   //Std Timing Descriptors
   {
      std_timg_t *pstdt = &EDID_buff.edi.base.std_timg0;
      for (u32_t itd=0; itd<8; itd++) {
         u32_t  flags = 0;
         u32_t  abs_offs;

         pgrp = new sttd_cl;
         if (pgrp == NULL) RCD_RETURN_FAULT(retU);

         abs_offs = calcGroupOffs(pstdt);
         pgrp->setAbsOffs(abs_offs);
         pgrp->setRelOffs(abs_offs);
         //check for unused descriptors
         {
            u8_t *ptr = reinterpret_cast <u8_t*> (&EDID_buff.edi.base);
            if ((reinterpret_cast <u16_t*> (ptr + pgrp->getAbsOffs() ))[0] == 0x0101) flags = EF_NU;
         }
         pgrp->init(reinterpret_cast <u8_t*> (pstdt), flags, NULL);
         EDI_BaseGrpAr.Append(pgrp);
         pstdt += 1;
      }
   }
   //DTD/MRL/WPT...
   {
      dsctor_u *pdsc = &EDID_buff.edi.base.descriptor0;
      for (u32_t itd=0; itd<4; itd++) {
         u32_t  abs_offs;

         pgrp = ParseDetDtor(0, pdsc, retU); //Base EDID struct idx == 0.
         if (pgrp == NULL) return retU;
         //offset in buffer
         abs_offs = calcGroupOffs(pdsc);
         pgrp->setAbsOffs(abs_offs);
         pgrp->setRelOffs(abs_offs);

         EDI_BaseGrpAr.Append(pgrp);
         pdsc += 1;
      }
   }

   num_valid_blocks = 1;
   n_extblk         = EDID_buff.edi.base.num_extblk;

   //free/used used bytes in the block
   EDI_BaseGrpAr.CalcDataSZ();

   RCD_RETURN_OK(retU);
}

edi_grp_cl* EDID_cl::ParseDetDtor(u32_t blkidx, dsctor_u* pdsc, rcode& retU) {
   u32_t       dsctype;
   edi_grp_cl *pgrp;

   //DTD
   if (pdsc->dtd.pix_clk != 0) {
      RCD_SET_OK(retU);

      pgrp = new dtd_cl;
      if (pgrp == NULL) {RCD_SET_FAULT(retU); return NULL; }

      pgrp->init(reinterpret_cast <u8_t*> (pdsc), 0, NULL);
      return pgrp;
   }

   dsctype = pdsc->mrl.desc_type;

   //not a DTD: types 0xFA...0xFF
   switch (dsctype) {
      case 0xFA: //AST: Additional Standard Timings identifiers (type 0xFA)
         pgrp = new ast_cl;
         if (pgrp == NULL) {RCD_SET_FAULT(retU); return NULL; }
         break;
      case 0xFB: //WPD: White Point Descriptor
         pgrp = new wpt_cl;
         if (pgrp == NULL) {RCD_SET_FAULT(retU); return NULL; }
         break;
      case 0xFC: //MND: Monitor Name Descriptor (type 0xFC)
         pgrp = new mnd_cl;
         if (pgrp == NULL) {RCD_SET_FAULT(retU); return NULL; }
         break;
      case 0xFD: //MRL: Monitor Range Limits
         pgrp = new mrl_cl;
         if (pgrp == NULL) {RCD_SET_FAULT(retU); return NULL; }
         break;
      case 0xFE: //UST: UnSpecified Text (type 0xFE)
         pgrp = new utx_cl;
         if (pgrp == NULL) {RCD_SET_FAULT(retU); return NULL; }
         break;
      case 0xFF: //MSN: Monitor Serial Number Descriptor (type 0xFF)
         pgrp = new msn_cl;
         if (pgrp == NULL) {RCD_SET_FAULT(retU); return NULL; }
         break;
      default:
         //UNK: Unknown Descriptor (fallback)
         pgrp = new unk_cl;
         if (pgrp == NULL) {RCD_SET_FAULT(retU); return NULL; }
         wxedid_RCD_SET_FAULT_VMSG(retU,
                                   "[E!] EDID block%u: unknown descriptor type=0x%02X @offset=%u",
                                   blkidx, dsctype, calcGroupOffs(pdsc));
         pGLog->PrintRcode(retU);
         break;
   }

   retU = pgrp->init(reinterpret_cast <u8_t*> (pdsc), 0, NULL);
   if (! RCD_IS_OK(retU)) {
      delete pgrp;
      pgrp = NULL;
   }

   return pgrp;
}

bool EDID_cl::VerifyChksum(u32_t block) {
   if (block > EDI_EXT2_IDX) return false;

   u32_t csum = 0;
   u8_t *pblk = EDID_buff.blk[block];

   for (u32_t itb=0; itb<EDI_BLK_SIZE; itb++) {
      csum += pblk[itb];
   }
   return ((csum & 0xFF) == 0);
}

u32_t EDID_cl::genChksum(u32_t block) {
   if (block > EDI_EXT2_IDX) return 0;

   u32_t csum = 0;
   u8_t *pblk = EDID_buff.blk[block];

   for (u32_t itb=0; itb<(EDI_BLK_SIZE-1); itb++) {
      csum += pblk[itb];
   }
   csum = (0x100 - (csum & 0xFF));
   pblk[EDI_BLK_SIZE-1] = csum;
   return csum;
}

void EDID_cl::Clear() {

   memset(EDID_buff.buff, 0, sizeof(edi_buf_t) );
}

void EDID_cl::CEA_Set_DTD_Offset(u8_t *pbuf, GroupAr_cl *p_grp_ar) {
   edi_grp_cl *pgrp;
   cea_hdr_t  *cea_hdr;
   u32_t       num_dtd;
   u32_t       idx;
   u32_t       grp_cnt;
   u32_t       dtd_offs = 0;

   cea_hdr = reinterpret_cast<cea_hdr_t*> (pbuf);
   grp_cnt = p_grp_ar->GetCount();
   num_dtd = 0;
   idx     = 1; //skip CEA header

   for (; idx<grp_cnt; ++idx) {
      u32_t  tid;
      pgrp  = p_grp_ar->Item(idx);
      tid   = pgrp->getTypeID();
      tid  &= ID_EDID_MASK;

      if (ID_DTD == tid) {
         if (num_dtd == 0) {
            dtd_offs = pgrp->getRelOffs();
         }
         num_dtd ++ ;
      }
   }

   if (num_dtd > 0) goto done;
   if (grp_cnt < 1) dtd_offs = 0; //No DBC and no DTD

done:
   cea_hdr->dtd_offs = dtd_offs;

   {
      u32_t num_dbc;

      num_dbc  = grp_cnt;
      num_dbc -= num_dtd;
      num_dbc -= 1; //hdr

      pGLog->slog.Printf("CEA_Set_DTD_Offset(): num_dbc=%u, num_dtd=%u, dtd_offs=%u",
                         num_dbc, num_dtd, dtd_offs);
      pGLog->DoLog();
   }
}

rcode EDID_cl::AssembleEDID() {
   edi_grp_cl  *pgrp;
   GroupAr_cl  *p_grp_ar;

   u8_t  *pbuf;
   u32_t  block;
   i32_t  blk_sz;
   u32_t  offs;
   u32_t  idx_grp;
   u32_t  subg_idx;
   rcode  retU;

   for (block=0; block<num_valid_blocks; ++block) {
      pbuf     = EDID_buff.blk[block];
      p_grp_ar = BlkGroupsAr  [block];
      blk_sz   = sizeof(edid_t);
      blk_sz  -= 1; //checksum byte
      offs     = 0;

      if (block == 0) blk_sz -= 1; //edid_t.num_extblk

      for (idx_grp=0; idx_grp<p_grp_ar->GetCount(); ++idx_grp) {
         u32_t       n_subg;
         u32_t       dat_sz;
         edi_grp_cl *p_subg;

         if (blk_sz <= 0) { goto err; }

         pgrp   = p_grp_ar->Item(idx_grp);
         dat_sz = pgrp->getDataSize();
         n_subg = pgrp->getSubGrpCount();

         pgrp->SpawnInstance(&pbuf[offs]);
         offs   += dat_sz;
         blk_sz -= dat_sz;

         for (subg_idx=0; subg_idx<n_subg; ++subg_idx) {

            if (blk_sz <= 0) { goto err; }

            p_subg = pgrp->getSubGroup(subg_idx);
            dat_sz = p_subg->getDataSize();

            p_subg->SpawnInstance(&pbuf[offs]);
            offs   += dat_sz;
            blk_sz -= dat_sz;
         }
      }

      //base EDID:
      if (block == 0) {
         if (blk_sz > 0) {
            wxedid_RCD_SET_FAULT_VMSG(retU, "[E!] AssembleEDID(): Base EDID block: missing descriptor @offset=%u", offs);
            return retU;
         }
      }

      //CEA:
      if (block == 1) {
         //Update DTD offset
         CEA_Set_DTD_Offset(pbuf, p_grp_ar);
         //clear unused bytes
         for (i32_t itb=0; itb<blk_sz; ++itb) {
            pbuf[offs] = 0;
            offs ++ ;
         }
      }

   }

   RCD_RETURN_OK(retU);

err:
   wxedid_RCD_SET_FAULT_VMSG(retU, "[E!] AssembleEDID(): Block[%u] size limit reached: [idx=%u] \'%s\', sub-group idx=%u",
                             block, idx_grp, (const char*) pgrp->GroupName.c_str(), subg_idx);
   return retU;
}

//Common handlers
rcode EDID_cl::BitVal(u32_t op, wxString& sval, u32_t& ival, edi_dynfld_t* p_field) {
   rcode  retU;
   u8_t  *inst;
   u8_t   bmask;

   inst = getValPtr(p_field);

   if ((p_field->field.flags & EF_BIT) == 0) RCD_RETURN_FAULT(retU);

   bmask = (1 << p_field->field.shift);

   if (op == OP_READ) { //read
      ival = ((inst[0] & bmask) >> p_field->field.shift);
      sval << ival;
      RCD_SET_OK(retU);
   } else { //write
      ulong val = 0;
      RCD_SET_FAULT(retU);

      if (op == OP_WRSTR) {
         //immediate min and max are used -> this is a bit value
         retU = getStrUint(sval, 10, 0, 1, val);
         if (! RCD_IS_OK(retU)) return retU;
      } else if (op == OP_WRINT) {
         if ((p_field->field.flags & EF_NI) != 0) RCD_RETURN_FAULT(retU);
         val = ival;
         RCD_SET_OK(retU);
      } else {
         RCD_RETURN_FAULT(retU); //wrong op code
      }
      val = (val << p_field->field.shift);
      inst[0] &= ~bmask;
      inst[0] |= (val & bmask);
   }
   return retU;
}

rcode EDID_cl::BitF8Val(u32_t op, wxString& sval, u32_t& ival, edi_dynfld_t* p_field) {
   rcode  retU;
   u8_t  *inst;

   inst = getValPtr(p_field);

   if ((p_field->field.flags & EF_BFLD) == 0) RCD_RETURN_FAULT(retU);
   if ((p_field->field.fldsize + p_field->field.shift) > 8) RCD_RETURN_FAULT(retU);

   if (op == OP_READ) {
      ulong bmask = (0xFF >> (8 - p_field->field.fldsize));
      ival = ((inst[0] >> p_field->field.shift) & bmask);

      if (p_field->field.flags & EF_INT) {
         sval.Empty(); sval << ival;
         RCD_SET_OK(retU);
      } else
      if (p_field->field.flags & EF_HEX) {
         sval.Printf("0x%02X", ival);
         RCD_SET_OK(retU);
      } else {
         ulong tmpv = ival;
         uint  itb;
         char  chbit[12]; chbit[11] = 0;

         //read by bit
         for (itb=0; itb<p_field->field.fldsize; itb++) {
            chbit[10-itb] = 0x30+(tmpv & 0x01); //to ASCII
            tmpv = (tmpv >> 1);
         }
         tmps = wxString::FromAscii(&chbit[(11-itb)]);
         sval = "0b";
         sval << tmps;
         RCD_SET_OK(retU);
      }

   } else {
      ulong tmpv = 0;
      int   base;
      RCD_SET_FAULT(retU);
      ulong bmask = ((0xFF >> (8 - p_field->field.fldsize)) << p_field->field.shift);

      if (op == OP_WRSTR) {

         if (p_field->field.flags & EF_INT) {
            base = 10;
         } else if (p_field->field.flags & EF_HEX) {
            base = 16;
         } else {
            base = 2;
         }
         retU = getStrUint(sval, base, p_field->field.minv, p_field->field.maxv, tmpv);
         if (! RCD_IS_OK(retU)) return retU;

      } else
      if (op == OP_WRINT) {
         if ((p_field->field.flags & EF_NI) != 0) RCD_RETURN_FAULT(retU);
         tmpv = ival;
         RCD_SET_OK(retU);
      } else {
         RCD_RETURN_FAULT(retU); //wrong op code
      }
      tmpv = ((tmpv << p_field->field.shift) & bmask);
      inst[0] &= ~bmask;
      inst[0] |= tmpv;
   }
   return retU;
}

rcode EDID_cl::ByteVal(u32_t op, wxString& sval, u32_t& ival, edi_dynfld_t* p_field) {
   rcode  retU;
   u8_t  *inst;

   inst = getValPtr(p_field);

   if ((p_field->field.flags & (EF_BIT|EF_STR)) != 0) RCD_RETURN_FAULT(retU);

   if (op == OP_READ) { //read
      ival = inst[0];
      if (p_field->field.flags & EF_INT) {
         sval.Empty(); sval << ival;
         RCD_SET_OK(retU);
      } else
      if (p_field->field.flags & EF_HEX) {
         sval.Printf("0x%02X", ival);
         RCD_SET_OK(retU);
      } else {
         RCD_SET_FAULT(retU);
      }

   } else { //write
      ulong val = 0;
      RCD_SET_FAULT(retU);

      if (op == OP_WRSTR) {
         if (p_field->field.flags & EF_INT) {
            retU = getStrUint(sval, 10, p_field->field.minv, p_field->field.maxv, val);
         }
         if (p_field->field.flags & EF_HEX) {
            if (sval.SubString(0, 1) != "0x") RCD_RETURN_FAULT(retU);
            retU = getStrUint(sval, 16, p_field->field.minv, p_field->field.maxv, val);
         }
         if (! RCD_IS_OK(retU)) return retU;
      } else
      if (op == OP_WRINT) {
         if ((p_field->field.flags & EF_NI) != 0) RCD_RETURN_FAULT(retU);
         val = ival;
         RCD_SET_OK(retU);
      } else {
         RCD_RETURN_FAULT(retU); //wrong op code
      }
      inst[0] = val;
   }
   return retU;
}

rcode EDID_cl::FldPadString(u32_t op, wxString& sval, u32_t& ival, edi_dynfld_t* p_field) {
   rcode  retU;
   u32_t  itb;
   u8_t  *inst;

   inst = getValPtr(p_field);

   u32_t maxl = p_field->field.fldsize;
   if (maxl > 128) RCD_RETURN_FAULT(retU);

   char cbuff[maxl+1];

   if (op == OP_READ) {
      //max maxl chars allowed
      for (itb=0; itb<maxl; itb++) {
         cbuff[itb] = inst[itb];
      }
      cbuff[itb] = 0;
      sval = wxString::FromAscii(cbuff);
      RCD_SET_OK(retU);

   } else {
      if (op == OP_WRINT) RCD_RETURN_FAULT(retU);
      if (sval.Len() > maxl) RCD_RETURN_FAULT(retU);

      memcpy(cbuff, sval.ToAscii(), sval.Len());

      for (itb=0; itb<sval.Len(); itb++) {
         char chr = cbuff[itb];
         if (chr == 0) break;
         inst[itb] = chr;
      }
      //padding
      if (itb < maxl) inst[itb++] = 0x0A; //LF
      for (; itb<maxl; itb++) {
         inst[itb] = 0x20; //SP
      }
      RCD_SET_OK(retU);
   }
   ival = 0;
   return retU;
}

rcode EDID_cl::getStrUint(wxString& sval, int base, u32_t minv, u32_t maxv, ulong& val) {
   rcode retU;
   ulong tmpv;

   RCD_SET_OK(retU);

   //check conversion base
   if (base != 10) { // expect '0b' or '0x' prefix
      u32_t slen = sval.Len();
      if (slen < 3) RCD_RETURN_FAULT(retU);

      wxString prefix = sval.SubString(0, 1);
      switch (base) {
         case 16:
            if (prefix.Cmp("0x") != 0) RCD_RETURN_FAULT(retU);
            break;
         case 2:
            if (prefix.Cmp("0b") != 0) RCD_RETURN_FAULT(retU);
            break;
         default:
            RCD_RETURN_FAULT(retU);
      }

      if (! (sval.SubString(2, slen)).ToULong(&tmpv, base)) {
         RCD_RETURN_FAULT(retU);
      }
   } else {
      if (! sval.ToULong(&tmpv, base)) {
         RCD_RETURN_FAULT(retU);
      }
   }

   if (tmpv > maxv) {
      RCD_SET_FAULT(retU);
   }
   if (tmpv < minv) {
      RCD_SET_FAULT(retU);
   }
   val = tmpv;
   return retU;
}

rcode EDID_cl::getStrDouble(wxString& sval, const double minv, const double maxv, double& val) {
   rcode  retU;
   double dval;

   RCD_SET_OK(retU);

   if (! sval.ToDouble(&dval)) {
      RCD_SET_FAULT(retU);
   }
   if (dval > maxv) {
      RCD_SET_FAULT(retU);
   }
   if (dval < minv) {
      RCD_SET_FAULT(retU);
   }
   val = dval;
   return retU;
}

rcode EDID_cl::Gamma(u32_t op, wxString& sval, u32_t& ival, edi_dynfld_t* p_field) {
   rcode   retU;
   uint    tmpv;
   double  fval;
   void*   inst = NULL;

   inst = getValPtr(p_field);

   if (op == OP_READ) {
      tmpv = ( (reinterpret_cast <u8_t*> (inst))[0] + 100);
      ival = tmpv;
      fval = tmpv;
      sval.Printf("%.02f", (fval/100.0));
   } else {
      if (op == OP_WRINT) RCD_RETURN_FAULT(retU);

      retU = getStrDouble(sval, 1.0, 3.54, fval);
      if (! RCD_IS_OK(retU)) return retU;

      tmpv = (fval*100.0);
      *(reinterpret_cast <u8_t*> (inst)) = (tmpv - 100);
   }

   RCD_RETURN_OK(retU);
}

rcode EDID_cl::rdByteStr(wxString& sval, u8_t* pstrb, u32_t slen) {
   rcode  retU;
   if (pstrb == NULL) RCD_RETURN_FAULT(retU);

   sval = "0x";
   for (u32_t itb=0; itb<slen; itb++) {
      tmps.Printf("%02X", pstrb[itb]);
      sval << tmps;
   }

   RCD_RETURN_OK(retU);
}

rcode EDID_cl::rdByteStrLE(wxString& sval, u8_t* pstrb, u32_t slen) {
   rcode  retU;
   if (pstrb == NULL) RCD_RETURN_FAULT(retU);

   sval = "0x";
   for (ssize_t itb=(slen-1); itb>=0; itb--) {
      tmps.Printf("%02X", pstrb[itb]);
      sval << tmps;
   }

   RCD_RETURN_OK(retU);
}

rcode EDID_cl::wrByteStr(wxString& sval, u8_t* pstrb, u32_t slen) {
   rcode  retU;
   if (pstrb == NULL) RCD_RETURN_FAULT(retU);

   //required prefix
   if (sval.SubString(0, 1).Cmp("0x") != 0) RCD_RETURN_FAULT(retU);

   u32_t clen = sval.Len();
   if ((clen%2) != 0) RCD_RETURN_FAULT(retU);
   if (((clen-2)/2) > slen) RCD_RETURN_FAULT(retU);

   clen-- ;
   u32_t itc  = 2;
   ulong tmpv  = 0;
   for (u32_t itb=0; itb<slen; itb++) {
      if (itc >= clen) break;
      if (! sval.SubString(itc, itc+1).ToULong(&tmpv, 16)) {
         RCD_RETURN_FAULT(retU);
      }
      pstrb[itb] = static_cast <u8_t> (tmpv);
      itc+=2;
   }

   RCD_RETURN_OK(retU);
}

rcode EDID_cl::wrByteStrLE(wxString& sval, u8_t* pstrb, u32_t slen) {
   rcode  retU;
   if (pstrb == NULL) RCD_RETURN_FAULT(retU);

   //required prefix
   if (sval.SubString(0, 1).Cmp("0x") != 0) {
      RCD_RETURN_FAULT(retU);
   }

   u32_t itc = sval.Len();
   if (itc < 4) { //1 byte minimum: 0xBB
      RCD_RETURN_FAULT(retU);}
   if ((itc%2) != 0) { //incomplete byte
      RCD_RETURN_FAULT(retU);}
   itc = (itc-2)/2;
   if (itc > slen) { //too long
      RCD_RETURN_FAULT(retU);}

   ssize_t itb = (slen-1);
   ulong tmpv  = 0;
   if (itc < slen) {
      do {
         pstrb[itb--] = 0;
         slen-- ;
      } while (itc < slen);
   }

   itc = 2;
   for (; itb>=0; itb--) {
      if (! sval.SubString(itc, itc+1).ToULong(&tmpv, 16)) {
         RCD_RETURN_FAULT(retU);
      }
      pstrb[itb] = static_cast <u8_t> (tmpv);
      itc+=2;
   }

   RCD_RETURN_OK(retU);
}

rcode EDID_cl::ByteStr(u32_t op, wxString& sval, u32_t& ival, edi_dynfld_t* p_field) {
   rcode  retU;
   u8_t  *inst;

   RCD_SET_OK(retU);

   inst = getValPtr(p_field);

   if ((p_field->field.flags & EF_STR) == 0) RCD_RETURN_FAULT(retU);

   if (op == OP_READ) {
      if ((p_field->field.flags & EF_LE) == 0) {
         retU = rdByteStr(sval, inst, p_field->field.fldsize);
      } else {
         retU = rdByteStrLE(sval, inst, p_field->field.fldsize);
      }
   } else {
      if (op == OP_WRINT) RCD_RETURN_FAULT(retU);

      if ((p_field->field.flags & EF_LE) == 0) {
         retU = wrByteStr(sval, inst, p_field->field.fldsize);
      } else {
         retU = wrByteStrLE(sval, inst, p_field->field.fldsize);
      }
   }
   ival = 0;
   return retU;
}

//BED: Base EDID data : handlers
rcode EDID_cl::MfcId(u32_t op, wxString& sval, u32_t& ival, edi_dynfld_t* p_field) {
   rcode  retU;
   u8_t  *inst;

   inst = getValPtr(p_field); //EDID_buff.edi.base.mfc_id.ar8[x]

   mfc_id_u mfc_swap;
   char cbuff[4];

   if (op == OP_READ) {
      //MfcId: ASCII letters A-Z less 0x40

      //swap bytes in PNP_ID - BE->LE byte order:
      mfc_swap.ar8[0] = inst[1];
      mfc_swap.ar8[1] = inst[0];
      ival = mfc_swap.u16;

      cbuff[0] = 0x40+mfc_swap.mfc.ltr1;
      cbuff[1] = 0x40+mfc_swap.mfc.ltr2;
      cbuff[2] = 0x40+mfc_swap.mfc.ltr3;
      cbuff[3] = 0;
      sval = wxString::FromAscii(cbuff);
   } else {
      if (op == OP_WRINT) RCD_RETURN_FAULT(retU);
      if (sval.Len() != 3) RCD_RETURN_FAULT(retU);

      memcpy(cbuff, sval.ToAscii(), 3);
      mfc_swap.mfc.ltr1 = (cbuff[0]-0x40) & 0x1F;
      mfc_swap.mfc.ltr2 = (cbuff[1]-0x40) & 0x1F;
      mfc_swap.mfc.ltr3 = (cbuff[2]-0x40) & 0x1F;
      mfc_swap.mfc.reserved = 0;

      inst[0] = mfc_swap.ar8[1]; ////EDID_buff.edi.base.mfc_id.ar8[x]
      inst[1] = mfc_swap.ar8[0];
   }

   RCD_RETURN_OK(retU);
}

rcode EDID_cl::ProdSN(u32_t op, wxString& sval, u32_t& ival, edi_dynfld_t* p_field) {
   rcode  retU;
   u32_t *inst;

   inst = (u32_t*) getValPtr(p_field); //EDID_buff.edi.base.serial;

   if (op == OP_READ) {
      sval << *inst;
      ival  = *inst;
      RCD_SET_OK(retU);
   } else {
      ulong tmpv;
      RCD_SET_FAULT(retU);

      if (op == OP_WRSTR) {
         if (! sval.ToULong(&tmpv, 10)) RCD_RETURN_FAULT(retU);
         RCD_SET_OK(retU);
      } else if (op == OP_WRINT) {
         tmpv = ival;
         RCD_SET_OK(retU);
      }
      if (! RCD_IS_OK(retU)) return retU;
      *inst = (u32_t) tmpv;
   }
   return retU;
}

rcode EDID_cl::ProdWk(u32_t op, wxString& sval, u32_t& ival, edi_dynfld_t* p_field) {
   rcode  retU;
   u8_t  *inst;

   inst = getValPtr(p_field); //EDID_buff.edi.base.prodweek

   if (op == OP_READ) {
      sval << (int) *inst;
      ival = *inst;
      RCD_SET_OK(retU);
   } else {
      RCD_SET_FAULT(retU);

      if (op == OP_WRSTR) {
         ulong tmpv;
         retU = getStrUint(sval, 10, 0, 255, tmpv);
         if (! RCD_IS_OK(retU)) return retU;
         ival = tmpv;
      }
      if (op == OP_WRINT) {
         RCD_SET_OK(retU);
      }
      if (RCD_IS_OK(retU)) {
         if ((ival == 0) || ((ival > 52) && (ival != 255)) ) {
            RCD_RETURN_FAULT(retU);
         }
         *inst = ival;
      }
   }
   return retU;
}

rcode EDID_cl::ProdYr(u32_t op, wxString& sval, u32_t& ival, edi_dynfld_t* p_field) {
   rcode  retU;
   u8_t  *inst;

   inst = getValPtr(p_field); //EDID_buff.edi.base.year

   if (op == OP_READ) {
      ival = (*inst + 1990);
      sval << ival;
      RCD_SET_OK(retU);
   } else {
      ulong tmpv = 0;
      RCD_SET_FAULT(retU);

      if (op == OP_WRSTR) {
         retU = getStrUint(sval, 10, 1990, 2245, tmpv);
         if (! RCD_IS_OK(retU)) return retU;
      } else if (op == OP_WRINT) {
         tmpv = ival;
         RCD_SET_OK(retU);
      }
      *inst = ((tmpv - 1990) & 0xFF);
   }
   return retU;
}

//BED: Base EDID data
const char  edibase_cl::CodN[] = "BED";
const char  edibase_cl::Name[] = "Basic EDID data";
const char  edibase_cl::Desc[] = "Basic EDID data";

const edi_field_t edibase_cl::fields[] = {
   {&EDID_cl::ByteStr, NULL, offsetof(edid_t, hdr), 0, 8, EF_STR|EF_HEX|EF_RD|EF_NI, 0, 0, "header",
   "*magic* byte sequence for EDID structure identification:\n 00 FF FF FF FF FF FF 00" },
   {&EDID_cl::MfcId, NULL, offsetof(edid_t, mfc_id), 0, 2, EF_STR|EF_RD|EF_NI, 0, 0, "mfc_id",
   "Short manufacturer id: 3 capital letters.\n2 bytes in Big Endian byte order, "
   "5bits per letter, 1 bit (msb) left reserved = 0." },
   {&EDID_cl::ByteStr, NULL, offsetof(edid_t, prod_id), 0, 2, EF_STR|EF_HEX|EF_LE|EF_RD, 0, 0xFFFF, "prod_id",
   "Product identifier number" },
   {&EDID_cl::ProdSN, NULL, offsetof(edid_t, serial), 0, 4, EF_INT|EF_RD, 0, 0xFFFFFFFF, "serial",
   "Product serial number, 32bit" },
   {&EDID_cl::ProdWk, NULL, offsetof(edid_t, prodweek), 0, 1, EF_INT|EF_RD, 0, 255, "prodweek",
   "Week of the year in which the product was manufactured.\n"
   "If value=255 (0xFF), then the Year field means the model release year." },
   {&EDID_cl::ProdYr, NULL, offsetof(edid_t, year), 0, 1, EF_INT|EF_RD, 0, 255, "year",
   "Year of manufacuring or model release, "
   "when week=255. values: 0-255 -> 1990–2245" },
   {&EDID_cl::ByteVal, NULL, offsetof(edid_t, edid_ver), 0, 1, EF_BYTE|EF_INT|EF_RD, 1, 1, "edid_ver",
   "EDID version" },
   {&EDID_cl::ByteVal, NULL, offsetof(edid_t, edid_rev), 0, 1, EF_BYTE|EF_INT|EF_RD, 0, 4, "edid_rev",
   "EDID revision" },
   {&EDID_cl::ByteVal, NULL, offsetof(edid_t, edid_rev)+1, 0, 1, EF_BYTE|EF_INT|EF_RD, 0, 3, "num_extblk",
   "Number of extension blocks" },
   {&EDID_cl::ByteVal, NULL, offsetof(edid_t, edid_rev)+2, 0, 1, EF_BYTE|EF_HEX|EF_RD, 0, 255, "checksum",
   "Block checksum: sum of all bytes mod 256 must equals zero." }
};

#pragma GCC diagnostic ignored "-Wunused-parameter"
rcode edibase_cl::init(const u8_t* inst, u32_t orflags, edi_grp_cl* parent) {
   static const u32_t fcount = 10;

   rcode         retU;
   const edid_t *pedid;

   pedid      = reinterpret_cast<const edid_t*> (inst);
   parent_grp = parent;
   type_id    = ID_BED | T_EDID_FIXED;
   abs_offs   = offsetof(edid_t, hdr); //0
   rel_offs   = abs_offs;

   //num_extblk & chksum are copied to local buffer,
   //so the corresponding field offsets are changed
   CopyInstData(inst, offsetof(edid_t, vinput_dsc)); //20: last byte is edid_t.edid_rev (19).
   hdr_sz              = dat_sz;
   inst_data[dat_sz++] = pedid->num_extblk;
   inst_data[dat_sz++] = pedid->chksum;

   retU = init_fields(&fields[0], inst_data, fcount, 0, Name, Desc, CodN);
   return retU;
}
#pragma GCC diagnostic warning "-Wunused-parameter"

void edibase_cl::SpawnInstance(u8_t *pinst) {
   u32_t   dsz;
   edid_t *pedid;

   pedid  = reinterpret_cast<edid_t*> (pinst);
   dsz    = dat_sz;
   dsz   -= 2; //num_extblk & chksum

   memcpy(pinst, inst_data, dsz);

   pedid->num_extblk = inst_data[dsz++];
   pedid->chksum     = inst_data[dsz  ];
}

//VID: Video Input Descriptor
//VID: Video Input Descriptor: input type selector.
const vname_map_t vid_input_type[] = {
   {0, "Analog" },
   {1, "Digital"}
};

const vmap_t VID_input = {
   0, 2,
   vid_input_type
};

//VID: Video Input Descriptor : handlers
const char  vindsc_cl::CodN[] = "VID";
const char  vindsc_cl::Name[] = "Video Input Descriptor";
const char  vindsc_cl::Desc[] = "Video Input Descriptor.\n"
"Some fields have different meaning depending on input type. This results in "
"that some fields are overlapped and changing one value will change the other.\n\n"
"NOTE: Bits 1-6: Digital input: Mandatory zero for EDID version < v1.4.\n";

//VID: bit7=0, analog input
const edi_field_t vindsc_cl::in_analog[] = {
   {&EDID_cl::BitVal, &VID_input, 0, 7, 1, EF_BIT|EF_VS|EF_FGR, 0, 1, "Input Type",
   "Bit7: Input signal type: 1=digital, 0=analog" },
   {&EDID_cl::BitVal, NULL, 0, 0, 1, EF_BIT|EF_FGR, 0, 1, "vsync",
   "Bit0: Analog input: 1= if composite sync/sync-on-green is used, VSync pulse is \"serrated\". "
   "This is more commonly known as \"SandCastle Pulse\" - created from overlapped H-sync "
   "and V-sync pulses during V-Blank time." },
   {&EDID_cl::BitVal, NULL, 0, 1, 1, EF_BIT, 0, 1, "sync_green",
   "Bit1: Analog input: Sync_on_green supported (SOG)." },
   {&EDID_cl::BitVal, NULL, 0, 2, 1, EF_BIT, 0, 1, "comp_sync",
   "Bit2: Analog input: Composite sync (on HSync line) supported" },
   {&EDID_cl::BitVal, NULL, 0, 3, 1, EF_BIT, 0, 1, "sep_sync",
   "Bit3: Analog input: Separate sync supported" },
   {&EDID_cl::BitVal, NULL, 0, 4, 1, EF_BIT, 0, 1, "blank_black",
   "Bit4: Analog input: Blank-to-black setup or pedestal per appropriate Signal Level Standard expected" },
   {&EDID_cl::BitF8Val, NULL, 0, 5, 2, EF_BFLD, 0, 3, "sync_wh_lvl",
   "Bits 5,6: Analog input: Video white and sync levels, relative to blank:\n"
   "00: +0.7/−0.3 V;\n"
   "01: +0.714/−0.286 V;\n"
   "10: +1.0/−0.4 V;\n"
   "11: +0.7/0 V" }
};
//VID: bit7=1, digital input
const edi_field_t vindsc_cl::in_digital[] = {
   {&EDID_cl::BitVal, &VID_input, 0, 7, 1, EF_BIT|EF_VS|EF_FGR, 0, 1, "Input Type",
   "Bit7: Input signal type: 1=digital, 0=analog" },
   {&EDID_cl::BitVal, NULL, 0, 0, 1, EF_BIT, 0, 1, "VESA compat",
   "Bit0: Digital input: EDID v1.3: compatibility with VESA DFP v1.x TMDS CRGB, 1 pix per clock, "
   "up to 8 bits per color, MSB aligned, DE active high.\n\n"
   "NOTE#1: In EDID v1.4 this bit has changed meaning: LSB of \"Digital Interface Type\"\n"
   "NOTE#2: This field overlap fields defined for Analog input." },
   {&EDID_cl::BitF8Val, NULL, 0, 0, 4, EF_BFLD, 0, 0xF, "IF Type",
   "Bits 0-3: EDID v1.4: Digital input: Digital Interface Type:\n"
   "0000  = undefined / mandatory zero for EDID v1.3\n"
   "0001  = DVI / EDIDv1.3: VESA DFP v1.x compatibility flag\n"
   "0010  = HDMI-a\n"
   "0011  = HDMI-b\n"
   "0100  = MDDI\n"
   "0101  = DisplayPort\n"
   ">0101 = reserved\n\n"
   "NOTE: This field overlap fields defined for Analog input." },
   {&EDID_cl::BitF8Val, NULL, 0, 4, 3, EF_BFLD, 0, 0x7, "Color Depth",
   "Bits 4-6: Digital input: Color Bit Depth (bits per primary color):\n"
   "000 = undefined / mandatory zero for EDID v1.3\n"
   "001 = 6\n"
   "010 = 8\n"
   "011 = 10\n"
   "100 = 12\n"
   "101 = 14\n"
   "110 = 16\n"
   "111 = reserved\n\n"
   "NOTE: This field overlap fields defined for Analog input." },
};

#pragma GCC diagnostic ignored "-Wunused-parameter"
rcode vindsc_cl::init(const u8_t* inst, u32_t orflags, edi_grp_cl* parent) {
   rcode retU;

   parent_grp = parent;
   type_id    = ID_VID | T_EDID_FIXED;
   abs_offs   = offsetof(edid_t, vinput_dsc);
   rel_offs   = abs_offs;

   CopyInstData(inst, sizeof(vid_in_t));

   //pre-alloc buffer for array of fields
   dyn_fldar = (edi_field_t*) malloc( max_fcnt * sizeof(edi_field_t) );
   if (NULL == dyn_fldar) RCD_RETURN_FAULT(retU);

   //no error possible, init_fields is always called.
   retU = gen_data_layout(inst_data);

   retU = init_fields(&dyn_fldar[0], inst_data, dyn_fcnt, 0, Name, Desc, CodN);

   return retU;
}
#pragma GCC diagnostic warning "-Wunused-parameter"

rcode vindsc_cl::ForcedGroupRefresh() {
   rcode retU;

   //clear fields
   memset( (void*) dyn_fldar, 0, ( max_fcnt * EDI_FIELD_SZ ) );

   //no error possible, init_fields is always called.
   retU = gen_data_layout(inst_data);

   retU = init_fields(dyn_fldar, inst_data, dyn_fcnt, 0); //note: orflags are cleared.

   return retU;
}

rcode vindsc_cl::gen_data_layout(const u8_t* inst) {
   rcode  retU;
   int    md_digital;

   //note: EDID block 0: static descriptors use block0 ptr as instance ptr.
   md_digital = reinterpret_cast <const vid_in_t*> (inst)->digital.input_type;

   if (1 == md_digital) {
      memcpy( dyn_fldar, in_digital, (in_digital_fcnt * EDI_FIELD_SZ) );
      dyn_fcnt = in_digital_fcnt;
   } else {
      memcpy( dyn_fldar, in_analog, (in_analog_fcnt * EDI_FIELD_SZ) );
      dyn_fcnt = in_analog_fcnt;
   }

   RCD_RETURN_OK(retU);
}

//BDD: basic display descriptor : handlers
//BDD: basic display descriptor
const char  bddcs_cl::CodN[] = "BDD";
const char  bddcs_cl::Name[] = "Basic Display Descriptor";
const char  bddcs_cl::Desc[] = "Basic Display Descriptor";

const edi_field_t bddcs_cl::fields[] = {
   {&EDID_cl::ByteVal, NULL, offsetof(bdd_t, max_hsize), 0, 1, EF_BYTE|EF_INT|EF_CM, 0, 255, "max_hsize",
   "Max horizontal image size, in cm, 0=undefined" },
   {&EDID_cl::ByteVal, NULL, offsetof(bdd_t, max_vsize), 0, 1, EF_BYTE|EF_INT|EF_CM, 0, 255, "max_vsize",
   "Max vertical image size, in cm, 0=undefined" },
   {&EDID_cl::Gamma, NULL, offsetof(bdd_t, gamma), 0, 1, EF_FLT|EF_NI, 0, 255, "gamma",
   "Byte value = (gamma*100)-100 (range 1.00–3.54)" }
};

#pragma GCC diagnostic ignored "-Wunused-parameter"
rcode bddcs_cl::init(const u8_t* inst, u32_t orflags, edi_grp_cl* parent) {
   static const u32_t fcount = 3;
   rcode retU;

   parent_grp = parent;
   type_id    = ID_BDD | T_EDID_FIXED;
   abs_offs   = offsetof(edid_t, bdd);
   rel_offs   = abs_offs;

   CopyInstData(inst, sizeof(bdd_t));

   retU = init_fields(&fields[0], inst_data, fcount, 0, Name, Desc, CodN);
   return retU;
}
#pragma GCC diagnostic warning "-Wunused-parameter"

//SPF: Supported features : handlers
//SPF: Supported features
const char  spft_cl::CodN[] = "SPF";
const char  spft_cl::Name[] = "Supported features";
const char  spft_cl::Desc[] = "Supported features.";

const edi_field_t spft_cl::fields[] = {
   {&EDID_cl::BitVal, NULL, 0, 0, 1, EF_BIT, 0, 1, "gtf_support",
   "GTF (General Timing Formula) supported with default parameter values" },
   {&EDID_cl::BitVal, NULL, 0, 1, 1, EF_BIT, 0, 1, "dtd0_native",
   "If set to 1, DTD0 (offs=54) contains \"preferred timing mode\" - i.e. the native and "
   "thus recommended timings (pixel-correct).\nFor EDID v1.3+ DTD0 is always treated as \"preferred\", "
   "and this bit should be set to 1." },
   {&EDID_cl::BitVal, NULL, 0, 2, 1, EF_BIT, 0, 1, "std_srbg",
   "Standard sRGB colour space. Chromacity coords (bytes 25–34) must contain sRGB standard values." },
   {&EDID_cl::BitF8Val, NULL, 0, 3, 2, EF_BFLD, 0, 3, "vsig_format",
   "Video signal format:\nDisplay type (analog):\n"
   "00 = Monochrome or Grayscale;\n 01 = RGB color;\n"
   "10 = Non-RGB multi-color;\n 11 = Undefined\n"
   "Display type (digital):\n 00 = RGB 4:4:4;\n 01 = RGB 4:4:4 + YCrCb 4:4:4;\n"
   "10 = RGB 4:4:4 + YCrCb 4:2:2;\n 11 = RGB 4:4:4 + YCrCb 4:4:4 + YCrCb 4:2:2" },
   {&EDID_cl::BitVal, NULL, 0, 5, 1, EF_BIT, 0, 1, "dpms_off",
   "DPMS active-off supported" },
   {&EDID_cl::BitVal, NULL, 0, 6, 1, EF_BIT, 0, 1, "dpms_susp",
   "DPMS suspend supported" },
   {&EDID_cl::BitVal, NULL, 0, 7, 1, EF_BIT, 0, 1, "dpms_stby",
   "DPMS standby supported" }
};

#pragma GCC diagnostic ignored "-Wunused-parameter"
rcode spft_cl::init(const u8_t* inst, u32_t orflags, edi_grp_cl* parent) {
   const u32_t fcount = 7;

   rcode retU;

   parent_grp = parent;
   type_id    = ID_SPF | T_EDID_FIXED;
   abs_offs   = offsetof(edid_t, features);
   rel_offs   = abs_offs;

   CopyInstData(inst, sizeof(dsp_feat_t)); //1

   retU = init_fields(&fields[0], inst_data, fcount, 0, Name, Desc, CodN);
   return retU;
}
#pragma GCC diagnostic warning "-Wunused-parameter"

//CXY: Chromacity coords : handlers
rcode EDID_cl::ChrXY_getWriteVal(u32_t op, wxString& sval, u32_t& ival) {
   rcode retU;
   RCD_SET_FAULT(retU);

   if (op == OP_WRSTR) {
      double  dval;
      retU = getStrDouble(sval, 0.0, 0.999, dval);
      if (! RCD_IS_OK(retU)) return retU;
      ival = (dval*1024.0);
   } else if (op == OP_WRINT) {
      ival = (ival & 0x3F);
      RCD_SET_OK(retU);
   }
   return retU;
}

rcode EDID_cl::CHredX(u32_t op, wxString& sval, u32_t& ival, edi_dynfld_t* field) {
   rcode      retU;
   ulong      tmpv;
   chromxy_t *chrxy;

   chrxy = reinterpret_cast <chromxy_t*> (getInstancePtr(field));

   if (field == NULL) RCD_RETURN_FAULT(retU);

   if (op == OP_READ) {
      double  dval;
      tmpv  = (chrxy->red8h_x << 2);
      tmpv |= chrxy->rgxy_lsbits.red_x;
      ival  = tmpv;
      dval  = tmpv;
      if (sval.Printf("%.03f", (dval/1024.0)) < 0) {
         RCD_SET_FAULT(retU);
      } else {
         RCD_SET_OK(retU);
      }

   } else {
      retU = ChrXY_getWriteVal(op, sval, ival);
      if (!RCD_IS_OK(retU)) return retU;

      tmpv = ival;

      chrxy->rgxy_lsbits.red_x = (tmpv  & 0x03);
      chrxy->red8h_x           = (tmpv >> 2   );
   }
   return retU;
}

rcode EDID_cl::CHredY(u32_t op, wxString& sval, u32_t& ival, edi_dynfld_t* field) {
   rcode      retU;
   ulong      tmpv;
   chromxy_t *chrxy;

   chrxy = reinterpret_cast <chromxy_t*> (getInstancePtr(field));

   if (field == NULL) RCD_RETURN_FAULT(retU);

   if (op == OP_READ) {
      double  dval;
      tmpv  = (chrxy->red8h_y << 2);
      tmpv |= chrxy->rgxy_lsbits.red_y;
      ival  = tmpv;
      dval  = tmpv;
      if (sval.Printf("%.03f", (dval/1024.0)) < 0) {
         RCD_SET_FAULT(retU);
      } else {
         RCD_SET_OK(retU);
      }

   } else {
      retU = ChrXY_getWriteVal(op, sval, ival);
      if (!RCD_IS_OK(retU)) return retU;

      tmpv = ival;

      chrxy->rgxy_lsbits.red_y = (tmpv  & 0x03);
      chrxy->red8h_y           = (tmpv >> 2   );
   }
   return retU;
}

rcode EDID_cl::CHgrnX(u32_t op, wxString& sval, u32_t& ival, edi_dynfld_t* field) {
   rcode      retU;
   ulong      tmpv;
   chromxy_t *chrxy;

   chrxy = reinterpret_cast <chromxy_t*> (getInstancePtr(field));

   if (field == NULL) RCD_RETURN_FAULT(retU);

   if (op == OP_READ) {
      double  dval;
      tmpv  = (chrxy->green8h_x << 2);
      tmpv |= chrxy->rgxy_lsbits.green_x;
      ival  = tmpv;
      dval  = tmpv;
      if (sval.Printf("%.03f", (dval/1024.0)) < 0) {
         RCD_SET_FAULT(retU);
      } else {
         RCD_SET_OK(retU);
      }

   } else {
      retU = ChrXY_getWriteVal(op, sval, ival);
      if (!RCD_IS_OK(retU)) return retU;

      tmpv = ival;

      chrxy->rgxy_lsbits.green_x = (tmpv  & 0x03);
      chrxy->green8h_x           = (tmpv >> 2   );
   }
   return retU;
}

rcode EDID_cl::CHgrnY(u32_t op, wxString& sval, u32_t& ival, edi_dynfld_t* field) {
   rcode      retU;
   ulong      tmpv;
   chromxy_t *chrxy;

   chrxy = reinterpret_cast <chromxy_t*> (getInstancePtr(field));

   if (field == NULL) RCD_RETURN_FAULT(retU);

   if (op == OP_READ) {
      double  dval;
      tmpv  = (chrxy->green8h_y << 2);
      tmpv |= chrxy->rgxy_lsbits.green_y;
      ival  = tmpv;
      dval  = tmpv;
      if (sval.Printf("%.03f", (dval/1024.0)) < 0) {
         RCD_SET_FAULT(retU);
      } else {
         RCD_SET_OK(retU);
      }

   } else {
      retU = ChrXY_getWriteVal(op, sval, ival);
      if (!RCD_IS_OK(retU)) return retU;

      tmpv = ival;

      chrxy->rgxy_lsbits.green_y = (tmpv  & 0x03);
      chrxy->green8h_y           = (tmpv >> 2   );
   }
   return retU;
}

rcode EDID_cl::CHbluX(u32_t op, wxString& sval, u32_t& ival, edi_dynfld_t* field) {
   rcode      retU;
   ulong      tmpv;
   chromxy_t *chrxy;

   chrxy = reinterpret_cast <chromxy_t*> (getInstancePtr(field));

   if (field == NULL) RCD_RETURN_FAULT(retU);

   if (op == OP_READ) {
      double  dval;
      tmpv  = (chrxy->blue8h_x << 2);
      tmpv |= chrxy->bwxy_lsbits.blue_x;
      ival  = tmpv;
      dval  = tmpv;
      if (sval.Printf("%.03f", (dval/1024.0)) < 0) {
         RCD_SET_FAULT(retU);
      } else {
         RCD_SET_OK(retU);
      }

   } else {
      retU = ChrXY_getWriteVal(op, sval, ival);
      if (!RCD_IS_OK(retU)) return retU;

      tmpv = ival;

      chrxy->bwxy_lsbits.blue_x = (tmpv  & 0x03);
      chrxy->blue8h_x           = (tmpv >> 2   );
   }
   return retU;
}

rcode EDID_cl::CHbluY(u32_t op, wxString& sval, u32_t& ival, edi_dynfld_t* field) {
   rcode      retU;
   ulong      tmpv;
   chromxy_t *chrxy;

   chrxy = reinterpret_cast <chromxy_t*> (getInstancePtr(field));

   if (field == NULL) RCD_RETURN_FAULT(retU);

   if (op == OP_READ) {
      double  dval;
      tmpv  = (chrxy->blue8h_y << 2);
      tmpv |= chrxy->bwxy_lsbits.blue_y;
      ival  = tmpv;
      dval  = tmpv;
      if (sval.Printf("%.03f", (dval/1024.0)) < 0) {
         RCD_SET_FAULT(retU);
      } else {
         RCD_SET_OK(retU);
      }

   } else {
      retU = ChrXY_getWriteVal(op, sval, ival);
      if (!RCD_IS_OK(retU)) return retU;

      tmpv = ival;

      chrxy->bwxy_lsbits.blue_y = (tmpv  & 0x03);
      chrxy->blue8h_y           = (tmpv >> 2   );
   }
   return retU;
}

rcode EDID_cl::CHwhtX(u32_t op, wxString& sval, u32_t& ival, edi_dynfld_t* field) {
   rcode      retU;
   ulong      tmpv;
   chromxy_t *chrxy;

   chrxy = reinterpret_cast <chromxy_t*> (getInstancePtr(field));

   if (field == NULL) RCD_RETURN_FAULT(retU);

   if (op == OP_READ) {
      double  dval;
      tmpv  = (chrxy->white8h_x << 2);
      tmpv |= chrxy->bwxy_lsbits.white_x;
      ival  = tmpv;
      dval  = tmpv;
      if (sval.Printf("%.03f", (dval/1024.0)) < 0) {
         RCD_SET_FAULT(retU);
      } else {
         RCD_SET_OK(retU);
      }

   } else {
      retU = ChrXY_getWriteVal(op, sval, ival);
      if (!RCD_IS_OK(retU)) return retU;

      tmpv = ival;

      chrxy->bwxy_lsbits.white_x = (tmpv  & 0x03);
      chrxy->white8h_x           = (tmpv >> 2   );
   }
   return retU;
}

rcode EDID_cl::CHwhtY(u32_t op, wxString& sval, u32_t& ival, edi_dynfld_t* field) {
   rcode      retU;
   ulong      tmpv;
   chromxy_t *chrxy;

   chrxy = reinterpret_cast <chromxy_t*> (getInstancePtr(field));

   if (field == NULL) RCD_RETURN_FAULT(retU);

   if (op == OP_READ) {
      double  dval;
      tmpv  = (chrxy->white8h_y << 2);
      tmpv |= chrxy->bwxy_lsbits.white_y;
      ival  = tmpv;
      dval  = tmpv;
      if (sval.Printf("%.03f", (dval/1024.0)) < 0) {
         RCD_SET_FAULT(retU);
      } else {
         RCD_SET_OK(retU);
      }

   } else {
      retU = ChrXY_getWriteVal(op, sval, ival);
      if (!RCD_IS_OK(retU)) return retU;

      tmpv = ival;

      chrxy->bwxy_lsbits.white_y = (tmpv  & 0x03);
      chrxy->white8h_y           = (tmpv >> 2   );
   }
   return retU;
}

//CXY: Chromacity coords
const char  chromxy_cl::CodN[] = "CXY";
const char  chromxy_cl::Name[] = "Chromacity Coords";
const char  chromxy_cl::Desc[] = "CIE Chromacity Coordinates.\n"
"The values are 10bit bitfields, value range of 0 - 1023 is converted to a fixed point fractional "
"in range 0.0 - 0.999.\n\n"
"https://en.wikipedia.org/wiki/CIE_1931_color_space";

const edi_field_t chromxy_cl::fields[] = {
   {&EDID_cl::CHredX, NULL, 0, 0, 10, EF_BFLD|EF_FLT|EF_GPD|EF_NI, 0, 1, "red_x", "" },
   {&EDID_cl::CHredY, NULL, 0, 0, 10, EF_BFLD|EF_FLT|EF_GPD|EF_NI, 0, 1, "red_y", "" },
   {&EDID_cl::CHgrnX, NULL, 0, 0, 10, EF_BFLD|EF_FLT|EF_GPD|EF_NI, 0, 1, "green_x", "" },
   {&EDID_cl::CHgrnY, NULL, 0, 0, 10, EF_BFLD|EF_FLT|EF_GPD|EF_NI, 0, 1, "green_y", "" },
   {&EDID_cl::CHbluX, NULL, 0, 0, 10, EF_BFLD|EF_FLT|EF_GPD|EF_NI, 0, 1, "blue_x", "" },
   {&EDID_cl::CHbluY, NULL, 0, 0, 10, EF_BFLD|EF_FLT|EF_GPD|EF_NI, 0, 1, "blue_y", "" },
   {&EDID_cl::CHwhtX, NULL, 0, 0, 10, EF_BFLD|EF_FLT|EF_GPD|EF_NI, 0, 1, "white_x", "" },
   {&EDID_cl::CHwhtY, NULL, 0, 0, 10, EF_BFLD|EF_FLT|EF_GPD|EF_NI, 0, 1, "white_y", "" }
};

#pragma GCC diagnostic ignored "-Wunused-parameter"
rcode chromxy_cl::init(const u8_t* inst, u32_t orflags, edi_grp_cl* parent) {
   static const u32_t fcount = 8;

   rcode retU;

   parent_grp = parent;
   type_id    = ID_CXY | T_EDID_FIXED;
   abs_offs   = offsetof(edid_t, chromxy);
   rel_offs   = abs_offs;

   CopyInstData(inst, sizeof(chromxy_t));

   retU = init_fields(&fields[0], inst_data, fcount, 0, Name, Desc, CodN);
   return retU;
}
#pragma GCC diagnostic warning "-Wunused-parameter"

//ETM: Established Timings : no dedicated handlers
//ETM: Established Timings
const char  resmap_cl::CodN[] = "ETM";
const char  resmap_cl::Name[] = "Established Timings Map";
const char  resmap_cl::Desc[] =
"Standard display modes (VESA): each field is a flag telling whether particular display "
"mode is supported.\nField name describes the resolution and verical refresh rate:\n"
"Hres x Vres x Vrefresh, i = interlaced.\nreservedX are modes reserved for manufacturers.";

const edi_field_t resmap_cl::fields[] = {
//byte 35
{&EDID_cl::BitVal, NULL, 0, 0, 1, EF_BIT|EF_GPD, 0, 1, "800x600x60", "" },
{&EDID_cl::BitVal, NULL, 0, 1, 1, EF_BIT|EF_GPD, 0, 1, "800x600x56", "" },
{&EDID_cl::BitVal, NULL, 0, 2, 1, EF_BIT|EF_GPD, 0, 1, "640x480x75", "" },
{&EDID_cl::BitVal, NULL, 0, 3, 1, EF_BIT|EF_GPD, 0, 1, "640x480x72", "" },
{&EDID_cl::BitVal, NULL, 0, 4, 1, EF_BIT|EF_GPD, 0, 1, "640x480x67", "" },
{&EDID_cl::BitVal, NULL, 0, 5, 1, EF_BIT|EF_GPD, 0, 1, "640x480x60", "" },
{&EDID_cl::BitVal, NULL, 0, 6, 1, EF_BIT|EF_GPD, 0, 1, "720x400x88", "" },
{&EDID_cl::BitVal, NULL, 0, 7, 1, EF_BIT|EF_GPD, 0, 1, "720x400x70", "" },
//byte 36:
{&EDID_cl::BitVal, NULL, 1, 0, 1, EF_BIT|EF_GPD, 0, 1, "1280x1024x75", "" },
{&EDID_cl::BitVal, NULL, 1, 1, 1, EF_BIT|EF_GPD, 0, 1, "1024x768x75" , "" },
{&EDID_cl::BitVal, NULL, 1, 2, 1, EF_BIT|EF_GPD, 0, 1, "1024x768x72" , "" },
{&EDID_cl::BitVal, NULL, 1, 3, 1, EF_BIT|EF_GPD, 0, 1, "1024x768x60" , "" },
{&EDID_cl::BitVal, NULL, 1, 4, 1, EF_BIT|EF_GPD, 0, 1, "1024x768x87i", "" },
{&EDID_cl::BitVal, NULL, 1, 5, 1, EF_BIT|EF_GPD, 0, 1, "832x624x75"  , "" },
{&EDID_cl::BitVal, NULL, 1, 6, 1, EF_BIT|EF_GPD, 0, 1, "800x600x75"  , "" },
{&EDID_cl::BitVal, NULL, 1, 7, 1, EF_BIT|EF_GPD, 0, 1, "800x600x72"  , "" },
//byte 37:
{&EDID_cl::BitVal, NULL, 2, 0, 1, EF_BIT|EF_GPD, 0, 1, "reserved0", "" },
{&EDID_cl::BitVal, NULL, 2, 1, 1, EF_BIT|EF_GPD, 0, 1, "reserved1", "" },
{&EDID_cl::BitVal, NULL, 2, 2, 1, EF_BIT|EF_GPD, 0, 1, "reserved2", "" },
{&EDID_cl::BitVal, NULL, 2, 3, 1, EF_BIT|EF_GPD, 0, 1, "reserved3", "" },
{&EDID_cl::BitVal, NULL, 2, 4, 1, EF_BIT|EF_GPD, 0, 1, "reserved4", "" },
{&EDID_cl::BitVal, NULL, 2, 5, 1, EF_BIT|EF_GPD, 0, 1, "reserved5", "" },
{&EDID_cl::BitVal, NULL, 2, 6, 1, EF_BIT|EF_GPD, 0, 1, "reserved6", "" },
{&EDID_cl::BitVal, NULL, 2, 7, 1, EF_BIT|EF_GPD, 0, 1, "1152x870x75", "" }
};

#pragma GCC diagnostic ignored "-Wunused-parameter"
rcode resmap_cl::init(const u8_t* inst, u32_t orflags, edi_grp_cl* parent) {
   static const u32_t fcount = 24;

   rcode retU;

   parent_grp = parent;
   type_id    = ID_ETM | T_EDID_FIXED;
   abs_offs   = offsetof(edid_t, res_map);
   rel_offs   = abs_offs;

   CopyInstData(inst, sizeof(est_map_t));

   retU = init_fields(&fields[0], inst_data, fcount, 0, Name, Desc, CodN);
   return retU;
}
#pragma GCC diagnostic warning "-Wunused-parameter"

//STI: std timing descriptors : handlers
rcode EDID_cl::StdTxres8(u32_t op, wxString& sval, u32_t& ival, edi_dynfld_t* p_field) {
   rcode       retU;
   std_timg_t* inst = NULL;

   inst = reinterpret_cast <std_timg_t*> (getInstancePtr(p_field));
   if (inst == NULL) RCD_RETURN_FAULT(retU);

   if (op == OP_READ) {
      ival   = inst->x_res8;
      if (ival == 0x01) p_field->field.flags |= EF_NU; //unused field
      ival  += 31;
      ival <<= 3;
      sval << ival;
      RCD_SET_OK(retU);
   } else {
      ulong tmpv = 0;
      RCD_SET_FAULT(retU);

      if (op == OP_WRSTR) {
         retU = getStrUint(sval, 10, p_field->field.minv, p_field->field.maxv, tmpv);
         if (! RCD_IS_OK(retU)) return retU;
      } else if (op == OP_WRINT) {
         tmpv = ival;
         RCD_SET_OK(retU);
      }

      tmpv = (tmpv >> 3)-31;
      if (tmpv != 0x01) p_field->field.flags &= ~EF_NU;
      inst->x_res8 = (tmpv & 0xFF);
   }
   return retU;
}

rcode EDID_cl::StdTvsync(u32_t op, wxString& sval, u32_t& ival, edi_dynfld_t* p_field) {
   rcode       retU;
   std_timg_t *inst = NULL;


   inst = reinterpret_cast <std_timg_t*> (getInstancePtr(p_field));
   if (inst == NULL) RCD_RETURN_FAULT(retU);

   ival = (reinterpret_cast <u8_t*> (inst))[1];
   if (ival == 0x01) {
      p_field->field.flags |= EF_NU;
   } else {
      p_field->field.flags &= ~EF_NU;
   }

   if (op == OP_READ) {
      ival  = inst->v_freq;
      ival += 60;
      sval << ival;
      RCD_SET_OK(retU);
   } else {
      ulong tmpv = 0;
      RCD_SET_FAULT(retU);

      if (op == OP_WRSTR) {
         retU = getStrUint(sval, 10, 60, 123, tmpv);
         if (! RCD_IS_OK(retU)) return retU;
      } else if (op == OP_WRINT) {
         tmpv = ival;
         RCD_SET_OK(retU);
      }
      inst->v_freq = ((tmpv - 60) & 0x3F);
   }
   return retU;
}

//STI: Std Timing Descriptor
const char  sttd_cl::CodN[] = "STI";
const char  sttd_cl::Name[] = "Standard Timing Information";
const char  sttd_cl::Desc[] = "Standard Timing Information.\n"
"If the block is unused, both bytes are set to 0x01 01, NU flag is set.\n"
"In such case the interpreted values are:\n X-res8 = 256;\n V-freq = 61; \n"
"pix_ratio = 0b00";

static const char stiXres8dsc[] = "Byte value: ((X-res / 8) - 31) : 256–2288 pixels, value 00 is "
"reserved.\n"
"Vertical resolution is calculated on the adapter side using pixel ratio.";
static const char stiVsyncDsc[] = "V-freq: (60–123 Hz).\n"
"Bitfield (6bits) value is stored as Vfreq-60 (0-63)";
static const char stiPixRatioDsc[] = "pixel ratio:\n 00=16:10;\n 01=4:3;\n 10=5:4;\n 11=16:9.\n"
"EDID versions prior to 1.3 defined 00 as ratio 1:1.";


const edi_field_t sttd_cl::fields[] = {
   {&EDID_cl::StdTxres8, NULL, offsetof(std_timg_t, x_res8), 0, 1, EF_INT|EF_FGR|EF_PIX, 256, 2288, "X-res8",
   stiXres8dsc },
   {&EDID_cl::StdTvsync, NULL, offsetof(std_timg_t, x_res8)+1, 0, 6, EF_INT|EF_BFLD|EF_FGR|EF_HZ, 0, 63, "V-freq",
   stiVsyncDsc },
   {&EDID_cl::BitF8Val, NULL, offsetof(std_timg_t, x_res8)+1, 6, 2, EF_BFLD, 0, 3, "pix_ratio",
   stiPixRatioDsc }
};

rcode sttd_cl::init(const u8_t* inst, u32_t orflags, edi_grp_cl* parent) {
   static const u32_t fcount = 3;

   rcode retU;

   parent_grp = parent;
   type_id    = ID_STI | T_EDID_FIXED;

   CopyInstData(inst, sizeof(std_timg_t));

   //validator
   //check for unused descriptors
   if (reinterpret_cast <u16_t*> (inst_data)[0] == 0x0101) orflags |= EF_NU;

   retU = init_fields(&fields[0], inst_data, fcount, 0, Name, Desc, CodN);
   return retU;
}

//DTD : detailed timing descriptor : handlers
rcode EDID_cl::DTD_PixClk(u32_t op, wxString& sval, u32_t& ival, edi_dynfld_t* p_field) {
   rcode   retU;
   double  fval;
   dtd_t  *inst = NULL;

   inst = reinterpret_cast <dtd_t*> (getInstancePtr(p_field));
   if (inst == NULL) RCD_RETURN_FAULT(retU);

   if (op == OP_READ) {
      ival = inst->pix_clk;
      fval = ival;
      sval.Printf("%.02f", (fval/100.0));
      RCD_SET_OK(retU);
   } else {
      uint tmpv = 0;
      RCD_SET_FAULT(retU);

      if (op == OP_WRSTR) {
         retU = getStrDouble(sval, 0.01, 655.35, fval);
         tmpv = (fval*100.0);
      } else if (op == OP_WRINT) {
         tmpv = ival;
         RCD_SET_OK(retU);
      }
      if (! RCD_IS_OK(retU)) return retU;

      inst->pix_clk = (tmpv & 0xFFFF);
   }
   return retU;
}

rcode EDID_cl::DTD_HApix(u32_t op, wxString& sval, u32_t& ival, edi_dynfld_t* p_field) {
   rcode   retU;
   dtd_t  *inst = NULL;

   inst = reinterpret_cast <dtd_t*> (getInstancePtr(p_field));
   if (inst == NULL) RCD_RETURN_FAULT(retU);

   if (op == OP_READ) {
      ival = inst->HApix_8lsb;
      ival |= (inst->HApix_4msb << 8);
      sval << ival;
      RCD_SET_OK(retU);
   } else {
      ulong   tmpv;
      RCD_SET_FAULT(retU);

      if (op == OP_WRSTR) {
         retU = getStrUint(sval, 10, p_field->field.minv, p_field->field.maxv, tmpv);
      } else if (op == OP_WRINT) {
         tmpv = ival;
         RCD_SET_OK(retU);
      }
      if (! RCD_IS_OK(retU)) return retU;

      inst->HApix_8lsb = (tmpv & 0xFF);
      inst->HApix_4msb = ((tmpv >> 8) & 0x0F);
   }

   return retU;
}

rcode EDID_cl::DTD_HBpix(u32_t op, wxString& sval, u32_t& ival, edi_dynfld_t* p_field) {
   rcode   retU;
   dtd_t  *inst = NULL;

   inst = reinterpret_cast <dtd_t*> (getInstancePtr(p_field));
   if (inst == NULL) RCD_RETURN_FAULT(retU);

   if (op == OP_READ) {
      ival = inst->HBpix_8lsb;
      ival |= (inst->HBpix_4msb << 8);
      sval << ival;
      RCD_SET_OK(retU);
   } else {
      ulong   tmpv;
      RCD_SET_FAULT(retU);

      if (op == OP_WRSTR) {
         retU = getStrUint(sval, 10, p_field->field.minv, p_field->field.maxv, tmpv);
      } else if (op == OP_WRINT) {
         tmpv = ival;
         RCD_SET_OK(retU);
      }
      if (! RCD_IS_OK(retU)) return retU;

      inst->HBpix_8lsb = (tmpv & 0xFF);
      inst->HBpix_4msb = ((tmpv >> 8) & 0x0F);
   }
   return retU;
}

rcode EDID_cl::DTD_VAlin(u32_t op, wxString& sval, u32_t& ival, edi_dynfld_t* p_field) {
   rcode   retU;
   dtd_t  *inst = NULL;

   inst = reinterpret_cast <dtd_t*> (getInstancePtr(p_field));
   if (inst == NULL) RCD_RETURN_FAULT(retU);

   if (op == OP_READ) {
      ival = inst->VAlin_8lsb;
      ival |= (inst->VAlin_4msb << 8);
      sval << ival;
      RCD_SET_OK(retU);
   } else {
      ulong   tmpv;
      RCD_SET_FAULT(retU);

      if (op == OP_WRSTR) {
         retU = getStrUint(sval, 10, p_field->field.minv, p_field->field.maxv, tmpv);
      } else if (op == OP_WRINT) {
         tmpv = ival;
         RCD_SET_OK(retU);
      }
      if (! RCD_IS_OK(retU)) return retU;

      inst->VAlin_8lsb = (tmpv & 0xFF);
      inst->VAlin_4msb = ((tmpv >> 8) & 0x0F);
   }
   return retU;
}

rcode EDID_cl::DTD_VBlin(u32_t op, wxString& sval, u32_t& ival, edi_dynfld_t* p_field) {
   rcode   retU;
   dtd_t  *inst = NULL;

   inst = reinterpret_cast <dtd_t*> (getInstancePtr(p_field));
   if (inst == NULL) RCD_RETURN_FAULT(retU);

   if (op == OP_READ) {
      ival = inst->VBlin_8lsb;
      ival |= (inst->VBlin_4msb << 8);
      sval << ival;
      RCD_SET_OK(retU);
   } else {
      ulong   tmpv;
      RCD_SET_FAULT(retU);

      if (op == OP_WRSTR) {
         retU = getStrUint(sval, 10, p_field->field.minv, p_field->field.maxv, tmpv);
      } else if (op == OP_WRINT) {
         tmpv = ival;
         RCD_SET_OK(retU);
      }
      if (! RCD_IS_OK(retU)) return retU;

      inst->VBlin_8lsb = (tmpv & 0xFF);
      inst->VBlin_4msb = ((tmpv >> 8) & 0x0F);
   }
   RCD_SET_OK(retU);
   return retU;
}

rcode EDID_cl::DTD_HOsync(u32_t op, wxString& sval, u32_t& ival, edi_dynfld_t* p_field) {
   rcode   retU;
   dtd_t  *inst = NULL;

   inst = reinterpret_cast <dtd_t*> (getInstancePtr(p_field));
   if (inst == NULL) RCD_RETURN_FAULT(retU);

   if (op == OP_READ) {
      ival = inst->HOsync_8lsb;
      ival |= (inst->HOsync_2msb << 8);
      sval << ival;
      RCD_SET_OK(retU);
   } else {
      ulong   tmpv;
      RCD_SET_FAULT(retU);

      if (op == OP_WRSTR) {
         retU = getStrUint(sval, 10, p_field->field.minv, p_field->field.maxv, tmpv);
      } else if (op == OP_WRINT) {
         tmpv = ival;
         RCD_SET_OK(retU);
      }
      if (! RCD_IS_OK(retU)) return retU;

      inst->HOsync_8lsb = (tmpv & 0xFF);
      inst->HOsync_2msb = ((tmpv >> 8) & 0x03);
   }
   return retU;
}

rcode EDID_cl::DTD_HsyncW(u32_t op, wxString& sval, u32_t& ival, edi_dynfld_t* p_field) {
   rcode   retU;
   dtd_t  *inst = NULL;

   inst = reinterpret_cast <dtd_t*> (getInstancePtr(p_field));
   if (inst == NULL) RCD_RETURN_FAULT(retU);

   if (op == OP_READ) {
      ival = inst->HsyncW_8lsb;
      ival |= (inst->HsyncW_2msb << 8);
      sval << ival;
      RCD_SET_OK(retU);
   } else {
      ulong   tmpv;
      RCD_SET_FAULT(retU);

      if (op == OP_WRSTR) {
         retU = getStrUint(sval, 10, p_field->field.minv, p_field->field.maxv, tmpv);
      } else if (op == OP_WRINT) {
         tmpv = ival;
         RCD_SET_OK(retU);
      }
      if (! RCD_IS_OK(retU)) return retU;

      inst->HsyncW_8lsb = (tmpv & 0xFF);
      inst->HsyncW_2msb = ((tmpv >> 8) & 0x03);
   }
   return retU;
}

rcode EDID_cl::DTD_VOsync(u32_t op, wxString& sval, u32_t& ival, edi_dynfld_t* p_field) {
   rcode   retU;
   dtd_t  *inst = NULL;

   inst = reinterpret_cast <dtd_t*> (getInstancePtr(p_field));
   if (inst == NULL) RCD_RETURN_FAULT(retU);

   if (op == OP_READ) {
      ival = inst->VOsync_4lsb;
      ival |= (inst->VOsync_2msb << 4);
      sval << ival;
      RCD_SET_OK(retU);
   } else {
      ulong   tmpv;
      RCD_SET_FAULT(retU);

      if (op == OP_WRSTR) {
         retU = getStrUint(sval, 10, p_field->field.minv, p_field->field.maxv, tmpv);
      } else if (op == OP_WRINT) {
         tmpv = ival;
         RCD_SET_OK(retU);
      }
      if (! RCD_IS_OK(retU)) return retU;

      inst->VOsync_4lsb = (tmpv & 0x0F);
      inst->VOsync_2msb = ((tmpv >> 4) & 0x03);
   }
   return retU;
}

rcode EDID_cl::DTD_VsyncW(u32_t op, wxString& sval, u32_t& ival, edi_dynfld_t* p_field) {
   rcode   retU;
   dtd_t  *inst = NULL;

   inst = reinterpret_cast <dtd_t*> (getInstancePtr(p_field));
   if (inst == NULL) RCD_RETURN_FAULT(retU);

   if (op == OP_READ) {
      ival = inst->VsyncW_4lsb;
      ival |= (inst->VsyncW_2msb << 4);
      sval << ival;
      RCD_SET_OK(retU);
   } else {
      ulong   tmpv;
      RCD_SET_FAULT(retU);

      if (op == OP_WRSTR) {
         retU = getStrUint(sval, 10, p_field->field.minv, p_field->field.maxv, tmpv);
      } else if (op == OP_WRINT) {
         tmpv = ival;
         RCD_SET_OK(retU);
      }
      if (! RCD_IS_OK(retU)) return retU;

      inst->VsyncW_4lsb = (tmpv & 0x0F);
      inst->VsyncW_2msb = ((tmpv >> 4) & 0x03);
   }
   return retU;
}

rcode EDID_cl::DTD_Hsize(u32_t op, wxString& sval, u32_t& ival, edi_dynfld_t* p_field) {
   rcode   retU;
   dtd_t  *inst = NULL;

   inst = reinterpret_cast <dtd_t*> (getInstancePtr(p_field));
   if (inst == NULL) RCD_RETURN_FAULT(retU);

   if (op == OP_READ) {
      ival = inst->Hsize_8lsb;
      ival |= (inst->Hsize_4msb << 8);
      sval << ival;
      RCD_SET_OK(retU);
   } else {
      ulong   tmpv;
      RCD_SET_FAULT(retU);

      if (op == OP_WRSTR) {
         retU = getStrUint(sval, 10, p_field->field.minv, p_field->field.maxv, tmpv);
      } else if (op == OP_WRINT) {
         tmpv = ival;
         RCD_SET_OK(retU);
      }
      if (! RCD_IS_OK(retU)) return retU;

      inst->Hsize_8lsb = (tmpv & 0xFF);
      inst->Hsize_4msb = ((tmpv >> 8) & 0x0F);
   }
   return retU;
}

rcode EDID_cl::DTD_Vsize(u32_t op, wxString& sval, u32_t& ival, edi_dynfld_t* p_field) {
   rcode   retU;
   dtd_t  *inst = NULL;

   inst = reinterpret_cast <dtd_t*> (getInstancePtr(p_field));
   if (inst == NULL) RCD_RETURN_FAULT(retU);

   if (op == OP_READ) {
      ival = inst->Vsize_8lsb;
      ival |= (inst->Vsize_4msb << 8);
      sval << ival;
      RCD_SET_OK(retU);
   } else {
      ulong   tmpv;
      RCD_SET_FAULT(retU);

      if (op == OP_WRSTR) {
         retU = getStrUint(sval, 10, p_field->field.minv, p_field->field.maxv, tmpv);
      } else if (op == OP_WRINT) {
         tmpv = ival;
         RCD_SET_OK(retU);
      }
      if (! RCD_IS_OK(retU)) return retU;

      inst->Vsize_8lsb = (tmpv & 0xFF);
      inst->Vsize_4msb = ((tmpv >> 8) & 0x0F);
   }
   return retU;
}

//DTD : detailed timing descriptor
const char  dtd_cl::CodN[] = "DTD";
const char  dtd_cl::Name[] = "Detailed Timing Descriptor";
const char  dtd_cl::Desc[] = "Detailed Timing Descriptor";

const edi_field_t dtd_cl::fields[] = {
   {&EDID_cl::DTD_PixClk, NULL, offsetof(dtd_t, pix_clk), 0, 2, EF_FLT|EF_MHZ, 1, 65535,
   "Pixel clock",
   "Pixel clock in 10 kHz units (0.01–655.35 MHz, LE).\n"
   "val=0 is reserved -> indicates alternate descriptor format MSN, MRL, WPD, ...\n\n"
   "NOTE:\nMost of gfx/video cards require pixel clock value divisible by 0.25MHz" },
   {&EDID_cl::DTD_HApix, NULL, offsetof(dtd_t, HApix_8lsb), 0, 12, EF_INT|EF_BFLD|EF_PIX, 0, 4095,
   "H-Active pix",
   "Horizontal active pixels (0–4095), X-resolution." },
   {&EDID_cl::DTD_HBpix, NULL, offsetof(dtd_t, HBpix_8lsb), 0, 12, EF_INT|EF_BFLD|EF_PIX, 0, 4095,
   "H-Blank pix",
   "H-blank pixels (0–4095).\n"
   "This field defines a H-Blank time as number of pixel clock pulses. H-Blank time is a time "
   "break between drawing 2 consecutive lines on the screen. During H-blank time H-sync pulse is sent "
   "to the monitor to ensure correct horizontal alignment of lines." },
   {&EDID_cl::DTD_VAlin, NULL, offsetof(dtd_t, VAlin_8lsb), 0, 12, EF_INT|EF_BFLD|EF_PIX, 0, 4095,
   "V-Active lines",
   "Vertical active lines (0–4095), V-resolution." },
   {&EDID_cl::DTD_VBlin, NULL, offsetof(dtd_t, VBlin_8lsb), 0, 12, EF_INT|EF_BFLD|EF_PIX, 0, 4095,
   "V-Blank lines",
   "V-blank lines (0–4095).\n"
   "This field defines a V-Blank time as number of H-lines. During V-Blank time V-sync pulse is sent "
   "to the monitor to ensure correct vertical alignment of the picture." },
   {&EDID_cl::DTD_HOsync, NULL, offsetof(dtd_t, HOsync_8lsb), 0, 10, EF_INT|EF_BFLD|EF_PIX, 0, 1023,
   "H-Sync offs",
   "H-sync offset in pixel clock pulses (0–1023) from blanking start. This offset value is "
   "responsible for horizontal picture alignment. Higher values are shifting the picture to the "
   "left edge of the screen." },
   {&EDID_cl::DTD_HsyncW, NULL, offsetof(dtd_t, HsyncW_8lsb), 0, 10, EF_INT|EF_BFLD|EF_PIX, 0, 1023,
   "H-Sync width",
   "H-sync pulse width (time) in pixel clock pulses (0–1023)." },
   {&EDID_cl::DTD_VOsync, NULL, offsetof(dtd_t, Vsize_8lsb)+1, 0, 6, EF_INT|EF_BFLD|EF_PIX, 0, 63,
   "V-Sync offs",
   "V-sync offset as number of H-lines (0–63). This offset value is responsible for vertical "
   "picture alignment. Higher values are shifting the picture to the top edge of the screen." },
   {&EDID_cl::DTD_VsyncW, NULL, offsetof(dtd_t, Vsize_8lsb)+1, 0, 6, EF_INT|EF_BFLD|EF_PIX, 0, 63,
   "V-Sync width",
   "V-sync pulse width (time) as number of H-lines (0–63)" },
   {&EDID_cl::DTD_Hsize, NULL, offsetof(dtd_t, Hsize_8lsb), 0, 12, EF_INT|EF_BFLD|EF_MM, 0, 4095,
   "H-Size",
   "Horizontal display size, mm (0–4095)" },
   {&EDID_cl::DTD_Vsize, NULL, offsetof(dtd_t, Vsize_8lsb), 0, 12, EF_INT|EF_BFLD|EF_MM, 0, 4095,
   "V-Size",
   "Vertical display size, mm (0–4095)" },
   {&EDID_cl::ByteVal, NULL, offsetof(dtd_t, Hborder_pix), 0, 1, EF_BYTE|EF_INT|EF_PIX, 0, 255,
   "H-Border pix",
   "Horizontal border pixels: overscan compensation (each side; total is twice this)" },
   {&EDID_cl::ByteVal, NULL, offsetof(dtd_t, Vborder_lin), 0, 1, EF_BYTE|EF_INT|EF_PIX, 0, 255,
   "V-Border lines",
   "Vertical border pixels: overscan compensation (each side; total is twice this)" },
//Features flags
   {&EDID_cl::BitF8Val, NULL, offsetof(dtd_t, features), 3, 2, EF_BFLD, 0, 3, "sync_type",
   "Sync type:\n 00=Analog composite\n 01=Bipolar analog composite\n"
   " 10=Digital composite (on HSync)\n 11=Digital separate" },
   {&EDID_cl::BitVal, NULL, offsetof(dtd_t, features), 1, 1, EF_BIT, 0, 1, "Hsync_type",
   "Analog sync: 1: Sync on all 3 RGB lines, 0: sync_on_green\n"
   "Digital: HSync polarity: 1=positive" },
   {&EDID_cl::BitVal, NULL, offsetof(dtd_t, features), 2, 1, EF_BIT, 0, 1, "Vsync_type",
   "Separated digital sync: Vsync polarity: 1=positive\n"
   "Other types: composite VSync (HSync during VSync)" },
   {&EDID_cl::BitVal, NULL, offsetof(dtd_t, features), 0, 1, EF_BIT, 0, 1, "il2w_stereo",
   "2-way line-interleaved stereo, if \"stereo_mode\" is not 0b00" },
   {&EDID_cl::BitF8Val, NULL, offsetof(dtd_t, features), 5, 2, EF_BFLD, 0, 3, "stereo_mode",
   "Stereo mode:\n00=No stereo\nother values depend on \"il2w_stereo\" (bit 0):\n"
   "il2w_stereo=0:\n"
   "01= Field sequential stereo, stereo sync=1 during right image\n"
   "10= Field sequential stereo, stereo sync=1 during left image\n"
   "11= 4-way interleaved stereo\n"
   "il2w_stereo=1:\n"
   "01= 2-way interleaved stereo, right image on even lines\n"
   "10= 2-way interleaved stereo, left image on even lines\n"
   "11= Side-by-side interleaved stereo" },
   {&EDID_cl::BitVal, NULL, offsetof(dtd_t, features), 7, 1, EF_BIT, 0, 1, "interlace",
   "interlaced" }
};

#pragma GCC diagnostic ignored "-Wunused-parameter"
rcode dtd_cl::init(const u8_t* inst, u32_t orflags, edi_grp_cl* parent) {
   static const u32_t fcount = 19;

   rcode retU;

   parent_grp = parent;
   type_id    = ID_DTD;

   CopyInstData(inst, sizeof(dtd_t));

   retU = init_fields(&fields[0], inst_data, fcount, 0, Name, Desc, CodN);
   return retU;
}
#pragma GCC diagnostic warning "-Wunused-parameter"

//----> Alternative Descriptors

static const char AltDesc[] = "Defined descriptor types:\n"
" 0xFF: (MSN) Monitor serial number (text), code page 437\n"
" 0xFE: (UTX) Unspecified text (text), code page 437\n"
" 0xFD: (MRL) Monitor range limits: (Mandatory) 6- or 13-byte binary descriptor.\n"
" 0xFC: (MND) Monitor name (text), padded with 0A 20 20 (LF, SP). code page 437.\n"
" 0xFB: (WPD) Additional white point data. 2x 5-byte descriptors, padded with 0A 20 20.\n"
" 0xFA: (AST) Additional standard timing identifiers. 6x 2-byte descriptors, padded with 0A.\n\n"
" 0x00-0x0F reserved for vendors, interpreted as UNK (unknown structures).\n"
" 0x10: (VBX) VirtualBox: currently interpreted as UNK (unknown)";

//MRL: Monitor Range Limits Descriptor:
rcode EDID_cl::MRL_Hdr(u32_t op, wxString& sval, u32_t& ival, edi_dynfld_t* p_field) {
   rcode   retU;
   mrl_t  *inst = NULL;

   inst = reinterpret_cast <mrl_t*> (getInstancePtr(p_field));
   if (inst == NULL) RCD_RETURN_FAULT(retU);

   if (op == OP_READ) {
      retU = rdByteStr(sval, inst->zero_hdr, p_field->field.fldsize);
      if (! RCD_IS_OK(retU)) return retU;
      ival =  inst->zero_hdr[0];
      ival = (inst->zero_hdr[1] << 8);
      ival = (inst->zero_hdr[2] << 16);
   } else {
      ulong   tmpv;
      RCD_SET_FAULT(retU);

      if (op == OP_WRSTR) {
         retU = getStrUint(sval, 16, p_field->field.minv, p_field->field.maxv, tmpv);
      } else if (op == OP_WRINT) {
         tmpv = ival;
         RCD_SET_OK(retU);
      }
      if (! RCD_IS_OK(retU)) return retU;

      inst->zero_hdr[0] = (tmpv & 0xFF);
      tmpv = tmpv >> 8;
      inst->zero_hdr[1] = (tmpv & 0xFF);
      tmpv = tmpv >> 8;
      inst->zero_hdr[2] = (tmpv & 0xFF);
   }
   return retU;
}

rcode EDID_cl::MRL_GTFM(u32_t op, wxString& sval, u32_t& ival, edi_dynfld_t* p_field) {
   rcode   retU;
   mrl_t  *inst = NULL;

   inst = reinterpret_cast <mrl_t*> (getInstancePtr(p_field));
   if (inst == NULL) RCD_RETURN_FAULT(retU);

   if (op == OP_READ) {
      ival = inst->gtf_m;
      sval << ival;
      RCD_SET_OK(retU);
   } else {
      ulong   tmpv;
      RCD_SET_FAULT(retU);

      if (op == OP_WRSTR) {
         retU = getStrUint(sval, 10, p_field->field.minv, p_field->field.maxv, tmpv);
      } else if (op == OP_WRINT) {
         tmpv = ival;
         RCD_SET_OK(retU);
      }
      if (! RCD_IS_OK(retU)) return retU;

      inst->gtf_m = (tmpv & 0xFFFF);
   }
   return retU;
}

rcode EDID_cl::MRL_MaxPixClk(u32_t op, wxString& sval, u32_t& ival, edi_dynfld_t* p_field) {
   rcode   retU;
   mrl_t  *inst = NULL;

   inst = reinterpret_cast <mrl_t*> (getInstancePtr(p_field));
   if (inst == NULL) RCD_RETURN_FAULT(retU);

   if (op == OP_READ) {
      ival = (inst->max_pixclk * 10);
      sval << ival;
      RCD_SET_OK(retU);
   } else {
      ulong   tmpv;
      RCD_SET_FAULT(retU);

      if (op == OP_WRSTR) {
         retU = getStrUint(sval, 10, p_field->field.minv, p_field->field.maxv, tmpv);
      } else if (op == OP_WRINT) {
         tmpv = ival;
         RCD_SET_OK(retU);
      }
      if (! RCD_IS_OK(retU)) return retU;

      inst->max_pixclk = ((tmpv/10) & 0xFF);
   }

   return retU;
}

//MRL : Monitor Range Limits Descriptor (type 0xFD)
const char  mrl_cl::CodN[] = "MRL";
const char  mrl_cl::Name[] = "Monitor Range Limits";
const char *mrl_cl::Desc   = AltDesc;

static const char altHdrDsc[] = "Zero hdr (3 bytes) -> DTD:pix_clk=0, HApix_8lsb=0 -> "
                                "alternative descriptor type (means not a DTD)";
static const char altZeroDsc[] = "Mandatory zero for a non-DTD (alternative) descriptor.";

const edi_field_t mrl_cl::fields[] = {
//header
   {&EDID_cl::MRL_Hdr, NULL, offsetof(mrl_t, zero_hdr), 0, 3, EF_STR|EF_HEX|EF_RD, 0, 0xFFFFFF,
   "zero_hdr", altHdrDsc },
   {&EDID_cl::ByteVal, NULL, offsetof(mrl_t, desc_type), 0, 1, EF_BYTE|EF_HEX|EF_RD, 0, 255, "desc_type",
   "MRL: Monitor Range Limits Descriptor (type 0xFD)" },
   {&EDID_cl::ByteVal, NULL, offsetof(mrl_t, zero_req1), 0, 1, EF_BYTE|EF_INT|EF_RD, 0, 0, "zero_req",
   altZeroDsc },
//data: bytes 5-17
   {&EDID_cl::ByteVal, NULL, offsetof(mrl_t, min_Vfreq), 0, 1, EF_BYTE|EF_INT|EF_HZ, 0, 255, "min_Vfreq",
   "minimal V-frequency: 1-255Hz" },
   {&EDID_cl::ByteVal, NULL, offsetof(mrl_t, max_Vfreq), 0, 1, EF_BYTE|EF_INT|EF_HZ, 0, 255, "max_Vfreq",
   "maximum V-frequency: 1-255Hz" },
   {&EDID_cl::ByteVal, NULL, offsetof(mrl_t, min_Hfreq), 0, 1, EF_BYTE|EF_INT|EF_KHZ, 0, 255, "min_Hfreq",
   "minimal H-frequency: 1-255kHz" },
   {&EDID_cl::ByteVal, NULL, offsetof(mrl_t, max_Hfreq), 0, 1, EF_BYTE|EF_INT|EF_KHZ, 0, 255, "max_Hfreq",
   "maximum H-frequency: 1-255kHz" },
   {&EDID_cl::MRL_MaxPixClk, NULL, offsetof(mrl_t, max_pixclk), 0, 1, EF_INT|EF_MHZ, 10, 2550,
   "max_pixclk",
   "Max pixel clock rate.\nByte value rounded up to 10 MHz multiple (10–2550 MHz)" },
   {&EDID_cl::ByteVal, NULL, offsetof(mrl_t, extd_timg), 0, 1, EF_BYTE|EF_HEX, 0, 2, "extd_timg",
   "Extended timing info (bytes 12-17):\n00: Not used (NU flag), padded with 0A 20 20 20 20 20 20.\n"
   "02: Secondary GTF supported, parameters in bytes 12-17 of the descriptor" },
   {&EDID_cl::ByteVal, NULL, offsetof(mrl_t, resvd), 0, 1, EF_BYTE|EF_INT|EF_RD, 0, 0, "resvd",
   "Mandatory zero if ext_timg is 0x02 otherwise it should be 0x0A (10)" },
   {&EDID_cl::ByteVal, NULL, offsetof(mrl_t, sfreq_sec), 0, 1, EF_BYTE|EF_INT, 0, 255, "sfreq_sec",
   "Start frequency for secondary curve, 2 kHz units (0–510 kHz)" },
   {&EDID_cl::ByteVal, NULL, offsetof(mrl_t, gtf_c), 0, 1, EF_BYTE|EF_INT, 0, 255, "gtf_c",
   "GTF C value, multiplied by 2: 0...255 ->  0...127.5" },
   {&EDID_cl::MRL_GTFM, NULL, offsetof(mrl_t, gtf_m), 0, 2, EF_INT, 0, 65535, "gtf_m",
   "GTF M value (0–65535, LE)" },
   {&EDID_cl::ByteVal, NULL, offsetof(mrl_t, gtf_k), 0, 1, EF_BYTE|EF_INT, 0, 255, "gtf_k",
   "GTF K value (0–255)" },
   {&EDID_cl::ByteVal, NULL, offsetof(mrl_t, gtf_j), 0, 1, EF_BYTE|EF_INT, 0, 255, "gtf_j",
   "GTF J value, multiplied by 2: 0...255 ->  0...127.5" }
};

#pragma GCC diagnostic ignored "-Wunused-parameter"
rcode mrl_cl::init(const u8_t* inst, u32_t orflags, edi_grp_cl* parent) {
   static const u32_t fcount = 15;

   rcode retU;

   parent_grp = parent;
   type_id    = ID_MRL;

   CopyInstData(inst, sizeof(mrl_t));

   retU = init_fields(&fields[0], inst_data, fcount, 0, Name, Desc, CodN);
   if (! RCD_IS_OK(retU)) return retU;

   //validator
   if (reinterpret_cast <mrl_t*> (inst_data)->extd_timg == 0) {
      //Extended timing information = 0 -> bytes 11-17 are padded with 0x0A0202(02...)
      for (u32_t itf=9; itf<15; itf++) {
         FieldsAr.Item(itf)->field.flags |= EF_NU;
      }
   }
   return retU;
}
#pragma GCC diagnostic warning "-Wunused-parameter"

//WPD: White Point Descriptor
rcode EDID_cl::WPD_pad(u32_t op, wxString& sval, u32_t& ival, edi_dynfld_t* p_field) {
   rcode   retU;
   wpd_t  *inst = NULL;

   inst = reinterpret_cast <wpd_t*> (getInstancePtr(p_field));
   if (inst == NULL) RCD_RETURN_FAULT(retU);

   if (op == OP_READ) {
      ival  =  inst->pad[0];
      ival |= (inst->pad[1] << 8);
      ival |= (inst->pad[2] << 16);
      sval.Printf("%06X", ival);
      RCD_SET_OK(retU);
   } else {
      ulong tmpv = 0;
      RCD_SET_FAULT(retU);

      if (op == OP_WRSTR) {
         retU = getStrUint(sval, 16, p_field->field.minv, p_field->field.maxv, tmpv);
         if (! RCD_IS_OK(retU)) return retU;
      } else if (op == OP_WRINT) {
         tmpv = ival;
         RCD_SET_OK(retU);
      }

      inst->pad[0] = (tmpv & 0xFF);
      tmpv = tmpv >> 8;
      inst->pad[1] = (tmpv & 0xFF);
      tmpv = tmpv >> 8;
      inst->pad[2] = (tmpv & 0xFF);
   }
   return retU;
}

//WPD: White Point Descriptor (type 0xFB)
const char  wpt_cl::CodN[] = "WPD";
const char  wpt_cl::Name[] = "White Point Descriptor";
const char *wpt_cl::Desc   = AltDesc;


const edi_field_t wpt_cl::fields[] = {
//header
   {&EDID_cl::MRL_Hdr, NULL, offsetof(wpd_t, zero_hdr), 0, 3, EF_STR|EF_HEX|EF_RD, 0, 0xFFFFFF,
   "zero_hdr", altHdrDsc },
   {&EDID_cl::ByteVal, NULL, offsetof(wpd_t, desc_type), 0, 1, EF_BYTE|EF_HEX|EF_RD, 0, 255, "desc_type",
   "WPT: White Point Descriptor (type 0xFB)" },
   {&EDID_cl::ByteVal, NULL, offsetof(wpd_t, zero_req1), 0, 1, EF_BYTE|EF_INT|EF_RD, 0, 0, "zero_req",
   altZeroDsc },
//data: bytes 5-17
//white point desc. 1
   {&EDID_cl::ByteVal, NULL, offsetof(wpd_t, wp1_idx), 0, 1, EF_BYTE, 0, 255, "wp1_idx",
   "White point1 index number (1–255), 0 -> descriptor not used." },
   {&EDID_cl::ByteVal, NULL, offsetof(wpd_t, wp1x_8msb), 0, 1, EF_BYTE, 0, 255, "wp1_x",
   "White point1 x value." },
   {&EDID_cl::ByteVal, NULL, offsetof(wpd_t, wp1y_8msb), 0, 1, EF_BYTE, 0, 255, "wp1_y",
   "White point1 y value." },
   {&EDID_cl::Gamma, NULL, offsetof(wpd_t, wp1_gamma), 0, 1, 0, 0, 255, "wp1_gamma",
   "(gamma*100)-100 (1.0 ... 3.54)" },
//white point desc. 2
   {&EDID_cl::ByteVal, NULL, offsetof(wpd_t, wp2_idx), 0, 1, EF_BYTE, 0, 255, "wp2_idx",
   "White point1 index number (1–255), 0 -> descriptor not used." },
   {&EDID_cl::ByteVal, NULL, offsetof(wpd_t, wp2x_8msb), 0, 1, EF_BYTE, 0, 255, "wp2_x",
   "White point2 x value." },
   {&EDID_cl::ByteVal, NULL, offsetof(wpd_t, wp2y_8msb), 0, 1, EF_BYTE, 0, 255, "wp2_y",
   "White point2 y value." },
   {&EDID_cl::Gamma, NULL, offsetof(wpd_t, wp2_gamma), 0, 1, 0, 0, 255, "wp2_gamma",
   "(gamma*100)-100 (1.0 ... 3.54)" },
//pad
   {&EDID_cl::WPD_pad, NULL, offsetof(wpd_t, pad), 0, 3, EF_STR|EF_HEX|EF_RD, 0, 0x0A2020, "pad",
   "unused, should be 0A 20 20. (LF,SP,SP)" }
};

#pragma GCC diagnostic ignored "-Wunused-parameter"
rcode wpt_cl::init(const u8_t* inst, u32_t orflags, edi_grp_cl* parent) {
   static const u32_t fcount = 12;

   rcode retU;

   parent_grp = parent;
   type_id    = ID_WPD;

   CopyInstData(inst, sizeof(wpd_t));

   retU = init_fields(&fields[0], inst_data, fcount, 0, Name, Desc, CodN);
   return retU;
}
#pragma GCC diagnostic warning "-Wunused-parameter"

//Monitor Name Descriptor
//MND: Monitor Name Descriptor (type 0xFC)
const char  mnd_cl::CodN[] = "MND";
const char  mnd_cl::Name[] = "Monitor Name Descriptor";
const char *mnd_cl::Desc   = AltDesc;

const edi_field_t mnd_cl::fields[] = {
//header
   {&EDID_cl::MRL_Hdr, NULL, offsetof(mnd_t, zero_hdr), 0, 3, EF_STR|EF_HEX|EF_RD, 0, 0xFFFFFF,
   "zero_hdr", altHdrDsc },
   {&EDID_cl::ByteVal, NULL, offsetof(mnd_t, desc_type), 0, 1, EF_BYTE|EF_HEX|EF_RD, 0, 255, "desc_type",
   "MND: Monitor Name Descriptor (type 0xFC)" },
   {&EDID_cl::ByteVal, NULL, offsetof(mnd_t, zero_req1), 0, 1, EF_BYTE|EF_INT|EF_RD, 0, 0, "zero_req",
   altZeroDsc },
//data: bytes 5-17
   {&EDID_cl::FldPadString, NULL, offsetof(mnd_t, text), 0, 13, EF_STR|EF_NI, 0, 0, "Monitor name",
   "Monitor name: text string padded with 0A 20 20 (LF,SP,SP), max 13 chars." }
};

#pragma GCC diagnostic ignored "-Wunused-parameter"
rcode mnd_cl::init(const u8_t* inst, u32_t orflags, edi_grp_cl* parent) {
   static const u32_t fcount = 4;

   rcode retU;

   parent_grp = parent;
   type_id    = ID_MND;

   CopyInstData(inst, sizeof(mnd_t));

   retU = init_fields(&fields[0], inst_data, fcount, 0, Name, Desc, CodN);
   return retU;
}
#pragma GCC diagnostic warning "-Wunused-parameter"

//Monitor Serial Number
//MSN: Monitor Serial Number Descriptor (type 0xFF)
const char  msn_cl::CodN[] = "MSN";
const char  msn_cl::Name[] = "Monitor Serial Number";
const char *msn_cl::Desc   = AltDesc;

const edi_field_t msn_cl::fields[] = {
//header
   {&EDID_cl::MRL_Hdr, NULL, offsetof(mnd_t, zero_hdr), 0, 3, EF_STR|EF_HEX|EF_RD, 0, 0xFFFFFF,
   "zero_hdr", altHdrDsc },
   {&EDID_cl::ByteVal, NULL, offsetof(mnd_t, desc_type), 0, 1, EF_BYTE|EF_HEX|EF_RD, 0, 255, "desc_type",
   "MSN: Monitor Serial Number descriptor (type 0xFF)" },
   {&EDID_cl::ByteVal, NULL, offsetof(mnd_t, zero_req1), 0, 1, EF_BYTE|EF_INT|EF_RD, 0, 0, "zero_req",
   altZeroDsc },
//data: bytes 5-17
   {&EDID_cl::FldPadString, NULL, offsetof(mnd_t, text), 0, 13, EF_STR|EF_RD|EF_NI, 0, 0, "Monitor SN",
   "Monitor serial number: text string padded with 0A 20 20 (LF,SP,SP), max 13 chars." }
};

#pragma GCC diagnostic ignored "-Wunused-parameter"
rcode msn_cl::init(const u8_t* inst, u32_t orflags, edi_grp_cl* parent) {
   static const u32_t fcount = 4;

   rcode retU;

   parent_grp = parent;
   type_id    = ID_MSN;

   CopyInstData(inst, sizeof(mnd_t));

   retU = init_fields(&fields[0], inst_data, fcount, 0, Name, Desc, CodN);
   return retU;
}
#pragma GCC diagnostic warning "-Wunused-parameter"

//Unspecified TeXt
//UTX: UnSpecified Text (type 0xFE)
const char  utx_cl::CodN[] = "UTX";
const char  utx_cl::Name[] = "Unspecified Text";
const char *utx_cl::Desc   = AltDesc;

const edi_field_t utx_cl::fields[] = {
//header
   {&EDID_cl::MRL_Hdr, NULL, offsetof(mnd_t, zero_hdr), 0, 3, EF_STR|EF_HEX|EF_RD, 0, 0xFFFFFF,
   "zero_hdr", altHdrDsc },
   {&EDID_cl::ByteVal, NULL, offsetof(mnd_t, desc_type), 0, 1, EF_BYTE|EF_HEX|EF_RD, 0, 255, "desc_type",
   "UTX: UnSpecified Text (type 0xFE)" },
   {&EDID_cl::ByteVal, NULL, offsetof(mnd_t, zero_req1), 0, 1, EF_BYTE|EF_INT|EF_RD, 0, 0, "zero_req",
   altZeroDsc },
//data: bytes 5-17
   {&EDID_cl::FldPadString, NULL, offsetof(mnd_t, text), 0, 13, EF_STR|EF_RD|EF_NI, 0, 0, "UnSpec Txt",
   "Unspecified (general purpose) text string padded with 0A 20 20 (LF,SP,SP), max 13 chars." }
};

#pragma GCC diagnostic ignored "-Wunused-parameter"
rcode utx_cl::init(const u8_t* inst, u32_t orflags, edi_grp_cl* parent) {
   static const u32_t fcount = 4;

   rcode retU;

   parent_grp = parent;
   type_id    = ID_UST;

   CopyInstData(inst, sizeof(mnd_t));

   retU = init_fields(&fields[0], inst_data, fcount, 0, Name, Desc, CodN);
   return retU;
}
#pragma GCC diagnostic warning "-Wunused-parameter"

//Additional Standard Timings
//AST: Additional Standard Timings identifiers (type 0xFA)
const char  ast_cl::CodN[] = "AST";
const char  ast_cl::Name[] = "Additional Standard Timings";
const char *ast_cl::Desc   = AltDesc;

const edi_field_t ast_cl::fields[] = {
//header
   {&EDID_cl::MRL_Hdr, NULL, offsetof(mnd_t, zero_hdr), 0, 3, EF_STR|EF_HEX|EF_RD, 0, 0xFFFFFF, "zero_hdr",
   altHdrDsc },
   {&EDID_cl::ByteVal, NULL, offsetof(mnd_t, desc_type), 0, 1, EF_BYTE|EF_HEX|EF_RD, 0, 255, "desc_type",
   "UST: UnSpecified Text (type 0xFE)" },
   {&EDID_cl::ByteVal, NULL, offsetof(mnd_t, zero_req1), 0, 1, EF_BYTE|EF_INT|EF_RD, 0, 0, "zero_req",
   altZeroDsc },
//STI0:
   {&EDID_cl::StdTxres8, NULL, offsetof(ast_t, x0_res8), 0, 1, EF_INT|EF_PIX, 256, 2288, "X-res8-0",
   stiXres8dsc },
   {&EDID_cl::StdTvsync, NULL, offsetof(ast_t, x0_res8)+1, 0, 5, EF_INT|EF_BFLD|EF_HZ, 0, 63, "V-freq0",
   stiVsyncDsc },
   {&EDID_cl::BitF8Val, NULL, offsetof(ast_t, x0_res8)+1, 6, 2, EF_BFLD, 0, 3, "pix_ratio0",
   stiPixRatioDsc },
//STI1:
   {&EDID_cl::StdTxres8, NULL, offsetof(ast_t, x1_res8), 0, 1, EF_INT|EF_PIX, 256, 2288, "X-res8-1",
   stiXres8dsc },
   {&EDID_cl::StdTvsync, NULL, offsetof(ast_t, x1_res8)+1, 0, 5, EF_INT|EF_BFLD|EF_HZ, 0, 63, "V-freq1",
   stiVsyncDsc },
   {&EDID_cl::BitF8Val, NULL, offsetof(ast_t, x1_res8)+1, 6, 2, EF_BFLD, 0, 3, "pix_ratio1",
   stiPixRatioDsc },
//STI2:
   {&EDID_cl::StdTxres8, NULL, offsetof(ast_t, x2_res8), 0, 1, EF_INT|EF_PIX, 256, 2288, "X-res8-2",
   stiXres8dsc },
   {&EDID_cl::StdTvsync, NULL, offsetof(ast_t, x2_res8)+1, 0, 5, EF_INT|EF_BFLD|EF_HZ, 0, 63, "V-freq2",
   stiVsyncDsc },
   {&EDID_cl::BitF8Val, NULL, offsetof(ast_t, x2_res8)+1, 6, 2, EF_BFLD, 0, 3, "pix_ratio2",
   stiPixRatioDsc },
//STI3:
   {&EDID_cl::StdTxres8, NULL, offsetof(ast_t, x3_res8), 0, 1, EF_INT|EF_PIX, 256, 2288, "X-res8-3",
   stiXres8dsc },
   {&EDID_cl::StdTvsync, NULL, offsetof(ast_t, x3_res8)+1, 0, 5, EF_INT|EF_BFLD|EF_HZ, 0, 63, "V-freq3",
   stiVsyncDsc },
   {&EDID_cl::BitF8Val, NULL, offsetof(ast_t, x3_res8)+1, 6, 2, EF_BFLD, 0, 3, "pix_ratio3",
   stiPixRatioDsc },
//STI4:
   {&EDID_cl::StdTxres8, NULL, offsetof(ast_t, x4_res8), 0, 1, EF_INT|EF_PIX, 256, 2288, "X-res8-4",
   stiXres8dsc },
   {&EDID_cl::StdTvsync, NULL, offsetof(ast_t, x4_res8)+1, 0, 5, EF_INT|EF_BFLD|EF_HZ, 0, 63, "V-freq4",
   stiVsyncDsc },
   {&EDID_cl::BitF8Val, NULL, offsetof(ast_t, x4_res8)+1, 6, 2, EF_BFLD, 0, 3, "pix_ratio4",
   stiPixRatioDsc },
//STI5:
   {&EDID_cl::StdTxres8, NULL, offsetof(ast_t, x5_res8), 0, 1, EF_INT|EF_PIX, 256, 2288, "X-res8-5",
   stiXres8dsc },
   {&EDID_cl::StdTvsync, NULL, offsetof(ast_t, x5_res8)+1, 0, 5, EF_INT|EF_BFLD|EF_HZ, 0, 63, "V-freq5",
   stiVsyncDsc },
   {&EDID_cl::BitF8Val, NULL, offsetof(ast_t, x5_res8)+1, 6, 2, EF_BFLD, 0, 3, "pix_ratio5",
   stiPixRatioDsc },
   {&EDID_cl::ByteVal, NULL, offsetof(ast_t, pad), 0, 1, EF_BYTE|EF_INT, 0, 255, "pad",
   "pad: should be 0x0A (10)" }
};

#pragma GCC diagnostic ignored "-Wunused-parameter"
rcode ast_cl::init(const u8_t* inst, u32_t orflags, edi_grp_cl* parent) {
   static const u32_t fcount = 22;

   rcode retU;

   parent_grp = parent;
   type_id    = ID_AST;

   CopyInstData(inst, sizeof(ast_t));

   retU = init_fields(&fields[0], inst_data, fcount, 0, Name, Desc, CodN);
   return retU;
}
#pragma GCC diagnostic warning "-Wunused-parameter"

//UNK: Unknown Descriptor (type != 0xFA..0xFF)
const char  unk_cl::CodN[] = "UNK";
const char  unk_cl::Name[] = "Unknown Descriptor";
const char *unk_cl::Desc   = AltDesc;

const edi_field_t unk_cl::fields[] = {
//header
   {&EDID_cl::MRL_Hdr, NULL, offsetof(unk_t, zero_hdr), 0, 3, EF_STR|EF_HEX|EF_RD, 0, 0xFFFFFF,
   "zero_hdr", altHdrDsc },
   {&EDID_cl::ByteVal, NULL, offsetof(unk_t, desc_type), 0, 1, EF_BYTE|EF_HEX|EF_RD, 0, 255, "desc_type",
   "UNK: Unknown Descriptor (type != 0xFA-0xFF)" },
   {&EDID_cl::ByteVal, NULL, offsetof(unk_t, zero_req1), 0, 1, EF_BYTE|EF_INT|EF_RD, 0, 0, "zero_req",
   altZeroDsc }
//data: bytes 5-17: unknown
};

#pragma GCC diagnostic ignored "-Wunused-parameter"
rcode unk_cl::init(const u8_t* inst, u32_t orflags, edi_grp_cl* parent) {
   enum {
      fcount   =  3,
      unk_fcnt = 13
   };

   rcode        retU;
   edi_field_t *p_fld;

   parent_grp = parent;
   type_id    = ID_UNK;

   CopyInstData(inst, sizeof(unk_t));

   //pre-alloc buffer for array of fields:
   dyn_fldar = (edi_field_t*) malloc( (fcount + unk_fcnt) * EDI_FIELD_SZ );
   if (NULL == dyn_fldar) RCD_RETURN_FAULT(retU);

   dyn_fcnt  = fcount;
   dyn_fcnt += unk_fcnt;

   memcpy(dyn_fldar, fields, (fcount * EDI_FIELD_SZ) );

   p_fld  = dyn_fldar;
   p_fld += fcount;

   //descriptor data interpreted as unknown
   insert_unk_byte(p_fld, unk_fcnt, offsetof(unk_t, unkb));

   retU = init_fields(&dyn_fldar[0], inst_data, dyn_fcnt, 0, Name, Desc, CodN);
   return retU;
}
#pragma GCC diagnostic warning "-Wunused-parameter"

//copy local data back to EDID buffer:
//hdr_sz != 0 : sub-groups keep their own instance data copy
//hdr_sz == 0 : no sub-groups, data in root group
void edi_grp_cl::SpawnInstance(u8_t *pinst) {
   u32_t bsz;
   bsz = getDataSize();
   memcpy(pinst, inst_data, bsz);
}

//copy sub-group data to parent' group local data buffer
rcode edi_grp_cl::AssembleGroup() {
   rcode       retU;
   u32_t       dlen;
   u32_t       dsz;
   u32_t       gpidx;
   u8_t       *pbuf;
   edi_grp_cl *psubg;
   GroupAr_cl *sub_ar;

   sub_ar = getSubArray();

   if (sub_ar == NULL) {
      RCD_RETURN_OK(retU);
   }

   dlen = (31 - hdr_sz);
   pbuf = &inst_data[hdr_sz];

   for (gpidx=0; gpidx<sub_ar->GetCount(); ++gpidx) {
      psubg = sub_ar->Item(gpidx);
      dsz   = psubg ->getTotalSize();

      if (dlen < dsz) RCD_RETURN_FAULT(retU);

      memcpy(pbuf, psubg->getInsPtr(), dsz);
      pbuf += dsz;
      dlen -= dsz;
   }

   RCD_RETURN_OK(retU);
}

//called from derived class::Clone()
edi_grp_cl* edi_grp_cl::base_clone(rcode& rcd, edi_grp_cl* pgrp, u8_t* inst, u32_t orflags) {
   rcode       retU;
   GroupAr_cl *sub_ar;

   if (NULL == pgrp) return pgrp;

   //special case: cea_unkdat_cl has variable size
   if (ID_CEA_UDAT == (type_id & ID_SUBGRP_MASK)) {
      pgrp->dat_sz  = dat_sz;
   }

   retU = pgrp->init(inst, orflags, NULL);
   if (!RCD_IS_OK(retU)) {
      rcd = retU;
      //the fault message is logged, but ignored
      if (retU.detail.rcode > RCD_FVMSG) {
         delete pgrp;
         return NULL;
      }
   }
   rcd = retU; //messages passed with RCD_TRUE

   //Copy the entire local data buffer: (override init())
   //This allows to keep the original data when the group length
   //is manually changed.
   memcpy(pgrp->getInsPtr(), inst_data, 32);

   pgrp->abs_offs   = abs_offs;
   pgrp->rel_offs   = rel_offs;
   pgrp->grp_ar     = grp_ar;
   pgrp->grp_idx    = grp_idx;
   pgrp->parent_grp = parent_grp;
   pgrp->type_id    = type_id;

   //restore parent array pointer
   sub_ar = getSubArray();

   if (sub_ar != NULL) {
      sub_ar = pgrp->getSubArray();
      sub_ar->setParentArray(grp_ar);
   }

   return pgrp;
}

rcode edi_grp_cl::create_selector(edi_dynfld_t *pfld) {
   rcode retU;

   if (pfld == NULL) RCD_RETURN_FAULT(retU);

   if (pfld->field.vmap == NULL) {
      pfld->selector = NULL;
      RCD_RETURN_FAULT(retU);
   }

   pfld->selector = new wxMenu();
   if (pfld->selector == NULL) {
      RCD_RETURN_FAULT(retU);
   }

   for (u32_t itm=0; itm<pfld->field.vmap->nval; itm++) {
      wxString    tmps;
      int         tmpv;
      const char* vname;

      //reserved values have NULL vname ptr
      vname = pfld->field.vmap->vmap[itm].name;
      if (NULL == vname) continue;

      tmpv = pfld->field.vmap->vmap[itm].val;
      tmps.Printf("[%d] ", tmpv);
      tmps << wxString::FromAscii(vname);
      pfld->selector->Append( tmpv, tmps);
   }

   RCD_RETURN_OK(retU);
}

rcode edi_grp_cl::init_fields(const edi_field_t* field_ar, const u8_t* inst, u32_t fcount, u32_t orflags,
                              const char *pname, const char *pdesc, const char *pcodn)
{
   rcode   retU;
   RCD_SET_OK(retU);

   if (fcount == 0) RCD_RETURN_FAULT(retU);

   if (pcodn != NULL) CodeName  = wxString::FromAscii(pcodn);
   if (pname != NULL) GroupName = wxString::FromAscii(pname);
   if (pdesc != NULL) GroupDesc = wxString::FromAscii(pdesc);

   FieldsAr.Empty();

   for (u32_t itf=0; itf<fcount; itf++) {
      edi_dynfld_t *pfld;
      pfld = new edi_dynfld_t;
      if (pfld == NULL) RCD_RETURN_FAULT(retU);

      memcpy( &pfld->field, &field_ar[itf], EDI_FIELD_SZ);

      pfld->field.flags |= orflags;
      pfld->base         = const_cast<u8_t*> (inst);

      if ((pfld->field.flags & EF_VS) == 0) {
         pfld->selector = NULL;
      } else {
         //create value selector for this field
         retU = create_selector(pfld);
         //do not exit on fault : selector == NULL, vmap == NULL
      }

      FieldsAr.Add(pfld);
   }
   return retU;
}

void edi_grp_cl::clear_fields() {
   for (u32_t itf=0; itf<FieldsAr.GetCount(); itf++) {
      edi_dynfld_t* pfield = FieldsAr.Item(itf);
      if (pfield != NULL) {
         if (pfield->selector != NULL) delete pfield->selector;
         delete pfield;
      }
   }
}

//Insert subgroup of unknown bytes
rcode dbc_grp_cl::Append_UNK_DAT(const u8_t* inst, u32_t dlen, u32_t orflags, u32_t abs_offs, u32_t rel_offs, edi_grp_cl* parent_grp) {
   rcode retU;
   edi_grp_cl* pgrp = new cea_unkdat_cl;

   //special case: cea_unkdat_cl has variable size
   pgrp->setDataSize(dlen);

   retU = pgrp->init(inst, orflags, parent_grp);
   if (! RCD_IS_OK(retU)) return retU;

   pgrp->setAbsOffs(abs_offs);
   pgrp->setRelOffs(rel_offs);

   subgroups.Append(pgrp);

   RCD_RETURN_OK(retU);
}

