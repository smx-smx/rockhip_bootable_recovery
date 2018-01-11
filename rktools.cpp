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
    //Volume* v = volume_for_path(BASEPARAMER_PARTITION_NAME);
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


void *thrd_led_func(/*void *arg*/void *) {
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
void setFlashPoint(){
    Volume* v = volume_for_mount_point(sd_path);
    if(v == NULL){
        printf("(%s:%d) unknown volume for path [/mnt/external_sd]\n", __func__, __LINE__);
    }

    //int ret = -1;
    if(strcmp(v->fs_options, SD_POINT_0) == 0){
        printf("(%s:%d) sdcard is /dev/block/mmcblk0.\n", __func__, __LINE__);
        setenv(EMMC_POINT_NAME, EMMC_POINT_1, 0);
    }else{
        printf("(%s:%d) sdcard is /dev/block/mmcblk1.\n", __func__, __LINE__);
        setenv(EMMC_POINT_NAME, EMMC_POINT_0, 0);
    }
    printf("emmc_point is %s\n", getenv(EMMC_POINT_NAME));
}



void dumpCmdArgs(int argc, char** argv) {
    fprintf(stdout, "=== start %s:%d ===\n", __func__, __LINE__);
    for(int i = 0; i < argc; i++)
    {
        fprintf(stdout, "argv[%d] =  %s.\n", i, argv[i]);
    }
}
