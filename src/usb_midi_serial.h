//Modify the chips's USB settings to allow for a MIDI connection AND an emulated serial port connection
//PlatformIO teensy core files location
//Windows: %userprofile%/.platformio/packages/framework-arduinoteensy/cores/usb_midi
//Unix:  ~/.platformio/packages/framework-arduinoteensy/cores/usb_midi
//Modify usb_private.h STR_PRODUCT to change hardware name



#define STR_PRODUCT             L"Mega MIDI"


#if defined(USB_MIDI)
#define VENDOR_ID		0x16C0
#define PRODUCT_ID		0x0480
//#define PRODUCT_ID		0x0500
#define MANUFACTURER_NAME	{'A','i','d','a','n',' ','L','a','w','r','e','n','c','e'}
#define MANUFACTURER_NAME_LEN	14
#define PRODUCT_NAME		{'M','e','g','a',' ','M','I','D','I'}
#define PRODUCT_NAME_LEN	9
#define EP0_SIZE		64
#define NUM_ENDPOINTS         5
#define NUM_USB_BUFFERS	30
#define NUM_INTERFACE		3
#define CDC_IAD_DESCRIPTOR	1
#define CDC_STATUS_INTERFACE	0
#define CDC_DATA_INTERFACE	1	// Serial
#define CDC_ACM_ENDPOINT	1
#define CDC_RX_ENDPOINT       2
#define CDC_TX_ENDPOINT       3
#define CDC_ACM_SIZE          16
#define CDC_RX_SIZE           64
#define CDC_TX_SIZE           64
#define MIDI_INTERFACE        2	// MIDI
#define MIDI_TX_ENDPOINT      4
#define MIDI_TX_SIZE          64
#define MIDI_RX_ENDPOINT      5
#define MIDI_RX_SIZE          64
#define ENDPOINT1_CONFIG	ENDPOINT_TRANSIMIT_ONLY
#define ENDPOINT2_CONFIG	ENDPOINT_RECEIVE_ONLY
#define ENDPOINT3_CONFIG	ENDPOINT_TRANSIMIT_ONLY
#define ENDPOINT4_CONFIG	ENDPOINT_TRANSIMIT_ONLY
#define ENDPOINT5_CONFIG	ENDPOINT_RECEIVE_ONLY
#endif

