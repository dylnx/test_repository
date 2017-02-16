/*
===============================================================================
                            COPYRIGHT NOTICE

-------------------------------------------------------------------------------

    $Workfile: pk_errno.h $
    $Revision: v1.0 $ 
    $Modtime: 2/17/2017 12:27p $

-------------------------------------------------------------------------------
    Description: ParkingLot API Fuc Error Codes
-------------------------------------------------------------------------------

==============================================================================*/

#ifndef PK_ERRNO_H
#define PK_ERRON_H

#ifdef __cplusplus
extern "C"
#endif /*__cplusplus*/

/*
*=======================================================================
* ParkingLot Error Codes
*-----------------------------------------------------------------------
*=======================================================================
*/
typedef enum PK_STATUS_enum
    {
    PK_SUCCESS,		 /* No error			 */
    Pk_TIME_OUT,         /* A wait timed out		 */ 
    PK_OS_ERROR,	 /* OS define error		 */
    PK_FILE_ERROR,       /* File error                   */
    PK_FILE_EMPTY,       /* File empty                   */
    PK_BAD_PARAMETER_1,	 /* Function Parameter 1 invalid */
    PK_BAD_PARAMETER_2,  /* Function Parameter 2 invalid */
    PK_BAD_PARAMETER_3,  /* Function Parameter 3 invalid */
    PK_BAD_PARAMETER_4,  /* Function Parameter 4 invalid */
    PK_BAD_PARAMETER_5,  /* Function Parameter 5 invalid */
    PK_BAD_PARAMETER_6,  /* Function Parameter 6 invalid */
    PK_BAD_PARAMETER_7,  /* Function Parameter 7 invalid */
    PK_BAD_PARAMETER_8,  /* Function Parameter 8 invalid */
    PK_SEND_ERROR,       /* send operation failed        */
    PK_MAX_ERROR_CODE    /* Invalid error code           */
    }PK_STATUS;

extern const int pk_nerr;
extern const char *pk_errlist[];

#ifdef __cplusplus
};
#endif /* __cplusplus */

#endif /* PK_ERRON_H */

