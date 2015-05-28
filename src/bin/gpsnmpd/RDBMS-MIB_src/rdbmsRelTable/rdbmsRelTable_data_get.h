/*
 * Note: this file originally auto-generated by mib2c using
 *       version : 1.18.2.1 $ of : mfd-data-get.m2c,v $
 *
 * $Id: rdbmsRelTable_data_get.h,v 1.1 2007/05/05 03:15:25 eggyknap Exp $
 *
 * @file rdbmsRelTable_data_get.h
 *
 * @addtogroup get
 *
 * Prototypes for get functions
 *
 * @{
 */
#ifndef RDBMSRELTABLE_DATA_GET_H
#define RDBMSRELTABLE_DATA_GET_H

#ifdef __cplusplus
extern "C" {
#endif

/* *********************************************************************
 * GET function declarations
 */

/* *********************************************************************
 * GET Table declarations
 */
/**********************************************************************
 **********************************************************************
 ***
 *** Table rdbmsRelTable
 ***
 **********************************************************************
 **********************************************************************/
/*
 * rdbmsRelTable is subid 9 of rdbmsObjects.
 * Its status is Current.
 * OID: .1.3.6.1.2.1.39.1.9, length: 9
*/
    /*
     * indexes
     */
    int rdbmsDbIndex_map(long *mib_rdbmsDbIndex_val_ptr, long raw_rdbmsDbIndex_val);
    int applIndex_map(long *mib_applIndex_val_ptr, long raw_applIndex_val);

    int rdbmsRelState_map(u_long *mib_rdbmsRelState_val_ptr, u_long raw_rdbmsRelState_val);
    int rdbmsRelState_get( rdbmsRelTable_rowreq_ctx *rowreq_ctx, u_long * rdbmsRelState_val_ptr );
    int rdbmsRelActiveTime_map(char **mib_rdbmsRelActiveTime_val_ptr_ptr, size_t *mib_rdbmsRelActiveTime_val_ptr_len_ptr, char *raw_rdbmsRelActiveTime_val_ptr, size_t raw_rdbmsRelActiveTime_val_ptr_len, int allow_realloc);
    int rdbmsRelActiveTime_get( rdbmsRelTable_rowreq_ctx *rowreq_ctx, char **rdbmsRelActiveTime_val_ptr_ptr, size_t *rdbmsRelActiveTime_val_ptr_len_ptr );


int rdbmsRelTable_indexes_set_tbl_idx(rdbmsRelTable_mib_index *tbl_idx, long rdbmsDbIndex_val, long applIndex_val);
int rdbmsRelTable_indexes_set(rdbmsRelTable_rowreq_ctx *rowreq_ctx, long rdbmsDbIndex_val, long applIndex_val);




#ifdef __cplusplus
}
#endif

#endif /* RDBMSRELTABLE_DATA_GET_H */
/** @} */
