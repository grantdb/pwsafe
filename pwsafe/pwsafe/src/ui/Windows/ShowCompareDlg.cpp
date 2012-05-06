/*
* Copyright (c) 2003-2012 Rony Shapiro <ronys@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/
/// \file ShowCompareDlg.cpp
//-----------------------------------------------------------------------------

#include "stdafx.h"

#include "ShowCompareDlg.h"
#include "PWHistDlg.h"
#include "DboxMain.h"

#include "core/ItemData.h"
#include "core/Util.h"
#include "core/core.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

CShowCompareDlg::CShowCompareDlg(CItemData *pci, CItemData *pci_other, CWnd *pParent)
  : CPWDialog(CShowCompareDlg::IDD, pParent),
  m_pci(pci), m_pci_other(pci_other), m_ShowIdenticalFields(BST_UNCHECKED)
{
  ASSERT(m_pci != NULL && m_pci_other != NULL && pParent != NULL);
  
  m_pDbx = (DboxMain *)pParent;

  // Set up DCA to string values
  m_DCA.resize(PWSprefs::maxDCA + 1);

  m_DCA[PWSprefs::DoubleClickAutoType] = IDSC_DCAAUTOTYPE;
  m_DCA[PWSprefs::DoubleClickBrowse] = IDSC_DCABROWSE;
  m_DCA[PWSprefs::DoubleClickBrowsePlus] = IDSC_DCABROWSEPLUS;
  m_DCA[PWSprefs::DoubleClickCopyNotes] = IDSC_DCACOPYNOTES;
  m_DCA[PWSprefs::DoubleClickCopyPassword] = IDSC_DCACOPYPASSWORD;
  m_DCA[PWSprefs::DoubleClickCopyPasswordMinimize] = IDSC_DCACOPYPASSWORDMIN;
  m_DCA[PWSprefs::DoubleClickCopyUsername] = IDSC_DCACOPYUSERNAME;
  m_DCA[PWSprefs::DoubleClickViewEdit] = IDSC_DCAVIEWEDIT;
  m_DCA[PWSprefs::DoubleClickRun] = IDSC_DCARUN;
  m_DCA[PWSprefs::DoubleClickSendEmail] = IDSC_DCASENDEMAIL;
}

CShowCompareDlg::~CShowCompareDlg()
{
}

void CShowCompareDlg::DoDataExchange(CDataExchange *pDX)
{
  CPWDialog::DoDataExchange(pDX);

  DDX_Control(pDX, IDC_ITEMLIST, m_ListCtrl);
}

BEGIN_MESSAGE_MAP(CShowCompareDlg, CPWDialog)
  ON_BN_CLICKED(IDC_SHOW_IDENTICAL_FIELDS, OnShowIdenticalFields)
END_MESSAGE_MAP()

BOOL CShowCompareDlg::OnInitDialog()
{
  CPWDialog::OnInitDialog();

  // Add grid lines
  m_ListCtrl.SetExtendedStyle(LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT |
                              m_ListCtrl.GetExtendedStyle());

  // Insert List columns
  CString cs_text;
  cs_text.LoadString(IDS_SELECTFIELD);
  m_ListCtrl.InsertColumn(0, cs_text);
  cs_text.LoadString(IDS_CURRENT_ENTRY);
  m_ListCtrl.InsertColumn(1, cs_text);
  cs_text.LoadString(IDS_COMPARISON_ENTRY);
  m_ListCtrl.InsertColumn(2, cs_text);

  PopulateResults(false);

  return TRUE;
}

void CShowCompareDlg::PopulateResults(const bool bShowAll)
{
  // Populate List view
  // Our preferred field order
  const int iFields[] = {
    CItemData::NAME,                        // Special processing
    CItemData::PASSWORD,                    // Special processing
    CItemData::ENTRYTYPE,                   // Special processing
    CItemData::URL, CItemData::AUTOTYPE,
    CItemData::RUNCMD, CItemData::EMAIL,
    CItemData::DCA, CItemData::SHIFTDCA,
    CItemData::PROTECTED, CItemData::SYMBOLS,
    CItemData::POLICY, CItemData::POLICYNAME,
    CItemData::CTIME, CItemData::PMTIME, CItemData::ATIME, CItemData::XTIME,
    CItemData::RMTIME, CItemData::XTIME_INT, CItemData::PWHIST, CItemData::NOTES
  };

  // Clear out contents
  m_ListCtrl.SetRedraw(FALSE);
  m_ListCtrl.DeleteAllItems();

  const StringX sxOpenBracket(L"["), sxColon(L":"), sxCloseBracket(L"]"),
          sxGTU(L"[Group:Title:Username]");
  StringX sxNo, sxYes, sxGTU1, sxGTU2, sxGTUBase1, sxGTUBase2;
  LoadAString(sxNo, IDS_NO);
  sxNo = sxNo.substr(1);    // Remove leading ampersand
  LoadAString(sxYes, IDS_YES);
  sxYes = sxYes.substr(1);  // Remove leading ampersand
  CItemData *pci(m_pci), *pci_other(m_pci_other);
  CItemData *pci_base(NULL), *pci_other_base(NULL);

  sxGTU1 = sxOpenBracket +
             m_pci->GetGroup() + sxColon + 
             m_pci->GetTitle() + sxColon +
             m_pci->GetUser() + sxCloseBracket;
  sxGTU2 = sxOpenBracket +
             m_pci_other->GetGroup() + sxColon +
             m_pci_other->GetTitle() + sxColon +
             m_pci_other->GetUser() + sxCloseBracket;

  if (m_pci->IsAlias() || m_pci->IsShortcut()) {
    pci_base = m_pDbx->GetBaseEntry(m_pci);
    sxGTUBase1 = sxOpenBracket +
               pci_base->GetGroup() + sxColon + 
               pci_base->GetTitle() + sxColon +
               pci_base->GetUser() + sxCloseBracket;
    // If shortcut - use base entry for everything
    if (m_pci->IsShortcut())
      pci = pci_base;
  }
  if (m_pci_other->IsAlias() || m_pci_other->IsShortcut()) {
    pci_other_base = m_pDbx->GetBaseEntry(m_pci_other);
    sxGTUBase2 = sxOpenBracket +
               pci_other_base->GetGroup() + sxColon + 
               pci_other_base->GetTitle() + sxColon +
               pci_other_base->GetUser() + sxCloseBracket;
    // If shortcut - use base entry for everything
    if (m_pci_other->IsShortcut())
      pci_other = pci_other_base;
  }

  int iPos = 0;

  for (int j = 0; j < sizeof(iFields) / sizeof(iFields[0]); j++) {
    const int i = iFields[j];
    DWORD dw(0);

    if (i == CItemData::NAME) {
      // Special processing - put in [g:t:u]
      iPos = m_ListCtrl.InsertItem(iPos, sxGTU.c_str());
      m_ListCtrl.SetItemText(iPos, 1, sxGTU1.c_str());
      m_ListCtrl.SetItemText(iPos, 2, sxGTU2.c_str());
      dw = CSCWListCtrl::REDTEXT;
      m_ListCtrl.SetItemData(iPos, dw);
      iPos++;
      continue;
    }

    if (i == CItemData::ENTRYTYPE) {
      // Special processing: NOTE TESTS ARE DONE USING ORIGINAL m_pci & m_pci_other
      /*
        Entry 1 / 2 | Normal/Base | Alias | Shortcut |
        ------------|-------------|-------|----------|
        Normal/Base |      a      |   b   |    d     |
        ------------|-------------|-------|----------|
        Alias       |      b      |   c   |    e     |
        ------------|-------------|-------|----------|
        Shortcut    |      d      |   e   |    f     |
        ----------------------------------------------
        
        a. If both are normal entries or base entries - ignore this field
        b. If one is normal/base and the other is an alias - use the base's password
           and show the alias's base [g:t:u] here
        c. If both are aliases, - use their respective base's passwords
           and show their base [g:t:u] here
        d. If one is a shortcut, use information from its base entry and show its base
           [g:t:u] here.
        e. If one is an alias and the other a shortcut, use the alias's base for
           its password, use all information from the shortcut's base entry and show
           their base [g:t:u] here.
        f. If both are shortcuts, use information from their base entries and show
           their base [g:t:u] here.
      */
      const CString cs_type(MAKEINTRESOURCE(IDS_ENTRYTYPE));
      const CString cs_et1 = GetEntryTypeString(m_pci->GetEntryType());
      const CString cs_et2 = GetEntryTypeString(m_pci_other->GetEntryType());

      CString cs_label;
      StringX sxText1, sxText2;
      bool bAddBaseGTURow(false);

      // 'a' : both normal or base entries
      // However, if both normal or same type of base, don't show unless "Show All"
      if (!bShowAll && !m_pci->IsDependent() && !m_pci_other->IsDependent() &&
          cs_et1 == cs_et2)
        continue;

      // 'b' or 'c' : 1 normal/base & 1 alias or both aliases (no shortcuts)
      // However, if both aliases of same base, don't show unless "Show All"
      if (!bShowAll && m_pci->IsAlias() && m_pci_other->IsAlias() && 
          sxGTUBase1 == sxGTUBase2)
        continue;

      if ((m_pci->IsAlias() || m_pci_other->IsAlias()) &&
          (!m_pci->IsShortcut() && !m_pci_other->IsShortcut())) {
        cs_label.LoadString(IDS_EXP_ABASE);
        sxText1 = m_pci->IsAlias() ? sxGTUBase1 : L"-";
        sxText2 = m_pci_other->IsAlias() ? sxGTUBase2 : L"-";
        bAddBaseGTURow = true;
      }

      // 'd' or 'f' : 1 shortcut & 1 normal/base or both shortcuts (no aliases)
      // However, if both shortcuts of same base, don't show unless "Show All"
      if (!bShowAll && m_pci->IsShortcut() && m_pci_other->IsShortcut() && 
          sxGTUBase1 == sxGTUBase2)
        continue;

      if ((m_pci->IsShortcut() && !m_pci_other->IsAlias()) ||
          (!m_pci->IsAlias() && m_pci_other->IsShortcut())) {
        cs_label.LoadString(IDS_EXP_SBASE);
        sxText1 = m_pci->IsShortcut() ? sxGTUBase1 : L"-";
        sxText2 = m_pci_other->IsShortcut() ? sxGTUBase2 : L"-";
        bAddBaseGTURow = true;
      }

      // 'e' : 1 shortcut & 1 alias
      if ((m_pci->IsShortcut() && m_pci_other->IsAlias()) ||
          (m_pci->IsAlias() && m_pci_other->IsShortcut())) {
        const CString cs_label1(MAKEINTRESOURCE(m_pci->IsAlias() ? IDS_EXP_ABASE : IDS_EXP_SBASE));
        const CString cs_label2(MAKEINTRESOURCE(m_pci_other->IsAlias() ? IDS_EXP_ABASE : IDS_EXP_SBASE));
        cs_label = cs_label1 + L" / " + cs_label2;
        sxText1 = sxGTUBase1;
        sxText2 = sxGTUBase2;
        bAddBaseGTURow = true;
      }

      // Show entry types
      iPos = m_ListCtrl.InsertItem(iPos, cs_type);
      m_ListCtrl.SetItemText(iPos, 1, cs_et1);
      m_ListCtrl.SetItemText(iPos, 2, cs_et2);
      if (cs_et1 != cs_et2)
        dw = CSCWListCtrl::REDTEXT;
      m_ListCtrl.SetItemData(iPos, dw);
      iPos++;

      // If required, show the base entry [g:t:u]
      if (bAddBaseGTURow) {
        iPos = m_ListCtrl.InsertItem(iPos, cs_label);
        m_ListCtrl.SetItemText(iPos, 1, sxText1.c_str());
        m_ListCtrl.SetItemText(iPos, 2, sxText2.c_str());
        if (sxText1 != sxText2)
          dw = CSCWListCtrl::REDTEXT;
        m_ListCtrl.SetItemData(iPos, dw);
        iPos++;
        continue;
      }
      continue;
    }

    // Now use pci, pci_other, which will different if either entry is a shortcut
    time_t t1(0), t2(0);
    short int si1, si2;
    StringX sxValue1, sxValue2;
    stringT sFieldName = pci->FieldName((CItemData::FieldType)i);

    // Get field values
    // For aliases - use base entry passwords
    if (i == CItemData::PASSWORD && m_pci->IsAlias())
      sxValue1 = pci_base->GetFieldValue((CItemData::FieldType)i);
    else
      sxValue1 = pci->GetFieldValue((CItemData::FieldType)i);

    if (i == CItemData::PASSWORD && m_pci_other->IsAlias())
      sxValue2 = pci_other_base->GetFieldValue((CItemData::FieldType)i);
    else
      sxValue2 = pci_other->GetFieldValue((CItemData::FieldType)i);

    // Always add group/title/user fields - otherwise only if different values
    // Unless user wants all fields
    if (bShowAll || sxValue1 != sxValue2) {
      iPos = m_ListCtrl.InsertItem(iPos, sFieldName.c_str());
      m_ListCtrl.SetItemData(iPos, LVCFMT_LEFT);
      if (!pci->CItemData::IsTextField((unsigned char)i)) {
        switch (i) {
          case CItemData::CTIME:      /* 07 */
            pci->GetCTime(t1);
            pci_other->GetCTime(t2);
            if (t1 == 0) sxValue1 = L"N/A";
            if (t2 == 0) sxValue2 = L"N/A";
            break;
          case CItemData::PMTIME:     /* 08 */
            pci->GetPMTime(t1);
            pci_other->GetPMTime(t2);
            if (t1 == 0) sxValue1 = L"N/A";
            if (t2 == 0) sxValue2 = L"N/A";
            break;
          case CItemData::ATIME:      /* 09 */
            pci->GetATime(t1);
            pci_other->GetATime(t2);
            if (t1 == 0) sxValue1 = L"N/A";
            if (t2 == 0) sxValue2 = L"N/A";
            break;
          case CItemData::XTIME:      /* 0a */
            pci->GetXTime(t1);
            pci_other->GetXTime(t2);
            if (t1 == 0) sxValue1 = L"N/A";
            if (t2 == 0) sxValue2 = L"N/A";
            break;
          case CItemData::RMTIME:     /* 0c */
            pci->GetRMTime(t1);
            pci_other->GetRMTime(t2);
            if (t1 == 0) sxValue1 = L"N/A";
            if (t2 == 0) sxValue2 = L"N/A";
            break;
          case CItemData::XTIME_INT:  /* 11 */
          case CItemData::PROTECTED:  /* 15 */
            break;
          case CItemData::DCA:        /* 13 */
          case CItemData::SHIFTDCA:   /* 17 */
            pci->GetDCA(si1, i == CItemData::SHIFTDCA);
            pci_other->GetDCA(si2, i == CItemData::SHIFTDCA);
            sxValue1 = GetDCAString(si1, i == CItemData::SHIFTDCA);
            sxValue2 = GetDCAString(si2, i == CItemData::SHIFTDCA);
            break;
          default:
            ASSERT(0);
        }
        if (i == CItemData::CTIME  || i == CItemData::ATIME || i == CItemData::XTIME ||
            i == CItemData::PMTIME || i == CItemData::RMTIME) {
          if (t1 != 0)
            sxValue1 = PWSUtil::ConvertToDateTimeString(t1, PWSUtil::TMC_EXPORT_IMPORT);
          if (t2 != 0)
            sxValue2 = PWSUtil::ConvertToDateTimeString(t2, PWSUtil::TMC_EXPORT_IMPORT);
        }
        if (i == CItemData::PROTECTED) {
          sxValue1 = sxValue1.empty() ? sxNo : sxYes;
          sxValue2 = sxValue2.empty() ? sxNo : sxYes;
        }
      }
      if (i == CItemData::PWHIST) {
        size_t num_err1, num_err2, MaxPWHistory1, MaxPWHistory2;
        PWHistList pwhistlist1, pwhistlist2;
        bool status1 = CreatePWHistoryList(sxValue1,
                                      MaxPWHistory1,
                                      num_err1,
                                      pwhistlist1,
                                      PWSUtil::TMC_EXPORT_IMPORT);
        bool status2 = CreatePWHistoryList(sxValue2,
                                      MaxPWHistory2,
                                      num_err2,
                                      pwhistlist2,
                                      PWSUtil::TMC_EXPORT_IMPORT);

        // If any password history value is different - it must be red
        if (sxValue1 != sxValue2)
          dw = LVCFMT_LEFT | CSCWListCtrl::REDTEXT;
        else
          dw = LVCFMT_LEFT;
        m_ListCtrl.SetItemData(iPos, dw);
        
        // Now add sub-fields
        iPos++;

        sxValue1 =  status1 ? sxYes : sxNo;
        sxValue2 =  status2 ? sxYes : sxNo;
        if (bShowAll || sxValue1 != sxValue2) {
          LoadAString(sFieldName, IDS_PWHACTIVE);
          iPos = m_ListCtrl.InsertItem(iPos, sFieldName.c_str());
          m_ListCtrl.SetItemText(iPos, 1, sxValue1.c_str());
          m_ListCtrl.SetItemText(iPos, 2, sxValue2.c_str());
          dw = LVCFMT_RIGHT;
          if (sxValue1 != sxValue2)
            dw |= CSCWListCtrl::REDTEXT;
          m_ListCtrl.SetItemData(iPos, dw);
          iPos++;
        }

        Format(sxValue1, L"%d", MaxPWHistory1);
        Format(sxValue2, L"%d", MaxPWHistory2);
        if (bShowAll || sxValue1 != sxValue2) {
          LoadAString(sFieldName, IDS_PWHMAX);
          iPos = m_ListCtrl.InsertItem(iPos, sFieldName.c_str());
          m_ListCtrl.SetItemText(iPos, 1, sxValue1.c_str());
          m_ListCtrl.SetItemText(iPos, 2, sxValue2.c_str());
          dw = LVCFMT_RIGHT;
          if (sxValue1 != sxValue2)
            dw |= CSCWListCtrl::REDTEXT;
          m_ListCtrl.SetItemData(iPos, dw);
          iPos++;
        }

        Format(sxValue1, L"%d", pwhistlist1.size());
        Format(sxValue2, L"%d", pwhistlist2.size());
        if (bShowAll || sxValue1 != sxValue2) {
          LoadAString(sFieldName, IDS_PWHNUM);
          iPos = m_ListCtrl.InsertItem(iPos, sFieldName.c_str());
          m_ListCtrl.SetItemText(iPos, 1, sxValue1.c_str());
          m_ListCtrl.SetItemText(iPos, 2, sxValue2.c_str());
          dw = LVCFMT_RIGHT;
          if (sxValue1 != sxValue2)
            dw |= CSCWListCtrl::REDTEXT;
          m_ListCtrl.SetItemData(iPos, dw);
          iPos++;
        }

        LoadAString(sFieldName, IDS_PWHENTRY);
        size_t maxentries = max(pwhistlist1.size(), pwhistlist2.size());
        StringX sxBlank = L" ";
        for (size_t n = 0; n < maxentries; n++) {
          m_ListCtrl.SetItemData(iPos, LVCFMT_RIGHT);
          if (n < pwhistlist1.size()) {
            sxValue1 = pwhistlist1[n].changedate + sxBlank + pwhistlist1[n].password;
          }  else
            sxValue1.clear();
          if (n < pwhistlist2.size()) {
            sxValue2 = pwhistlist2[n].changedate + sxBlank + pwhistlist2[n].password;
          } else
            sxValue2.clear();

          if (bShowAll || sxValue1 != sxValue2) {
            stringT str;
            Format(str, L"%s - %d", sFieldName.c_str(), n+1);
            iPos = m_ListCtrl.InsertItem(iPos, str.c_str());
            m_ListCtrl.SetItemText(iPos, 1, sxValue1.c_str());
            m_ListCtrl.SetItemText(iPos, 2, sxValue2.c_str());
            dw = LVCFMT_RIGHT;
            if (sxValue1 != sxValue2)
              dw |= CSCWListCtrl::REDTEXT;
            m_ListCtrl.SetItemData(iPos, dw);
            iPos++;
          }
        }
      } else {
        m_ListCtrl.SetItemText(iPos, 1, sxValue1.c_str());
        m_ListCtrl.SetItemText(iPos, 2, sxValue2.c_str());
        dw = LVCFMT_LEFT;
        if (sxValue1 != sxValue2)
          dw |= CSCWListCtrl::REDTEXT;
        if (i == CItemData::PASSWORD)
          dw |= CSCWListCtrl::PASSWORDFONT;
        m_ListCtrl.SetItemData(iPos, dw);
      }
    }
    iPos++;
  }

  CRect rectLV;
	m_ListCtrl.GetClientRect(rectLV);
  int nColWidth  = (rectLV.Width()) / 3;
  int nColWidth0 = int(0.8 * nColWidth);
  int nColWidth12 = int(1.08 * nColWidth);

  m_ListCtrl.SetColumnWidth(0, nColWidth0);
  m_ListCtrl.SetColumnWidth(1, nColWidth12);
  m_ListCtrl.SetColumnWidth(2, nColWidth12);

  m_ListCtrl.SetRedraw(TRUE);
  m_ListCtrl.Invalidate();
}

void CShowCompareDlg::OnShowIdenticalFields()
{
  m_ShowIdenticalFields = ((CButton *)GetDlgItem(IDC_SHOW_IDENTICAL_FIELDS))->GetCheck();

  PopulateResults(m_ShowIdenticalFields == BST_CHECKED);
}

CString CShowCompareDlg::GetDCAString(const int iValue, const bool isShift)
{
  UINT ui(0);
  if (iValue == -1)
    ui = m_DCA[PWSprefs::GetInstance()->GetPref(isShift ?
                                          PWSprefs::ShiftDoubleClickAction :
                                          PWSprefs::DoubleClickAction)];
  else
    ui = m_DCA[iValue];

  CString cs;
  cs.LoadString(ui);
  return cs;
}

CString CShowCompareDlg::GetEntryTypeString(CItemData::EntryType et)
{
  UINT ui(IDSC_UNKNOWN);
  switch (et) {
    case CItemData::ET_NORMAL:
      ui = IDS_EXP_NORMAL;
      break;
    case CItemData::ET_ALIASBASE:
      ui = IDS_EXP_ABASE;
      break;
    case CItemData::ET_ALIAS:
      ui = IDSC_ALIAS;
      break;
    case CItemData::ET_SHORTCUTBASE:
      ui = IDS_EXP_SBASE;
      break;
    case CItemData::ET_SHORTCUT:
      ui = IDSC_SHORTCUT;
      break;
    case CItemData::ET_INVALID:
    default:
      ASSERT(0);
  }
  CString cs(MAKEINTRESOURCE(ui));
  return cs;
}
