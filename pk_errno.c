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
    /*  0:PK_SUCCESS                  */  "No error",
    /*  1:RFM2G_TIMED_OUT             */  "A wait timed out",
    /*  2:RFM2G_OS_ERROR              */  "Function failed for other OS defined error",
    /*  3:RFM2G_BAD_PARAMETER_1       */  "Function Parameter 1 in RFM2g API call is either NULL or invalid",
    /*  4:RFM2G_BAD_PARAMETER_2       */  "Function Parameter 2 in RFM2g API call is either NULL or invalid",
    /*  5:RFM2G_BAD_PARAMETER_3       */  "Function Parameter 3 in RFM2g API call is either NULL or invalid",
    /*  6:RFM2G_BAD_PARAMETER_4       */  "Function Parameter 4 in RFM2g API call is either NULL or invalid",
    /*  7:RFM2G_BAD_PARAMETER_5       */  "Function Parameter 5 in RFM2g API call is either NULL or invalid",
    /*  8:RFM2G_BAD_PARAMETER_6       */  "Function Parameter 6 in RFM2g API call is either NULL or invalid",
    /*  9:RFM2G_BAD_PARAMETER_7       */  "Function Parameter 7 in RFM2g API call is either NULL or invalid",
    /* 10:RFM2G_BAD_PARAMETER_8       */  "Function Parameter 8 in RFM2g API call is either NULL or invalid",
    /* 11:RFM2G_MAX_ERROR_CODE        */  "Invalid error code"
    };

const int pk_nerr = sizeof(pk_errlist) / sizeof(char*);

#ifdef __cplusplus
};
#endif /* __cplusplus */
