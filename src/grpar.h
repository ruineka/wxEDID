/***************************************************************
 * Name:      grpar.h
 * Purpose:   EDID group arrays
 * Author:    Tomasz Pawlak (tomasz.pawlak@wp.eu)
 * Created:   2021-02-12
 * Copyright: Tomasz Pawlak (C) 2021-2022
 * License:   GPLv3+
 **************************************************************/

#ifndef GRP_AR_H
#define GRP_AR_H 1

#include <wx/dynarray.h>

#include "def_types.h"
#include "rcode/rcode.h"

class edi_grp_cl;
WX_DECLARE_OBJARRAY(edi_grp_cl*, wxArrGroup_cl);

class GroupAr_cl : public wxArrGroup_cl {
   protected:
      GroupAr_cl *parent_ar;

      i32_t used_sz; //block/group used space
      i32_t free_sz; //block/group free space left

      void  base_CalcDataSZ(i32_t blk_sz);

      bool  base_CanMoveUp (u32_t idx);
      bool  base_CanMoveDn (u32_t idx);
      bool  base_CanInsert (edi_grp_cl* pgrp);

      edi_grp_cl* base_Cut (u32_t);

      void  base_Paste     (u32_t idx, edi_grp_cl* pgrp);
      void  base_Delete    (u32_t idx);
      void  base_MoveUp    (u32_t idx);
      void  base_MoveDn    (u32_t idx);
      void  base_InsertUp  (u32_t idx, edi_grp_cl* pgrp);
      void  base_InsertDn  (u32_t idx, edi_grp_cl* pgrp);

   public:

              void  Append       (edi_grp_cl* pgrp);
              void  UpdateAbsOffs(u32_t idx, u32_t abs_offs, bool b_updt_soffs = true);
              void  UpdateRelOffs(u32_t idx, u32_t abs_offs, u32_t rel_offs);

      virtual edi_grp_cl* Cut  (u32_t ) {return NULL;};

      virtual bool  CanMoveUp  (u32_t ) {return false;};
      virtual bool  CanMoveDn  (u32_t ) {return false;};
      virtual bool  CanInsertUp(u32_t , edi_grp_cl* ) {return false;};
      virtual bool  CanInsertDn(u32_t , edi_grp_cl*) {return false;};

              bool  CanPaste   (u32_t idx, edi_grp_cl* src_grp);
              bool  CanCut     (u32_t idx);
              bool  CanDelete  (u32_t idx);
      inline  bool  CanInsert  (edi_grp_cl* pgrp) {return base_CanInsert(pgrp);};
              bool  CanInsInto (u32_t idx, edi_grp_cl* pgrp);

      virtual void  Paste      (u32_t , edi_grp_cl*) {return;};
      virtual void  Delete     (u32_t ) {return;};
      virtual void  MoveUp     (u32_t ) {return;};
      virtual void  MoveDn     (u32_t ) {return;};
      virtual void  InsertUp   (u32_t , edi_grp_cl*) {return;};
      virtual void  InsertDn   (u32_t , edi_grp_cl*) {return;};
      virtual void  InsertInto (edi_grp_cl*, edi_grp_cl*) {return;};

      inline  void        setParentArray(GroupAr_cl* parr) {parent_ar = parr;};
      inline  GroupAr_cl* getParentArray() {return parent_ar;};

      inline  i32_t getUsedSize  () {return used_sz;};
      inline  i32_t getFreeSpace () {return free_sz;};
      inline  i32_t getTotalSpace() {return (used_sz + free_sz);};

      virtual void  CalcDataSZ(edi_grp_cl*) {return;};

      GroupAr_cl() : parent_ar(NULL), used_sz(0), free_sz(0) { Alloc(16);};

      ~GroupAr_cl() {
         Clear();
      };
};

//Base EDID block groups
class EDID_GrpAr_cl : public GroupAr_cl {
   public:
      void  CalcDataSZ (edi_grp_cl *pgrp = NULL);

      bool  CanMoveUp  (u32_t idx) {return base_CanMoveUp(idx);};
      bool  CanMoveDn  (u32_t idx) {return base_CanMoveDn(idx);};
      bool  CanInsertUp(u32_t idx, edi_grp_cl* pgrp);
      bool  CanInsertDn(u32_t idx, edi_grp_cl* pgrp);

      edi_grp_cl* Cut  (u32_t idx) {return base_Cut(idx);};

      void  Paste      (u32_t idx, edi_grp_cl* pgrp) { base_Paste   (idx, pgrp);};
      void  Delete     (u32_t idx) { base_Delete  (idx);};
      void  MoveUp     (u32_t idx) { base_MoveUp  (idx);};
      void  MoveDn     (u32_t idx) { base_MoveDn  (idx);};
      void  InsertUp   (u32_t idx, edi_grp_cl* pgrp) { base_InsertUp(idx, pgrp);};
      void  InsertDn   (u32_t idx, edi_grp_cl* pgrp) { base_InsertDn(idx, pgrp);};
};

//CEA Extension block groups
class CEA_GrpAr_cl : public GroupAr_cl {
   public:
      void  CalcDataSZ (edi_grp_cl *pgrp = NULL);

      bool  CanMoveUp  (u32_t idx);
      bool  CanMoveDn  (u32_t idx);
      bool  CanInsertUp(u32_t idx, edi_grp_cl* pgrp);
      bool  CanInsertDn(u32_t idx, edi_grp_cl* pgrp);

      edi_grp_cl* Cut  (u32_t idx) {return base_Cut(idx);};

      void  Paste      (u32_t idx, edi_grp_cl* pgrp) { base_Paste   (idx, pgrp);};
      void  Delete     (u32_t idx) { base_Delete  (idx);};
      void  MoveUp     (u32_t idx) { base_MoveUp  (idx);};
      void  MoveDn     (u32_t idx) { base_MoveDn  (idx);};
      void  InsertUp   (u32_t idx, edi_grp_cl* pgrp) { base_InsertUp(idx, pgrp);};
      void  InsertDn   (u32_t idx, edi_grp_cl* pgrp) { base_InsertDn(idx, pgrp);};
};

//DBC sub-groups
class DBC_GrpAr_cl : public GroupAr_cl {
   public:
      void  CalcDataSZ (edi_grp_cl *pgrp);

      bool  CanMoveUp  (u32_t idx) {return base_CanMoveUp(idx);};
      bool  CanMoveDn  (u32_t idx) {return base_CanMoveDn(idx);};
      bool  CanInsertUp(u32_t idx, edi_grp_cl* pgrp);
      bool  CanInsertDn(u32_t idx, edi_grp_cl* pgrp);

      edi_grp_cl* Cut  (u32_t idx);

      void  Paste      (u32_t idx, edi_grp_cl* pgrp);
      void  Delete     (u32_t idx);
      void  MoveUp     (u32_t idx);
      void  MoveDn     (u32_t idx);
      void  InsertUp   (u32_t idx, edi_grp_cl* pgrp);
      void  InsertDn   (u32_t idx, edi_grp_cl* pgrp);
      void  InsertInto (edi_grp_cl* pdstg, edi_grp_cl* pgrp);
};

#endif /* GRP_AR_H */
