project(platform_bootable_recovery C CXX)

set(CROSS FALSE)
set(CMAKE_CXX_STANDARD 14)

add_definitions(-D_FILE_OFFSET_BITS=64 -D_LARGE_FILE)

include_directories(
	${CMAKE_SOURCE_DIR}
	external/platform/external/selinux/libselinux/include
	external/platform/bionic/libc/include
	external/platform/system/core/include
	external/platform/system/core/logwrapper/include
	external/platform/system/core/base/include
	external/platform/system/core/fs_mgr/include
	external/platform/system/core/fs_mgr/include_fstab
	external/platform/system/core/libcutils/include
	external/platform/system/extras/ext4_utils/include
	external/platform/system/extras/ext4_utils/include/ext4_utils
	external/platform/system/vold
	external/platform/external/e2fsprogs/lib
	external/platform/system/core/libziparchive/include
)
if(NOT CROSS)
	include_directories(
		external/platform/system/core/debuggerd/libdebuggerd/test
	)
endif()

#add_compile_options(-fno-rtti)

add_library(android_deps
	external/platform/system/core/base/logging.cpp
	external/platform/system/core/libcutils/properties.cpp
	external/platform/external/e2fsprogs/lib/uuid/gen_uuid.c
	external/platform/external/e2fsprogs/lib/uuid/unpack.c
	external/platform/system/extras/ext4_utils/ext4_utils.c
	external/platform/system/core/fs_mgr/fs_mgr_boot_config.cpp
	external/platform/external/e2fsprogs/lib/uuid/pack.c
	external/platform/system/core/base/file.cpp
	external/platform/system/core/base/strings.cpp
	external/platform/system/core/base/stringprintf.cpp
	external/platform/system/core/base/properties.cpp
	external/platform/system/core/fs_mgr/fs_mgr_fstab.cpp
	external/platform/system/core/fs_mgr/fs_mgr_format.cpp
	external/platform/system/core/fs_mgr/fs_mgr_slotselect.cpp
	external/platform/system/extras/ext4_utils/wipe.c
	external/platform/system/extras/ext4_utils/ext4_sb.c
	external/platform/system/core/logwrapper/logwrap.c
	external/platform/system/core/logwrapper/logwrap.c
	external/platform/system/core/libcutils/klog.cpp
	external/platform/system/core/libcutils/android_get_control_file.cpp
	#external/platform/system/core/liblog/logger_lock.c
	#external/platform/system/core/liblog/logger_write.c
	#external/platform/system/core/liblog/logger_read.c
)

add_subdirectory(rkupdate)

add_executable(test_update test_update.cpp)
target_link_libraries(test_update rkupdate)
