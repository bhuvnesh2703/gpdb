//---------------------------------------------------------------------------
//	Greenplum Database
//	Copyright (C) 2012 EMC, Corp.
//
//	@filename:
//		CDXLScalarSubqueryQuantified.cpp
//
//	@doc:
//		Implementation of quantified subquery operator
//---------------------------------------------------------------------------

#include "naucrates/dxl/operators/CDXLScalarSubqueryQuantified.h"

#include "gpos/string/CWStringDynamic.h"

#include "naucrates/dxl/operators/CDXLNode.h"
#include "naucrates/dxl/xml/CXMLSerializer.h"

#include "naucrates/dxl/CDXLUtils.h"

using namespace gpos;
using namespace gpdxl;
using namespace gpmd;

//---------------------------------------------------------------------------
//	@function:
//		CDXLScalarSubqueryQuantified::CDXLScalarSubqueryQuantified
//
//	@doc:
//		Constructor
//
//---------------------------------------------------------------------------
CDXLScalarSubqueryQuantified::CDXLScalarSubqueryQuantified(
	CMemoryPool *mp, IMDId *scalar_op_mdid, CMDName *scalar_op_mdname,
	ULONG colid)
	: CDXLScalar(mp),
	  m_scalar_op_mdid(scalar_op_mdid),
	  m_scalar_op_mdname(scalar_op_mdname),
	  m_colid(colid)
{
	m_in_clause_colids = nullptr;
	GPOS_ASSERT(scalar_op_mdid->IsValid());
	GPOS_ASSERT(nullptr != scalar_op_mdname);
}

CDXLScalarSubqueryQuantified::CDXLScalarSubqueryQuantified(
		CMemoryPool *mp,
		ULongPtrArray *in_clause_colids)
		: CDXLScalar(mp),
		  m_in_clause_colids(in_clause_colids)
{
	m_scalar_op_mdid = nullptr;
	m_scalar_op_mdname= nullptr;
	m_colid = 0;
}

//---------------------------------------------------------------------------
//	@function:
//		CDXLScalarSubqueryQuantified::~CDXLScalarSubqueryQuantified
//
//	@doc:
//		Destructor
//
//---------------------------------------------------------------------------
CDXLScalarSubqueryQuantified::~CDXLScalarSubqueryQuantified()
{
	CRefCount::SafeRelease(m_scalar_op_mdid);
	if (m_scalar_op_mdname != nullptr)
	{
		GPOS_DELETE(m_scalar_op_mdname);
	}
	CRefCount::SafeRelease(m_in_clause_colids);

}

//---------------------------------------------------------------------------
//	@function:
//		CDXLScalarSubqueryQuantified::SerializeToDXL
//
//	@doc:
//		Serialize operator in DXL format
//
//---------------------------------------------------------------------------
void
CDXLScalarSubqueryQuantified::SerializeToDXL(CXMLSerializer *xml_serializer,
											 const CDXLNode *dxlnode) const
{
	const CWStringConst *element_name = GetOpNameStr();
	xml_serializer->OpenElement(
		CDXLTokens::GetDXLTokenStr(EdxltokenNamespacePrefix), element_name);

	if (m_scalar_op_mdname != nullptr)
	{
		// serialize operator id and name
		xml_serializer->AddAttribute(CDXLTokens::GetDXLTokenStr(EdxltokenOpName),
									 m_scalar_op_mdname->GetMDName());
	}

	if (m_scalar_op_mdid != nullptr)
	{
		m_scalar_op_mdid->Serialize(xml_serializer,
									CDXLTokens::GetDXLTokenStr(EdxltokenOpNo));
	}

	if (m_colid != 0)
	{
		// serialize computed column id
		xml_serializer->AddAttribute(CDXLTokens::GetDXLTokenStr(EdxltokenColId),
									 m_colid);
	}

	if (m_in_clause_colids != nullptr)
	{
		CWStringDynamic *str_in_colids =
				CDXLUtils::Serialize(m_mp, m_in_clause_colids);
		xml_serializer->AddAttribute(CDXLTokens::GetDXLTokenStr(EdxltokenColId),
									 str_in_colids);
		GPOS_DELETE(str_in_colids);
	}

	dxlnode->SerializeChildrenToDXL(xml_serializer);
	xml_serializer->CloseElement(
		CDXLTokens::GetDXLTokenStr(EdxltokenNamespacePrefix), element_name);
}

#ifdef GPOS_DEBUG
//---------------------------------------------------------------------------
//	@function:
//		CDXLScalarSubqueryQuantified::AssertValid
//
//	@doc:
//		Checks whether operator node is well-structured
//
//---------------------------------------------------------------------------
void
CDXLScalarSubqueryQuantified::AssertValid(const CDXLNode *dxlnode,
										  BOOL validate_children) const
{
	GPOS_ASSERT(2 == dxlnode->Arity());

	CDXLNode *pdxlnScalarChild = (*dxlnode)[EdxlsqquantifiedIndexScalar];
	CDXLNode *pdxlnRelationalChild =
		(*dxlnode)[EdxlsqquantifiedIndexRelational];

	GPOS_ASSERT(EdxloptypeScalar ==
				pdxlnScalarChild->GetOperator()->GetDXLOperatorType());
	GPOS_ASSERT(EdxloptypeLogical ==
				pdxlnRelationalChild->GetOperator()->GetDXLOperatorType());

	dxlnode->AssertValid(validate_children);
}
#endif	// GPOS_DEBUG

// EOF
