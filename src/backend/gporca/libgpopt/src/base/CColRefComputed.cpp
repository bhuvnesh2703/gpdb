//---------------------------------------------------------------------------
//	Greenplum Database
//	Copyright (C) 2011 EMC Corp.
//
//	@filename:
//		CColRefComputed.cpp
//
//	@doc:
//		Implementation of column reference class for computed columns
//---------------------------------------------------------------------------

#include "gpopt/base/CColRefComputed.h"

#include "gpos/base.h"

using namespace gpopt;


//---------------------------------------------------------------------------
//	@function:
//		CColRefComputed::CColRefComputed
//
//	@doc:
//		ctor
//		takes ownership of string; verify string is properly formatted
//
//---------------------------------------------------------------------------
CColRefComputed::CColRefComputed(const IMDType *pmdtype, INT type_modifier,
								 ULONG id, const CName *pname,
								 BOOL ndv_preserving)
	: CColRef(pmdtype, type_modifier, id, pname)
{
	GPOS_ASSERT(nullptr != pmdtype);
	GPOS_ASSERT(pmdtype->MDId()->IsValid());
	GPOS_ASSERT(nullptr != pname);
	m_ndv_preserving = ndv_preserving;
}


//---------------------------------------------------------------------------
//	@function:
//		CColRefComputed::~CColRefComputed
//
//	@doc:
//		ctor
//
//---------------------------------------------------------------------------
CColRefComputed::~CColRefComputed() = default;


// EOF
