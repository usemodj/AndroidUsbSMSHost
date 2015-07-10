//============================================================================
// Name        : AndroidUsbSMSHost.cpp
// Author      : jinny
// Version     :
// Copyright   : NodeSoft @all right reserved
// Description : Hello World in C++, Ansi-style
//============================================================================

#include <iostream>
#include <string>
 #include <boost/locale.hpp>
#include <iconv.h>
using namespace std;
using namespace boost::locale;

#include <stdio.h>
#include <usb.h>
#include <libusb.h>
#include <string.h>
#include <unistd.h>

//#define IN 0x85
//#define OUT 0x07
#define IN 0x81
#define OUT 0x02

// lsusb -v
//#define VID 0x18d1
//#define PID 0x4E22
#define VID 0x18D1 	// idVendor
#define PID 0x4EE2 	// idProduct

#define ACCESSORY_PID 0x2d01
#define ACCESSORY_PID_ALT 0x2D00

#define LEN 2
#define BUFFER 1024
/*
If you are on Ubuntu you will require libusb as well as the headers...
We installed the headers with "apt-get source libusb"
gcc usbtest.c -I/usr/include/ -o usbtest -lusb-1.0 -I/usr/include/ -I/usr/include/libusb-1.0

Tested for Nexus S with Gingerbread 2.3.4

 Properties > C/C++ Build > Settings> GCC Linker > Libraries: usb-1.0

 Debug> sudo ./UsbTestClient
*/
int euckr2utf8(char *source, char *dest, int dest_size);
int utf82euckr(char *source, char *dest, int dest_size);

static int mainPhase();
static int init(void);
static int deInit(void);
static void error(int code);
static void status(int code);
static int isUsbAccessory();
static int setupAccessory(
	const char* manufacturer,
	const char* modelName,
	const char* description,
	const char* version,
	const char* uri,
	const char* serialNumber);

//static
static struct libusb_device_handle* handle;
//static char stop;
//static char success = 0;

int main (int argc, char *argv[]){
	//wcout.imbue(locale("ko-KR"));
	//std::locale::global( std::locale( "ko-KR" ) );
	//std::locale::global(std::locale(::setlocale(LC_ALL,".OCP")));
	//setlocale(LC_ALL, "korean");
	 generator gen;
	// Create locale generator
	std::locale::global(gen(""));
	//std::locale::global(gen("ko_KR.UTF-8"));

	if(isUsbAccessory() < 0){
		if(init() < 0){
			deInit();
			return 0;
		}

		//doTransfer();
		if(setupAccessory(
			"NodeSoft",
			"AndoridUsbSMS",
			"Android USB SMS Host",
			"1.0",
			"http://nodesoft.co.kr",
			"SN20150709") < 0){
			fprintf(stdout, "Error setting up accessory\n");
			deInit();
			return -1;
		};
	}

	if(mainPhase() < 0){
		fprintf(stdout, "Error during main phase\n");
		deInit();
		return -1;
	}
	deInit();
	fprintf(stdout, "Done, no errors\n");
	return 0;
}

static int mainPhase(){
	unsigned char buffer[BUFFER];
	unsigned char buffer2[BUFFER];
	int response = 0;
	static int transferred;
	string line;

	cout << "입력 문자 형식: " << "smsNumber#smsText" << endl;
	getline(cin, line);
	//strcpy((char *)buffer, line.c_str());
	//strcpy((char *)buffer, "01081313208#Hello world. 행복하세요.");
	//cout << buffer << endl;
	euckr2utf8( (char*)line.c_str(), (char*)buffer, BUFFER);

	response = libusb_bulk_transfer(handle, OUT, buffer, strlen((const char*)buffer), &transferred, 0);
	//response = libusb_interrupt_transfer(handle, OUT, buffer, strlen((const char*)buffer), &transferred, 0);
	 if(response < 0){error(response);return -1;}

	while ((response = libusb_bulk_transfer(handle, IN, buffer, BUFFER, &transferred, 0)) == 0) {
	//while ((response = libusb_interrupt_transfer(handle, IN, buffer, BUFFER, &transferred, 0)) == 0) {
		 	buffer[transferred]= '\0';
		 	//setlocale(LC_ALL, "korean");
		 	//wprintf(L"한글");
		 	//fprintf(stdout, "%s\n", buffer);
		 	utf82euckr((char *)buffer, (char *)buffer2, BUFFER);
		 	cout << buffer2 << endl;
		 	break;
	 }
	 if(response < 0){error(response);return -1;}
	 return 0;
}


static int init(){
	//TODO: RESET process
	if(handle != NULL){
		libusb_release_interface(handle, 0);
		libusb_close(handle);
	}
	libusb_init(NULL);

	if((handle = libusb_open_device_with_vid_pid(NULL, VID, PID)) == NULL){
		fprintf(stdout, "Problem acquiring handle\n");
		return -1;
	}
	libusb_claim_interface(handle, 0);
	return 0;
}

static int deInit(){
	//TODO free all transfers individually...
	//if(ctrlTransfer != NULL)
	//	libusb_free_transfer(ctrlTransfer);
	if(handle != NULL){
		libusb_release_interface(handle, 0);
		libusb_close(handle);
	}
	libusb_exit(NULL);
	return 0;
}
static int isUsbAccessory() {
  int res;
  libusb_init(NULL);
  if((handle = libusb_open_device_with_vid_pid(NULL, VID,  ACCESSORY_PID)) == NULL) {
    fprintf(stdout, "Device is not USB Accessory Mode\n");
    res = -1;
  } else {
    // already usb accessory mode
    fprintf(stdout, "Device is already USB Accessory Mode\n");
    libusb_claim_interface(handle, 0);
    res = 0;
  }
  return res;
}

static int setupAccessory(
	const char* manufacturer,
	const char* modelName,
	const char* description,
	const char* version,
	const char* uri,
	const char* serialNumber){

	unsigned char ioBuffer[2];
	int devVersion;
	int response;
	int tries = 5;
	  libusb_init(NULL);
	// Values to check for AOA capability
	response = libusb_control_transfer(
		handle, //handle
		0xC0, //bmRequestType(USB_DIRECTION_IN and USB_TYPE_VENDOR)
		51, //bRequest(0x33)
		0, //wValue
		0, //wIndex
		ioBuffer, //data
		2, //wLength
        0 //timeout
	);

	if(response <= 0){error(response);return-1;}

	devVersion = ioBuffer[1] << 8 | ioBuffer[0];
	fprintf(stdout,"Version Code Device: %d\n", devVersion);

	usleep(1000);//sometimes hangs on the next transfer :(
	/*
	 *  send identifying string information to the device.
	 *
	 * Characteristics for control requests
	 *  Request type: USB_DIR_OUT and USB_TYPE_VENDOR
	 *  Request: 52(ox34)
	 *  Value: 0
	 *  Index: String ID
	 *  Data: Zero terminated('\0') UTF8 string sent from accessory to device
	 *  �� String ID ��0�� means that the following data is for  Manufacturer name.
	 *  �� String ID ��1�� means it is for  Model name.
	 *  �� String ID ��2�� means it is for  Description.
	 *  �� String ID ��3�� means it is for  Version.��
	 *  �� String ID ��4�� means it is for  URI.
	 *  �� String ID ��5�� means it is for  Serial number
	 */

	response = libusb_control_transfer(handle,0x40,52,0,0, (unsigned char*)manufacturer, strlen(manufacturer)+1,0);
	if(response < 0){error(response);return -1;}

	response = libusb_control_transfer(handle,0x40,52,0,1,(unsigned char*)modelName, strlen(modelName)+1,0);
	if(response < 0){error(response);return -1;}
	response = libusb_control_transfer(handle,0x40,52,0,2,(unsigned char*)description, strlen(description)+1,0);
	if(response < 0){error(response);return -1;}
	response = libusb_control_transfer(handle,0x40,52,0,3,(unsigned char*)version, strlen(version)+1,0);
	if(response < 0){error(response);return -1;}
	response = libusb_control_transfer(handle,0x40,52,0,4,(unsigned char*)uri, strlen(uri)+1,0);
	if(response < 0){error(response);return -1;}
	response = libusb_control_transfer(handle,0x40,52,0,5,(unsigned char*)serialNumber, strlen(serialNumber)+1,0);
	if(response < 0){error(response);return -1;}

	fprintf(stdout,"Accessory Identification sent: %d\n", devVersion);

	/*
	 * request the device to start up in accessory mode.
	 *
	 * Characteristics of a control vendor request on endpoint 0 to the android device
	 *  Request type: USB_DIR_OUT and USB_TYPE_VENDOR
	 *  Request: 53(0x35)
	 *  Value: 0
	 *  Index: 0
	 *  Data: None
	 */
	response = libusb_control_transfer(handle,0x40,53,0,0,NULL,0,0);
	if(response < 0){error(response);return -1;}

	fprintf(stdout,"Attempted to put device into accessory mode: %d\n", devVersion);

	if(handle != NULL)
		libusb_release_interface(handle, 0);

	int pid = PID;
	for(;;){//attempt to connect to new PID, if that doesn't work try ACCESSORY_PID_ALT
		tries--;
		fprintf(stdout, " tries %d PID: %x", tries, pid);
		if((handle = libusb_open_device_with_vid_pid(NULL, VID, pid)) == NULL){
			pid = ACCESSORY_PID;
			if(tries < 0){
				return -1;
			}
		}else{
			break;
		}
		sleep(1);
	}
	libusb_claim_interface(handle, 0);
	fprintf(stdout, "Interface claimed, ready to transfer data\n");
	return 0;
}

static void error(int code){
	fprintf(stdout,"\n");

	switch(code){
	case LIBUSB_ERROR_IO:
		fprintf(stdout,"Error: LIBUSB_ERROR_IO\nInput/output error.\n");
		break;
	case LIBUSB_ERROR_INVALID_PARAM:
		fprintf(stdout,"Error: LIBUSB_ERROR_INVALID_PARAM\nInvalid parameter.\n");
		break;
	case LIBUSB_ERROR_ACCESS:
		fprintf(stdout,"Error: LIBUSB_ERROR_ACCESS\nAccess denied (insufficient permissions).\n");
		break;
	case LIBUSB_ERROR_NO_DEVICE:
		fprintf(stdout,"Error: LIBUSB_ERROR_NO_DEVICE\nNo such device (it may have been disconnected).\n");
		break;
	case LIBUSB_ERROR_NOT_FOUND:
		fprintf(stdout,"Error: LIBUSB_ERROR_NOT_FOUND\nEntity not found.\n");
		break;
	case LIBUSB_ERROR_BUSY:
		fprintf(stdout,"Error: LIBUSB_ERROR_BUSY\nResource busy.\n");
		break;
	case LIBUSB_ERROR_TIMEOUT:
		fprintf(stdout,"Error: LIBUSB_ERROR_TIMEOUT\nOperation timed out.\n");
		break;
	case LIBUSB_ERROR_OVERFLOW:
		fprintf(stdout,"Error: LIBUSB_ERROR_OVERFLOW\nOverflow.\n");
		break;
	case LIBUSB_ERROR_PIPE:
		fprintf(stdout,"Error: LIBUSB_ERROR_PIPE\nPipe error.\n");
		break;
	case LIBUSB_ERROR_INTERRUPTED:
		fprintf(stdout,"Error:LIBUSB_ERROR_INTERRUPTED\nSystem call interrupted (perhaps due to signal).\n");
		break;
	case LIBUSB_ERROR_NO_MEM:
		fprintf(stdout,"Error: LIBUSB_ERROR_NO_MEM\nInsufficient memory.\n");
		break;
	case LIBUSB_ERROR_NOT_SUPPORTED:
		fprintf(stdout,"Error: LIBUSB_ERROR_NOT_SUPPORTED\nOperation not supported or unimplemented on this platform.\n");
		break;
	case LIBUSB_ERROR_OTHER:
		fprintf(stdout,"Error: LIBUSB_ERROR_OTHER\nOther error.\n");
		break;
	default:
		fprintf(stdout, "Error: unknown error\n");
		break;
	}

}


static void status(int code){
	fprintf(stdout,"\n");
	switch(code){
		case LIBUSB_TRANSFER_COMPLETED:
			fprintf(stdout,"Success: LIBUSB_TRANSFER_COMPLETED\nTransfer completed.\n");
			break;
		case LIBUSB_TRANSFER_ERROR:
			fprintf(stdout,"Error: LIBUSB_TRANSFER_ERROR\nTransfer failed.\n");
			break;
		case LIBUSB_TRANSFER_TIMED_OUT:
			fprintf(stdout,"Error: LIBUSB_TRANSFER_TIMED_OUT\nTransfer timed out.\n");
			break;
		case LIBUSB_TRANSFER_CANCELLED:
			fprintf(stdout,"Error: LIBUSB_TRANSFER_CANCELLED\nTransfer was canceled.\n");
			break;
		case LIBUSB_TRANSFER_STALL:
			fprintf(stdout,"Error: LIBUSB_TRANSFER_STALL\nFor bulk/interrupt endpoints: halt condition detected (endpoint stalled).\nFor control endpoints: control request not supported.\n");
			break;
		case LIBUSB_TRANSFER_NO_DEVICE:
			fprintf(stdout,"Error: LIBUSB_TRANSFER_NO_DEVICE\nDevice was disconnected.\n");
			break;
		case LIBUSB_TRANSFER_OVERFLOW:
			fprintf(stdout,"Error: LIBUSB_TRANSFER_OVERFLOW\nDevice sent more data than requested.\n");
			break;
		default:
			fprintf(stdout,"Error: unknown error\nTry again(?)\n");
			break;
	}

}

int euckr2utf8(char *source, char *dest, int dest_size)
{
    iconv_t it;
    char *pout;
    size_t in_size, out_size;

    it = iconv_open("UTF-8", "EUC-KR");
    in_size = strlen(source);
    out_size = dest_size;
    pout = dest;
    if (iconv(it, &source, &in_size, &pout, &out_size) < 0)
        return(-1);
    iconv_close(it);
    return(pout - dest);
    /* return(out_size); */
}

int utf82euckr(char *source, char *dest, int dest_size)
{
    iconv_t it;
    char *pout;
    size_t in_size, out_size;

    it = iconv_open("EUC-KR", "UTF-8");
    in_size = strlen(source);
    out_size = dest_size;
    pout = dest;
    if (iconv(it, &source, &in_size, &pout, &out_size) < 0)
        return(-1);
    iconv_close(it);
    return(pout - dest);
    /* return(out_size); */
}
