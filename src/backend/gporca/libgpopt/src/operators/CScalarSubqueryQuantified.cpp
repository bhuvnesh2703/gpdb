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
	CMemoryPool *mp, IMDId *scalar_op_mdid, const CWStringConst *pstrScalarOp,
	const CColRef *colref)
	: CScalar(mp),
	  m_scalar_op_mdid(scalar_op_mdid),
	  m_pstrScalarOp(pstrScalarOp),
	  m_pcr(colref)
{
	GPOS_ASSERT(scalar_op_mdid->IsValid());
	GPOS_ASSERT(nullptr != pstrScalarOp);
	GPOS_ASSERT(nullptr != colref);
}

CScalarSubqueryQuantified::CScalarSubqueryQuantified(
	CMemoryPool *mp, CColRefSet *pcrsSubquery)
	: CScalar(mp),
	  m_pcrSubquery(pcrsSubquery)
{
	m_scalar_op_mdid = nullptr;
	m_pstrScalarOp = nullptr;
	m_pcr = nullptr;

	GPOS_ASSERT(nullptr != pcrsSubquery);
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
	CRefCount::SafeRelease(m_scalar_op_mdid);
	if (m_pstrScalarOp != nullptr)
		GPOS_DELETE(m_pstrScalarOp);
	CRefCount::SafeRelease(m_pcrSubquery);
}

//---------------------------------------------------------------------------
//	@function:
//		CScalarSubqueryQuantified::PstrOp
//
//	@doc:
//		Operator name
//
//---------------------------------------------------------------------------
const CWStringConst *
CScalarSubqueryQuantified::PstrOp() const
{
	return m_pstrScalarOp;
}

//---------------------------------------------------------------------------
//	@function:
//		CScalarSubqueryQuantified::MdIdOp
//
//	@doc:
//		Scalar operator metadata id
//
//---------------------------------------------------------------------------
IMDId *
CScalarSubqueryQuantified::MdIdOp() const
{
	return m_scalar_op_mdid;
}

//---------------------------------------------------------------------------
//	@function:
//		CScalarSubqueryQuantified::MdidType
//
//	@doc:
//		Type of scalar's value
//
//---------------------------------------------------------------------------
IMDId *
CScalarSubqueryQuantified::MdidType() const
{
	CMDAccessor *md_accessor = COptCtxt::PoctxtFromTLS()->Pmda();
	IMDId *mdid_type =
		md_accessor->RetrieveScOp(m_scalar_op_mdid)->GetResultTypeMdid();

	GPOS_ASSERT(
		md_accessor->PtMDType<IMDTypeBool>()->MDId()->Equals(mdid_type));

	return mdid_type;
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
	if (m_scalar_op_mdid != nullptr)
		return gpos::CombineHashes(
			COperator::HashValue(),
			gpos::CombineHashes(m_scalar_op_mdid->HashValue(),
								gpos::HashPtr<CColRef>(m_pcr)));
	return gpos::CombineHashes(
		COperator::HashValue(),
		gpos::HashPtr<CColRef>(m_pcr));
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
	if (m_scalar_op_mdid != nullptr)
		return popSsq->Pcr() == m_pcr && popSsq->MdIdOp()->Equals(m_scalar_op_mdid);
	return popSsq->Pcr() == m_pcr;
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
	CColRefSet *pcrsSubquery = GPOS_NEW(mp) CColRefSet(mp, *m_pcrSubquery);
	pcrsSubquery->Exclude(pcrsChildOutput);
	if (pcrsSubquery->Size() >0 )
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
	if (PstrOp())
		os << "(" << PstrOp()->GetBuffer() << ")";
	os << "[";
	if (m_pcr != nullptr)
		m_pcr->OsPrint(os);
	os << "]";

	return os;
}


// EOF
