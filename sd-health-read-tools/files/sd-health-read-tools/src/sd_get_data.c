
#include <assert.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <scsi/sg_lib.h>
#include <scsi/sg_pt.h>
#include <libudev.h>

#include "sd_get_data.h"

#define SCSI_PASS_THROUGH_RETRY
#define SWAP_ENDIAN(x) (((x & 0xFF000000) >> 24) | ((x & 0x00FF0000) >> 8) | ((x & 0x0000FF00) << 8) | ((x & 0x000000FF) << 24)) 
#define SD_BLOCK_SIZE	512

/*
#define SD_HEALTH_STATUS_DEBUG  1
#define SD_INQUIRY_DEBUG  1
#define SD_CID_DEBUG 1
#define SD_STATUS_DEBUG 1 
#define SD_HEALTH_STATUS_CDB_DEBUG 1
#define SCSI_INQUIRY_DEBUG 1
*/

static int SCSIPassThroughDirect(int hFile, 
				void *pCDB, 
				uint8_t nCDBLength,
				__attribute__ ((unused)) SCSI_PASS_THROUGH_STATUS *sptsStatus, 
				void *pBuffer,
				uint32_t dwBufferLength, 
				int bWrite, int DEBUG_SCSI, int VERBOSE_SCSI)
{
	uint8_t sense_buffer[32];
	struct sg_pt_base *ptvp;
	int result, resid, category, sense_len;
	char buffer[512];
	int status = 0;

	assert(dwBufferLength != 1);

	memset(sense_buffer, 0, sizeof(sense_buffer));
	memset(buffer, 0, sizeof(buffer));


	/* construct a SCSI pass through object */
	ptvp = construct_scsi_pt_obj();
	/* set our desired CDB */
	set_scsi_pt_cdb(ptvp, pCDB, nCDBLength);
	/* set the sense return buffer */
	set_scsi_pt_sense(ptvp, sense_buffer, sizeof(sense_buffer));

	/* set the data in/out/no-data for this CDB */
	if (dwBufferLength == 0) {
		/* default is no data */
		;
	} else if (bWrite == TRUE) {
		set_scsi_pt_data_out(ptvp, pBuffer, dwBufferLength);
	} else {
		set_scsi_pt_data_in(ptvp, pBuffer, dwBufferLength);
	}

	/* send the pass through command */
	result = do_scsi_pt(ptvp, hFile, SCSI_PASS_THROUGH_TIMEOUT, VERBOSE_SCSI);
	if (result < 0) {
		printf("do_scsi_pt result %d\n", result);
	} 
	else if (result == SCSI_PT_DO_BAD_PARAMS) {
		printf("SCSI_PT_DO_BAD_PARAMS\n");
	} 
	else if (result == SCSI_PT_DO_TIMEOUT) {
		printf("SCSI_PT_DO_TIMEOUT\n");
	}

	resid = get_scsi_pt_resid(ptvp);
	category = get_scsi_pt_result_category(ptvp);
	switch (category) {
	case SCSI_PT_RESULT_GOOD:
		/* bytes returned is dwBufferLength - resid */
		if (DEBUG_SCSI) {
			printf("DEBUG: buffer size %d, got %d\n",
			       dwBufferLength, dwBufferLength - resid);
		}
		break;
	case SCSI_PT_RESULT_STATUS:
		/* get result string */
		sg_get_scsi_status_str(get_scsi_pt_status_response(ptvp),
				       sizeof(buffer), buffer);
		printf("SCSI_PT_RESULT_STATUS: scsi status: %s\n", buffer);
		destruct_scsi_pt_obj(ptvp);
		status = -1;
		return status;
		break;
	case SCSI_PT_RESULT_SENSE:
		/* handle sense code */
		sense_len = get_scsi_pt_sense_len(ptvp);
		sg_get_sense_str("", sense_buffer, sense_len, (VERBOSE_SCSI > 1),
				 sizeof(buffer), buffer);
		printf("SCSI_PT_RESULT_SENSE: sense %s", buffer);
		destruct_scsi_pt_obj(ptvp);
		status = -1;
		return status;
		break;
	case SCSI_PT_RESULT_TRANSPORT_ERR:
		get_scsi_pt_transport_err_str(ptvp, sizeof(buffer), buffer);
		printf("SCSI_PT_RESULT_TRANSPORT_ERR: %s\n", buffer);
		destruct_scsi_pt_obj(ptvp);
		status = -1;
		return status;
		break;
	case SCSI_PT_RESULT_OS_ERR:
		get_scsi_pt_os_err_str(ptvp, sizeof(buffer), buffer);
		printf("SCSI_PT_RESULT_OS_ERR: %s\n", buffer);
		destruct_scsi_pt_obj(ptvp);
		status = -1;
		return status;
		break;
	default:
		printf("unknown result: category %d\n", category);
		destruct_scsi_pt_obj(ptvp);
		status = -1;
		return status;
		break;
	}

	destruct_scsi_pt_obj(ptvp);
	return status;
}

/**********************************************************************
DATE    : 2022. 11. 21
AUTHOR  : MIN-HYEOK KIM
MODIFIER: DAE-HYUN HAN
PARAMS  : *pstSDWhiteList	- SD card white list array value
          kiProductCnt		- SD card product count
          uiSDManuID		- SD card manufacturer ID
		  pcSDPnm			- SD card product name value
RETURN  : iReturn
COMMENTS: The function with check white list
**********************************************************************/
static int white_list_check( SD_WHITE_LIST * pstSDWhiteList, const int kiProductCnt, uint8_t uiSDManuID, char * pcSDPnm )
{
	int iReturn = -1;

	printf( "SD product count : %d\n", kiProductCnt );
	printf( "Current SD card MID : 0x%x\n", uiSDManuID );
	printf( "Current SD card PNM : %s\n", pcSDPnm );
	
	for( int iLoopCnt = 0; iLoopCnt < kiProductCnt; iLoopCnt++ )
	{
		if( uiSDManuID == pstSDWhiteList[iLoopCnt].Manufacturer_ID )
		{
			int iIsDifferent = strcmp( pstSDWhiteList[iLoopCnt].Product_Name, pcSDPnm );
			
			if( 0 != iIsDifferent )
			{
				continue;
			}
			else
			{
				iReturn = 0;
				
				return iReturn;
			}
		}
	}

	printf( "Health monitor is not supported for this sd card.\n" );
	
	return iReturn;
}

/**********************************************************************
DATE    : 2022. 11. 21
AUTHOR  : MIN-HYEOK KIM
MODIFIER: DAE-HYUN HAN
PARAMS  : *pStatusData		- SD card white list array value
          kiProductCnt		- SD card product count
RETURN  : iReturn
COMMENTS: The function with insert white list data
HISTORY : 2022. 11. 23 - Delete some sd card items( exclude i400 seriese ).
**********************************************************************/
static int sd_get_white_list( SD_WHITE_LIST * pstSDWhiteList, const int kiProductCnt )
{
	int iReturn = 0;

	SD_WHITE_LIST staTmpSDWhiteList[DEF_MAX_SD_CARD_WHITE_LIST_COUNT];
	
	// Micron Manufacturer ID - 0x09
	const uint8_t kuiManufacturerID = 0x09;
	
	// MB*** - Micron SD card i300 & i400 serise
	// i300 serise
	// MB58B - 128GB, MB68B - 256GB, MB78B - 512GB, MB98B - 1TB
	// i400 serise
	// MB4AC -  64GB, MB5BC - 128GB, MB6BC - 256GB
	// MB7BC - 512GB, MB9BC -   1TB, MBABC - 1.5TB
	char caProductName[DEF_MAX_SD_CARD_WHITE_LIST_COUNT][DEF_MAX_SD_CARD_PRODUCT_NAME_LENGTH] = { "MB58B", "MB68B", "MB78B", "MB98B", "MB4AC",
                                                                                                  "MB5BC", "MB6BC", "MB7BC", "MB9BC", "MBABC" };

	memset( staTmpSDWhiteList, 0x00, sizeof( staTmpSDWhiteList ) );

	for( int iLoopCnt = 0; iLoopCnt < kiProductCnt; iLoopCnt++ )
	{
		staTmpSDWhiteList[iLoopCnt].Manufacturer_ID = kuiManufacturerID;
		
		strcpy( staTmpSDWhiteList[iLoopCnt].Product_Name, caProductName[iLoopCnt] );
	}

	memcpy( pstSDWhiteList, staTmpSDWhiteList, sizeof( staTmpSDWhiteList ) );

	return iReturn;
}



static int hex_to_str_pnm(char *hex_str, uint8_t *hex_arr, int hex_arr_len)
{

	char hex_str_dump[sizeof(&hex_str) / sizeof(char)]="";
	for(int i = 0; i < hex_arr_len; i++){
		char buf[10]="";
		sprintf(buf,"%c",(int)hex_arr[i]);
		strcat(hex_str_dump,buf);
	} 
	strcpy(hex_str,hex_str_dump);
	return 0; 
}

/**********************************************************************
DATE    : 2022. 11. 16
AUTHOR  : MIN-HYEOK KIM
MODIFIER: DAE-HYUN HAN
PARAMS  : hFile				- USB handle file
          *pStatusData		- Status data value
          iIsDebugStatus	- Debugging value
RETURN  : iReturn
COMMENTS: The function with reading sd card status.
**********************************************************************/
int sd_get_status( int hFile, SD_STATUS_DATA * pStatusData, int iIsDebugStatus )
{
	SD_OVER_USB_CDB soucCDB;
	uint8_t aiInputData[sizeof( SD_STATUS_DATA )];
	int iReturn = 0;

	memset( &soucCDB,    0x00, sizeof(        soucCDB ) );
	memset( aiInputData, 0x00, sizeof( SD_STATUS_DATA ) );

	soucCDB.SD_STATUS.OperationCode		= SDOUSB_CDB_OPCODE;
	soucCDB.SD_STATUS.ServiceAction[0]	= HIBYTE( SDOUSB_CDB_STATUS );
	soucCDB.SD_STATUS.ServiceAction[1]	= LOBYTE( SDOUSB_CDB_STATUS );
	soucCDB.SD_STATUS.AllocationLength	= 0x02;

	memset( pStatusData, 0x00, sizeof( SD_STATUS_DATA ) );

	iReturn = SCSIPassThroughDirect( hFile, &soucCDB, sizeof( soucCDB.SD_STATUS ), NULL, aiInputData, sizeof( aiInputData ), 0, 0, 0 );
	printf( "%s::%d:iReturn is %d\n", __FUNCTION__, __LINE__, iReturn );

	if( 0 > iReturn )
	{
		printf( "sd get status failed %d\n", errno );

		return iReturn;
	}

	if( NULL != pStatusData )
	{
		memcpy( pStatusData, aiInputData, sizeof( SD_STATUS_DATA ) );

		if( 1 == iIsDebugStatus )
		{
			printf( "SD status dumping:" );

			for( int iLoopCnt = 0; iLoopCnt < sizeof( aiInputData ); iLoopCnt++ )
			{
				if( 0 == ( iLoopCnt % 2 ) )
				{
					printf( "\r\n%03Xh: ", iLoopCnt );
				}
				
				printf( "%02x ", aiInputData[iLoopCnt] );
			}	

			printf( "\r\n- Card Present Value  : %02x \r\n\n", pStatusData->CardPresent );
		}
	}
	
	return iReturn;
}

static int sd_get_cid(int hFile, SD_CID_DATA *cidData, int SD_CID_DEBUG)
{
	SD_OVER_USB_CDB soucCDB;
	uint8_t data_in[sizeof(SD_CID_DATA)];
	int status = 0;

	memset(&soucCDB, 0x00, sizeof(soucCDB));
	memset(&data_in[0], 0x00, sizeof(data_in));
	memset(cidData, 0x00, sizeof(SD_CID_DATA));

	soucCDB.SD_VENDOR.OperationCode = 0xCF;
	soucCDB.SD_VENDOR.vendorCdb.mVend_Func_Code = 0x18;
	soucCDB.SD_VENDOR.vendorCdb.mReserved3 = 16;
	soucCDB.SD_VENDOR.vendorCdb.mReserved1 = 0;
	soucCDB.SD_VENDOR.vendorCdb.mReserved2 = 0;
	soucCDB.SD_VENDOR.vendorCdb.mReserved4 = 0;


	status = SCSIPassThroughDirect(hFile, &soucCDB, sizeof(soucCDB.SD_VENDOR), NULL,data_in, sizeof(data_in), 0,0,0);
	printf("%s::%d:status is %d\n", __FUNCTION__, __LINE__, status);
	if (status < 0) 
	{
		printf("sd get cid failed %d\n", errno);
		CloseHandle(hFile);
		return status;
	}

	if (NULL != cidData)
	{
		memcpy(cidData, data_in, sizeof(SD_CID_DATA));

		if (SD_CID_DEBUG == 1 )
		{
			printf("CID dumping:");
			for (int i = 0; i < sizeof(data_in); i++)
			{
				if (0 == (i % 16)) printf("\r\n%03Xh: ", i);
				if (0 == (i % 8)) putchar(' ');
				printf("%02x ", data_in[i]);
			}
			printf("\r\n");
			printf("- Manufacturer ID (1byte)   	: %02x \n",cidData->Manufacturer_ID);
			printf("- OEM/Application ID (2byte)	: %c%c \n",cidData->OEM_Application_ID[0],cidData->OEM_Application_ID[1]);
			printf("- Product Name (5byte) 	    	: %c%c%c%c%c \n",cidData->Product_Name[0],cidData->Product_Name[1]
														,cidData->Product_Name[2],cidData->Product_Name[3]
														,cidData->Product_Name[4]);
			printf("- Product Revision (4bit.4bit)  : %d.%d \n", cidData->Product_Revision_HI,cidData->Product_Revision_LO);
			printf("- Product Serial NO (4byte) 	: %02x%02x%02x%02x \n",cidData->Product_Serial_NO[0],cidData->Product_Serial_NO[1] 
													,cidData->Product_Serial_NO[2],cidData->Product_Serial_NO[3]);
			printf("\r\n\n");			
		}

	}
	return status;
}

static int sd_get_csd(int hFile, SD_CID_DATA *cidData, int SD_CID_DEBUG)
{
	SD_OVER_USB_CDB soucCDB;
	uint8_t data_in[sizeof(SD_CID_DATA)];
	int status = 0;

	memset(&soucCDB, 0x00, sizeof(soucCDB));
	memset(&data_in[0], 0x00, sizeof(data_in));
	memset(cidData, 0x00, sizeof(SD_CID_DATA));

	soucCDB.SD_VENDOR.OperationCode = 0xCF;
	soucCDB.SD_VENDOR.vendorCdb.mVend_Func_Code = 0x1A;
	soucCDB.SD_VENDOR.vendorCdb.mReserved3 = 16;
	soucCDB.SD_VENDOR.vendorCdb.mReserved1 = 0;
	soucCDB.SD_VENDOR.vendorCdb.mReserved2 = 0;
	soucCDB.SD_VENDOR.vendorCdb.mReserved4 = 0;


	status = SCSIPassThroughDirect(hFile, &soucCDB, sizeof(soucCDB.SD_VENDOR), NULL,data_in, sizeof(data_in), 0,0,0);
	printf("%s::%d:status is %d\n", __FUNCTION__, __LINE__, status);
	if (status < 0) 
	{
		printf("sd get csd failed %d\n", errno);
		CloseHandle(hFile);
		return status;
	}

	if (NULL != cidData)
	{
		memcpy(cidData, data_in, sizeof(SD_CID_DATA));

		if (SD_CID_DEBUG == 1 )
		{
			printf("CSD dumping:");
			for (int i = 0; i < sizeof(data_in); i++)
			{
				if (0 == (i % 16)) printf("\r\n%03Xh: ", i);
				if (0 == (i % 8)) putchar(' ');
				printf("%02x ", data_in[i]);
			}
			printf("\r\n\n");
		}
	}
	return status;
}

static int sd_get_ssr(int hFile, SD_STATUS_REGISTER *ssrData, int SD_CID_DEBUG)
{
    SD_OVER_USB_CDB soucCDB;
    uint8_t data_in[sizeof(SD_STATUS_REGISTER)];
    int status = 0;

    memset(&soucCDB, 0x00, sizeof(soucCDB));
    memset(&data_in[0], 0x00, sizeof(data_in));
    memset(ssrData, 0x00, sizeof(SD_STATUS_REGISTER));

    soucCDB.SD_VENDOR.OperationCode = 0xCF;
    soucCDB.SD_VENDOR.vendorCdb.mVend_Func_Code = 0xC1;
    soucCDB.SD_VENDOR.vendorCdb.mReserved1 = 0;
    soucCDB.SD_VENDOR.vendorCdb.mReserved2 = 0;
    soucCDB.SD_VENDOR.vendorCdb.mReserved3 = 65;  // transfer length field -> SMSC Vendor SCSI Commands_for SD and MMC.pdf  16pg
    soucCDB.SD_VENDOR.vendorCdb.mReserved4 = 0;


    status = SCSIPassThroughDirect(hFile, &soucCDB, sizeof(soucCDB.SD_VENDOR), NULL,data_in, sizeof(data_in), 0,0,0);
    printf("%s::%d:status is %d\n", __FUNCTION__, __LINE__, status);
    if (status < 0)
    {
        printf("sd get ssr failed %d\n", errno);
        CloseHandle(hFile);
        return status;
    }

    if (NULL != ssrData)
    {
        memcpy(ssrData, data_in, sizeof(SD_STATUS_REGISTER));

        if (SD_CID_DEBUG == 1 )
        {
            printf("SSR dumping:");
            for (int i = 0; i < sizeof(data_in); i++)
            {
                if (0 == (i % 16)) printf("\r\n%03Xh: ", i);
                if (0 == (i % 8)) putchar(' ');
                printf("%02x ", data_in[i]);
            }
            printf("\r\n\n");

            printf("- SD card Type (2byte)       	: %02x%02x \n",ssrData->SD_Card_Type[0],ssrData->SD_Card_Type[1]);
            printf("- Size of Protected Area (4byte): %02x%02x%02x%02x \n",ssrData->Size_Of_Protected_Area[0],ssrData->Size_Of_Protected_Area[1],ssrData->Size_Of_Protected_Area[2],ssrData->Size_Of_Protected_Area[3]);
            printf("- Speed Class (1byte)       	: %02x \n",ssrData->Speed_Class);
            printf("- Performance Move (1byte)      : %02x \n",ssrData->Performance_Move);
            printf("- UHS Speed Grade (4bit)        : %d \n", ssrData->UHS_Speed_Grade);
            printf("- UHS AU Size (4bit)            : %d \n", ssrData->UHS_AU_Size);
            printf("- Video Speed Class (1byte)     : %02x \n",ssrData->Video_Speed_Class);
            printf("- App Perf Class (2byte)        : %02x%02x \n",ssrData->App_Perf_Class[0], ssrData->App_Perf_Class[1]);

            printf("\r\n\n");
        }

    }
    return status;
}


static int scsi_inquiry(int hFile, INQUIRYDATA* iInquiryData, int SCSI_INQUIRY_DEBUG)
{
	uint8_t data_in[sizeof(INQUIRYDATA)];
	CDB cdbCDB;
	int status = 0;

	memset(&cdbCDB, 0x00, sizeof(cdbCDB));
	memset(&data_in[0], 0x00, sizeof(data_in));
	memset(iInquiryData, 0x00, sizeof(INQUIRYDATA));

	cdbCDB.CDB6INQUIRY3.OperationCode = SCSIOP_INQUIRY;
	cdbCDB.CDB6INQUIRY3.AllocationLength = INQUIRYDATABUFFERSIZE;

	status = SCSIPassThroughDirect(hFile, &cdbCDB,sizeof(cdbCDB.CDB6INQUIRY3), NULL,data_in, INQUIRYDATABUFFERSIZE, 0,0,0);
	printf("%s::%d:status is %d\n", __FUNCTION__, __LINE__, status);
	if (status < 0) 
	{
		printf("scsi inquiry failed %d\n", errno);
		CloseHandle(hFile);
		return status;
	}
	if (NULL != iInquiryData) 
	{
		memcpy(iInquiryData, data_in, sizeof(INQUIRYDATA));

		if (SCSI_INQUIRY_DEBUG == 1 )
		{
			/* dump out the data */
			printf("SCSI Inquiry:\n");
			printf("- Vendor Id: %s\n", iInquiryData->VendorId);
			printf("- Product Id: %s\n", iInquiryData->ProductId);
			printf("- Revision: %s\n", iInquiryData->ProductRevisionLevel);
			printf("\r\n");			
		}
	}
	return status; 

}


static int sd_inquiry(int hFile, SD_INQUIRY_DATA* souidData, int SD_INQUIRY_DEBUG)
{
	SD_OVER_USB_CDB soucCDB;
	uint8_t data_in[sizeof(SD_INQUIRY_DATA)];
	int status = 0;

	memset(&soucCDB, 0x00, sizeof(soucCDB));
	memset(&data_in[0], 0x00, sizeof(data_in));
	memset(souidData, 0x00, sizeof(SD_INQUIRY_DATA));

	soucCDB.SD_INQUIRY.OperationCode    = SDOUSB_CDB_OPCODE;
	soucCDB.SD_INQUIRY.ServiceAction[0] = HIBYTE(SDOUSB_CDB_INQUIRY);
	soucCDB.SD_INQUIRY.ServiceAction[1] = LOBYTE(SDOUSB_CDB_INQUIRY);
	soucCDB.SD_INQUIRY.AllocationLength = sizeof(data_in);
	soucCDB.SD_INQUIRY.Signature[0]     = HIBYTE(SD_OVER_USB_INQUIRY_SIGNATURE);
	soucCDB.SD_INQUIRY.Signature[1]     = LOBYTE(SD_OVER_USB_INQUIRY_SIGNATURE);

	status = SCSIPassThroughDirect(hFile, &soucCDB, sizeof(soucCDB.SD_INQUIRY), NULL, data_in, sizeof(data_in), 0,0,0);
	printf("%s::%d:status is %d\n", __FUNCTION__, __LINE__, status);
	if (status < 0) 
	{
		printf("sd inquiry failed %d\n", errno);
		CloseHandle(hFile);
		return status;
	}

	if (NULL != souidData)
	{
		memcpy(souidData, data_in, sizeof(SD_INQUIRY_DATA));

		if (SD_INQUIRY_DEBUG == 1 ){
			/* write data to stdout in CSV format */
			/* dump_data_block */
			int i = 0;
			printf("SD Inquiry dumping:");
			for (i = 0; i < sizeof(data_in); i++)
			{
				if (0 == (i % 16)) printf("\r\n%03Xh: ", i);
				if (0 == (i % 8)) putchar(' ');
				printf("%02x ", (uint8_t)data_in[i]);
			}
			printf("\r\n");
			printf("- ProtocolVersion: 0x%02X\r\n", souidData->ProtocolVersion);
			printf("- Signature: 0x%02X 0x%02X\r\n", souidData->Signature[0], souidData->Signature[1]);
			printf("- SDHostControllerVersionMajor: 0x%02X\r\n", souidData->SDHostControllerVersionMajor);
			printf("- SDHostControllerVersionMinor: 0x%02X\r\n", souidData->SDHostControllerVersionMinor);
			printf("- Slot: 0x%02X\r\n", souidData->Slot);
			printf("- ProtocolClass: 0x%02X\r\n", souidData->ProtocolClass);
			printf("- VendorID: 0x%02X\r\n", souidData->VendorID);
			printf("- VendorData: 0x%02X\r\n", souidData->VendorData[0]);
			printf("\r\n\n");
		} 
	}

	return status;	
}


//sd lib function 

/**********************************************************************
DATE	: 2022. 11. 16
AUTHOR	: MIN-HYEOK KIM
MODIFIER: DAE-HYUN HAN
PARAMS	: *hFile		- USB handle file
		  pcFixDevName	- USB device name
		  iIsDebugInit	- Debugging value
RETURN	: iReturn
COMMENTS: The function with checking initialization SD card.
		  USB device use scsi generic(sg). /dev/sg(n)
**********************************************************************/
int sd_init_check( int * hFile, char * pcFixDevName, int iIsDebugInit )
{     
	int iReturn = 0;
	INQUIRYDATA iInquiryData;
	SD_INQUIRY_DATA souidData;

	iReturn = scsi_pt_open_device( pcFixDevName, 1, 1 );
	
	if( 0 > iReturn )
	{
		printf( "open device %s failed %d\n", pcFixDevName, errno );
		return iReturn;
	}

	memcpy( hFile, &iReturn, sizeof( *hFile ) );

	iReturn = scsi_inquiry( *hFile, &iInquiryData, iIsDebugInit );
	
	if( 0 > iReturn )
	{
		return iReturn; 
	}

	iReturn = sd_inquiry( *hFile, &souidData, iIsDebugInit );
	
	if( 0 > iReturn )
	{
		return iReturn; 
	}

	return iReturn; 
}

/**********************************************************************
DATE    : 2022. 11. 21
AUTHOR  : MIN-HYEOK KIM
MODIFIER: DAE-HYUN HAN
PARAMS  : hFile				- SD card handle
          * stSDCidData		- SD card CID data value
          iIsDebugStatus	- Debugging value
RETURN  : iReturn
COMMENTS: The function with checking support to SD card in this system.
**********************************************************************/
int  sd_support_check( int hFile, SD_CID_DATA * stSDCidData, int iIsDebugStatus )
{
	int iReturn = 0;
	const int kiWhiteListCnt = DEF_MAX_SD_CARD_WHITE_LIST_COUNT;
	char caHexData[DEF_MAX_HEX_DATA_LENGTH];
	SD_WHITE_LIST staWhiteList[kiWhiteListCnt];
	SD_STATUS_DATA stStatusData;

    iReturn = sd_get_status( hFile, &stStatusData, iIsDebugStatus );

	if( 0 > iReturn )
	{
		return iReturn;
	}
	else
	{
		if( 0 == stStatusData.CardPresent )
		{
			printf( "sd card is not inserted \n" );
			iReturn = -1;

			return iReturn;
		}
		else if( 1 == stStatusData.CardPresent )
		{
            iReturn = sd_get_csd( hFile, stSDCidData, iIsDebugStatus );
            iReturn = sd_get_cid( hFile, stSDCidData, iIsDebugStatus );
			
			if( 0 > iReturn )
			{
				return iReturn;
			}
			else
			{
				sd_get_white_list( staWhiteList, kiWhiteListCnt );
				hex_to_str_pnm( caHexData, stSDCidData->Product_Name, sizeof( stSDCidData->Product_Name ) );
				iReturn = white_list_check( staWhiteList, kiWhiteListCnt, stSDCidData->Manufacturer_ID, caHexData );
				
				if( 0 > iReturn )
				{
					return iReturn;
				}
			}
		}
	}

	return iReturn;
}

int sd_speed_check( int hFile, SD_STATUS_REGISTER * stSDSsrData, int iIsDebugStatus )
{
    int iReturn = 0;
    const int kiWhiteListCnt = DEF_MAX_SD_CARD_WHITE_LIST_COUNT;
    char caHexData[DEF_MAX_HEX_DATA_LENGTH];
    SD_WHITE_LIST staWhiteList[kiWhiteListCnt];
    SD_STATUS_DATA stStatusData;

    iReturn = sd_get_status( hFile, &stStatusData, iIsDebugStatus );

    if( 0 > iReturn )
    {
        return iReturn;
    }
    else
    {
        if( 0 == stStatusData.CardPresent )
        {
            printf( "sd card is not inserted \n" );
            iReturn = -1;

            return iReturn;
        }
        else if( 1 == stStatusData.CardPresent )
        {
            iReturn = sd_get_ssr( hFile, stSDSsrData, iIsDebugStatus );

            if( 0 > iReturn )
            {
                return iReturn;
            }
        }
    }

    return iReturn;
}


int sd_get_health_status(int hFile, SD_HEALTH_DATA *healthData, uint8_t vendor_type, int vendor_flag, int SD_HEALTH_STATUS_DEBUG) //vendor_flag default value is 0 
{
	/* #define SDHostCmd56(hFile, block_size, block_count, arg, xfer_mode, response)  \
	 *	libSDHostCmd(hFile, block_size, block_count, arg, xfer_mode, COMMAND_REG(56, (HC_REG_DATA_PRESENT | HC_REG_CRC_ENABLE), HC_REG_48B_RESPONSE), response)
	 *	(*)read-in 512 bytes ==> SDHostCmdWithData
	 */
	const uint16_t serviceaction = SDOUSB_CDB_COMMAND_WITH_DATA;
	const uint16_t blocksize = SD_BLOCK_SIZE;
	const uint16_t blockcount = 1;
	const uint16_t transfermode = SDHC_TRANFER_MODE_REG_DATA_TRANSFER_DIRECTION_READ;
	const uint16_t command = COMMAND_REG(K_SD_CMD_56, (HC_REG_DATA_PRESENT | HC_REG_IDXCHK_ENABLE | HC_REG_CRC_ENABLE), HC_REG_48B_RESPONSE);

    uint32_t vendor_arg = 0x00000000; 

	switch(vendor_type)
	{
		case 0x03:      //western digital 
			vendor_arg = 0x00000001;     
			break;
		case 0x09:      //micron 
			if (vendor_flag == 0){
				vendor_arg = 0x110005FB;
			}
			else if (vendor_flag == 1){
				vendor_arg = 0x110005FD;	
			}
			break;
		case 0xad:      //longsys 
			vendor_arg = 0x00000001; 
			break; 
	}

	SD_OVER_USB_CDB soucCDB;
	uint8_t data_in[SD_BLOCK_SIZE];
	int status = FALSE;

	/* ref) USB82642/2642 SDIO Over USB User's Guide - 3.3.1 SD_COMMAND CDB
	 */
	memset(&soucCDB, 0x00, sizeof(soucCDB));
	memset(&data_in[0], 0xff, sizeof(data_in));
	memset(healthData, 0x00, sizeof(SD_HEALTH_DATA));

	soucCDB.SD_COMMAND.OperationCode    = SDOUSB_CDB_OPCODE;
	soucCDB.SD_COMMAND.ServiceAction[0] = HIBYTE(serviceaction);
	soucCDB.SD_COMMAND.ServiceAction[1] = LOBYTE(serviceaction);
	soucCDB.SD_COMMAND.BlockSize[0]     = HIBYTE(blocksize);
	soucCDB.SD_COMMAND.BlockSize[1]     = LOBYTE(blocksize);
	soucCDB.SD_COMMAND.BlockCount[0]    = HIBYTE(blockcount);
	soucCDB.SD_COMMAND.BlockCount[1]    = LOBYTE(blockcount);
	soucCDB.SD_COMMAND.ArgumentByDWORD  = SWAP_ENDIAN_32(vendor_arg);
	soucCDB.SD_COMMAND.TransferMode[0]  = HIBYTE(transfermode);
	soucCDB.SD_COMMAND.TransferMode[1]  = LOBYTE(transfermode);
	soucCDB.SD_COMMAND.Command[0]       = HIBYTE(command);
	soucCDB.SD_COMMAND.Command[1]       = LOBYTE(command);
	soucCDB.SD_COMMAND.Control          = 0x00;

	assert(sizeof(data_in) == (blocksize * blockcount));
	status = SCSIPassThroughDirect(hFile, &soucCDB, sizeof(soucCDB.SD_COMMAND), NULL, data_in, sizeof(data_in), 0,0,0);
	printf("%s::%d:status is %d\n", __FUNCTION__, __LINE__, status);
	if (status < 0) 
	{
		printf("sd get health status failed %d\n", errno);
		CloseHandle(hFile);
		return status;
	}
	if (NULL != healthData)
	{

		if (SD_HEALTH_STATUS_DEBUG == 1)
		{
			/* dump_data_block */
			int i = 0;
			printf("CMD56 data block dumping:");
			for (i = 0; i < sizeof(data_in); i++)
			{
				if (0 == (i % 16)) printf("\r\n%03Xh: ", i); 
				if (0 == (i % 8)) putchar(' ');
				printf("%02x ", (uint8_t)data_in[i]);
			}
			printf("\r\n");			
		}


		if (vendor_type == 0x03 )
		{                   //western digital 
			memcpy(&(healthData->Health_Present_Data), &data_in[8], sizeof(healthData->Health_Present_Data));	
			if (SD_HEALTH_STATUS_DEBUG == 1)
			{
				printf("- Health Status in: %d percent\r\n", data_in[8]);
				printf("- Spare bad block count: %02x\r\n", data_in[21]);
				printf("- Runtime bad block(UECC) count: %02x %02x\r\n", data_in[24],data_in[25]);
				printf("\r\n\n");				
			}	
		}
		else if( vendor_type == 0x09 )
		{             //micorn
			if ( vendor_flag == 0 )
			{
				memcpy(&(healthData->Health_Present_Data), &data_in[8], sizeof(healthData->Health_Present_Data));	// TLC / QLC percentage 
				if (SD_HEALTH_STATUS_DEBUG == 1)
				{
					printf("- Health Status in: %d percent\r\n", data_in[8]); 
					printf("\r\n\n");
				} 				
			} 
			else if ( vendor_flag == 1 )
			{      
				if (SD_HEALTH_STATUS_DEBUG == 1)
				{ 	
					printf("- Factory bad block count: %02x %02x\r\n", data_in[95],data_in[96]);
					printf("- Spare block count: %02x %02x\r\n", data_in[93],data_in[94]);
					printf("- Runtime bad block count: %02x %02x\r\n", data_in[106],data_in[107]);
					printf("\r\n\n");
				}
			}
		}
		else if (vendor_type == 0xad)
		{                //longsys
			uint8_t data_conv = 100 - data_in[44];
			data_in[44] = data_conv;

			memcpy(&(healthData->Health_Present_Data), &data_in[44], sizeof(healthData->Health_Present_Data));
			if (SD_HEALTH_STATUS_DEBUG == 1)
			{
				printf("- SMART Tag: %02x%02x%02x%02x\r\n", data_in[3],data_in[2],data_in[1],data_in[0]);
				printf("- SMART Versions: %02x%02x%02x%02x\r\n", data_in[7],data_in[6],data_in[5],data_in[4]);
				printf("- Xor Value: %02x%02x%02x%02x\r\n", data_in[11],data_in[10],data_in[9],data_in[8]);
				printf("- Size of SMart: %02x%02x%02x%02x\r\n", data_in[15],data_in[14],data_in[13],data_in[12]);
	    		printf("- Original Bad Block: %02x%02x%02x%02x\r\n", data_in[19],data_in[18],data_in[17],data_in[16]);
				printf("- Increase Bad Block: %02x%02x%02x%02x\r\n", data_in[23],data_in[22],data_in[21],data_in[20]);
				printf("- Write All Sect Num: %02x%02x%02x%02x%02x%02x%02x%02x\r\n", data_in[31],data_in[30],data_in[29],data_in[28],data_in[27],data_in[26],data_in[25],data_in[24]);
				printf("- replace Block Left: %02x%02x%02x%02x\r\n", data_in[35],data_in[34],data_in[33],data_in[32]);
				printf("- DegreOfWear: %02x%02x%02x%02x\r\n", data_in[39],data_in[38],data_in[37],data_in[36]);
				printf("- Sector Total: %02x%02x%02x%02x\r\n", data_in[43],data_in[42],data_in[41],data_in[40]);
				printf("- remain life time ==> HEALTH STATUS in: %d percent\r\n", data_in[44]); //default is 100, convert[ 100 - health_value_org ]
				printf("- RemainWrGBNum: %02x%02x%02x%02x\r\n", data_in[51],data_in[50],data_in[49],data_in[48]);
				printf("- LifeTimeTotal: %02x%02x%02x%02x\r\n", data_in[55],data_in[54],data_in[53],data_in[52]);
				printf("- PhyWrGBNum: %02x%02x%02x%02x\r\n", data_in[59],data_in[58],data_in[57],data_in[56]);
				printf("\r\n\n");				
			} 			
		}
	}
	return status;
}


int sd_close_check(int *hFile, char *pfix_dev_name, SD_HEALTH_DATA *healthData, SD_CID_DATA *cidData)
{
	int status = 0; 
	int _hFile = -1;

	memset(healthData, 0x00, sizeof(SD_HEALTH_DATA));
	memset(cidData, 0x00, sizeof(SD_CID_DATA));

	status = scsi_pt_close_device(*hFile);
	if (status < 0) 
	{
	    printf("close device %s failed %d\n", pfix_dev_name, errno);
	    return status;
	}
	memcpy(hFile,&_hFile,sizeof(*hFile));
	status = 0; 
	return status;
}
