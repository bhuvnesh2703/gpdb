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
	GPOS_ASSERT(scalar_op_mdid->IsValid());
	GPOS_ASSERT(nullptr != scalar_op_mdname);
}

CDXLScalarSubqueryQuantified::CDXLScalarSubqueryQuantified(
	CMemoryPool *mp,
	ULongPtrArray *colrefs)
	: CDXLScalar(mp),
	  m_colrefs(colrefs)
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
		GPOS_DELETE(m_scalar_op_mdname);
	if (m_colrefs != nullptr)
		CRefCount::SafeRelease(m_colrefs);
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

	if (this->GetDXLOperator() == EdxlopScalarSubqueryAll)
	{
		// serialize operator id and name
		xml_serializer->AddAttribute(CDXLTokens::GetDXLTokenStr(EdxltokenOpName),
									 m_scalar_op_mdname->GetMDName());
		m_scalar_op_mdid->Serialize(xml_serializer,
									CDXLTokens::GetDXLTokenStr(EdxltokenOpNo));
	}

	// serialize computed column id
	xml_serializer->AddAttribute(CDXLTokens::GetDXLTokenStr(EdxltokenColId),
								 m_colid);

	// serialize distribution columns
	CWStringDynamic *str_colref_ids =
			CDXLUtils::Serialize(m_mp, m_colrefs);

	xml_serializer->AddAttribute(
			CDXLTokens::GetDXLTokenStr(EdxltokenColId),
			str_colref_ids);
	GPOS_DELETE(str_colref_ids);

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
