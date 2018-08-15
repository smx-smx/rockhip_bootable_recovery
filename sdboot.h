/*************************************************************************
	> File Name: sdboot.h
	> Author: jkand.huang
	> Mail: jkand.huang@rock-chips.com
	> Created Time: Fri 20 Jan 2017 03:24:58 PM CST
 ************************************************************************/

#ifndef _SDBOOT_H
#define _SDBOOT_H
#include <vector>
#include <string>
#include <sstream>

#define EX_SDCARD_ROOT "/mnt/external_sd"
#define USB_ROOT "/mnt/usb_storage"
#define MAX_ARGS 100
#define SD_POINT_NAME "sd_point_name"

typedef struct {
    char* name;
    char* value;
}RKSdBootCfgItem;

typedef struct{
    std::string strKey;
    std::string strValue;
}STRUCT_SD_CONFIG_ITEM,*PSTRUCT_SD_CONFIG_ITEM;

typedef std::vector<STRUCT_SD_CONFIG_ITEM> VEC_SD_CONFIG;

class SDBoot{
public:
    SDBoot();
    bool isSDboot();
    bool isUSBboot();
    std::vector<std::string> get_args(int argc, char **argv);
    void ensure_usb_mounted();
    void ensure_sd_mounted();
    int do_rk_mode_update(const char *pFile);
    void check_device_remove();
    int do_rk_factory_mode();
	int do_rk_direct_sd_update(const char *pFile);
	bool do_direct_parse_config_file(const char *pConfigFile,VEC_SD_CONFIG &vecItem);
private:
    int status;
    bool bSDBoot;
    bool bUsbBoot;
    bool bUpdateModel;
    bool bSDMounted;
    bool bUsbMounted;
    std::string IN_SDCARD_ROOT;
    std::string USB_DEVICE_PATH;
    void bootwhere();
    std::vector<std::string> get_args_from_sd(int argc, char **argv);
    std::vector<std::string> get_args_from_usb(int argc, char **argv);
    std::vector<std::string> get_sd_config(char *path, int argc, char **argv);
    bool parse_config(char *pConfig,VEC_SD_CONFIG &vecItem);
    bool parse_config_file(const char *pConfigFile,VEC_SD_CONFIG &vecItem);
    int mount_usb_device();
    void checkSDRemoved(); 
    void checkUSBRemoved(); 
    int do_sd_mode_update(const char *pFile);
    int do_usb_mode_update(const char *pFile);
};
#endif
