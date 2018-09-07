/*************************************************************************
	> File Name: rktools.cpp
	> Author: jkand.huang
	> Mail: jkand.huang@rock-chips.com
	> Created Time: Mon 23 Jan 2017 02:36:42 PM CST
 ************************************************************************/

#include <iostream>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include "sdboot.h"
#include "rktools.h"
#include <fs_mgr.h>
#include "roots.h"
#include "common.h"
#include <cutils/properties.h>
#include <pthread.h>
#include <dirent.h>
#include <sys/stat.h>
#include <errno.h>

static bool isLedFlash = false;
static pthread_t tid;
static int last_state = 0;

using namespace std;
char* check_media_package(const char *path){
    char *reallyPath;
    SDBoot mountDevice;

    if(strncmp(path, "/mnt/external_sd", 16) == 0){
        mountDevice.ensure_sd_mounted();
    }else if(strncmp(path, "/mnt/usb_storage", 16) == 0){
        mountDevice.ensure_usb_mounted();
    }

    if(strncmp(path, "/mnt/media_rw", 13) == 0){
        printf("start to find package in /mnt/usb_storage/ .\n");
        reallyPath = (char*)malloc(PATH_LEN);
        mountDevice.ensure_usb_mounted();
        memset(reallyPath, 0, PATH_LEN);
        strcpy(reallyPath, usb_path);

        const char *filename = strstr(path, "update.zip");
        if(filename == NULL)
            filename = strstr(path, "update.img");
        if(filename == NULL){
            printf("check_media_package: filename is null\n");
            free(reallyPath);
            reallyPath = NULL;
            return NULL;
        }

        strcat(reallyPath, filename); 
        if((access(reallyPath, F_OK))==0)
        {
            printf("check_media_package: find package ok is %s.\n", reallyPath);
            return reallyPath;
        }
        
        mountDevice.ensure_sd_mounted();
        memset(reallyPath, 0, PATH_LEN);
        strcpy(reallyPath, sd_path);
        strcat(reallyPath, filename);
        if((access(reallyPath, F_OK))==0)
        {
            printf("check_media_package: find package ok is %s.\n", reallyPath);
            return reallyPath;
        }
        free(reallyPath);
        reallyPath = NULL;
    }

    return NULL;
}
/**
 * 从/proc/cmdline 获取串口的节点
 *
*/
char *getSerial(){
    char *ans = (char*)malloc(20);
    char param[1024];
    int fd, ret;
    char *s = NULL;
    fd = open("/proc/cmdline", O_RDONLY);
    ret = read(fd, (char*)param, 1024);
    printf("cmdline=%s\n",param);
    s = strstr(param,"console");
    if(s == NULL){
        printf("no found console in cmdline\n");
        free(ans);
        ans = NULL;
        return ans;
    }else{
        s = strstr(s, "=");
        if(s == NULL){
            free(ans);
            ans = NULL;
            return ans;
        }

        strcpy(ans, "/dev/");
        char *str = ans + 5;
        s++;
        while(*s != ' '){
            *str = *s;
            str++;
            s++;
        }
        *str = '\0';
        printf("read console from cmdline is %s\n", ans);
    }

    return ans;
}

/**
 * reset hdmi after restore factory.
*/
int erase_baseparameter() {
    Volume* v = volume_for_mount_point(BASEPARAMER_PARTITION_NAME);
    if (v == NULL) {
        printf("unknown volume baseparamer, not erase baseparamer\n");
        return -1;
    }

    int file;
    file = open(v->blk_device, O_RDWR);
    if (file < 0){
        printf("baseparamer file can not be opened");
        return -1;
    }
    lseek(file, BASEPARAMER_PARTITION_SIZE, SEEK_SET);

    //size of baseparameter.
    char buf[BASEPARAMER_PARTITION_SIZE];
    memset(buf, 0, BASEPARAMER_PARTITION_SIZE);
    read(file, buf, BASEPARAMER_PARTITION_SIZE);

    lseek(file, 0L, SEEK_SET);
    write(file, (char*)(&buf), BASEPARAMER_PARTITION_SIZE);
    close(file);
    sync();

    return 0;
}


void *thrd_led_func(void *arg) {
    FILE * ledFd = NULL;
    bool onoff = false;
    char real_net_file_path[128] = "\0";
    if((ledFd = fopen(NET_FILE_PATH, "w")) != NULL){
        strcpy(real_net_file_path, NET_FILE_PATH);
        fclose(ledFd);
    }else if((ledFd = fopen(NET_FILE_PATH_NEW, "w")) != NULL){
        strcpy(real_net_file_path, NET_FILE_PATH_NEW);
        fclose(ledFd);
    }

    while(isLedFlash) {
        ledFd = fopen(real_net_file_path, "w");
        if(ledFd == NULL)
        {
            usleep(500 * 1000);
            continue;
        }
        if(onoff) {
            fprintf(ledFd, "%d", OFF_VALUE);
            onoff = false;
        }else {
            fprintf(ledFd, "%d", ON_VALUE);
            onoff = true;
        }

        fclose(ledFd);
        usleep(500 * 1000);
    }

    printf("stopping led thread, close led and exit\n");

    ledFd = fopen(real_net_file_path, "w");
    if(ledFd != NULL){
        fprintf(ledFd, "%d", last_state);
        fclose(ledFd);
    }
    pthread_exit(NULL);
    return NULL;
}

void startLed() {
    isLedFlash = true;
    if (pthread_create(&tid,NULL,thrd_led_func,NULL)!=0) {
        printf("Create led thread error!\n");
    }

    printf("tid in led pthread: %ld.\n",tid);
}

void stopLed(int state) {
    last_state = state;
    void *tret;
    isLedFlash = false;

    if (pthread_join(tid, &tret)!=0){
        printf("Join led thread error!\n");
    }else {
        printf("join led thread success!\n");
    }
}


/**
 *  设置flash 节点
 */
static char result_point[4][20]; //0-->emmc, 1-->sdcard, 2-->SDIO, 3-->SDcombo
int readFile(DIR* dir, char* filename){
    char name[30] = {'\0'};
    strcpy(name, filename);
    strcat(name, "/type");
    int fd = openat(dirfd(dir), name, O_RDONLY);
    if(fd == -1){
        printf("Error: openat %s error %s.\n", name, strerror(errno));
        return -1;
    }
    char resultBuf[10] = {'\0'};
    read(fd, resultBuf, sizeof(resultBuf));
    for(int i = 0; i < (int)strlen(resultBuf); i++){
        if(resultBuf[i] == '\n'){
            resultBuf[i] = '\0';
            break;
        }
    }
    for(int i = 0; i < 4; i++){
        if(strcmp(typeName[i], resultBuf) == 0){
            //printf("type is %s.\n", typeName[i]);
            return i;
        }
    }

    printf("Error:no found type!\n");
    return -1;
}

void init_sd_emmc_point(){
    DIR* dir = opendir("/sys/bus/mmc/devices/");
    if(dir != NULL){
		printf("exit=================\n");
        memset(result_point, 0, 4*20);
        dirent* de;
        while((de = readdir(dir))){
            if(strncmp(de->d_name, "mmc", 3) == 0){
                //printf("find mmc is %s.\n", de->d_name);
                char flag = de->d_name[3];
                int ret = -1;
                ret = readFile(dir, de->d_name);
                if(ret != -1){
                    strcpy(result_point[ret], point_items[flag - '0']);
                }else{
                    strcpy(result_point[ret], "");
                }
            }
        }

    }
    closedir(dir);
}

void setFlashPoint(){
    init_sd_emmc_point();
    setenv(EMMC_POINT_NAME, result_point[MMC], 1);
    //SDcard 有两个挂载点
	printf("111\n");
    if(access(result_point[SD], F_OK) == 0)
        setenv(SD_POINT_NAME_2, result_point[SD], 1);
    char name_t[22];
    if(strlen(result_point[SD]) > 0){
        strcpy(name_t, result_point[SD]);
        strcat(name_t, "p1");
    }
    if(access(name_t, F_OK) == 0)
        setenv(SD_POINT_NAME, name_t, 1);

    printf("emmc_point is %s\n", getenv(EMMC_POINT_NAME));
    printf("sd_point is %s\n", getenv(SD_POINT_NAME));
    printf("sd_point_2 is %s\n", getenv(SD_POINT_NAME_2));
}

void dumpCmdArgs(int argc, char** argv) {
    fprintf(stdout, "=== start %s:%d ===\n", __func__, __LINE__);
    for(int i = 0; i < argc; i++)
    {
        fprintf(stdout, "argv[%d] =  %s.\n", i, argv[i]);
    }
}

int getEmmcState() {
    char bootmode[256];
    int result = 0;

    property_get("ro.boot.mode", bootmode, "unknown");
    printf("ro.boot.mode = %s \n", bootmode);

    if(!strcmp(bootmode, "nvme")) {
        result = 2;
    } else if(!strcmp(bootmode, "emmc")) {
        result = 1;
    }else {
        result = 0;
    }

    return result;
}

/**
 * 分割字符串
 */
void SplitString(const std::string& s, std::string& result, const std::string& c){
#if 0
    if(getEmmcState() != 0){
        result = s;
        return ;
    }
    std::string::size_type pos1, pos2;
    pos2 = s.find(c);
    pos1 = 0;
    while(std::string::npos != pos2){
        result = s.substr(pos1, pos2-pos1);
        pos1 = pos2 + c.size();
        pos2 = s.find(c, pos1);
    }
    if(pos1 != s.length())
        result = s.substr(pos1);
    result = "/dev/block/rknand_" + result;
#else
	result = s;
#endif
    printf("SplitString :: result is %s.\n", result.c_str());
}

bool IsSpecialName(const char *str){
    if(getEmmcState() != 0){
        return false;
    }
    string fileName[5] = {"uboot.img", "trust.img", "resource.img", "recovery.img", "boot.img"};
    for(int i = 0; i < 5; i++){
        if(fileName[i].compare(str) == 0){
            printf("func is %s.\n", fileName[i].c_str());
            return true;
        }
    }
    return false;
}
