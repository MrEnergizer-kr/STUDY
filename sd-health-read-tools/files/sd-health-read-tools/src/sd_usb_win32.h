#define FALSE (0)
#define TRUE (!(FALSE))

typedef union _CDB {
	struct _CDB6INQUIRY3 {
		uint8_t OperationCode;    // 0x12 - SCSIOP_INQUIRY
		uint8_t EnableVitalProductData : 1;
		uint8_t CommandSupportData : 1;
		uint8_t Reserved1 : 6;
		uint8_t PageCode;
		uint8_t Reserved2;
		uint8_t AllocationLength;
		uint8_t Control;
	} __attribute__ ((packed))  CDB6INQUIRY3;

	uint32_t AsUlong[4];
	uint8_t AsByte[16];
} __attribute__ ((packed)) CDB;

#define SCSIOP_INQUIRY                  0x12
#define INQUIRYDATABUFFERSIZE 36

typedef struct _SCSI_PASS_THROUGH_DIRECT {
    uint16_t Length;
    uint8_t ScsiStatus;
    uint8_t PathId;
    uint8_t TargetId;
    uint8_t Lun;
    uint8_t CdbLength;
    uint8_t SenseInfoLength;
    uint8_t DataIn;
    uint32_t DataTransferLength;
    uint32_t TimeOutValue;
    void *DataBuffer;
    uint32_t SenseInfoOffset;
    uint8_t Cdb[16];
} __attribute__ ((packed)) SCSI_PASS_THROUGH_DIRECT, *PSCSI_PASS_THROUGH_DIRECT;

#define SCSI_IOCTL_DATA_OUT          0
#define SCSI_IOCTL_DATA_IN           1
#define SCSI_IOCTL_DATA_UNSPECIFIED  2


typedef struct _SENSE_DATA {
	uint8_t ErrorCode:7;
	uint8_t Valid:1;
	uint8_t SegmentNumber;
	uint8_t SenseKey:4;
	uint8_t Reserved:1;
	uint8_t IncorrectLength:1;
	uint8_t EndOfMedia:1;
	uint8_t FileMark:1;
	uint8_t Information[4];
	uint8_t AdditionalSenseLength;
	uint8_t CommandSpecificInformation[4];
	uint8_t AdditionalSenseCode;
	uint8_t AdditionalSenseCodeQualifier;
	uint8_t FieldReplaceableUnitCode;
	uint8_t SenseKeySpecific[3];
} __attribute__ ((packed)) SENSE_DATA, *PSENSE_DATA;

#define SENSE_BUFFER_SIZE 18
#define MAX_SENSE_BUFFER_SIZE 255

#define SCSISTAT_GOOD                  0x00
#define SCSISTAT_CHECK_CONDITION       0x02
#define SCSISTAT_CONDITION_MET         0x04
#define SCSISTAT_BUSY                  0x08
#define SCSISTAT_INTERMEDIATE          0x10
#define SCSISTAT_INTERMEDIATE_COND_MET 0x14
#define SCSISTAT_RESERVATION_CONFLICT  0x18
#define SCSISTAT_COMMAND_TERMINATED    0x22
#define SCSISTAT_QUEUE_FULL            0x28


typedef struct _INQUIRYDATA {
	uint8_t DeviceType : 5;
	uint8_t DeviceTypeQualifier : 3;
	uint8_t DeviceTypeModifier : 7;
	uint8_t RemovableMedia : 1;
	union {
		uint8_t Versions;
		struct {
			uint8_t ANSIVersion : 3;
			uint8_t ECMAVersion : 3;
			uint8_t ISOVersion : 2;
		} __attribute__ ((packed)) ;
	} __attribute__ ((packed)) ;
	uint8_t ResponseDataFormat : 4;
	uint8_t HiSupport : 1;
	uint8_t NormACA : 1;
	uint8_t TerminateTask : 1;
	uint8_t AERC : 1;
	uint8_t AdditionalLength;
	uint8_t Reserved;
	uint8_t Addr16 : 1;               // defined only for SIP devices.
	uint8_t Addr32 : 1;               // defined only for SIP devices.
	uint8_t AckReqQ: 1;               // defined only for SIP devices.
	uint8_t MediumChanger : 1;
	uint8_t MultiPort : 1;
	uint8_t ReservedBit2 : 1;
	uint8_t EnclosureServices : 1;
	uint8_t ReservedBit3 : 1;
	uint8_t SoftReset : 1;
	uint8_t CommandQueue : 1;
	uint8_t TransferDisable : 1;      // defined only for SIP devices.
	uint8_t LinkedCommands : 1;
	uint8_t Synchronous : 1;          // defined only for SIP devices.
	uint8_t Wide16Bit : 1;            // defined only for SIP devices.
	uint8_t Wide32Bit : 1;            // defined only for SIP devices.
	uint8_t RelativeAddressing : 1;
	uint8_t VendorId[8];
	uint8_t ProductId[16];
	uint8_t ProductRevisionLevel[4];
	uint8_t VendorSpecific[20];
	uint8_t Reserved3[40];
} __attribute__ ((packed))  INQUIRYDATA, *PINQUIRYDATA;

#define CloseHandle scsi_pt_close_device
#define ZeroMemory(var, len) memset(var, 0, len)

#ifdef __cplusplus
inline uint8_t HIBYTE(uint16_t value) {
	return ((value & 0xFF00) >> 8);
}

inline uint8_t LOBYTE(uint16_t value) {
	return (value & 0xFF);
}

inline uint16_t HIWORD(uint32_t value) {
	return ((value & 0xFFFF0000) >> 16);
}

inline uint16_t LOWORD(uint32_t value) {
	return (value & 0xFFFF);
}
#else
#define HIBYTE(value)	(((value) & 0xFF00) >> 8)
#define LOBYTE(value)	((value) & 0xFF)
#define HIWORD(value)	(((value) & 0xFFFF0000) >> 16)
#define LOWORD(value)	((value) & 0xFFFF)
#endif /* __cplusplus */
