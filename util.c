
/*****************************************************************************
*
*  FUNCTION:   PKErrorMsg
*
*  PURPOSE:    Returns a pointer to a text string describing the ErrorCode
*  PARAMETERS: ErrorCode (I): Return code from a function
*  RETURNS:    Pointer to a text string describing the ErrorCode
*  GLOBALS:    pk_errstring
*
******************************************************************************/

#include "pk_errno.h"


char * PKErrorMsg(PK_STATUS ErrorCode)
{
    /* Validate the ErrorCode which should be zero or negative */
    if( (ErrorCode < 0)  ||  (ErrorCode > PK_MAX_ERROR_CODE ))
    {
        ErrorCode = PK_MAX_ERROR_CODE;
    }

    return( (char*) pk_errlist[ ErrorCode ] );

}  /* end of PKErrorMsg() */

