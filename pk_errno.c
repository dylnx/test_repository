/*=============================================================================
                            COPYRIGHT NOTICE

-------------------------------------------------------------------------------

    $Workfile: pk_errno.c $
    $Revision: v1.0 $ 
    $Modtime: 2/17/2017 12:50 $

-------------------------------------------------------------------------------
    Description: ParkingLot API Fuc Error Codes
-------------------------------------------------------------------------------

==============================================================================*/


#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

const char *pk_errlist[] =
    {
    /*   0:PK_SUCCESS               */  "No error",
    /*   1:PK_TIMED_OUT             */  "A wait timed out",
    /*   2:PK_OS_ERROR              */  "Function failed for other OS defined error",
    /*   3:PK_FILE_ERROR            */  "File error",
    /*   4:PK_FILE_EMPTY            */  "File empty",
    /*   5:PK_BAD_PARAMETER_1       */  "Function Parameter 1 is either NULL or invalid",
    /*   6:PK_BAD_PARAMETER_2       */  "Function Parameter 2 is either NULL or invalid",
    /*   7:PK_BAD_PARAMETER_3       */  "Function Parameter 3 is either NULL or invalid",
    /*   8:PK_BAD_PARAMETER_4       */  "Function Parameter 4 is either NULL or invalid",
    /*   9:PK_BAD_PARAMETER_5       */  "Function Parameter 5 is either NULL or invalid",
    /*  10:PK_BAD_PARAMETER_6       */  "Function Parameter 6 is either NULL or invalid",
    /*  11:PK_BAD_PARAMETER_7       */  "Function Parameter 7 is either NULL or invalid",
    /*  12:PK_BAD_PARAMETER_8       */  "Function Parameter 8 is either NULL or invalid",
    /*  13:PK_SEND_ERROR            */  "send operation failed",
    /*  14:PK_MAX_ERROR_CODE        */  "Invalid error code"
    };

const int pk_nerr = sizeof(pk_errlist) / sizeof(char*);

#ifdef __cplusplus
};
#endif /* __cplusplus */
