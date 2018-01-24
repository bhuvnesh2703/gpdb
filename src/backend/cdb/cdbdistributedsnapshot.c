/*-------------------------------------------------------------------------
 *
 * cdbdistributedsnapshot.c
 *
 * Copyright (c) 2007-2008, Greenplum inc
 *
 *-------------------------------------------------------------------------
 */
#include "postgres.h"
#include "cdb/cdbdistributedsnapshot.h"
#include "cdb/cdblocaldistribxact.h"
#include "access/distributedlog.h"
#include "miscadmin.h"
#include "access/transam.h"
#include "cdb/cdbvars.h"
#include "utils/tqual.h"

/*
 * Purpose of this function is on pretty same lines as
 * HeapTupleSatisfiesVacuum() just more from distributed perspective.
 *
 * Helps to determine the status of tuples for VACUUM, PagePruning and
 * FREEZING purposes. Here, what we mainly want to know is:
 * - if a tuple is potentially visible to *any* running transaction GLOBALLY
 * in cluster. If so, it can't be removed yet by VACUUM.
 * - also, if a tuple is visible to *all* current and future transactions,
 *   then it can be freezed by VACUUM.
 *
 * xminAllDistributedSnapshots is a cutoff XID (obtained from distributed
 * snapshot). Tuples deleted by dxids >= xminAllDistributedSnapshots are
 * deemed "recently dead"; they might still be visible to some open
 * transaction globally, so we can't remove them, even if we see that the
 * deleting transaction has committed and even if locally its lower than
 * OldestXmin.
 *
 * Function is coded with conservative mind-set, to make sure tuples are
 * deleted or freezed only if can be evaluated and guaranteed to be known
 * meeting above mentioned criteria. So, any scenarios in which global
 * snapshot can't be checked it returns to not do anything to the tuple. For
 * example running vacuum in utility mode for particular QE directly, in which
 * case don't have distributed snapshot to check against, it will not allow
 * marking tuples DEAD just based on local information.
 */
bool
localXidSatisfiesAnyDistributedSnapshot(TransactionId localXid)
{
	DistributedSnapshotCommitted distributedSnapshotCommitted;
	Assert(TransactionIdIsNormal(localXid));

	/*
	 * In general expect this function to be called only for normal xid, as
	 * more performant for caller to avoid the call based on
	 * TransactionIdIsNormal() check but just in case was called can safely
	 * return false.
	 */
	if (!TransactionIdIsNormal(localXid))
		return false;

	/*
	 * For single user mode operation like initdb time, let the vacuum
	 * cleanout and freeze tuples.
	 */
	if (!IsUnderPostmaster || !IsNormalProcessingMode())
		return false;

	/*
	 * If don't have snapshot, can't check the global visibility and hence
	 * return not to perform clean the tuple.
	 */
	if (NULL == SerializableSnapshot)
		return true;

	/* Only if we have distributed snapshot, evaluate against it */
	if (SerializableSnapshot->haveDistribSnapshot)
	{
		distributedSnapshotCommitted =
			DistributedSnapshotWithLocalMapping_CommittedTest(
				&SerializableSnapshot->distribSnapshotWithLocalMapping,
				localXid,
				true,
				true);

		switch (distributedSnapshotCommitted)
		{
			case DISTRIBUTEDSNAPSHOT_COMMITTED_INPROGRESS:
				return true;

			case DISTRIBUTEDSNAPSHOT_COMMITTED_IGNORE:
				return false;

			default:
				elog(ERROR,
					 "unrecognized distributed committed test result: %d for localXid %u",
					 (int) distributedSnapshotCommitted, localXid);
				break;
		}
	}

	/*
	 * If don't have distributed snapshot to check, return it can be seen and
	 * hence not to be cleaned-up.
	 */
	return true;
}

/*
 * DistributedSnapshotWithLocalMapping_CommittedTest
 *		Is the given XID still-in-progress according to the
 *      distributed snapshot?  Or, is the transaction strictly local
 *      and needs to be tested with the local snapshot?
 */
DistributedSnapshotCommitted 
DistributedSnapshotWithLocalMapping_CommittedTest(
	DistributedSnapshotWithLocalMapping		*dslm,
	TransactionId 							localXid,
	bool									isXmax,
	bool isVacuumCheck)
{
	DistributedSnapshotHeader *header = &dslm->header;
	DistributedSnapshotMapEntry *inProgressEntryArray = dslm->inProgressEntryArray;
	
	int32							count;
	uint32							i;
	bool							found;
	DistributedTransactionId		distribXid = InvalidDistributedTransactionId;

	count = header->count;

	/*
	 * Checking the distributed committed log can be expensive, so
	 * make a scan through our distributed snapshot looking for a
	 * possible corresponding local xid...
	 */
	for (i = 0; i < count; i++)
	{
		if (localXid == inProgressEntryArray[i].localXid)
		{
			WATCH_VISIBILITY_ADDPAIR(
				WATCH_VISIBILITY_XMIN_DISTRIBUTED_SNAPSHOT_IN_PROGRESS_FOUND_BY_LOCAL, isXmax);

			return DISTRIBUTEDSNAPSHOT_COMMITTED_INPROGRESS;
		}
	}

	/*
	 * Is this local xid in a process-local cache we maintain?
	 */
	found = LocalDistribXactCache_CommittedFind(
											localXid, 
											dslm->header.distribTransactionTimeStamp,
											&distribXid);

	if (found)
	{
		/*
		 * We cache local-only committed transactions for better
		 * performance, too.
		 */
		if (distribXid == InvalidDistributedTransactionId)
		{
			WATCH_VISIBILITY_ADDPAIR(
							WATCH_VISIBILITY_XMIN_LOCAL_DISTRIBUTED_CACHE_RETURNED_LOCAL, isXmax);

			return DISTRIBUTEDSNAPSHOT_COMMITTED_IGNORE;
		}

		// Fall below and evaluate the committed distributed transaction against
		// the distributed snapshot.
		WATCH_VISIBILITY_ADDPAIR(
				WATCH_VISIBILITY_XMIN_LOCAL_DISTRIBUTED_CACHE_RETURNED_DISTRIB, isXmax);
	}
	else
	{
		/*
		 * Small window -- but check if our LocalDistribXact element says
		 * we are preparing this transaction.  If so, the CLOG knows the
		 * commit and had the visibility routine call us before we've finished
		 * updating our data structures.
		 */
		if (!LocalDistribXact_LocalXidKnown(
										localXid,
										dslm->header.distribTransactionTimeStamp,
										&distribXid))
		{
			DistributedTransactionTimeStamp checkDistribTimeStamp;
			
			WATCH_VISIBILITY_ADDPAIR(
							WATCH_VISIBILITY_XMIN_NOT_KNOWN_BY_LOCAL_DISTRIBUTED_XACT, isXmax);

			/*
			 * Ok, now we must consult the distributed log.
			 */
			if (DistributedLog_CommittedCheck(
									localXid,
									&checkDistribTimeStamp,
									&distribXid))
			{
				/*
				 * We found it in the distributed log.
				 */
				Assert(checkDistribTimeStamp != 0);
				Assert(distribXid != InvalidDistributedTransactionId);

				/*
				 * Committed distributed transactions from other DTM starts are
				 * weeded out.
				 */
				if (checkDistribTimeStamp != header->distribTransactionTimeStamp)
				{
					WATCH_VISIBILITY_ADDPAIR( 
							WATCH_VISIBILITY_XMIN_DIFF_DTM_START_IN_DISTRIBUTED_LOG, isXmax);
									
					return DISTRIBUTEDSNAPSHOT_COMMITTED_IGNORE;
				}
				else
					WATCH_VISIBILITY_ADDPAIR(
							WATCH_VISIBILITY_XMIN_FOUND_IN_DISTRIBUTED_LOG, isXmax);
			}
			else
			{
				/*
				 * Since the local xid is committed (as determined by the
				 * visibility routine) and all of our data structures do not
				 * know of the transaction, it must be local-only.
				 */
				LocalDistribXactCache_AddCommitted(
												localXid, 
												dslm->header.distribTransactionTimeStamp,
												/* distribXid */ InvalidDistributedTransactionId);
				
				WATCH_VISIBILITY_ADDPAIR(
						WATCH_VISIBILITY_XMIN_KNOWN_LOCAL_IN_DISTRIBUTED_LOG, isXmax);

				return DISTRIBUTEDSNAPSHOT_COMMITTED_IGNORE;
			}
		}
		else
			WATCH_VISIBILITY_ADDPAIR(
					WATCH_VISIBILITY_XMIN_KNOWN_BY_LOCAL_DISTRIBUTED_XACT, isXmax);
	}
	
	/*
	 * We have a distributed committed xid that corresponds to the local xid.
	 */
	Assert(distribXid != InvalidDistributedTransactionId);

	/*
	 * If we did not find it in our cache, add it.
	 */
	if (!found)
		LocalDistribXactCache_AddCommitted(
										localXid, 
										dslm->header.distribTransactionTimeStamp,
										distribXid);

	Assert(header->xminAllDistributedSnapshots != InvalidDistributedTransactionId);
	/*
	 * If this distributed transaction is older than all the distributed
	 * snapshots, then we can ignore it from now on.
	 */
	Assert(header->xmin >= header->xminAllDistributedSnapshots);
		
	if (distribXid < dslm->header.xminAllDistributedSnapshots)
	{
		WATCH_VISIBILITY_ADDPAIR(
			WATCH_VISIBILITY_XMIN_LESS_THAN_ALL_CURRENT_DISTRIBUTED, isXmax);
		return DISTRIBUTEDSNAPSHOT_COMMITTED_IGNORE;
	}

	/*
	 * If called to check for purpose of vacuum, in-progress is not
	 * interesting to check and hence just return.
	 */
	if (isVacuumCheck)
		return DISTRIBUTEDSNAPSHOT_COMMITTED_INPROGRESS;

	/* Any xid < xmin is not in-progress */
	if (distribXid < header->xmin)
	{
		WATCH_VISIBILITY_ADDPAIR(
				WATCH_VISIBILITY_XMIN_LESS_THAN_DISTRIBUTED_SNAPSHOT_XMIN, isXmax);

		return DISTRIBUTEDSNAPSHOT_COMMITTED_VISIBLE;
	}

	/* Any xid >= xmax is in-progress, distributed xmax points to the
	 * committer, so it must be visible, so ">" instead of ">=" */
	if (distribXid > header->xmax)
	{
		elog((Debug_print_snapshot_dtm ? LOG : DEBUG5),
			 "distributedsnapshot committed but invisible: distribXid %d header->xmax %d header->xmin %d header->distribSnapshotId %d",
			 distribXid, header->xmax, header->xmin, header->distribSnapshotId);
		WATCH_VISIBILITY_ADDPAIR( 
						WATCH_VISIBILITY_XMIN_GREATER_THAN_EQUAL_DISTRIBUTED_SNAPSHOT_XMAX, isXmax);

		return DISTRIBUTEDSNAPSHOT_COMMITTED_INPROGRESS;
	}

	for (i = 0; i < count; i++)
	{
		if (distribXid == inProgressEntryArray[i].distribXid)
		{
			/*
			 * Save the relationship to the local xid so we may avoid
			 * checking the distributed committed log in a subsequent check.
			 */
			if (inProgressEntryArray[i].localXid == InvalidTransactionId)
				inProgressEntryArray[i].localXid = localXid;
			
			WATCH_VISIBILITY_ADDPAIR(
							WATCH_VISIBILITY_XMIN_DISTRIBUTED_SNAPSHOT_IN_PROGRESS_BY_DISTRIB, isXmax);

			return DISTRIBUTEDSNAPSHOT_COMMITTED_INPROGRESS;
		}
	}

	/*
	 * Not in-progress, therefore visible.
	 */
	WATCH_VISIBILITY_ADDPAIR(
					WATCH_VISIBILITY_XMIN_DISTRIBUTED_SNAPSHOT_NOT_IN_PROGRESS, isXmax);

	return DISTRIBUTEDSNAPSHOT_COMMITTED_VISIBLE;
}

/*
 * Reset all fields except header.maxCount and the malloc'd pointer
 * for inProgressXidArray.
 */
void
DistributedSnapshot_Reset(DistributedSnapshot *distributedSnapshot)
{
	DistributedSnapshotHeader *header = &distributedSnapshot->header;
	
	header->distribTransactionTimeStamp = 0;
	header->xminAllDistributedSnapshots = InvalidDistributedTransactionId;		
	header->distribSnapshotId = 0;		
	header->xmin = InvalidDistributedTransactionId;		
	header->xmax = InvalidDistributedTransactionId;		
	header->count = 0;	
	// Leave maxCount alone
	// Leave inProgressXidArray alone.
}

/*
 * Make a copy of a DistributedSnapshot, allocating memory for the in-progress
 * array if necessary.
 */
void
DistributedSnapshot_Copy(
	DistributedSnapshot *target,
	DistributedSnapshot *source)
{
	if (source->header.maxCount <= 0 ||
	    source->header.count > source->header.maxCount)
		elog(ERROR,"Invalid distributed snapshot (maxCount %d, count %d)",
		     source->header.maxCount, source->header.count);

	DistributedSnapshot_Reset(target);

	elog((Debug_print_full_dtm ? LOG : DEBUG5),
		 "DistributedSnapshot_Copy target maxCount %d, inProgressXidArray %p, and "
		 "source maxCount %d, count %d, inProgressXidArray %p", 
	 	 target->header.maxCount,
	 	 target->inProgressXidArray,
	 	 source->header.maxCount,
		 source->header.count,
		 source->inProgressXidArray);

	/*
	 * If we have allocated space for the in-progress distributed
	 * transactions, check against that space.  Otherwise,
	 * use the source maxCount as guide in allocating space.
	 */
	if (target->header.maxCount > 0)
	{
		Assert(target->inProgressXidArray != NULL);
		
		if(source->header.count > target->header.maxCount)
			elog(ERROR,"Too many distributed transactions for snapshot (maxCount %d, count %d)",
			     target->header.maxCount, source->header.count);
	}
	else
	{
		Assert(target->inProgressXidArray == NULL);
		
		target->inProgressXidArray = 
			(DistributedTransactionId*)
					malloc(source->header.maxCount * sizeof(DistributedTransactionId));
		if (target->inProgressXidArray == NULL)
			ereport(ERROR,
					(errcode(ERRCODE_OUT_OF_MEMORY),
					 errmsg("out of memory")));
		target->header.maxCount = source->header.maxCount;
	}

	target->header.distribTransactionTimeStamp = source->header.distribTransactionTimeStamp;
	target->header.xminAllDistributedSnapshots = source->header.xminAllDistributedSnapshots;
	target->header.distribSnapshotId = source->header.distribSnapshotId;

	target->header.xmin = source->header.xmin;
	target->header.xmax = source->header.xmax;
	target->header.count = source->header.count;

	memcpy(
		target->inProgressXidArray, 
		source->inProgressXidArray, 
		source->header.count * sizeof(DistributedTransactionId));
}

char* 
DistributedSnapshotCommittedToString(
	DistributedSnapshotCommitted distributedSnapshotCommitted)
{
	switch (distributedSnapshotCommitted)
	{
	case DISTRIBUTEDSNAPSHOT_COMMITTED_INPROGRESS:
		return "In-Progress";
		
	case DISTRIBUTEDSNAPSHOT_COMMITTED_VISIBLE:
		return "Visible";

	case DISTRIBUTEDSNAPSHOT_COMMITTED_IGNORE:
		return "Ignore";

	default:
		return "Unknown";
	}
}
