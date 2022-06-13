project(platform_bootable_recovery C CXX)

set(CROSS FALSE)
set(CMAKE_CXX_STANDARD 14)

add_definitions(-D_FILE_OFFSET_BITS=64 -D_LARGE_FILE)

include_directories(
	${CMAKE_SOURCE_DIR}
	external/platform/external/selinux/libselinux/include
	external/platform/bionic/libc/include
	external/system/core/include
	external/system/core/logwrapper/include
	external/system/core/base/include
	external/system/core/fs_mgr/include
	external/system/core/fs_mgr/include_fstab
	external/system/core/libcutils/include
	external/platform/system/extras/ext4_utils/include
	external/platform/system/extras/ext4_utils/include/ext4_utils
	external/system/vold
	external/platform/external/e2fsprogs/lib
	external/system/core/libziparchive/include
)
if(NOT CROSS)
	include_directories(
		external/system/core/debuggerd/libdebuggerd/test
	)
endif()

#add_compile_options(-fno-rtti)

add_library(android_deps
	external/system/core/base/logging.cpp
	external/system/core/libcutils/properties.cpp
	external/platform/external/e2fsprogs/lib/uuid/gen_uuid.c
	external/platform/external/e2fsprogs/lib/uuid/unpack.c
	external/platform/system/extras/ext4_utils/ext4_utils.c
	external/system/core/fs_mgr/fs_mgr_boot_config.cpp
	external/platform/external/e2fsprogs/lib/uuid/pack.c
	external/system/core/base/file.cpp
	external/system/core/base/strings.cpp
	external/system/core/base/stringprintf.cpp
	external/system/core/base/properties.cpp
	external/system/core/fs_mgr/fs_mgr_fstab.cpp
	external/system/core/fs_mgr/fs_mgr_format.cpp
	external/system/core/fs_mgr/fs_mgr_slotselect.cpp
	external/platform/system/extras/ext4_utils/wipe.c
	external/platform/system/extras/ext4_utils/ext4_sb.c
	external/system/core/logwrapper/logwrap.c
	external/system/core/logwrapper/logwrap.c
	external/system/core/libcutils/klog.cpp
	external/system/core/libcutils/android_get_control_file.cpp
	#external/system/core/liblog/logger_lock.c
	#external/system/core/liblog/logger_write.c
	#external/system/core/liblog/logger_read.c
)

add_subdirectory(rkupdate)

add_executable(test_update test_update.cpp)
target_link_libraries(test_update rkupdate)
