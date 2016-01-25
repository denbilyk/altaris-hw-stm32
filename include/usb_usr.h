//usb_usr.h

#ifndef __USB_USR_H
#define __USB_USR_H


#define CMD_READ_AUTH (0x1B)
#define CMD_READ_HOST (0x1C)
#define CMD_READ_SSID (0x1D)
#define CMD_READ_SSID_PASS (0x1E)

#define CMD_ERASE	   (0x29)
#define CMD_WRITE_AUTH (0x2A)
#define CMD_WRITE_HOST (0x2B)
#define CMD_WRITE_PORT (0x2C)
#define CMD_WRITE_SSID (0x2D)
#define CMD_WRITE_SSID_PASS (0x2E)


void check_data_request(const char* auth, const char* host, const char* port, const char* ssid, const char* ssid_pass);
void check_auth_write_command();

#endif
