/***************************************************************
 * Name:      grpar.cpp
 * Purpose:   EDID group arrays
 * Author:    Tomasz Pawlak (tomasz.pawlak@wp.eu)
 * Created:   2021-02-12
 * Copyright: Tomasz Pawlak (C) 2021-2022
 * License:   GPLv3+
 **************************************************************/

#include "debug.h"
#include "rcdunits.h"
#ifndef idGRP_AR
   #error "grpar.cpp: missing unit ID"
#endif
#define RCD_UNIT idGRP_AR
#include "rcode/rcode.h"

#include "wxedid_rcd_scope.h"

RCD_AUTOGEN_DEFINE_UNIT

#include "EDID_class.h"

#include <wx/arrimpl.cpp>
WX_DEFINE_OBJARRAY(wxArrGroup_cl);

//group array: base class
void GroupAr_cl::Append(edi_grp_cl* pgrp) {
   pgrp->setArrayIdx(this, GetCount() );
   Add(pgrp);
}

void GroupAr_cl::UpdateRelOffs(u32_t idx, u32_t abs_offs, u32_t rel_offs) {
   edi_grp_cl *pgrp;
   u32_t       datsz;

   for (; idx<GetCount(); ++idx) {
      pgrp      = Item(idx);

      pgrp->setAbsOffs(abs_offs);
      pgrp->setRelOffs(rel_offs);
      pgrp->setIndex  (idx);

      datsz     = pgrp->getDataSize();
      rel_offs += datsz;
      abs_offs += datsz;
   }
}

//if b_updt_soffs == false, don't update subgroups' relative offsets,
//if b_updt_soffs == true, update all subgroups' relative offsets, basing on abs_offs
void GroupAr_cl::UpdateAbsOffs(u32_t idx, u32_t abs_offs, bool b_updt_soffs) {
   edi_grp_cl *pgrp;
   u32_t       datsz;
   u32_t       rel_offs;

   rel_offs  = abs_offs;
   rel_offs %= sizeof(edid_t);

   for (; idx<GetCount(); ++idx) {
      bool  b_soffs;
      pgrp     = Item(idx);
      b_soffs  = b_updt_soffs;
      b_soffs &= (pgrp->getSubGrpCount() > 0);

      if (b_soffs) {
         edi_grp_cl *psubg;
         GroupAr_cl *sub_ar;
         u32_t       sub_rel;
         u32_t       sub_abs;

         sub_ar   = pgrp  ->getSubArray();
         psubg    = sub_ar->Item(0);
         sub_rel  = psubg ->getRelOffs();
         sub_abs  = abs_offs;
         sub_abs += sub_rel;
         sub_ar->UpdateRelOffs(0, sub_abs, sub_rel);
      }

      pgrp->setAbsOffs(abs_offs);
      pgrp->setRelOffs(rel_offs);
      pgrp->setIndex(idx);

      datsz     = pgrp->getTotalSize();
      abs_offs += datsz;
      rel_offs += datsz;
   }
}

edi_grp_cl* GroupAr_cl::base_Cut(u32_t idx) {
   u32_t        abs_offs;
   edi_grp_cl  *pgrp;
   edi_grp_cl **p_pgrp;

   p_pgrp   = Detach(idx);
   pgrp     = *p_pgrp;
   abs_offs = pgrp->getAbsOffs();

   if (idx < GetCount()) {
      UpdateAbsOffs(idx, abs_offs);
   }
   CalcDataSZ(NULL);

   return pgrp;
}

void GroupAr_cl::base_Paste(u32_t idx, edi_grp_cl* pgrp) {
   u32_t       abs_offs;
   edi_grp_cl *pgrp_idx;
   GroupAr_cl *sub_ar;

   pgrp_idx = Item(idx);
   abs_offs = pgrp_idx->getAbsOffs();

   pgrp->setArrayIdx(this, idx);

   sub_ar = pgrp->getSubArray();
   if (sub_ar != NULL) {
      sub_ar->setParentArray(this);
   }

   Detach(idx);
   delete pgrp_idx;

   Insert(pgrp, idx);
   UpdateAbsOffs(idx, abs_offs);
   CalcDataSZ(NULL);
}

void GroupAr_cl::base_Delete(u32_t idx) {
   u32_t       abs_offs;
   edi_grp_cl *pgrp;

   pgrp     = Item(idx);
   abs_offs = pgrp->getAbsOffs();

   Detach(idx);
   delete pgrp;

   if (idx < GetCount()) {
      UpdateAbsOffs(idx, abs_offs);
   }
   CalcDataSZ(NULL);
}

void GroupAr_cl::base_MoveUp(u32_t idx) {
   //MoveUp(idx) == MoveDn(idx-1)
   idx -- ;
   base_MoveDn(idx);
}

void GroupAr_cl::base_MoveDn(u32_t idx) {
   u32_t        abs_offs;
   edi_grp_cl  *pgrp;
   edi_grp_cl **p_pgrp;

   p_pgrp    = Detach(idx);
   pgrp      = *p_pgrp;
   abs_offs  = pgrp->getAbsOffs();
   idx      ++ ;
   Insert(pgrp, idx);
   idx      -- ; //next group becomes prev. group

   UpdateAbsOffs(idx, abs_offs);
}

void GroupAr_cl::base_InsertUp(u32_t idx, edi_grp_cl* pgrp) {
   u32_t       abs_offs;
   edi_grp_cl *pgrp_idx;

   pgrp_idx = Item(idx);
   abs_offs = pgrp_idx->getAbsOffs();

   pgrp->setArrayIdx(this, idx);

   Insert(pgrp, idx);
   UpdateAbsOffs(idx, abs_offs);
   CalcDataSZ(NULL);
}

void GroupAr_cl::base_InsertDn(u32_t idx, edi_grp_cl* pgrp) {
   //InsertDn(idx) == InsertUp(idx+1)
   idx ++ ;
   if (idx < GetCount()) {
      base_InsertUp(idx, pgrp);
      return;
   }
   //insert after last group
   idx -- ;
   base_InsertUp(idx, pgrp);
   base_MoveDn(idx);
}

bool GroupAr_cl::base_CanMoveUp(u32_t idx) {
   bool bret = false;

   if ( (idx > 0) && (idx < GetCount()) ) {
      edi_grp_cl *pgrp;
      u32_t       tid;

      //group is not fixed
      pgrp = Item(idx);
      tid  = pgrp->getTypeID();
      bret = ( 0 == (tid & (T_EDID_FIXED | T_DBC_FIXED)) );
      if (! bret) return false;

      //previous group is not fixed
      idx  -- ;
      pgrp  = Item(idx);
      tid   = pgrp->getTypeID();

      bret = ( 0 == (tid & (T_EDID_FIXED | T_DBC_FIXED)) );
   }
   return bret;
}

bool GroupAr_cl::base_CanMoveDn(u32_t idx) {
   bool  bret = false;

   if (idx < (GetCount()-1) ) {
      edi_grp_cl *pgrp;
      u32_t       tid;

      //group is not fixed
      pgrp = Item(idx);
      tid  = pgrp->getTypeID();
      bret = ( 0 == (tid & (T_EDID_FIXED | T_DBC_FIXED)) );
      if (! bret) return false;

      //next group is not fixed
      idx  ++ ;
      pgrp  = Item(idx);
      tid   = pgrp->getTypeID();

      bret = ( 0 == (tid & (T_EDID_FIXED | T_DBC_FIXED)) );
   }
   return bret;
}

bool GroupAr_cl::base_CanInsert(edi_grp_cl* pgrp) {
   i32_t bsz;
   bool  bret;

   bsz  = pgrp->getTotalSize();
   bret = (bsz <= free_sz);

   //parent block free space
   if (parent_ar != NULL) {
      bret &= parent_ar->CanInsert(pgrp);
   }
   return bret;
}

bool GroupAr_cl::CanInsInto(u32_t idx, edi_grp_cl* pgrp) {
   bool        bret;
   u32_t       tid_src;
   u32_t       tid_dst;
   GroupAr_cl *dst_ar;
   edi_grp_cl *grp_dst;

   grp_dst = Item(idx);
   dst_ar  = grp_dst->getSubArray();
   if (NULL == dst_ar) return false;

   //check subgroup free space, then parent array (*this*) free space
   bret = dst_ar->CanInsert(pgrp);
   if (! bret) return bret;


   tid_dst  = grp_dst->getTypeID();
   tid_src  = pgrp->getTypeID();

   //parent group type must match
   tid_src &= ID_PARENT_MASK;
   tid_dst &= ID_PARENT_MASK;
   bret    &= (tid_src == tid_dst);

   return bret;
}

bool GroupAr_cl::CanPaste(u32_t idx, edi_grp_cl* src_grp) {
   bool        bret;
   edi_grp_cl *dst_grp;
   u32_t       tid_src;
   u32_t       tid_dst;
   i32_t       src_bsz;
   i32_t       dst_bsz;

   dst_grp = Item(idx);
   tid_dst = dst_grp->getTypeID();
   tid_src = src_grp->getTypeID();
   bret    = (tid_dst == tid_src); //type must match
   if (! bret) return bret;

   //DBC blocks of the same type can have different sizes
   src_bsz  = src_grp->getTotalSize();
   dst_bsz  = dst_grp->getTotalSize();

   if (src_bsz > dst_bsz) {
      src_bsz -= dst_bsz;
      bret    &= (src_bsz <= free_sz);

      //parent block free space
      if (parent_ar != NULL) {
         dst_bsz  = parent_ar->getFreeSpace();
         bret    &= (src_bsz <= dst_bsz);
      }
   }
   return bret;
}

bool GroupAr_cl::CanCut(u32_t idx) {
   bool        bret;
   edi_grp_cl *pgrp;
   u32_t       tid;

   pgrp = Item(idx);
   tid  = pgrp->getTypeID();
   bret = ( 0 == (tid & (T_EDID_FIXED | T_DBC_FIXED)) );

   return bret;
}

bool GroupAr_cl::CanDelete(u32_t idx) {
   bool        bret;
   edi_grp_cl *pgrp;
   u32_t       tid;

   pgrp = Item(idx);
   tid  = pgrp->getTypeID();
   bret = ( 0 == (tid & (T_EDID_FIXED | T_DBC_FIXED)) );

   return bret;
}

void GroupAr_cl::base_CalcDataSZ(i32_t blk_sz) {
   u32_t       idx_grp;
   u32_t       idx_subg;
   edi_grp_cl *pgrp;

   used_sz = 0;

   for (idx_grp=0; idx_grp<GetCount(); ++idx_grp) {
      u32_t       dat_sz;
      edi_grp_cl *p_subg;

      pgrp     = Item(idx_grp);
      dat_sz   = pgrp->getDataSize();
      used_sz += dat_sz;

      for (idx_subg=0; idx_subg<pgrp->getSubGrpCount(); ++idx_subg) {

         p_subg   = pgrp->getSubGroup(idx_subg);
         dat_sz   = p_subg->getDataSize();
         used_sz += dat_sz;
      }
   }

   free_sz  = blk_sz;
   free_sz -= used_sz;
}

//EDID
#pragma GCC diagnostic ignored "-Wunused-parameter"
void EDID_GrpAr_cl::CalcDataSZ(edi_grp_cl *pgrp) {
   //blk size -1 chsum byte, -1 num_of_extensions byte
   base_CalcDataSZ( sizeof(edid_t) -2);
}

bool EDID_GrpAr_cl::CanInsertUp(u32_t idx, edi_grp_cl* pgrp) {
   bool  bret;
   u32_t tid;

   bret = CanInsert(pgrp);
   if (! bret) return bret;

   tid  = pgrp->getTypeID();
   tid &= ID_EDID_MASK;

   bret  = (tid >= ID_DTD);
   bret &= (tid <= ID_UNK);

   return bret;
}
#pragma GCC diagnostic warning "-Wunused-parameter"

bool EDID_GrpAr_cl::CanInsertDn(u32_t idx, edi_grp_cl* pgrp) {

   return CanInsertUp(idx, pgrp);
}

//CEA
bool CEA_GrpAr_cl::CanMoveUp(u32_t idx) {
   bool        bret;
   u32_t       tid;
   edi_grp_cl *pgrp;

   bret = base_CanMoveUp(idx);
   if (! bret) return bret;

   pgrp  = Item(idx);
   tid   = pgrp->getTypeID();

   if (ID_DTD == (tid & ID_EDID_MASK)) {
      //DTD and previous group is not a DBC
      idx  -- ;
      pgrp  = Item(idx);
      tid   = pgrp->getTypeID();
      bret  = (ID_DTD == (tid & ID_EDID_MASK));

      return bret;
   }
   //DBC
   return true;
}

bool CEA_GrpAr_cl::CanMoveDn(u32_t idx) {
   bool        bret;
   u32_t       tid;
   edi_grp_cl *pgrp;

   bret = base_CanMoveDn(idx);
   if (! bret) return bret;

   pgrp  = Item(idx);
   tid   = pgrp->getTypeID();

   if (ID_DTD == (tid & ID_EDID_MASK)) return true;

   //DBC and next group is not a DTD
   idx  ++ ;
   pgrp  = Item(idx);
   tid   = pgrp->getTypeID();
   bret  = (ID_DTD != (tid & ID_EDID_MASK));

   return bret;
}

#pragma GCC diagnostic ignored "-Wunused-parameter"
void CEA_GrpAr_cl::CalcDataSZ(edi_grp_cl *pgrp) {
   //blk size -1 chksum byte
   base_CalcDataSZ( sizeof(edid_t) -1);
}
#pragma GCC diagnostic warning "-Wunused-parameter"

bool CEA_GrpAr_cl::CanInsertUp(u32_t idx, edi_grp_cl* pgrp) {
   bool        bret;
   u32_t       tid;
   u32_t       tid_idx;
   edi_grp_cl *pgrp_idx;

   bret = CanInsert(pgrp);
   if (! bret) return bret;

   pgrp_idx = Item(idx);
   tid_idx  = pgrp_idx->getTypeID();
   tid      = pgrp->getTypeID();

   //types allowed for insertion
   bret  = (ID_ADB     <= (tid & ID_CEA_MASK   ));
   bret &= (ID_CEA_UTC >= (tid & ID_CEA_MASK   ));
   bret &= (0          == (tid & ID_SUBGRP_MASK));
   bret |= (ID_DTD     == (tid & ID_EDID_MASK  ));
   if (! bret) return bret;

   if (ID_DTD != (tid & ID_EDID_MASK)) {
      //not a DTD:
      //can insert only if block@idx is not fixed
      //and the group type is CEA
      bret      = (0 == (tid_idx &  T_EDID_FIXED    ));
      bret     &= (0 != (tid_idx & (ID_CEA_EXT_MASK | ID_CEA_MASK) ));
      if (! bret) return bret;

      //can insert only above existing DTD
      idx      -- ; //BUG: crash on CHD header
      pgrp_idx  = Item(idx);
      tid_idx   = pgrp_idx->getTypeID();
      bret      = (ID_DTD != (tid_idx & ID_EDID_MASK));

      return bret;
   }
   //Insert DTD: only above existing DTD
   bret = (ID_DTD == (tid_idx & ID_EDID_MASK));

   return bret;
}

bool CEA_GrpAr_cl::CanInsertDn(u32_t idx, edi_grp_cl* pgrp) {
   bool        bret;
   u32_t       tid;
   u32_t       tid_idx;
   edi_grp_cl *pgrp_idx;

   bret = CanInsert(pgrp);
   if (! bret) return bret;

   pgrp_idx = Item(idx);
   tid_idx  = pgrp_idx->getTypeID();
   tid      = pgrp->getTypeID();

   //types allowed for insertion
   bret  = (ID_ADB     <= (tid & ID_CEA_MASK   ));
   bret &= (ID_CEA_UTC >= (tid & ID_CEA_MASK   ));
   bret &= (0          == (tid & ID_SUBGRP_MASK));
   bret |= (ID_DTD     == (tid & ID_EDID_MASK  ));
   if (! bret) return bret;

   if (ID_DTD == (tid & ID_EDID_MASK)) {
      //DTD:
      //can insert after existing DTD
      if (ID_DTD == (tid_idx & ID_EDID_MASK)) return true;

      bret  = true;
      idx  ++ ;

      if (idx < GetCount() ) {
         //can insert if next block is a DTD
         pgrp_idx  = Item(idx);
         tid_idx   = pgrp_idx->getTypeID();
         tid_idx  &= ID_EDID_MASK;
         bret      = (ID_DTD == tid_idx);
      }
      return bret;
   }
   //not a DTD:
   //can insert only if block@idx is not a DTD
   //and the group type is CEA
   bret  = (ID_DTD != (tid_idx &  ID_EDID_MASK    ));
   bret &= (     0 != (tid_idx & (ID_CEA_EXT_MASK | ID_CEA_MASK) ));

   return bret;
}

//DBC
void DBC_GrpAr_cl::CalcDataSZ(edi_grp_cl *pgrp) {
   i32_t  hdr_sz;
   u32_t  tid;

   hdr_sz = 1;
   tid    = pgrp->getTypeID();
   tid   &= ID_CEA_EXT_MASK;

   if (tid != 0) hdr_sz ++ ;

   base_CalcDataSZ(32 - hdr_sz);
}

bool DBC_GrpAr_cl::CanInsertUp(u32_t idx, edi_grp_cl* pgrp) {
   bool        bret;
   u32_t       tid_src;
   u32_t       tid_idx;
   edi_grp_cl *pgrp_idx;

   bret = CanInsert(pgrp);
   if (! bret) return bret;

   pgrp_idx = Item(idx);
   tid_idx  = pgrp_idx->getTypeID();
   tid_src  = pgrp->getTypeID();
   //can insert if block@idx not fixed
   bret     = (0 == (tid_idx & T_DBC_FIXED));
   //parent group type must match
   tid_src &= (ID_CEA_EXT_MASK | ID_CEA_MASK);
   tid_idx &= (ID_CEA_EXT_MASK | ID_CEA_MASK);
   bret    &= (tid_src == tid_idx);

   return bret;
}

bool DBC_GrpAr_cl::CanInsertDn(u32_t idx, edi_grp_cl* pgrp) {
   bool        bret;
   u32_t       tid_src;
   u32_t       tid_idx;
   edi_grp_cl *pgrp_idx;

   bret = CanInsert(pgrp);
   if (! bret) return bret;

   pgrp_idx = Item(idx);
   tid_idx  = pgrp_idx->getTypeID();
   tid_src  = pgrp->getTypeID();

   bret  = true;
   idx  ++ ;

   if (idx < GetCount() ) {
      //can insert if next block is not fixed
      pgrp_idx = Item(idx);
      tid_idx  = pgrp_idx->getTypeID();
      bret     = (0 == (tid_idx & T_DBC_FIXED));
   }
   //parent group type must match
   tid_src &= (ID_CEA_EXT_MASK | ID_CEA_MASK);
   tid_idx &= (ID_CEA_EXT_MASK | ID_CEA_MASK);
   bret    &= (tid_src == tid_idx);

   return bret;
}

edi_grp_cl* DBC_GrpAr_cl::Cut(u32_t idx) {
   u32_t        rel_offs;
   u32_t        abs_offs;
   u32_t        parent_idx;
   edi_grp_cl  *parent;
   edi_grp_cl  *pgrp;
   edi_grp_cl **p_pgrp;

   p_pgrp   = Detach(idx);
   pgrp     = *p_pgrp;
   abs_offs = pgrp->getAbsOffs();
   rel_offs = pgrp->getRelOffs();
   parent   = pgrp->getParentGrp();

   if (idx < GetCount()) {
      UpdateRelOffs(idx, abs_offs, rel_offs);
   }
   CalcDataSZ(parent);
   parent->setDataSize(used_sz);

   abs_offs   = parent->getAbsOffs();
   parent_idx = parent->getParentArIdx();
   parent_ar  ->UpdateAbsOffs(parent_idx, abs_offs, false);
   parent_ar  ->CalcDataSZ(NULL);

   return pgrp;
}

void DBC_GrpAr_cl::Paste(u32_t idx, edi_grp_cl* pgrp) {
   u32_t       abs_offs;
   u32_t       rel_offs;
   u32_t       parent_idx;
   edi_grp_cl *pgrp_idx;
   edi_grp_cl *parent;

   pgrp_idx = Item(idx);
   parent   = pgrp_idx->getParentGrp();
   abs_offs = pgrp_idx->getAbsOffs();
   rel_offs = pgrp_idx->getRelOffs();

   pgrp->setParentGrp(parent);
   pgrp->setAbsOffs(abs_offs);
   pgrp->setRelOffs(rel_offs);
   pgrp->setArrayIdx(this, idx);

   Detach(idx);
   delete pgrp_idx;

   Insert(pgrp, idx);
   CalcDataSZ(parent);
   parent->setDataSize(used_sz);

   //DBC blocks of the same type can have different sizes
   parent_idx = parent->getParentArIdx();
   parent_ar  ->UpdateAbsOffs(parent_idx, abs_offs, true);
   parent_ar  ->CalcDataSZ(NULL);
}

void DBC_GrpAr_cl::Delete(u32_t idx) {
   u32_t       abs_offs;
   u32_t       rel_offs;
   u32_t       parent_idx;
   edi_grp_cl *pgrp;
   edi_grp_cl *parent;

   pgrp     = Item(idx);
   abs_offs = pgrp->getAbsOffs();
   rel_offs = pgrp->getRelOffs();
   parent   = pgrp->getParentGrp();

   Detach(idx);
   delete pgrp;

   if (idx < GetCount()) {
      UpdateRelOffs(idx, abs_offs, rel_offs);
   }
   CalcDataSZ(parent);
   parent->setDataSize(used_sz);

   abs_offs   = parent->getAbsOffs();
   parent_idx = parent->getParentArIdx();
   parent_ar  ->UpdateAbsOffs(parent_idx, abs_offs, false);
   parent_ar  ->CalcDataSZ(NULL);
}

void DBC_GrpAr_cl::MoveUp(u32_t idx) {
   //MoveUp(idx) == MoveDn(idx-1)
   idx -- ;
   MoveDn(idx);
}

void DBC_GrpAr_cl::MoveDn(u32_t idx) {
   u32_t        abs_offs;
   u32_t        rel_offs;
   edi_grp_cl  *pgrp;
   edi_grp_cl **p_pgrp;

   p_pgrp    = Detach(idx);
   pgrp      = *p_pgrp;
   abs_offs  = pgrp->getAbsOffs();
   rel_offs  = pgrp->getRelOffs();
   idx      ++ ;

   Insert  (pgrp, idx);
   pgrp->setIndex(idx);
   idx      -- ; //next group becomes prev. group

   UpdateRelOffs(idx, abs_offs, rel_offs);
}

void DBC_GrpAr_cl::InsertUp(u32_t idx, edi_grp_cl* pgrp) {
   u32_t       abs_offs;
   u32_t       rel_offs;
   u32_t       parent_idx;
   edi_grp_cl *pgrp_idx;
   edi_grp_cl *parent;

   pgrp_idx = Item(idx);
   abs_offs = pgrp_idx->getAbsOffs();
   rel_offs = pgrp_idx->getRelOffs();
   parent   = pgrp_idx->getParentGrp();

   pgrp->setParentGrp(parent);
   pgrp->setArrayIdx(this, idx);

   Insert(pgrp, idx);
   CalcDataSZ(parent);
   UpdateRelOffs(idx, abs_offs, rel_offs);
   parent->setDataSize(used_sz);

   abs_offs   = parent->getAbsOffs();
   parent_idx = parent->getParentArIdx();
   parent_ar  ->UpdateAbsOffs(parent_idx, abs_offs, false);
   parent_ar  ->CalcDataSZ(NULL);
}

void DBC_GrpAr_cl::InsertDn(u32_t idx, edi_grp_cl* pgrp) {
   //InsertDn(idx) == InsertUp(idx+1)
   idx ++ ;
   if (idx < GetCount()) {
      InsertUp(idx, pgrp);
      return;
   }
   //insert after last sub-group
   idx -- ;
   InsertUp(idx, pgrp);
   MoveDn  (idx);
}

void DBC_GrpAr_cl::InsertInto(edi_grp_cl* pdstg, edi_grp_cl* pgrp) {
   u32_t       abs_offs;
   u32_t       rel_offs;
   u32_t       parent_idx;

   abs_offs  = pdstg->getAbsOffs();
   rel_offs  = pdstg->getHeaderSize();
   abs_offs += rel_offs;

   Insert(pgrp, 0); //@idx==0
   CalcDataSZ(pdstg);
   UpdateRelOffs(0, abs_offs, rel_offs);
   pdstg->setDataSize(used_sz);

   abs_offs   = pdstg->getAbsOffs();
   parent_idx = pdstg->getParentArIdx();
   parent_ar  ->UpdateAbsOffs(parent_idx, abs_offs, false);
   parent_ar  ->CalcDataSZ(NULL);
}

