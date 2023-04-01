#include "sd_health_read.h"
#include "sd_get_data.h"

struct udev_device * get_child(struct udev * udev, struct udev_device * parent, const char * subsystem) {
	struct udev_device * child = NULL;
	struct udev_enumerate * enumerate = udev_enumerate_new(udev);
	udev_enumerate_add_match_parent(enumerate, parent);
	udev_enumerate_add_match_subsystem(enumerate, subsystem);
	udev_enumerate_scan_devices(enumerate);
	struct udev_list_entry * devices = udev_enumerate_get_list_entry(enumerate);
	struct udev_list_entry * entry;

	udev_list_entry_foreach(entry, devices) {
		const char * path = udev_list_entry_get_name(entry);
		child = udev_device_new_from_syspath(udev, path);
		break;
	}
	udev_enumerate_unref(enumerate);
	return child;
}

void enumerate_scsi_device(struct udev * udev, char( * scsi_arry_value)[10]) {
	struct udev_enumerate * enumerate = udev_enumerate_new(udev);
	udev_enumerate_add_match_subsystem(enumerate, "scsi");
	udev_enumerate_add_match_property(enumerate, "DEVTYPE", "scsi_device");
	udev_enumerate_scan_devices(enumerate);

	struct udev_list_entry * devices = udev_enumerate_get_list_entry(enumerate);
	struct udev_list_entry * entry;

	udev_list_entry_foreach(entry, devices) {
		const char * path = udev_list_entry_get_name(entry);
		struct udev_device * scsi = udev_device_new_from_syspath(udev, path);
		struct udev_device * scsi_generic = get_child(udev, scsi, "scsi_generic");
		struct udev_device * block = get_child(udev, scsi, "block");
		struct udev_device * usb = udev_device_get_parent_with_subsystem_devtype(scsi,"usb","usb_device");

		if (scsi_generic && usb) {
			strcpy(scsi_arry_value[0], udev_device_get_sysattr_value(usb, "idVendor"));
			strcpy(scsi_arry_value[1], udev_device_get_sysattr_value(usb, "idProduct"));
			strcpy(scsi_arry_value[2], udev_device_get_sysname(scsi_generic));
			strcpy(scsi_arry_value[3], udev_device_get_sysname(block));
			strcpy(scsi_arry_value[4], udev_device_get_sysattr_value(scsi, "vendor"));
			strcpy(scsi_arry_value[5], udev_device_get_devnode(scsi_generic));
			printf(" SCSI_DEVICE \n");
			printf(" VID:PID = %s:%s, scsi_generic_devname = %s, block_devname = %s, vendor = %s, sg_dev_node = %s\n",
				scsi_arry_value[0],
				scsi_arry_value[1],
				scsi_arry_value[2],
				scsi_arry_value[3],
				scsi_arry_value[4],
				scsi_arry_value[5]
			);
		}
		if (scsi_generic) {
			udev_device_unref(scsi_generic);
		}
		udev_device_unref(scsi);
	}
	udev_enumerate_unref(enumerate);
}

void enumerate_wifi_device(struct udev * udev, char( * wifi_arry_value)[10]) {
	struct udev_enumerate * enumerate = udev_enumerate_new(udev);
	udev_enumerate_add_match_subsystem(enumerate, "net");
	udev_enumerate_add_match_property(enumerate, "DEVTYPE", "wlan");
	udev_enumerate_scan_devices(enumerate);

	struct udev_list_entry * devices = udev_enumerate_get_list_entry(enumerate);
	struct udev_list_entry * entry;

	udev_list_entry_foreach(entry, devices) {
		const char * path = udev_list_entry_get_name(entry);
		struct udev_device * net = udev_device_new_from_syspath(udev, path);
		struct udev_device * usb = udev_device_get_parent_with_subsystem_devtype(net,"usb","usb_device");

		if (usb) {
			strcpy(wifi_arry_value[0], udev_device_get_sysattr_value(usb, "idVendor"));
			strcpy(wifi_arry_value[1], udev_device_get_sysattr_value(usb, "idProduct"));
			strcpy(wifi_arry_value[2], udev_device_get_property_value(net, "SUBSYSTEM"));
			strcpy(wifi_arry_value[3], udev_device_get_property_value(net, "DEVTYPE"));
			printf(" WIFI_DEVICE \n");
			printf(" VID:PID = %s:%s, SUBSYSTEM = %s, DEVTYPE = %s\n",
				wifi_arry_value[0],
				wifi_arry_value[1],
				wifi_arry_value[2],
				wifi_arry_value[3]
			);
		}
		udev_device_unref(net);
	}
	udev_enumerate_unref(enumerate);
}

int main(int argc, char * argv[]) {
	struct udev * udev;
	struct udev * udev_first; 
	struct udev_device * dev;
	struct udev_monitor * mon;
	int fd;
	int hFile = -1;
	int status = 0;
	char * pfix_dev_name;
	int debug = 1;
	int mode = 0;

	SD_CID_DATA cidData;
    SD_STATUS_REGISTER ssrData;
	SD_HEALTH_DATA healthData;
	SD_STATUS_DATA statusData;

	char first_scsi_device_udev_info[6][10];
	char first_wifi_device_udev_info[4][10];

	/* create udev object */
	udev = udev_new();
	udev_first = udev_new();

	if (!udev) {
		fprintf(stderr, "Can't create udev\n");
		status = -1;
		return status;
	}

	// pfix-add Get the device name from the command line
	if( NULL == argv[1] )
	{
		printf( "*************************user guide***************************\n" );
		printf( "  Version : %s\n", DEF_VERSION                                    ); 
		printf( "  /usr/bin/sd_health_read {path} {mode}\n"                        );
		printf( "  path : \n"                                                      ); 
		printf( "       ->  /dev/sda ~ /dev/sdx  or /dev/sg0 ~ /dev/sgn \n"        );
		printf( "  mode : \n"                                                      );
		printf( "       ->  NULL : sd_module_monitor  or  1 : device monitor \n"   );
		printf( "**************************************************************\n" );
		return status;
	}

	if (argv[2] != NULL) {
		if (strcmp(argv[2], "1") == 0) {
			mode = 1;
		} 
		else {
			printf("invalid/NULL input argv[2], start sd_module_monitor!! \n");
		}
	}

	if (mode == 0 ) {
		memset(& cidData, 0x00, sizeof(SD_CID_DATA));
		memset(& healthData, 0x00, sizeof(SD_HEALTH_DATA));
		memset(& statusData, 0x00, sizeof(SD_STATUS_DATA));

    	/* device check after boot   */
		enumerate_scsi_device(udev_first , first_scsi_device_udev_info);
		printf("\n");
		enumerate_wifi_device(udev_first , first_wifi_device_udev_info);
		printf("\n");
		udev_unref( udev_first );

		if (strncmp(first_scsi_device_udev_info[0], "0424", 4) == 0 && 
		strncmp(first_scsi_device_udev_info[1], "4041", 4) == 0 && 
		strncmp(&first_scsi_device_udev_info[2][0], "s", 1) == 0  && 
		strncmp(&first_scsi_device_udev_info[2][1], "g", 1) == 0 ) {   //sg{N} device check  
			pfix_dev_name = first_scsi_device_udev_info[5];
			printf("** MICROCHIP SCSI DEVICE IS CONNECTED\n\n");
			status = sd_init_check(& hFile, pfix_dev_name, debug);
			if (status < 0) {
				return status;
			}
			status = sd_get_status(hFile, & statusData, debug);
			if (status < 0) {
				return status;
			} 
			else {
				if (statusData.CardPresent == 0) {
					printf("** SDCARD IS REMOVED \n\n");
					sd_close_check(& hFile, pfix_dev_name, & healthData, & cidData);
				} 
				else {
					printf("** SDCARD IS CONNECTED \n\n");
                    status = sd_speed_check(hFile, & ssrData, debug);
                    status = sd_support_check(hFile, & cidData, debug);
                    if (status < 0) {
						return status;
					}
					else {
						status = sd_get_health_status(hFile,& healthData,cidData.Manufacturer_ID,0,debug);
						if (status < 0) {
							return status;
						}
						if (cidData.Manufacturer_ID == 0x09) {
							status = sd_get_health_status(hFile,& healthData,cidData.Manufacturer_ID,1,debug);
							if (status < 0) {
								return status;
							}
						}
					}
				}
			}
		} 
		else {
			printf("** MICROCHIP SCSI DEVICE IS REMOVED\n\n");
			memset(first_scsi_device_udev_info, 0, sizeof(first_scsi_device_udev_info[0][0]) * 5 * 10);
			strcpy(first_scsi_device_udev_info[2],"sgx");
			strcpy(first_scsi_device_udev_info[3],"sdx");
			pfix_dev_name = argv[1];
		}

		if (strncmp(first_wifi_device_udev_info[0], "1286", 4) == 0  && 
		strncmp(first_wifi_device_udev_info[1], "204a", 4) == 0  && 
		strncmp(first_wifi_device_udev_info[2], "net", 3) == 0 &&
		strncmp(first_wifi_device_udev_info[3], "wlan", 4) == 0 ) {
			printf("** Ublox wifi DEVICE IS CONNECTED\n\n");
		} 
		else {
			printf("** Ublox wifi DEVICE IS REMOVED\n\n");
		}
	}

	/* device uevent check from runtime */
	mon = udev_monitor_new_from_netlink(udev, "udev");
	udev_monitor_filter_add_match_subsystem_devtype(mon, "usb", NULL); //for wifi module
	udev_monitor_filter_add_match_subsystem_devtype(mon, "block", "disk"); //for micro sd card
	udev_monitor_filter_add_match_subsystem_devtype(mon, "scsi_generic", NULL); //for scsi_device( sd_module )
	udev_monitor_filter_add_match_subsystem_devtype(mon, "net", NULL); //for wifi module  
	udev_monitor_enable_receiving(mon);
	fd = udev_monitor_get_fd(mon);

	while (1) {
		fd_set fds;
		struct timeval tv;
		int ret;

		FD_ZERO(& fds);
		FD_SET(fd, & fds);
		tv.tv_sec = 0;
		tv.tv_usec = 0;

		if (mode == 0) {
			if (hFile != -1) {
				status = sd_close_check(& hFile, pfix_dev_name, & healthData, & cidData);
				if (status < 0) {
					return status;
				}
			}
		}

		ret = select(fd + 1, & fds, NULL, NULL, & tv);
		if (ret > 0 && FD_ISSET(fd, & fds)) {
			dev = udev_monitor_receive_device(mon);
			if (dev) {
				const char * device_udev_info[7] = {
					udev_device_get_action(dev),
					udev_device_get_sysname(dev),
					udev_device_get_devtype(dev),
					udev_device_get_property_value(dev, "DISK_MEDIA_CHANGE"),
					udev_device_get_sysattr_value(dev, "manufacturer"),
					udev_device_get_sysattr_value(dev, "idProduct"),
					udev_device_get_sysattr_value(dev, "idVendor")
				};

				for (int i = 0; i < 7; i++) {
					if (device_udev_info[i] == NULL) {
						device_udev_info[i] = "NULL";
					}
				}
				if (mode == 0) {
					memset(& statusData, 0x00, sizeof(statusData));

					if (strncmp(device_udev_info[0], "change", 6) == 0 && 
					strncmp(&device_udev_info[1][0], &first_scsi_device_udev_info[3][0], 1) == 0 &&
					strncmp(&device_udev_info[1][1], &first_scsi_device_udev_info[3][1], 1) == 0 && // sdb or sda
					strncmp(device_udev_info[2], "disk", 4) == 0 && 
					strncmp(device_udev_info[3],"1",1) == 0) {
						status = sd_init_check(& hFile, pfix_dev_name, debug);
						if (status < 0) {
							return status;
						}

						status = sd_get_status(hFile, & statusData, debug);
						if (status < 0) {
							return status;
						} 
						else {
							if (statusData.CardPresent == 0) {
								printf("** SDCARD IS REMOVED \n\n");
								sd_close_check(& hFile, pfix_dev_name, & healthData, & cidData);
							} 
							else {
								printf("** SDCARD IS CONNECTED \n\n");
                                status = sd_speed_check(hFile, & ssrData, debug);
                                status = sd_support_check(hFile, & cidData, debug);
                                if (status < 0) {
									return status;
								}
								else {
									status = sd_get_health_status(
										hFile,	
										& healthData,
										cidData.Manufacturer_ID,
										0,
										debug
									);
									if (status < 0) {
										return status;
									}
									if (cidData.Manufacturer_ID == 0x09) {
										status = sd_get_health_status(hFile,& healthData,cidData.Manufacturer_ID, 1,debug);
										if (status < 0) {
											return status;
										}
									}
								}
							}
						}
					} 
					else if (strncmp(device_udev_info[0], "add", 3) == 0 &&   
					strncmp(&device_udev_info[1][0], &first_scsi_device_udev_info[2][0], 1) == 0 &&
					strncmp(&device_udev_info[1][1], &first_scsi_device_udev_info[2][1], 1) == 0) { // scsi_device insert (sg1 or sg0 )
						printf("** MICROCHIP SCSI DEVICE IS CONNECTED\n\n");
						status = sd_init_check(& hFile, pfix_dev_name, debug);
						if (status < 0) {
							return status;
						}
						status = sd_get_status(hFile, & statusData, debug);
						if (status < 0) {
							return status;
						} 
						else {
							if (statusData.CardPresent == 0) {
								printf("** SDCARD IS REMOVED \n\n");
								sd_close_check(& hFile, pfix_dev_name, & healthData, & cidData);
							} 
							else {
								printf("** SDCARD IS CONNECTED \n\n");
                                status = sd_speed_check(hFile, & ssrData, debug);
                                status = sd_support_check(hFile, & cidData, debug);
                                if (status < 0) {
									return status;
								} 
								else {
									status = sd_get_health_status(
										hFile,
										& healthData,
										cidData.Manufacturer_ID,
										0,
										debug
									);
									if (status < 0) {
										return status;
									}
									if (cidData.Manufacturer_ID == 0x09) {
										status = sd_get_health_status(hFile,& healthData,cidData.Manufacturer_ID, 1,debug);
										if (status < 0) {
											return status;
										}
									}
								}
							}
						}
					} 
					else if (strncmp(device_udev_info[0], "remove", 6) == 0 && 
					strncmp(&device_udev_info[1][0], &first_scsi_device_udev_info[2][0], 1) == 0 &&
					strncmp(&device_udev_info[1][1], &first_scsi_device_udev_info[2][1], 1) == 0) { // scsi_device remove (sg1 or sg0)
						printf("** MICROCHIP SCSI DEVICE IS REMOVED\n\n");
					} 
					else if (strncmp(device_udev_info[0], "add", 3) == 0 && 
					strncmp(device_udev_info[2], "usb_device", 10) == 0 && // wifi module insert
					strncmp(device_udev_info[4], "Marvell", 7) == 0 && // wifi module 의 remove 조건은 wifi 로 볼수 없습니다.
					strncmp(device_udev_info[5], "204a", 4) == 0 && 
					strncmp(device_udev_info[6],"1286",4) == 0) {
						printf("** Ublox wifi DEVICE IS CONNECTED\n\n");
					}
					else if (strncmp(device_udev_info[0], "remove", 3) == 0 &&
					strncmp(device_udev_info[2], "ieee80211", 9) == 0) { // wifi module remove
						printf("** Ublox wifi DEVICE IS REMOVED\n\n");
					}
				}
				else {
					printf("****************************************\n"); 
					printf(" \n"); 
					printf("I: ACTION=%s\n", device_udev_info[0]);
					printf("I: DEVNAME=%s\n",device_udev_info[1]); 
					printf("I: DEVTYPE=%s\n",device_udev_info[2]); 
					printf("I: PROPERTY=DISK_MEDIA_CHANGE%s\n", device_udev_info[3]);
					printf("I: manudacturer=%s\n", device_udev_info[4]); 
					printf("I: idProduct=%s\n", device_udev_info[5]);
					printf("I: ipVendor=%s\n",device_udev_info[6]); 
					printf(" \n"); 
					printf("****************************************\n"); 
					printf("---\n");					
				}
				/* free dev */
				udev_device_unref(dev);
			}
		}
		/* 1 milliseconds */
		usleep(1000);
	}
	/* free udev */
	udev_unref(udev);
	return status;
}
