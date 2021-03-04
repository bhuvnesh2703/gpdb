//---------------------------------------------------------------------------
//	Greenplum Database
//	Copyright (C) 2011 EMC Corp.
//
//	@filename:
//		CScalarSubqueryQuantified.cpp
//
//	@doc:
//		Implementation of quantified subquery operator
//---------------------------------------------------------------------------

#include "gpopt/operators/CScalarSubqueryQuantified.h"

#include "gpos/base.h"

#include "gpopt/base/CColRefSet.h"
#include "gpopt/base/CDrvdPropScalar.h"
#include "gpopt/base/COptCtxt.h"
#include "gpopt/operators/CExpressionHandle.h"
#include "gpopt/xforms/CSubqueryHandler.h"
#include "naucrates/md/IMDScalarOp.h"
#include "naucrates/md/IMDTypeBool.h"

using namespace gpopt;
using namespace gpmd;

//---------------------------------------------------------------------------
//	@function:
//		CScalarSubqueryQuantified::CScalarSubqueryQuantified
//
//	@doc:
//		Ctor
//
//---------------------------------------------------------------------------
CScalarSubqueryQuantified::CScalarSubqueryQuantified(
	CMemoryPool *mp, CColRefSet *colrefset)
	: CScalar(mp),
	  m_pcrs(colrefset)
{
	GPOS_ASSERT(nullptr != colrefset);
}

//---------------------------------------------------------------------------
//	@function:
//		CScalarSubqueryQuantified::~CScalarSubqueryQuantified
//
//	@doc:
//		Dtor
//
//---------------------------------------------------------------------------
CScalarSubqueryQuantified::~CScalarSubqueryQuantified()
{
	m_pcrs->Release();
}

IMDId *
CScalarSubqueryQuantified::MdidType() const
{
	GPOS_ASSERT(!"Invalid function call: CScalarSubqueryQuantified::MdidType()");
	return nullptr;
}

IMDId *
CScalarSubqueryQuantified::MdIdOp() const
{
	GPOS_ASSERT(!"Invalid function call: CScalarSubqueryQuantified::MdIdOp()");
	return nullptr;
}

//---------------------------------------------------------------------------
//	@function:
//		CScalarSubqueryQuantified::HashValue
//
//	@doc:
//		Operator specific hash function
//
//---------------------------------------------------------------------------
ULONG
CScalarSubqueryQuantified::HashValue() const
{
	return gpos::CombineHashes(
		COperator::HashValue(),
		m_pcrs->HashValue());
}


//---------------------------------------------------------------------------
//	@function:
//		CScalarSubqueryQuantified::Matches
//
//	@doc:
//		Match function on operator level
//
//---------------------------------------------------------------------------
BOOL
CScalarSubqueryQuantified::Matches(COperator *pop) const
{
	if (pop->Eopid() != Eopid())
	{
		return false;
	}

	// match if contents are identical
	CScalarSubqueryQuantified *popSsq =
		CScalarSubqueryQuantified::PopConvert(pop);
	return popSsq->Pcrs()->Equals(m_pcrs);
}


//---------------------------------------------------------------------------
//	@function:
//		CScalarSubqueryQuantified::PcrsUsed
//
//	@doc:
//		Locally used columns
//
//---------------------------------------------------------------------------
CColRefSet *
CScalarSubqueryQuantified::PcrsUsed(CMemoryPool *mp, CExpressionHandle &exprhdl)
{
	// used columns is an empty set unless subquery column is an outer reference
	CColRefSet *pcrs = GPOS_NEW(mp) CColRefSet(mp);

	CColRefSet *pcrsChildOutput =
		exprhdl.DeriveOutputColumns(0 /* child_index */);
	CColRefSet *pcrsSubquery = GPOS_NEW(mp) CColRefSet(mp, *m_pcrs);
	pcrsSubquery->Exclude(pcrsChildOutput);
	if (pcrsSubquery->Size() > 0)
	{
		// subquery column is not produced by relational child, add it to used columns
		pcrs->Include(pcrsSubquery);
	}
	pcrsSubquery->Release();

	return pcrs;
}


//---------------------------------------------------------------------------
//	@function:
//		CScalarSubqueryQuantified::DerivePartitionInfo
//
//	@doc:
//		Derive partition consumers
//
//---------------------------------------------------------------------------
CPartInfo *
CScalarSubqueryQuantified::PpartinfoDerive(CMemoryPool *,  // mp,
										   CExpressionHandle &exprhdl) const
{
	CPartInfo *ppartinfoChild = exprhdl.DerivePartitionInfo(0);
	GPOS_ASSERT(nullptr != ppartinfoChild);
	ppartinfoChild->AddRef();
	return ppartinfoChild;
}


//---------------------------------------------------------------------------
//	@function:
//		CScalarSubqueryQuantified::OsPrint
//
//	@doc:
//		Debug print
//
//---------------------------------------------------------------------------
IOstream &
CScalarSubqueryQuantified::OsPrint(IOstream &os) const
{
	os << SzId();
	os << "[";
	m_pcrs->OsPrint(os);
	os << "]";

	return os;
}


// EOF
