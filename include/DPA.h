// *********************************************************************
//   General public DPA header file                                    *
// *********************************************************************
// Copyright (c) IQRF Tech s.r.o.
//
// File:    $RCSfile: DPA.h,v $
// Version: $Revision: 1.291 $
// Date:    $Date: 2021/01/27 18:15:35 $
//
// Revision history:
//   2021/01/22  Release for DPA 4.16
//   2020/09/03  Release for DPA 4.15
//   2020/04/03  Release for DPA 4.14
//   2020/02/27  Release for DPA 4.13
//   2020/01/09  Release for DPA 4.12
//   2019/12/11  Release for DPA 4.11
//   2019/10/09  Release for DPA 4.10
//   2019/06/12  Release for DPA 4.03
//   2019/06/03  Release for DPA 4.02
//   2019/03/07  Release for DPA 4.01
//   2019/01/10  Release for DPA 4.00
//   2018/10/25  Release for DPA 3.03
//   2017/11/16  Release for DPA 3.02
//   2017/08/14  Release for DPA 3.01
//   2017/03/13  Release for DPA 3.00
//   2016/09/12  Release for DPA 2.28
//   2016/04/14  Release for DPA 2.27
//   2016/03/03  Release for DPA 2.26
//   2016/01/21  Release for DPA 2.25
//   2015/12/01  Release for DPA 2.24
//   2015/10/23  Release for DPA 2.23
//   2015/09/25  Release for DPA 2.22
//   2015/09/03  Release for DPA 2.21
//   2015/08/05  Release for DPA 2.20
//   2014/10/31  Release for DPA 2.10
//   2014/04/30  Release for DPA 2.00
//   2013/10/03  Release for DPA 1.00
//
// *********************************************************************

// Online DPA documentation https://doc.iqrf.org/DpaTechGuide/
// IQRF Standards documentation https://www.iqrfalliance.org/iqrf-interoperability/

#ifndef _DPA_HEADER_
#define _DPA_HEADER_

#pragma pack( push, 1 )
//############################################################################################

// DPA version
#define	DPA_VERSION_MASTER			0x0416

#if defined( __CC5X__ ) && !defined( CC5XnotDPA )
// Compiled only at CC5X
#if __CC5X__ < 3703
#error Insufficient CC5X compiler version, V3.7C is minimum
#endif

#if IQRFOS < 404
#error IQRF OS 4.04+ is required
#endif

// Bank for custom variables
#pragma rambank = UserBank_01

// Main DPA API entry address (also start of the licensed FLASH)
#define	DPA_API_ADDRESS				__LICENSED_FLASH

// Main DPA entry address
#define	MAIN_DPA_ADDRESS			( DPA_API_ADDRESS + 4 )

// Main DPA API entry address stub
#define	DPA_API_ADDRESS_ENTRY		0x3A08

// Address of the DPA Custom Handler
#define	CUSTOM_HANDLER_ADDRESS		0x3A20

// Address of the DPA Custom Handler end + 1
#define	CUSTOM_HANDLER_ADDRESS_END	0x3D80

// DPA API entry function
uns8  DpaApiEntry( uns8 par1, uns8 par2, uns8 apiIndex );

// DPA API codes
#define	DPA_API_RFTX_DPAPACKET				0
#define	DPA_API_READ_CONFIG_BYTE			1
#define	DPA_API_SEND_TO_IFACEMASTER			2
#define	DPA_API_COORDINATOR_RFTX_DPAPACKET	3
#define	DPA_API_LOCAL_REQUEST				4
#define	DPA_API_SET_PERIPHERAL_ERROR		5
#define	DPA_API_SET_RF_DEFAULTS				6
#define DPA_API_LOCAL_FRC                   7
#define DPA_API_CRC8                        8
#define DPA_API_AGGREGATE_FRC               9

// Used buffer size symbols
#define	sizeofBufferRF						sizeof( bufferRF )
#define	sizeofBufferAUX						sizeof( bufferAUX )
#define	sizeofBufferCOM						sizeof( bufferCOM )
#define sizeofBufferINFO                    sizeof( bufferINFO )

// Converts conventional address to the linear one at PIC16F1938
// Note: This is workaround of CC5X preprocessor bug "No '#endif' detected"
#define error_ #error Address cannot be linearized
#define LINEARIZE_ADDRESS( a ) \
#if ( ((uns16)(a)) - ( ((uns16)(a)) / 0x80 ) * 0x80 ) >= 0x20 && ( ((uns16)(a)) - ( ((uns16)(a)) / 0x80 ) * 0x80 ) <= 0x6F && ( ((uns16)(a)) / 0x80 ) <= 12 \
( ( ( ((uns16)(a)) / 0x80 ) * 0x50 ) + ( ((uns16)(a)) - 0x20 - ( ((uns16)(a)) / 0x80 ) * 0x80 ) + 0x2000 ) \
#else \
errorX \
#endif
#undef error_

#define	STRUCTATTR

#else //__CC5X__
// Not compiled at CC5X

// Define CC5X types
typedef uint8_t	  uns8;
typedef uint16_t  uns16;

// Define some types missing at Arduino
typedef int8_t  int8;
typedef int16_t int16;

// Fake buffer sizes
#define	sizeofBufferRF    64
#define	sizeofBufferAUX   64
#define	sizeofBufferCOM   64
#define sizeofBufferINFO  64

// Disables alignment of members of structures
#if defined WIN32 || defined _WIN32 || defined _WIN64
#define WIN
#endif

#ifndef WIN
#define	STRUCTATTR	__attribute__((packed))
#else
#define	STRUCTATTR
#endif

#endif	// __CC5X__

// Indexes of configuration bytes used by DpaApiReadConfigByte( index )
// Checksum
#define	CFGIND_CHECKSUM			0x00
// Embedded peripherals
#define	CFGIND_DPA_PERIPHERALS	0x01
// DPA configuration flags #0
#define CFGIND_DPA_FLAGS0       0x05
// Main RF channel, used by the subordinate network
#define	CFGIND_CHANNEL_2ND_A	0x06
// Second RF channel, used by the subordinate network
#define	CFGIND_CHANNEL_2ND_B	0x07
// TX power
#define	CFGIND_TXPOWER 			0x08
// RX filter used by checkRF()
#define	CFGIND_RXFILTER			0x09
// toutRF for LP mode
#define	CFGIND_DPA_LP_TOUTRF	0x0A
// UART interface baud rate
#define	CFGIND_DPA_UART_IFACE_SPEED 0x0B
// Alternate DSM channel
#define	CFGIND_ALTERNATE_DSM_CHANNEL 0x0C
// DPA configuration flags #1
#define CFGIND_DPA_FLAGS1       0x0D
// Main RF channel
#define	CFGIND_CHANNEL_A		0x11
// Second RF channel
#define	CFGIND_CHANNEL_B		0x12

// 0: checks only mandatory precondition in order to prevent critical side-effects
// 1: as above plus checks meaningful parameter conditions
// 2: full implemented parameter checking (default)
#ifndef PARAM_CHECK_LEVEL
#define	PARAM_CHECK_LEVEL 2
#endif

// "foursome" at IFace structure
typedef struct
{
  // Node address low byte
  uns8	NADRlow;
  // Node address high byte
  uns8	NADRhigh;
  // Peripheral number
  uns8	PNUM;
  // Peripheral command
  uns8	PCMD;
  // HWPID
  uns16	HWPID;
} STRUCTATTR TDpaIFaceHeader;

// Maximum command PCMD value (except reserved 0x3F = CMD_GET_PER_INFO)
#define	PCMD_MAX					0x7f
// Bit mask at PCMD that indicates DPA Response message
#define	RESPONSE_FLAG				0x80

// IQMESH coordinator address
#define COORDINATOR_ADDRESS			0x00
// IQMESH broadcast address
#define BROADCAST_ADDRESS			0xff
// IQMESH temporary address, assigned by pre-bonding before authorization is done
#define TEMPORARY_ADDRESS			0xfe
// Address of the local device addressed by IFace
#define LOCAL_ADDRESS				0xfc
// Maximum IQMESH network device address
#define MAX_ADDRESS					( 240 - 1 )

// Time slots lengths in 10 ms
#define	MIN_STD_TIMESLOT	4
#define	MAX_STD_TIMESLOT	6

#define	MIN_LP_TIMESLOT		8
#define	MAX_LP_TIMESLOT		10

// Maximum number of DPA PData bytes ( minus 8 = 6B foursome + 8b error code + 8b DpaValue )
#define DPA_MAX_DATA_LENGTH			( sizeofBufferCOM - sizeof( TDpaIFaceHeader ) - 2 * sizeof( uns8 ) )

// Maximum number of peripherals info that can fit in the message
#define	MAX_PERIPHERALS_PER_BLOCK_INFO	( DPA_MAX_DATA_LENGTH / sizeof( TPeripheralInfoAnswer ) )

// Embedded peripheral numbers
#define	PNUM_COORDINATOR	0x00
#define	PNUM_NODE			0x01
#define	PNUM_OS				0x02
#define	PNUM_EEPROM			0x03
#define	PNUM_EEEPROM		0x04
#define	PNUM_RAM			0x05
#define	PNUM_LEDR			0x06
#define	PNUM_LEDG			0x07
#define	PNUM_SPI			0x08
#define	PNUM_IO				0x09
#define	PNUM_THERMOMETER	0x0A
#define	PNUM_PWM			0x0B
#define	PNUM_UART			0x0C
#define	PNUM_FRC			0x0D

// Number of the 1st user peripheral
#define	PNUM_USER			0x20
// Number of the last user peripheral
#define	PNUM_USER_MAX		0x3E
// Maximum peripheral number
#define	PNUM_MAX			0x7F

// Fake peripheral number used to flag DPA response with error sent by RF
#define	PNUM_ERROR_FLAG		0xFE
// Special peripheral used for enumeration
#define	PNUM_ENUMERATION	0xFF

// DPA Commands for embedded peripherals
#define	CMD_COORDINATOR_ADDR_INFO  0
#define	CMD_COORDINATOR_DISCOVERED_DEVICES 1
#define	CMD_COORDINATOR_BONDED_DEVICES 2
#define	CMD_COORDINATOR_CLEAR_ALL_BONDS 3
#define	CMD_COORDINATOR_BOND_NODE 4
#define	CMD_COORDINATOR_REMOVE_BOND 5
#define	CMD_COORDINATOR_DISCOVERY 7
#define	CMD_COORDINATOR_SET_DPAPARAMS 8
#define	CMD_COORDINATOR_SET_HOPS 9
#define	CMD_COORDINATOR_BACKUP 11
#define	CMD_COORDINATOR_RESTORE 12
#define	CMD_COORDINATOR_AUTHORIZE_BOND 13
#define	CMD_COORDINATOR_BRIDGE 14
#define	CMD_COORDINATOR_SMART_CONNECT 18
#define	CMD_COORDINATOR_SET_MID 19

#define	CMD_NODE_READ 0
#define	CMD_NODE_REMOVE_BOND 1
#define	CMD_NODE_BACKUP 6
#define	CMD_NODE_RESTORE 7
#define	CMD_NODE_VALIDATE_BONDS 8

#define	CMD_OS_READ 0
#define	CMD_OS_RESET 1
#define	CMD_OS_READ_CFG 2
#define	CMD_OS_RFPGM 3
#define	CMD_OS_SLEEP 4
#define	CMD_OS_BATCH 5
#define	CMD_OS_SET_SECURITY 6
#define	CMD_OS_INDICATE 7
#define	CMD_OS_RESTART 8
#define	CMD_OS_WRITE_CFG_BYTE 9
#define	CMD_OS_LOAD_CODE 10
#define	CMD_OS_SELECTIVE_BATCH 11
#define	CMD_OS_TEST_RF_SIGNAL 12
#define	CMD_OS_FACTORY_SETTINGS 13
#define	CMD_OS_WRITE_CFG 15

#define	CMD_RAM_READ 0
#define	CMD_RAM_WRITE 1
#define	CMD_RAM_READ_ANY    15

#define	CMD_EEPROM_READ CMD_RAM_READ
#define	CMD_EEPROM_WRITE CMD_RAM_WRITE

#define	CMD_EEEPROM_XREAD ( CMD_RAM_READ + 2 )
#define	CMD_EEEPROM_XWRITE ( CMD_RAM_WRITE + 2 )

#define	CMD_LED_SET_OFF 0
#define	CMD_LED_SET_ON 1
#define	CMD_LED_PULSE 3
#define	CMD_LED_FLASHING 4

#define	CMD_IO_DIRECTION  0
#define	CMD_IO_SET	1
#define	CMD_IO_GET	2

#define	CMD_THERMOMETER_READ 0

#define	CMD_UART_OPEN 0
#define	CMD_UART_CLOSE 1
#define	CMD_UART_WRITE_READ 2
#define	CMD_UART_CLEAR_WRITE_READ 3

#define	CMD_FRC_SEND 0
#define	CMD_FRC_EXTRARESULT 1
#define	CMD_FRC_SEND_SELECTIVE 2
#define	CMD_FRC_SET_PARAMS 3

#define	CMD_GET_PER_INFO  0x3f

// DPA peripheral type
typedef enum
{
  PERIPHERAL_TYPE_DUMMY = 0x00,
  PERIPHERAL_TYPE_COORDINATOR = 0x01,
  PERIPHERAL_TYPE_NODE = 0x02,
  PERIPHERAL_TYPE_OS = 0x03,
  PERIPHERAL_TYPE_EEPROM = 0x04,
  PERIPHERAL_TYPE_BLOCK_EEPROM = 0x05,
  PERIPHERAL_TYPE_RAM = 0x06,
  PERIPHERAL_TYPE_LED = 0x07,
  PERIPHERAL_TYPE_SPI = 0x08,
  PERIPHERAL_TYPE_IO = 0x09,
  PERIPHERAL_TYPE_UART = 0x0a,
  PERIPHERAL_TYPE_THERMOMETER = 0x0b,
  PERIPHERAL_TYPE_ADC = 0x0c,
  PERIPHERAL_TYPE_PWM = 0x0d,
  PERIPHERAL_TYPE_FRC = 0x0e,
  // Starts peripheral type number interval for user peripherals
  PERIPHERAL_TYPE_USER_AREA = 0x80
} TDpaPeripheralType;

// Peripheral extended information
typedef enum
{
  PERIPHERAL_TYPE_EXTENDED_DEFAULT = 0x00,
  PERIPHERAL_TYPE_EXTENDED_READ = 0x01,
  PERIPHERAL_TYPE_EXTENDED_WRITE = 0x02,
  PERIPHERAL_TYPE_EXTENDED_READ_WRITE = PERIPHERAL_TYPE_EXTENDED_READ | PERIPHERAL_TYPE_EXTENDED_WRITE
} TDpaPeripheralTypeExtended;

// Response packet error codes
typedef enum
{
  // No error
  STATUS_NO_ERROR = 0,

  // General fail
  ERROR_FAIL = 1,
  // Incorrect PCMD
  ERROR_PCMD = 2,
  // Incorrect PNUM or PCMD
  ERROR_PNUM = 3,
  // Incorrect Address value when addressing memory type peripherals
  ERROR_ADDR = 4,
  // Incorrect Data length
  ERROR_DATA_LEN = 5,
  // Incorrect Data
  ERROR_DATA = 6,
  // Incorrect HWPID used
  ERROR_HWPID = 7,
  // Incorrect NADR
  ERROR_NADR = 8,
  // IFACE data consumed by Custom DPA Handler
  ERROR_IFACE_CUSTOM_HANDLER = 9,
  // Custom DPA Handler is missing
  ERROR_MISSING_CUSTOM_DPA_HANDLER = 10,

  // Beginning of the user code error interval
  ERROR_USER_FROM = 0x20,
  // End of the user code error interval
  ERROR_USER_TO = 0x3f,

  // Bit/flag reserved for a future use
  STATUS_RESERVED_FLAG = 0x40,
  // Bit to flag asynchronous DPA Response from [N]
  STATUS_ASYNC_RESPONSE = 0x80,
  // Error code used to mark DPA Confirmation
  STATUS_CONFIRMATION = 0xff
} TErrorCodes;

// Embedded FRC commands
typedef enum
{
  // 2 bits
  FRC_Ping = 0x00,
  FRC_AcknowledgedBroadcastBits = 0x02,
  FRC_PrebondedAlive = 0x03,
  FRC_SupplyVoltage = 0x04,
  FRC_PrebondedMemoryCompare2B = 0x05,
  // 1 byte
  FRC_Temperature = 0x80,
  FRC_AcknowledgedBroadcastBytes = 0x81,
  FRC_MemoryRead = 0x82,
  FRC_MemoryReadPlus1 = 0x83,
  FRC_FrcResponseTime = 0x84,
  FRC_TestRFsignal = 0x85,
  // 4 bytes
  FRC_PrebondedMemoryRead4BPlus1 = 0xF8,
  FRC_MemoryRead4B = 0xFA
} TFRCommands;

// Intervals of user FRC codes
#define	FRC_USER_BIT_FROM	  0x40
#define	FRC_USER_BIT_TO		  0x7F
#define	FRC_USER_BYTE_FROM	  0xC0
#define	FRC_USER_BYTE_TO	  0xDF
#define	FRC_USER_2BYTE_FROM	  0xF0
#define	FRC_USER_2BYTE_TO	  0xF7
#define	FRC_USER_4BYTE_FROM	  0xFC
#define	FRC_USER_4BYTE_TO	  0xFF

// No HWPID specified
#define HWPID_Default         0x0000
// Use this type to override HWPID check
#define HWPID_DoNotCheck      0xFfFf

// RAM peripheral block definitions
#define	PERIPHERAL_RAM_LENGTH		48

// Start address of EEPROM peripheral in the real EEPROM
#ifndef COORDINATOR_CUSTOM_HANDLER // Node
#define	PERIPHERAL_EEPROM_START		( (uns8)0x00 )
#else // Coordinator
#define	PERIPHERAL_EEPROM_START		( (uns8)0x80 )
#endif

// Length of the internal EEPROM peripheral array
#define PERIPHERAL_EEPROM_LENGTH            ( (uns8)( 0xC0 - PERIPHERAL_EEPROM_START ) )

// Length of the readable area of serial EEEPROM from the EEEPROM DPA peripheral write point of view.
#define	EEEPROM_READ_LENGTH					0x8000
// Length of the writable area of serial EEEPROM from the EEEPROM DPA peripheral write point of view.
#define	EEEPROM_WRITE_LENGTH				0x4000

// Starting address of the IO Setup DPA storage at external EEPROM
#define IOSETUP_EEEPROM_ADDR                ( 0x0000 + sizeofBufferAUX )
// Length of the IO setup memory block
#define IOSETUP_LENGTH                      sizeofBufferAUX

// ---------------------------------------------------------

// Enumerate peripherals structure
typedef struct
{
  uns16	DpaVersion;
  uns8	UserPerNr;
  uns8	EmbeddedPers[PNUM_USER / 8];
  uns16	HWPID;
  uns16	HWPIDver;
  uns8	Flags;
  uns8	UserPer[( PNUM_MAX - PNUM_USER + 1 + 7 ) / 8];
} STRUCTATTR TEnumPeripheralsAnswer;

#define	FlagUserPer(UserPersArray,UserPerNumber)	UserPersArray[((UserPerNumber)-PNUM_USER) / 8] |= (uns8)0x01 << (((UserPerNumber)-PNUM_USER) % 8);

// Get peripheral info structure (CMD_GET_PER_INFO)
typedef struct
{
  uns8	PerTE;
  uns8	PerT;
  uns8	Par1;
  uns8	Par2;
} STRUCTATTR TPeripheralInfoAnswer;

// Error DPA response (PNUM_ERROR_FLAG)
typedef struct
{
  uns8	ErrN;
  uns8	PNUMoriginal;
} STRUCTATTR TErrorAnswer;

// Structure returned by CMD_COORDINATOR_ADDR_INFO
typedef struct
{
  uns8	DevNr;
  uns8	DID;
} STRUCTATTR TPerCoordinatorAddrInfo_Response;

// Structure for CMD_COORDINATOR_BOND_NODE
typedef struct
{
  uns8	ReqAddr;
  union {
    struct {
      uns8	BondingMask;
    } Previous;
    struct {
      uns8	BondingTestRetries;
    } Current;
  } BondNode;
} STRUCTATTR TPerCoordinatorBondNode_Request;

// Structure returned by CMD_COORDINATOR_BOND_NODE or CMD_COORDINATOR_SMART_CONNECT
typedef struct
{
  uns8	BondAddr;
  uns8	DevNr;
} STRUCTATTR TPerCoordinatorBondNodeSmartConnect_Response;

// Structure for CMD_COORDINATOR_REMOVE_BOND
typedef struct
{
  uns8	BondAddr;
} STRUCTATTR TPerCoordinatorRemoveBond_Request;

// Structure returned by CMD_COORDINATOR_REMOVE_BOND
typedef struct
{
  uns8  DevNr;
} STRUCTATTR TPerCoordinatorRemoveBond_Response;

// Structure for CMD_COORDINATOR_DISCOVERY
typedef struct
{
  uns8	TxPower;
  uns8	MaxAddr;
} STRUCTATTR TPerCoordinatorDiscovery_Request;

// Structure returned by CMD_COORDINATOR_DISCOVERY
typedef struct
{
  uns8	DiscNr;
} STRUCTATTR TPerCoordinatorDiscovery_Response;

// Structure for and also returned by CMD_COORDINATOR_SET_DPAPARAMS
typedef struct
{
  uns8	DpaParam;
} STRUCTATTR TPerCoordinatorSetDpaParams_Request_Response;

// Structure for and also returned by CMD_COORDINATOR_SET_HOPS
typedef struct
{
  uns8	RequestHops;
  uns8	ResponseHops;
} STRUCTATTR TPerCoordinatorSetHops_Request_Response;

// Structure for CMD_COORDINATOR_BACKUP and CMD_NODE_BACKUP
typedef struct
{
  uns8	Index;
} STRUCTATTR TPerCoordinatorNodeBackup_Request;

// Structure returned by CMD_COORDINATOR_BACKUP and CMD_NODE_BACKUP
typedef struct
{
  uns8	NetworkData[49];
} STRUCTATTR TPerCoordinatorNodeBackup_Response;

// Structure for CMD_COORDINATOR_RESTORE and CMD_NODE_RESTORE
typedef struct
{
  uns8	NetworkData[49];
} STRUCTATTR TPerCoordinatorNodeRestore_Request;

// Structure for CMD_COORDINATOR_AUTHORIZE_BOND
typedef struct
{
  uns8	ReqAddr;
  uns8	MID[4];
} STRUCTATTR TPerCoordinatorAuthorizeBond_Request;

// Structure returned by CMD_COORDINATOR_AUTHORIZE_BOND
typedef struct
{
  uns8	BondAddr;
  uns8	DevNr;
} STRUCTATTR TPerCoordinatorAuthorizeBond_Response;

// Structure for CMD_COORDINATOR_BRIDGE
typedef struct
{
  TDpaIFaceHeader subHeader;
  uns8	subPData[DPA_MAX_DATA_LENGTH - sizeof( TDpaIFaceHeader )];
} STRUCTATTR TPerCoordinatorBridge_Request;

// Structure returned by CMD_COORDINATOR_BRIDGE
typedef struct
{
  TDpaIFaceHeader subHeader;
  uns8	subRespCode;
  uns8	subDpaValue;
  uns8	subPData[DPA_MAX_DATA_LENGTH - sizeof( TDpaIFaceHeader ) - 2 * sizeof( uns8 )];
} STRUCTATTR TPerCoordinatorBridge_Response;

// Structure for CMD_COORDINATOR_SMART_CONNECT
typedef struct
{
  uns8  ReqAddr;
  uns8  BondingTestRetries;
  uns8  IBK[16];
  uns8  MID[4];
  uns8  reserved0;
  uns8  VirtualDeviceAddress;
  uns8  UserData[4];
  uns8  reserved1[10];
} STRUCTATTR TPerCoordinatorSmartConnect_Request;

// Structure for CMD_COORDINATOR_SET_MID
typedef struct
{
  uns8  MID[4];
  uns8	BondAddr;
} STRUCTATTR TPerCoordinatorSetMID_Request;

// Structure returned by CMD_NODE_READ
typedef struct
{
  uns8  ntwADDR;
  uns8  ntwVRN;
  uns8  ntwZIN;
  uns8  ntwDID;
  uns8  ntwPVRN;
  uns16 ntwUSERADDRESS;
  uns16 ntwID;
  uns8  ntwVRNFNZ;
  uns8  ntwCFG;
  uns8  Flags;
} STRUCTATTR TPerNodeRead_Response;

// Structures for CMD_NODE_VALIDATE_BONDS
typedef struct
{
  uns8	Address;
  uns8  MID[4];
} STRUCTATTR TPerNodeValidateBondsItem;

// Structure for CMD_NODE_VALIDATE_BONDS
typedef struct
{
  TPerNodeValidateBondsItem Bonds[DPA_MAX_DATA_LENGTH / sizeof( TPerNodeValidateBondsItem )];
} STRUCTATTR TPerNodeValidateBonds_Request;

// Structure returned by CMD_OS_READ
typedef struct
{
  uns8	MID[4];
  uns8	OsVersion;
  uns8	McuType;
  uns16	OsBuild;
  uns8	Rssi;
  uns8	SupplyVoltage;
  uns8	Flags;
  uns8	SlotLimits;
  uns8  IBK[16];
  // Enumerate peripherals part, variable length because of UserPer field
  uns16	DpaVersion;
  uns8	UserPerNr;
  uns8	EmbeddedPers[PNUM_USER / 8];
  uns16	HWPID;
  uns16	HWPIDver;
  uns8	FlagsEnum;
  uns8	UserPer[( PNUM_MAX - PNUM_USER + 1 + 7 ) / 8];
} STRUCTATTR TPerOSRead_Response;

// Structure returned by CMD_OS_READ_CFG
typedef struct
{
  uns8	Checksum;
  uns8	Configuration[31];
  uns8	RFPGM;
  uns8	Undocumented[1];
} STRUCTATTR TPerOSReadCfg_Response;

// Structure for CMD_OS_WRITE_CFG
typedef struct
{
  uns8	Undefined;
  uns8	Configuration[31];
  uns8	RFPGM;
} STRUCTATTR TPerOSWriteCfg_Request;

// Structures for CMD_OS_WRITE_CFG_BYTE
typedef struct
{
  uns8	Address;
  uns8	Value;
  uns8	Mask;
} STRUCTATTR TPerOSWriteCfgByteTriplet;

// Structure for CMD_OS_WRITE_CFG_BYTE
typedef struct
{
  TPerOSWriteCfgByteTriplet Triplets[DPA_MAX_DATA_LENGTH / sizeof( TPerOSWriteCfgByteTriplet )];
} STRUCTATTR TPerOSWriteCfgByte_Request;

// Structure for CMD_OS_SET_SECURITY
typedef struct
{
  uns8	Type;
  uns8	Data[16];
} STRUCTATTR TPerOSSetSecurity_Request;

// Structure for CMD_OS_LOAD_CODE
typedef struct
{
  uns8	Flags;
  uns16	Address;
  uns16	Length;
  uns16	CheckSum;
} STRUCTATTR TPerOSLoadCode_Request;

// Structure for CMD_OS_SLEEP
typedef struct
{
  uns16	Time;
  uns8	Control;
} STRUCTATTR TPerOSSleep_Request;

// Structure for CMD_OS_SELECTIVE_BATCH
typedef struct
{
  uns8	SelectedNodes[30];
  uns8	Requests[DPA_MAX_DATA_LENGTH - 30];
} STRUCTATTR TPerOSSelectiveBatch_Request;

// Structure for CMD_OS_TEST_RF_SIGNAL request
typedef struct
{
  uns8  Channel;
  uns8  RXfilter;
  uns16 Time;
} STRUCTATTR TPerOSTestRfSignal_Request;

// Structure for CMD_OS_TEST_RF_SIGNAL response
typedef struct
{
  uns8  Counter;
} STRUCTATTR TPerOSTestRfSignal_Response;

// Structure for CMD_OS_INDICATE request
typedef struct
{
  uns8  Control;
} STRUCTATTR TPerOSIndicate_Request;

// Structure for general memory request
typedef struct
{
  // Address of data to write or read
  uns8	Address;

  union
  {
    // Memory read request
    struct
    {
      // Length of data to read
      uns8	Length;
    } Read;

    // Size of Address field
#define	MEMORY_WRITE_REQUEST_OVERHEAD	( sizeof( uns8 ) )

    // Memory write request
    struct
    {
      uns8	PData[DPA_MAX_DATA_LENGTH - MEMORY_WRITE_REQUEST_OVERHEAD];
    } Write;

  } ReadWrite;
} STRUCTATTR TPerMemoryRequest;

// Structure for general extended memory request
typedef struct
{
  // Address of data to write or read
  uns16	Address;

  union
  {
    // Memory read request
    struct
    {
      // Length of data to read
      uns8	Length;
    } Read;

    // Size of Address field
#define	XMEMORY_WRITE_REQUEST_OVERHEAD	( sizeof( uns16 ) )

    // Memory write request
    struct
    {
      uns8	PData[DPA_MAX_DATA_LENGTH - XMEMORY_WRITE_REQUEST_OVERHEAD];
    } Write;

  } ReadWrite;
} STRUCTATTR TPerXMemoryRequest;

// Structure for CMD_IO requests
typedef struct
{
  uns8  Port;
  uns8  Mask;
  uns8  Value;
} STRUCTATTR TPerIOTriplet;

typedef struct
{
  uns8  Header;	// == PNUM_IO_DELAY
  uns16 Delay;
} STRUCTATTR TPerIODelay;

// Union for CMD_IO_SET and CMD_IO_DIRECTION requests
typedef union
{
  TPerIOTriplet Triplets[DPA_MAX_DATA_LENGTH / sizeof( TPerIOTriplet )];
  TPerIODelay   Delays[DPA_MAX_DATA_LENGTH / sizeof( TPerIODelay )];
} STRUCTATTR TPerIoDirectionAndSet_Request;

// Structure returned by CMD_THERMOMETER_READ
typedef struct
{
  int8  IntegerValue;
  int16 SixteenthValue;
} STRUCTATTR TPerThermometerRead_Response;

// Structure for CMD_UART_OPEN
typedef struct
{
  uns8  BaudRate;
} STRUCTATTR TPerUartOpen_Request;

// Structure for CMD_UART_[CLEAR_]WRITE_READ
typedef struct
{
  uns8  ReadTimeout;
  uns8  WrittenData[DPA_MAX_DATA_LENGTH - sizeof(uns8)];
} STRUCTATTR TPerUartWriteRead_Request;

// Structure for CMD_FRC_SEND
typedef struct
{
  uns8  FrcCommand;
  uns8	UserData[30];
} STRUCTATTR TPerFrcSend_Request;

// Structure for CMD_FRC_SEND_SELECTIVE
typedef struct
{
  uns8  FrcCommand;
  uns8	SelectedNodes[30];
  uns8	UserData[25];
} STRUCTATTR TPerFrcSendSelective_Request;

// Structure returned by CMD_FRC_SEND and CMD_FRC_SEND_SELECTIVE
typedef struct
{
  uns8  Status;
  uns8	FrcData[DPA_MAX_DATA_LENGTH - sizeof( uns8 )];
} STRUCTATTR TPerFrcSend_Response;

// Structure for request and response of CMD_FRC_SET_PARAMS
typedef struct
{
  uns8	FrcParams;
} STRUCTATTR TPerFrcSetParams_RequestResponse;

// Interface and CMD_COORDINATOR_BRIDGE confirmation structure
typedef struct
{
  // Number of hops
  uns8  Hops;
  // Time slot length in 10ms
  uns8  TimeSlotLength;
  // Number of hops for response
  uns8  HopsResponse;
} STRUCTATTR TIFaceConfirmation;

// ---------------------------------------------------------

// DPA Message data structure (packet w/o NADR, PNUM, PCMD, HWPID)
typedef union
{
  // General DPA request
  struct
  {
    uns8	PData[DPA_MAX_DATA_LENGTH];
  } Request;

  // General DPA response
  struct
  {
    uns8	PData[DPA_MAX_DATA_LENGTH];
  } Response;

  // Enumerate peripherals structure
  TEnumPeripheralsAnswer EnumPeripheralsAnswer;

  // Get peripheral info structure (CMD_GET_PER_INFO)
  TPeripheralInfoAnswer PeripheralInfoAnswer;

  // Get peripheral info structure (CMD_GET_PER_INFO) for more peripherals
  TPeripheralInfoAnswer PeripheralInfoAnswers[MAX_PERIPHERALS_PER_BLOCK_INFO];

  // Error DPA response (PNUM_ERROR_FLAG)
  TErrorAnswer ErrorAnswer;

  // Structure returned by CMD_COORDINATOR_ADDR_INFO
  TPerCoordinatorAddrInfo_Response PerCoordinatorAddrInfo_Response;

  // Structure for CMD_COORDINATOR_BOND_NODE
  TPerCoordinatorBondNode_Request PerCoordinatorBondNode_Request;

  // Structure returned by CMD_COORDINATOR_BOND_NODE or CMD_COORDINATOR_SMART_CONNECT
  TPerCoordinatorBondNodeSmartConnect_Response PerCoordinatorBondNodeSmartConnect_Response;

  // Structure for CMD_COORDINATOR_REMOVE_BOND
  TPerCoordinatorRemoveBond_Request PerCoordinatorRemoveBond_Request;

  // Structure returned by CMD_COORDINATOR_REMOVE_BOND
  TPerCoordinatorRemoveBond_Response PerCoordinatorRemoveBond_Response;

  // Structure for CMD_COORDINATOR_DISCOVERY
  TPerCoordinatorDiscovery_Request PerCoordinatorDiscovery_Request;

  // Structure returned by CMD_COORDINATOR_DISCOVERY
  TPerCoordinatorDiscovery_Response PerCoordinatorDiscovery_Response;

  // Structure for and also returned by CMD_COORDINATOR_SET_DPAPARAMS
  TPerCoordinatorSetDpaParams_Request_Response PerCoordinatorSetDpaParams_Request_Response;

  // Structure for and also returned by CMD_COORDINATOR_SET_HOPS
  TPerCoordinatorSetHops_Request_Response PerCoordinatorSetHops_Request_Response;

  // Structure for CMD_COORDINATOR_BACKUP and CMD_NODE_BACKUP
  TPerCoordinatorNodeBackup_Request PerCoordinatorNodeBackup_Request;

  // Structure returned by CMD_COORDINATOR_BACKUP and CMD_NODE_BACKUP
  TPerCoordinatorNodeBackup_Response PerCoordinatorNodeBackup_Response;

  // Structure for CMD_COORDINATOR_RESTORE and CMD_NODE_RESTORE
  TPerCoordinatorNodeRestore_Request PerCoordinatorNodeRestore_Request;

  // Structure for CMD_COORDINATOR_AUTHORIZE_BOND
  TPerCoordinatorAuthorizeBond_Request PerCoordinatorAuthorizeBond_Request;

  // Structure returned by CMD_COORDINATOR_AUTHORIZE_BOND
  TPerCoordinatorAuthorizeBond_Response PerCoordinatorAuthorizeBond_Response;

  // Structure for CMD_COORDINATOR_BRIDGE
  TPerCoordinatorBridge_Request PerCoordinatorBridge_Request;

  // Structure returned by CMD_COORDINATOR_BRIDGE
  TPerCoordinatorBridge_Response PerCoordinatorBridge_Response;

  // Structure for CMD_COORDINATOR_SMART_CONNECT
  TPerCoordinatorSmartConnect_Request PerCoordinatorSmartConnect_Request;

  // Structure for CMD_COORDINATOR_SET_MID
  TPerCoordinatorSetMID_Request PerCoordinatorSetMID_Request;

  // Structure returned by CMD_NODE_READ
  TPerNodeRead_Response PerNodeRead_Response;

  // Structure for CMD_NODE_VALIDATE_BONDS
  TPerNodeValidateBonds_Request PerNodeValidateBonds_Request;

  // Structure returned by CMD_OS_READ
  TPerOSRead_Response PerOSRead_Response;

  // Structure returned by CMD_OS_READ_CFG
  TPerOSReadCfg_Response PerOSReadCfg_Response;

  // Structure for CMD_OS_WRITE_CFG
  TPerOSWriteCfg_Request PerOSWriteCfg_Request;

  // Structure for CMD_OS_WRITE_CFG_BYTE
  TPerOSWriteCfgByte_Request PerOSWriteCfgByte_Request;

  // Structure for CMD_OS_SET_SECURITY
  TPerOSSetSecurity_Request PerOSSetSecurity_Request;

  // Structure for CMD_OS_LOAD_CODE
  TPerOSLoadCode_Request PerOSLoadCode_Request;

  // Structure for CMD_OS_SLEEP
  TPerOSSleep_Request PerOSSleep_Request;

  // Structure for CMD_OS_SELECTIVE_BATCH
  TPerOSSelectiveBatch_Request PerOSSelectiveBatch_Request;

  // Structure for CMD_OS_TEST_RF_SIGNAL request
  TPerOSTestRfSignal_Request PerOSTestRfSignal_Request;

  // Structure for CMD_OS_INDICATE request
  TPerOSIndicate_Request PerOSIndicate_Request;

  // Structure for CMD_OS_TEST_RF_SIGNAL response
  TPerOSTestRfSignal_Response PerOSTestRfSignal_Response;

  // Structure for general memory request
  TPerMemoryRequest MemoryRequest;

  // Structure for general extended memory request
  TPerXMemoryRequest XMemoryRequest;

  // Structure for CMD_IO requests
  TPerIoDirectionAndSet_Request PerIoDirectionAndSet_Request;

  // Structure returned by CMD_THERMOMETER_READ
  TPerThermometerRead_Response PerThermometerRead_Response;

  // Structure for CMD_UART_OPEN
  TPerUartOpen_Request PerUartOpen_Request;
  
  // Structure for CMD_UART_[CLEAR_]WRITE_READ
  TPerUartWriteRead_Request PerUartWriteRead_Request;

  // Structure for CMD_FRC_SEND
  TPerFrcSend_Request PerFrcSend_Request;

  // Structure returned by CMD_FRC_SEND and CMD_FRC_SEND_SELECTIVE
  TPerFrcSend_Response PerFrcSend_Response;

  // Structure for CMD_FRC_SEND_SELECTIVE
  TPerFrcSendSelective_Request PerFrcSendSelective_Request;

  // Structure for request and response of CMD_FRC_SET_PARAMS
  TPerFrcSetParams_RequestResponse PerFrcSetParams_RequestResponse;

  // Interface and CMD_COORDINATOR_BRIDGE confirmation structure
  TIFaceConfirmation IFaceConfirmation;
} TDpaMessage;

// Custom DPA Handler events
#define	DpaEvent_DpaRequest				  0
#define	DpaEvent_Interrupt				  1
#define	DpaEvent_Idle					  2
#define	DpaEvent_Init					  3
#define	DpaEvent_Notification			  4
#define	DpaEvent_AfterRouting			  5
#define	DpaEvent_BeforeSleep			  6
#define	DpaEvent_AfterSleep				  7
#define	DpaEvent_Reset					  8
#define	DpaEvent_DisableInterrupts		  9
#define	DpaEvent_FrcValue				  10
#define	DpaEvent_ReceiveDpaResponse		  11
#define	DpaEvent_IFaceReceive			  12
#define	DpaEvent_ReceiveDpaRequest		  13
#define	DpaEvent_BeforeSendingDpaResponse 14
#define	DpaEvent_PeerToPeer				  15
#define	DpaEvent_UserDpaValue			  17
#define	DpaEvent_FrcResponseTime		  18
#define	DpaEvent_BondingButton			  19
#define	DpaEvent_Indicate    			  20
#define DpaEvent_VerifyLocalFrc           21

#define DpaEvent_LAST                     DpaEvent_VerifyLocalFrc

// Types of the diagnostic DPA Value that is returned inside DPA response
typedef enum
{
  DpaValueType_RSSI = 0,
  DpaValueType_SupplyVoltage = 1,
  DpaValueType_System = 2,
  DpaValueType_User = 3
} TDpaValueType;

// Type (color) of LED peripheral
typedef enum
{
  LED_COLOR_RED = 0,
  LED_COLOR_GREEN = 1,
  LED_COLOR_BLUE = 2,
  LED_COLOR_YELLOW = 3,
  LED_COLOR_WHITE = 4,
  LED_COLOR_UNKNOWN = 0xff
} TLedColor;

// Baud rates
typedef enum
{
  DpaBaud_1200 = 0x00,
  DpaBaud_2400 = 0x01,
  DpaBaud_4800 = 0x02,
  DpaBaud_9600 = 0x03,
  DpaBaud_19200 = 0x04,
  DpaBaud_38400 = 0x05,
  DpaBaud_57600 = 0x06,
  DpaBaud_115200 = 0x07,
  DpaBaud_230400 = 0x08
} TBaudRates;

// Useful PNUM_IO definitions
typedef enum
{
  PNUM_IO_PORTA = 0x00,
  PNUM_IO_TRISA = 0x00,
  PNUM_IO_PORTB = 0x01,
  PNUM_IO_TRISB = 0x01,
  PNUM_IO_PORTC = 0x02,
  PNUM_IO_TRISC = 0x02,
  PNUM_IO_PORTE = 0x04,
  PNUM_IO_TRISE = 0x04,
  PNUM_IO_WPUB = 0x11,
  PNUM_IO_DELAY = 0xff
} PNUM_IO_Definitions;

// To test for enumeration peripherals request
#define IsDpaEnumPeripheralsRequestNoSize() ( _PNUM == PNUM_ENUMERATION && _PCMD == CMD_GET_PER_INFO )

#if PARAM_CHECK_LEVEL >= 2
#define IsDpaEnumPeripheralsRequest() ( IsDpaEnumPeripheralsRequestNoSize() && _DpaDataLength == 0 )
#else
#define IsDpaEnumPeripheralsRequest() IsDpaEnumPeripheralsRequestNoSize()
#endif

// To test for peripherals information request
#define IsDpaPeripheralInfoRequestNoSize()  ( _PNUM != PNUM_ENUMERATION && _PCMD == CMD_GET_PER_INFO )

#if PARAM_CHECK_LEVEL >= 2
#define IsDpaPeripheralInfoRequest()  ( IsDpaPeripheralInfoRequestNoSize() && _DpaDataLength == 0 )
#else
#define IsDpaPeripheralInfoRequest()  IsDpaPeripheralInfoRequestNoSize()
#endif

// Optimized macro for both testing enumeration peripherals ELSE peripherals information. See examples
#define	IfDpaEnumPeripherals_Else_PeripheralInfo_Else_PeripheralRequestNoSize() if ( _PCMD == CMD_GET_PER_INFO ) if ( _PNUM == PNUM_ENUMERATION )

#if PARAM_CHECK_LEVEL >= 2
#define IfDpaEnumPeripherals_Else_PeripheralInfo_Else_PeripheralRequest() if ( _DpaDataLength == 0 && _PCMD == CMD_GET_PER_INFO ) if ( _PNUM == PNUM_ENUMERATION )
#else
#define	IfDpaEnumPeripherals_Else_PeripheralInfo_Else_PeripheralRequest() IfDpaEnumPeripherals_Else_PeripheralInfo_Else_PeripheralRequestNoSize()
#endif

#ifdef __CC5X__

// DPA message at bufferRF
TDpaMessage DpaRfMessage @bufferRF;

// Note: Works only, because _DpaMessage.Request.PData is at the same location as bufferRF!
#define _FSR_DPA    _FSR_RF

// Actual allocation of the RAM Peripheral memory block @ UserBank_02
bank12 uns8  PeripheralRam[PERIPHERAL_RAM_LENGTH] @ 0x620;

// Actual DPA message parameters at memory
#define	_NADR			RX
#define _NADRhigh		RTAUX
#define _PNUM			PNUM
#define _PCMD			PCMD
#define _DpaDataLength	DLEN
#define _DpaMessage		DpaRfMessage

// Return actual DPA user routine event
#define	GetDpaEvent()	userReg0

// Stores DPA Params inside DPA request/response
#define	_DpaParams		PPAR
// Get DPA Value type out of the DPA Params
#define	DpaValueType()	( _DpaParams & 0b11 )

// When TRUE then encryptBufferRF/decryptBufferRF is done by AccessPassord
bit encryptByAccessPassword @ usedBank0[0x23].7;

// DP2P response time-slot time in 10 ms
#define DP2P_TIMESLOT   11

// DP2P request packet. Fills out the whole bufferRF.
typedef struct
{
  uns8  Header[3];  // 0x000000
  uns8  SelectedNodes[30];
  uns8  SlotLength;
  uns8  ResponseTxPower;
  uns8  Reserved;
  uns16	HWPID;
  uns8  PDATA[sizeofBufferRF - ( 3 + 30 + 1 + 1 + 1 ) * sizeof( uns8 ) - ( 1 ) * sizeof( uns16 )]; // size is 26 bytes
} STRUCTATTR TDP2Prequest;

// DP2P invite packet.
typedef struct
{
  uns8  Header[3];  // 0x000001
  uns8  NADR;
  uns8  Rand[12];
} STRUCTATTR TDP2Invite;

// DP2P confirm packet.
typedef struct
{
  uns8  Header[3];  // 0x000003
  uns8  NADR;
  uns8  Rand[12];
} STRUCTATTR TDP2Confirm;

// DP2P response packet.
typedef struct
{
  uns8  Header[3];  // 0xFfFfFf
  uns8  NADR;
  uns8  PDATA[DPA_MAX_DATA_LENGTH];
} STRUCTATTR TDP2Presponse;

// Include assembler definitions
#include "HexCodes.h"

// Next code must start at the IQRF APPLICATION routine entry point
#pragma origin __APPLICATION_ADDRESS

#endif	// __CC5X__

#pragma pack( pop )

#endif	// _DPA_HEADER_
//############################################################################################
