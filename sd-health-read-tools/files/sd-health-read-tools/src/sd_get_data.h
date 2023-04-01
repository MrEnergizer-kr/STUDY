#include <stdint.h>
#include "sd_usb_win32.h"
#include "sd_usb.h"

#define DEF_MAX_SD_CARD_WHITE_LIST_COUNT	20
#define DEF_MAX_HEX_DATA_LENGTH				10
#define DEF_MAX_SD_CARD_PRODUCT_NAME_LENGTH	10

#define DEF_APPLICATION_ID_SIZE				 2
#define DEF_PRODUCT_NAME_SIZE				 5
#define DEF_PRODUCT_SERIAL_NUMBER_SIZE		 4

typedef struct _SD_CID_DATA
{
	uint8_t Manufacturer_ID;
	uint8_t OEM_Application_ID[DEF_APPLICATION_ID_SIZE];
	uint8_t Product_Name[DEF_PRODUCT_NAME_SIZE];
	uint8_t Product_Revision_HI : 4;
	uint8_t Product_Revision_LO : 4;
	uint8_t Product_Serial_NO[DEF_PRODUCT_SERIAL_NUMBER_SIZE];
    uint8_t Reverved[3];
} SD_CID_DATA;

typedef struct _SD_STATUS_REGISTER
{
    //uint8_t Reserved1[65];
    uint8_t Reserved1[2];
	uint8_t SD_Card_Type[2];
	uint8_t Size_Of_Protected_Area[4];
    uint8_t Speed_Class;
    uint8_t Performance_Move;
    uint8_t Reserved2;
    uint8_t Erase_Size[2];
    uint8_t Reserved3;
    uint8_t UHS_Speed_Grade : 4;
    uint8_t UHS_AU_Size : 4;
	uint8_t	Video_Speed_Class;  // 16
    uint8_t Reserved4[3];
    uint8_t App_Perf_Class[2]; // 앞에 2 bit 자르기
    uint8_t Reserved5[44];
} SD_STATUS_REGISTER;

typedef struct _SD_HEALTH_DATA
{
	uint8_t Health_Present_Data;
	uint16_t Runtime_Bad_Block;
	uint16_t reserved1;
	uint16_t reserved2;
} SD_HEALTH_DATA;

typedef struct _SD_WHITE_LIST
{
	uint8_t Manufacturer_ID;
	char Product_Name[DEF_MAX_SD_CARD_PRODUCT_NAME_LENGTH];
} SD_WHITE_LIST;

static int SCSIPassThroughDirect(int hFile, 
				void *pCDB, 
				uint8_t nCDBLength,
				__attribute__ ((unused)) SCSI_PASS_THROUGH_STATUS *sptsStatus, 
				void *pBuffer,
				uint32_t dwBufferLength, 
				int bWrite,int DEBUG_SCSI, int VERBOSE_SCSI);

static int sd_inquiry(int hFile, SD_INQUIRY_DATA* souidData, int SD_INQUIRY_DEBUG);
static int scsi_inquiry(int hFile, INQUIRYDATA* iInquiryData, int SCSI_INQUIRY_DEBUG);
static int sd_get_cid(int hFile, SD_CID_DATA *cidData, int SD_CID_DEBUG);
static int sd_get_csd(int hFile, SD_CID_DATA *cidData, int SD_CID_DEBUG);
static int sd_get_ssr(int hFile, SD_STATUS_REGISTER *ssrData, int SD_CID_DEBUG);
static int white_list_check( SD_WHITE_LIST * pstSDWhiteList, const int kiProductCnt, uint8_t uiSDManuID, char * pcSDPnm );
static int hex_to_str_pnm(char *hex_str, uint8_t *hex_arr, int hex_arr_len);
int sd_get_status( int hFile, SD_STATUS_DATA *pStatusData, int iIsDebugStatus );
int sd_get_health_status(int hFile, SD_HEALTH_DATA *healthData, uint8_t vendor_type, int vendor_flag, int SD_HEALTH_STATUS_DEBUG);  //vendor_flag default value is 0
int sd_init_check( int * hFile, char * pcFixDevName, int iIsDebugInit );
int sd_support_check( int hFile, SD_CID_DATA * stSDCidData, int iIsDebugStatus );
int sd_speed_check( int hFile, SD_STATUS_REGISTER * stSDSsrData, int iIsDebugStatus );
int sd_close_check(int *hFile, char *pfix_dev_name, SD_HEALTH_DATA *healthData, SD_CID_DATA *cidData);
