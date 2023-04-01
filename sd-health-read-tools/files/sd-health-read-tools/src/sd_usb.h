/* Vendor CIDs */
#define CID_MICRON	(0xFEUL)
#define CID_HYNIX	(0x90UL)
#define CID_TOSHIBA (0x0000) /*--AS Dummy place holder--*/

/* EXT_CSD access values */
#define EXT_CSD_ACCESS_CMD_SET		(0)
#define EXT_CSD_ACCESS_SET_BITS		(1)
#define EXT_CSD_ACCESS_CLR_BITS		(2)
#define EXT_CSD_ACCESS_WRITE_BYTE	(3)

#define SD_OVER_USB_INQUIRY_SIGNATURE	(0x5D5C)
#define SCSI_PASS_THROUGH_TIMEOUT	(10) /* seconds */

#define SWAP_ENDIAN_64(x) \
  ((((x) >> 56) & 0x00000000000000FFULL) (((x) >> 40) & 0x000000000000FF00ULL) \
  ( ((x) >> 24) & 0x0000000000FF0000ULL) (((x) >>  8) & 0x00000000FF000000ULL) \
  ( ((x) <<  8) & 0x000000FF00000000ULL) (((x) << 24) & 0x0000FF0000000000ULL) \
  ( ((x) << 40) & 0x00FF000000000000ULL) (((x) << 56) & 0xFF00000000000000ULL) )
#define SWAP_ENDIAN_32(x) ((((x) & 0xFF000000UL) >> 24) | (((x) & 0x00FF0000UL) >> 8) | (((x) & 0x0000FF00UL) << 8) | (((x) & 0x000000FFUL) << 24)) 
#define SWAP_ENDIAN_16(x) ((((x) & 0xFF00U) >> 8) | (((x) & 0x00FFU) << 8)) 


#define SENSE_UNKNOWN         0x00
#define SENSE_NO_MEDIA        0x01
#define SENSE_READ_ERROR      0x02
#define SENSE_WRITE_ERROR     0x03
#define SENSE_NOT_READY       0x04
#define SENSE_ILLEGAL_REQUEST 0x05
#define SENSE_MEDIA_CHANGE    0x06
#define SENSE_FMT_FAILED      0x07


typedef struct _SCSI_PASS_THROUGH_DIRECT_WITH_BUFFER
{
	SCSI_PASS_THROUGH_DIRECT	sptd;
	uint32_t			Filler;	/* realign buffer to double word boundary */
	int8_t				ucSenseBuf[SENSE_BUFFER_SIZE];
} __attribute__ ((packed)) SCSI_PASS_THROUGH_DIRECT_WITH_BUFFER, *PSCSI_PASS_THROUGH_DIRECT_WITH_BUFFER;


typedef struct Param_Vendor_Cmd
{
	uint8_t mVend_Func_Code;
	uint8_t mReserved1;
	uint8_t mReserved2;
	uint8_t mReserved3;
	uint8_t mReserved4;
} Param_Vendor_Cmd_Type;



typedef struct _SCSI_PASS_THROUGH_STATUS
{
	uint32_t dwBytesTransferred;
	uint32_t dwLastError;
	uint8_t SCSIStatus;
	uint8_t SenseKey;
	uint8_t AdditionalSenseCode;
	uint8_t AdditionalSenseCodeQualifier;
	uint8_t SenseIdentifier;
} __attribute__ ((packed)) SCSI_PASS_THROUGH_STATUS, *PSCSI_PASS_THROUGH_STATUS;

typedef union
{
	uint32_t u32;
	struct
	{
		uint8_t hi;          /* msw msb */
		uint8_t lh;          /* msw lsb */
		uint8_t hl;          /* lsw msb */
		uint8_t lo;          /* lsw lsb */
	} u8;
} __attribute__ ((packed)) t_udw32;

typedef struct _SD_CMD52_RESP {
	union {
		uint8_t SDResponseRaw[16];
		
		struct {
			uint16_t stuff;
			uint8_t  response_flags;
			uint8_t  data;
		} response;
	};
} __attribute__ ((packed)) SD_CMD52_RESP;

typedef union _SD_OVER_USB_CDB
{
	struct _SD_INQUIRY {
		uint8_t OperationCode;
		uint8_t ServiceAction[2];
		uint8_t Reserved[3];
		uint8_t AllocationLength;
		uint8_t Signature[2];
		uint8_t Control;
	} __attribute__ ((packed)) SD_INQUIRY;

	struct _SD_HC_REGISTER_IO {
		uint8_t OperationCode;
		uint8_t ServiceAction[2];
		uint8_t Reserved1[3];
		uint8_t AllocationLength;
		uint8_t Reserved2;
		uint8_t RegisterOffset;
		uint8_t Control;
	} __attribute__ ((packed )) SD_HC_REGISTER_IO;

	struct _SD_COMMAND {
		uint8_t OperationCode;
		uint8_t ServiceAction[2];
		uint8_t BlockSize[2];
		uint8_t BlockCount[2];
		union {
			uint32_t ArgumentByDWORD;
			uint16_t ArgumentByWORD[2];
			uint8_t  Argument[4];
		} __attribute__ ((packed));
		uint8_t TransferMode[2];
		uint8_t Command[2];
		uint8_t Control;
	} __attribute__ ((packed)) SD_COMMAND;

	struct _SD_DATA_IO {
		uint8_t OperationCode;
		uint8_t ServiceAction[2];
		union {
			uint32_t AllocationLengthByDWORD;
			uint8_t  AllocationLength[4];
		} __attribute__ ((packed)) ;
		uint8_t Reserved2[2];
		uint8_t Control;
	} __attribute__ ((packed)) SD_DATA_IO;

	struct _SD_CARD_INITIALIZE {
		uint8_t OperationCode;
		uint8_t ServiceAction[2];
		uint8_t Reserved1[3];
		uint8_t AllocationLength;
#if 0
	uint8_t Reserved2[1];
#else
		uint8_t	InitDelay;
#endif
		uint8_t InitType;
		uint8_t Control;
	} __attribute__ ((packed)) SD_CARD_INITIALIZE;

	struct _SD_STATUS {
		uint8_t OperationCode;
		uint8_t ServiceAction[2];
		uint8_t Reserved1[3];
		uint8_t AllocationLength;
		uint8_t Reserved2[1];
		uint8_t ReadIntPending:1;
		uint8_t Reserved3:7;
		uint8_t Control;
	} __attribute__ ((packed)) SD_STATUS;
	struct _SD_VENDOR_CMD {
		uint8_t OperationCode;
		Param_Vendor_Cmd_Type vendorCdb;
	} SD_VENDOR;

	struct _SD_GATHER_WHILE {
		uint8_t OperationCode;
		uint8_t ServiceAction[2];
		uint8_t BufferLength[2];
		uint8_t DataArgument[3];
		uint8_t ConditionArgument[3];
		uint8_t ConditionMask;
		uint8_t ConditionContinuationValue;
		uint8_t ConditionTimeout;
		uint8_t Reserved;
		uint8_t Control;
	} __attribute__ ((packed)) SD_GATHER_WHILE;

	uint32_t AsUlong[4];
	uint8_t AsByte[16];
} __attribute__ ((packed)) SD_OVER_USB_CDB, *PSD_OVER_USB_CDB;

typedef struct _SD_INQUIRY_DATA {
	uint8_t ProtocolVersion;
	uint8_t Signature[2];
	uint8_t SDHostControllerVersionMajor;
	uint8_t SDHostControllerVersionMinor:7;
	uint8_t Reserved:1;
	uint8_t Slot:4;
	uint8_t ProtocolClass:4;
	/* BUG: union adds byte to struct!
	union {
		uint8_t ProtocolClass:4;
		struct {
			uint8_t HCRC:1;
			uint8_t DTOC:1;
			uint8_t AIC:1;
			uint8_t SC:1;
		};
	};
	*/
	uint8_t VendorID;
	uint8_t VendorData[1];
} __attribute__ ((packed)) SD_INQUIRY_DATA;

typedef enum _SD_OVER_USB_PROTOCOL_CLASS_LOOKUP
{
	SD_OVER_USB_PROTOCOL_CLASS_CLASS0 = 0x1,
	SD_OVER_USB_PROTOCOL_CLASS_CLASS1 = 0x3,
	SD_OVER_USB_PROTOCOL_CLASS_CLASS2 = 0x7,
	SD_OVER_USB_PROTOCOL_CLASS_CLASS3 = 0xE
} SD_OVER_USB_PROTOCOL_CLASS_LOOKUP;

typedef struct _SD_COMMAND_DATA {
	union {
		uint8_t SDResponseRaw[16];
		struct {
			uint8_t ResponseData[4];
			uint8_t Reserved[8];
			uint8_t AutoCMD12CardStatus[4];
		} __attribute__ ((packed)) SDResponseShort;
		struct {
			uint8_t ResponseData[15];
			uint8_t Reserved[1];
		} __attribute__ ((packed)) SDResponseLong;
	} __attribute__ ((packed)) ;
	uint8_t SDIOInterrupt:1;
	uint8_t Reserved1:7;
} __attribute__ ((packed)) SD_COMMAND_DATA;

typedef struct _SD_R5_RESP {
	union {
		uint8_t SDResponseRaw[16];
		struct {
			uint16_t stuff;
			uint8_t  response_flags;
			uint8_t  data;
		} __attribute__ ((packed)) response;
	} __attribute__ ((packed)) ;
	uint8_t SDIOInterrupt:1;
	uint8_t Reserved1:7;
} __attribute__ ((packed)) SD_R5_RESP;


typedef struct _SD_R1_RESP {
	union {
		uint8_t raw[17];
		struct {
			uint8_t command;
			uint8_t status[4];
			uint8_t crc;
		} __attribute__ ((packed)) r1;
	} __attribute__ ((packed)) ;
} __attribute__ ((packed)) SD_R1_RESP;

/* device status bit map */
#define STATUS_ADDR_RANGE     (1 << 31)
#define STATUS_ADDR_MISALIGN  (1 << 30)
#define STATUS_BLOCK_LEN      (1 << 29)
#define STATUS_ERASE_SEQ      (1 << 28)
#define STATUS_ERASE_PARAM    (1 << 27)
#define STATUS_WP_VIOLATION   (1 << 26)
#define STATUS_IS_LOCKED      (1 << 25)
#define STATUS_LOCK_FAILED    (1 << 24)
#define STATUS_CRC_ERROR      (1 << 23)
#define STATUS_ILLEGAL_CMD    (1 << 22)
#define STATUS_ECC_FAILED     (1 << 21)
#define STATUS_CC_ERROR       (1 << 20)
#define STATUS_GENERIC_ERROR  (1 << 19)
#define STATUS_OVEWRITE       (1 << 16)
#define STATUS_WP_ERASE_SKIP  (1 << 15)
#define STATUS_ERASE_RESET    (1 << 13)
#define STATUS_STATE_SHIFT 9
#define STATUS_STATE_MASK     (0xF << STATUS_STATE_SHIFT)
#define STATUS_DATA_READY     (1 << 8)
#define STATUS_SWITCH_ERROR   (1 << 7)
#define STATUS_EXCEPTION      (1 << 6)
#define STATUS_APP_CMD        (1 << 5)

/* device status state values */
#define DEVICE_STATE_IDLE  0
#define DEVICE_STATE_READY 1
#define DEVICE_STATE_IDENT 2
#define DEVICE_STATE_STBY  3
#define DEVICE_STATE_TRAN  4
#define DEVICE_STATE_DATA  5
#define DEVICE_STATE_RCV   6
#define DEVICE_STATE_PRG   7
#define DEVICE_STATE_DIS   8
#define DEVICE_STATE_BTST  9
#define DEVICE_STATE_SLP   10

#define DEVICE_STATUS_STATE(x) ((x & STATUS_STATE_MASK) >> STATUS_STATE_SHIFT)


typedef struct _SD_CARD_INITIALIZE_DATA {
	uint8_t SDMemoryFunctionType:4;
	uint8_t SDIO:1;
	uint8_t CardLocked:1;
	uint8_t CardError:1;
	uint8_t CardPresent:1;
	uint8_t BusClockSpeed:6;
	uint8_t Reserved3:2;
	uint8_t BusWidth:3;
	uint8_t Reserved2:1;
	uint8_t SDIOFunctions:3;
	uint8_t CardPower:1;
	uint8_t RCA[2];
	uint8_t CID[16];
	uint8_t MemoryOCR[4];
	uint8_t SDIOOCR[3];
} __attribute__ ((packed)) SD_CARD_INITIALIZE_DATA;

typedef enum _SD_MEMORY_FUNCTION_TYPE_LOOKUP
{
	SD_MEMORY_FUNCTION_TYPE_NONE           = 0x0,
	SD_MEMORY_FUNCTION_TYPE_SDSC_V101_V110 = 0x1,
	SD_MEMORY_FUNCTION_TYPE_SDSC_V200_V300 = 0x2,
	SD_MEMORY_FUNCTION_TYPE_SDHC_SDXC      = 0x3
} SD_MEMORY_FUNCTION_TYPE_LOOKUP;

#define SD_OVER_USB_INIT_POWER_OFF    0x01
#define SD_OVER_USB_INIT_POWER_ON     0x02
#define SD_OVER_USB_INIT_BASIC        0x04
#define SD_OVER_USB_INIT_BUS_CLOCK    0x08
#define SD_OVER_USB_INIT_BUS_WIDTH    0x10
#define SD_OVER_USB_INIT_SDIO_INTR    0x20
#define SD_OVER_USB_INIT_STATUS_ONLY  0x80
#define SD_OVER_USB_INIT_FULL_INITIALIZE  0x00

typedef struct _SD_STATUS_DATA {
	uint8_t CardPresent:1;
	uint8_t SDIOInterrupt:1;
	uint8_t Reserved1:6;
	uint8_t SDIOIntPending;
} __attribute__ ((packed)) SD_STATUS_DATA;

typedef struct _SD_GATHER_UNTIL_DATA {
	uint8_t FirstCondition;
	uint8_t LastCondition;
	uint8_t Overflow;
	uint8_t DataLength[2];
	uint8_t Data[507];
} __attribute__ ((packed)) SD_GATHER_UNTIL_DATA;

typedef struct _SD_HC_REGISTERS {
	union {

		uint8_t RegistersRaw[256];

		struct {
			uint32_t SDMASystemAddress;
			struct {
				uint16_t TransferBlockSize:12;
				uint16_t HostSDMABufferBoundary:3;
				uint16_t Reserved:1;
			} __attribute__ ((packed)) BlockSize;
			uint16_t BlockCount;
			uint32_t Argument0_1;
			struct {
				uint16_t DMAEnable:1;
				uint16_t BlockCountEnable:1;
				uint16_t AutoCMD12Enable:1;
				uint16_t Reserved:1;
				uint16_t DataTransferDirectionSelect:1;
				uint16_t MultiSingleBlockSelect:1;
				uint16_t Reserved1:10;
			} __attribute__ ((packed)) TransferMode;
			uint16_t Response[8];
			uint16_t DataBufferPort[2];
			uint32_t PresentState;
			uint8_t  HostControl;
			uint8_t  PowerControl;
			uint8_t  BlockGapControl;
			uint8_t  WakeupControl;
			uint16_t ClockControl;
			uint8_t  TimeoutControl;
			uint8_t  SoftwareReset;
			uint16_t NormalInteruptStatus;
			uint16_t ErrorInterruptStatus;
			uint16_t NormalInteruptStatusEnable;
			uint16_t ErrorInterruptStatusEnable;
			uint16_t NormalInteruptSignalEnable;
			uint16_t ErrorInterruptSignalEnable;
			uint16_t AutoCMD12ErrorStatus;
			uint16_t Reserved0;
			uint16_t Capabilities[4];  /* 2 & 3 are reserved */
			uint16_t MaximumCurrentCapabilities[2];
			uint16_t ForceEventAcmd12ErrorStatus[2];
			uint8_t  AdmaErrorStatus;
			uint8_t  Reserved1[3];
			uint16_t AdmaSystemAddress[4];
			uint8_t  Reserved2[0xB0];
			uint16_t SlotInterruptStatus;
			uint16_t HostControllerVersion;
		} __attribute__ ((packed)) RegistersV200;

		struct {
		} __attribute__ ((packed)) RegistersV300;
	} __attribute__ ((packed)) ;
} __attribute__ ((packed)) SD_HC_REGISTERS;


typedef struct
{
	uint16_t	block_size;
	uint16_t	block_count;
	uint32_t	argument;
	uint16_t	transfer_mode;
	uint16_t	command;
} __attribute__ ((packed)) SDHC_REG;

typedef enum
{
	NoResponse,
	Response1,
	Response1b,
	Response2,
	Response3,
	Response6,
	Response7
} SD_RESPONSE_TYPE;

#define MAX_RESPONSE_BYTE	17U

typedef struct
{
	SD_RESPONSE_TYPE	type;
	uint8_t			data[MAX_RESPONSE_BYTE];
} __attribute__ ((packed)) SD_COMMAND_RESPONSE;

typedef enum
{
	SD_TRANSFER_READ,
	SD_TRANSFER_WRITE,
	SD_TRANSFER_COMMAND
} SD_COMMAND_TRANSFER_TYPE;

/* Command Register */
#define SDHC_COMMAND_REG_COMMAND_INDEX_SHIFT				8U
#define SDHC_COMMAND_REG_COMMAND_TYPE_NORMAL				(0U << 6)
#define SDHC_COMMAND_REG_COMMAND_TYPE_SUSPEND				(1U << 6)
#define SDHC_COMMAND_REG_COMMAND_TYPE_RESUME				(2U << 6)
#define SDHC_COMMAND_REG_COMMAND_TYPE_ABORT				(3U << 6)
#define SDHC_COMMAND_REG_DATA_PRESENT					(1U << 5)
#define SDHC_COMMAND_REG_COMMAND_INDEX_CHECK_ENABLE			(1U << 4)
#define SDHC_COMMAND_REG_COMMAND_CRC_CHECK_ENABLE			(1U << 3)
#define SDHC_COMMAND_REG_RESPONSE_TYPE_NORESPONSE			0U
#define SDHC_COMMAND_REG_RESPONSE_TYPE_RESPONSE_LENGTH_136		1U
#define SDHC_COMMAND_REG_RESPONSE_TYPE_RESPONSE_LENGTH_48		2U
#define SDHC_COMMAND_REG_RESPONSE_TYPE_RESPONSE_LENGTH_48_CHECK_BUSY	3U

#define SDHC_COMMAND_REG_RESPONSE_TYPE_RESPONSE_R2	(SDHC_COMMAND_REG_RESPONSE_TYPE_RESPONSE_LENGTH_136 | SDHC_COMMAND_REG_COMMAND_CRC_CHECK_ENABLE)
#define SDHC_COMMAND_REG_RESPONSE_TYPE_RESPONSE_R3_R4	(SDHC_COMMAND_REG_RESPONSE_TYPE_RESPONSE_LENGTH_48)
#define SDHC_COMMAND_REG_RESPONSE_TYPE_RESPONSE_R1_R5_R6_R7	(SDHC_COMMAND_REG_RESPONSE_TYPE_RESPONSE_LENGTH_48 | SDHC_COMMAND_REG_COMMAND_INDEX_CHECK_ENABLE | SDHC_COMMAND_REG_COMMAND_CRC_CHECK_ENABLE)
#define SDHC_COMMAND_REG_RESPONSE_TYPE_RESPONSE_R1b_R5b	(SDHC_COMMAND_REG_RESPONSE_TYPE_RESPONSE_LENGTH_48_CHECK_BUSY | SDHC_COMMAND_REG_COMMAND_INDEX_CHECK_ENABLE | SDHC_COMMAND_REG_COMMAND_CRC_CHECK_ENABLE)

/* Transfer Mode Register */
#define SDHC_TRANFER_MODE_REG_RESPONSE_INTERRUPT_DISABLE		(1U << 8)
#define SDHC_TRANFER_MODE_REG_RESPONSE_ERROR_CHECK_ENABLE		(1U << 7)
#define SDHC_TRANFER_MODE_REG_RESPONSE_TYPE_R5				(1U << 6)
#define SDHC_TRANFER_MODE_REG_MULTI_BLOCK				(1U << 5)
#define SDHC_TRANFER_MODE_REG_DATA_TRANSFER_DIRECTION_READ		(1U << 4)
#define SDHC_TRANFER_MODE_REG_AUTO_CMD12				(1U << 2)
#define SDHC_TRANFER_MODE_REG_AUTO_CMD23				(2U << 2)
#define SDHC_TRANFER_MODE_REG_BLOCK_COUNT_ENABLE			1U


#define MAX_CDB_LENGTH	16

#define CDB_LENGTH_SD_COMMAND	16U

#define SDOUSB_CDB_OPCODE             0x5D

#define SDOUSB_CDB_INQUIRY            0x0000
#define SDOUSB_CDB_HC_REGISTER_READ   0x0002
#define SDOUSB_CDB_HC_REGISTER_WRITE  0x0003
#define SDOUSB_CDB_COMMAND            0x0004
#define SDOUSB_CDB_COMMAND_WITH_DATA  0x0005
#define SDOUSB_CDB_DATA_READ          0x0006
#define SDOUSB_CDB_DATA_WRITE         0x0007
#define SDOUSB_CDB_CARD_INITIALIZE    0x0008
#define SDOUSB_CDB_STATUS             0x000A

#define SDOU_CMD_SCSI_SERVICE_ACTION_SD_INQUIRY			0x0000U
#define SDOU_CMD_SCSI_SERVICE_ACTION_SD_HC_REGISTER_IO_READ	0x0002U
#define SDOU_CMD_SCSI_SERVICE_ACTION_SD_HC_REGISTER_IO_WRITE	0x0003U
#define SDOU_CMD_SCSI_SERVICE_ACTION_SD_COMMAND			0x0004U
#define SDOU_CMD_SCSI_SERVICE_ACTION_SD_COMMAND_WITH_DATA	0x0005U
#define SDOU_CMD_SCSI_SERVICE_ACTION_SD_DATA_IO_READ		0x0006U
#define SDOU_CMD_SCSI_SERVICE_ACTION_SD_DATA_IO_WRITE		0x0007U
#define SDOU_CMD_SCSI_SERVICE_ACTION_SD_CARD_INITIALIZE		0x0008U
#define SDOU_CMD_SCSI_SERVICE_ACTION_SD_STATUS			0x000AU


#define SDOUSB_CDB_GATHER_WHILE       0x8000

#define K_SD_CMD_8    8
#define K_SD_CMD_56  56
#define K_SD_CMD_52  52
#define K_SD_CMD_53  53
#define K_SD_CMD_18  18
#define K_SD_CMD_25  25
#define K_SD_CMD_2    2
#define K_SD_CMD_12  12
#define K_SD_CMD_9    9
#define K_SD_CMD_10  10
#define K_SD_CMD_13  13
#define K_SD_CMD_6    6
#define K_SD_CMD_7    7
#define K_SD_CMD_5    5
#define K_SD_CMD_0    0
#define K_SD_CMD_3    

#define SD_CMD_READ_SINGLE_BLOCK	  17
#define SD_CMD_READ_MULTIPLE_BLOCK	  18
#define SD_CMD_WRITE_SINGLE_BLOCK	  24
#define SD_CMD_WRITE_MULTIPLE_BLOCK	  25

/** SD Status define **/
#define K_SD_STATUS_CARD_PRESENT 0x01
#define K_SD_STATUS_IO_INTERRUPT 0x02

/* Host Register Offsets and Size */
#define HC_REG_SDMA_OFF           0x00
#define HC_REG_SDMA_LEN           0x04
#define HC_REG_BLOCK_SIZE_OFF     0x04
#define HC_REG_BLOCK_SIZE_LEN     0x02
#define HC_REG_BLOCK_COUNT_OFF    0x06
#define HC_REG_BLOCK_COUNT_LEN    0x02
#define HC_REG_ARGUMENT_OFF       0x08
#define HC_REG_ARGUMENT_LEN       0x04
#define HC_REG_TRANSFER_MODE_OFF  0x0C
#define HC_REG_TRANSFER_MODE_LEN  0x02
#define HC_REG_COMMAND_OFF        0x0E
#define HC_REG_COMMAND_LEN        0x02
#define HC_REG_RESPONSE_OFF       0x10
#define HC_REG_RESPONSE_LEN       0x16

#define HC_REG_NO_RESPONSE	  0x00
#define HC_REG_136B_RESPONSE      0x01
#define HC_REG_48B_RESPONSE	  0x02				
#define HC_REG_48B_BUSY_RESPONSE  0x03

#define HC_REG_CRC_ENABLE	  0x08
#define HC_REG_IDXCHK_ENABLE  0x10
#define HC_REG_DATA_PRESENT	  0x20

#define HC_REG_TRANSFER_MODE_1_HC_DMA         0x01
#define HC_REG_TRANSFER_MODE_1_BLK_CNT_ENABLE 0x02
#define HC_REG_TRANSFER_MODE_1_CMD12_ENABLE   0x04
#define HC_REG_TRANSFER_MODE_1_DATA_XFER_DIR  0x10
#define HC_REG_TRANSFER_MODE_1_MULTI_BLK_SEL  0x20
#define HC_REG_TRANSFER_MODE_READ             0x10
#define COMMAND_REG(_cmd_index, _flags, _response) (((_cmd_index) << 8) | ((_response) | (_flags)))

#define SDHostCmd52(hFile, arg, response) \
	SDHostCmd(hFile, 0, 0, arg, 0, COMMAND_REG(K_SD_CMD_52, HC_REG_CRC_ENABLE, HC_REG_48B_RESPONSE), response)

#define SDHostCmd52WithData(hFile, arg, data)  \
	SDHostCmdWithData(hFile, 0, 0, arg, 0, COMMAND_REG(K_SD_CMD_52, HC_REG_CRC_ENABLE, HC_REG_48B_RESPONSE), data, FALSE)

#define SDHostCmd53(hFile, block_size, block_count, arg, xfer_mode, response)  \
	SDHostCmd(hFile, block_size, block_count, arg, xfer_mode, COMMAND_REG(K_SD_CMD_53, (HC_REG_DATA_PRESENT | HC_REG_CRC_ENABLE), HC_REG_48B_RESPONSE), response)

#define SDHostCmd53WithData(hFile, block_size, block_count, arg, xfer_mode, data, write)  \
	SDHostCmdWithData(hFile, block_size, block_count, arg, xfer_mode, COMMAND_REG(K_SD_CMD_53, (HC_REG_DATA_PRESENT | HC_REG_CRC_ENABLE), HC_REG_48B_RESPONSE), data, write)


#define SDHostCmd18(hFile, block_size, block_count, arg, xfer_mode, response)  \
	SDHostCmd(hFile, block_size, block_count, arg, xfer_mode, COMMAND_REG(K_SD_CMD_18, (HC_REG_DATA_PRESENT | HC_REG_CRC_ENABLE), HC_REG_48B_RESPONSE), response)

#define SDHostCmd8(hFile, block_size, block_count, arg, xfer_mode, response)  \
	SDHostCmd(hFile, block_size, block_count, arg, xfer_mode, COMMAND_REG(8, (HC_REG_DATA_PRESENT | HC_REG_CRC_ENABLE), HC_REG_48B_RESPONSE), response)

#define SDHostCmd56(hFile, block_size, block_count, arg, xfer_mode, response)  \
	SDHostCmd(hFile, block_size, block_count, arg, xfer_mode, COMMAND_REG(56, (HC_REG_DATA_PRESENT | HC_REG_CRC_ENABLE), HC_REG_48B_RESPONSE), response)

#define SDHostCmd25(hFile, block_size, block_count, arg, xfer_mode, response)  \
	SDHostCmd(hFile, block_size, block_count, arg, xfer_mode, COMMAND_REG(K_SD_CMD_25, (HC_REG_DATA_PRESENT | HC_REG_CRC_ENABLE), HC_REG_48B_RESPONSE), response)

#define SDHostCmd2(hFile, response)  \
	SDHostCmd(hFile, 0, 0, 0, 0, COMMAND_REG(K_SD_CMD_2, HC_REG_CRC_ENABLE, HC_REG_136B_RESPONSE), response)

#define SDHostCmd12(hFile, response)  \
	SDHostCmd(hFile, 0, 0, 0, 0, COMMAND_REG(K_SD_CMD_12, HC_REG_CRC_ENABLE, HC_REG_48B_BUSY_RESPONSE), response)

#define SDHostCmd9(hFile, arg, response)  \
	SDHostCmd(hFile, 0, 0, arg, 0, COMMAND_REG(K_SD_CMD_9, HC_REG_CRC_ENABLE, HC_REG_136B_RESPONSE), response)

#define SDHostCmd10(hFile, arg, response)  \
	SDHostCmd(hFile, 0, 0, arg, 0, COMMAND_REG(K_SD_CMD_10, HC_REG_CRC_ENABLE, HC_REG_136B_RESPONSE), response)

#define SDHostCmd13(hFile, arg, response)  \
	SDHostCmd(hFile, 0, 0, arg, 0, COMMAND_REG(K_SD_CMD_13, HC_REG_CRC_ENABLE, HC_REG_48B_RESPONSE), response)

#define SDHostCmd6(hFile, arg, response)  \
	SDHostCmd(hFile, 0, 0, arg, 0, COMMAND_REG(K_SD_CMD_6, HC_REG_CRC_ENABLE, HC_REG_48B_RESPONSE), response)

#define SDHostCmd7(hFile, arg, response)  \
	SDHostCmd(hFile, 0, 0, arg, 0, COMMAND_REG(K_SD_CMD_7, HC_REG_CRC_ENABLE, HC_REG_48B_RESPONSE), response)

#define SDHostCmd5(hFile, arg, response)  \
	SDHostCmd(hFile, 0, 0, arg, 0, COMMAND_REG(K_SD_CMD_5, 0, HC_REG_48B_RESPONSE), response)

#define SDHostCmd60(hFile, arg, response)  \
	SDHostCmd(hFile, 0, 0, arg, 0, COMMAND_REG(60, HC_REG_CRC_ENABLE, HC_REG_48B_RESPONSE), response)

