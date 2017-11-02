//---------------------------------------------------------------------------
//	Greenplum Database
//	Copyright (C) 2012 Greenplum, Inc.
//
//	@filename:
//		CGPOptimizer.h
//
//	@doc:
//		Entry point to GP optimizer
//
//	@owner:
//		solimm1
//
//	@test:
//
//
//---------------------------------------------------------------------------
#ifndef CGPOptimizer_H
#define CGPOptimizer_H

#include "postgres.h"
#include "nodes/params.h"
#include "nodes/plannodes.h"
#include "nodes/parsenodes.h"

class CGPOptimizer
{
	private:

		// touch library initializers to enforce linker to include them
		static
		void TouchLibraryInitializers();

	public:

		// optimize given query using GP optimizer
		static
		PlannedStmt *PplstmtOptimize
			(
			Query *pquery,
			bool *pfUnexpectedFailure // output : set to true if optimizer unexpectedly failed to produce plan
			);

		// serialize planned statement into DXL
		static
		char *SzDXLPlan(Query *pquery);

		// gpopt initialize and terminate
		static
		void InitGPOPT();

		static
		void TerminateGPOPT();

		// Signal handler for ORCA
		static
		void SignalInterruptGPOPT(int iSignal);

		// Reset optimizer state to before SignalInterruptGPOPT() was called.
		// To be called after interrupts have been handled by ORCA.
		static
		void ResetInterruptsGPOPT();

};

#endif // CGPOptimizer_H
