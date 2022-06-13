#include <cstdio>
#include <string>

#include "rkupdate/Upgrade.h"
#include "rktools.h"
#include "roots.h"
#include "sdboot.h"

// dummy recovery ui
void* ui = nullptr;

static char *curPrompt = nullptr;;

static void my_handle_upgrade_callback(char *szPrompt){  
	printf("upgrade: %s\n", szPrompt);
}  

static void my_handle_upgrade_progress_callback(float portion, float seconds){
	printf("progress  portion: %.2f, seconds: %.2f\n", portion, seconds);
} 

int main(int argc, char *argv[]){
	if(argc < 2){
		fprintf(stderr, "Usage: %s [update.img]\n", argv[0]);
		return 1;
	}
  	
	load_volume_table();
  	setFlashPoint();
    SDBoot rksdboot;

	char *pFwPath = argv[1];
	bool bRet = do_rk_firmware_upgrade(pFwPath,
		(void *)my_handle_upgrade_callback,
		(void *)my_handle_upgrade_progress_callback);
	return 0;
}