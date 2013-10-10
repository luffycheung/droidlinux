/*
 **
 ** Copyright 2008, The Android Open Source Project
 ** Copyright 2010, Samsung Electronics Co. LTD
 **
 ** Licensed under the Apache License, Version 2.0 (the "License");
 ** you may not use this file except in compliance with the License.
 ** You may obtain a copy of the License at
 **
 **     http://www.apache.org/licenses/LICENSE-2.0
 **
 ** Unless required by applicable law or agreed to in writing, software
 ** distributed under the License is distributed on an "AS IS" BASIS,
 ** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 ** See the License for the specific language governing permissions and
 ** limitations under the License.
 */

#ifndef ANDROID_HARDWARE_CAMERA_SEC_H
#define ANDROID_HARDWARE_CAMERA_SEC_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>
#include <sys/mman.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <sys/poll.h>
#include <sys/stat.h>

#include <utils/threads.h>
#include <binder/MemoryBase.h>
#include <binder/MemoryHeapBase.h>
#include <utils/threads.h>


#include <linux/videodev2.h>
#include <videodev2_samsung.h>
#include "CameraParameters.h"

#include "JpegEncoder.h"

#include <camera/CameraHardwareInterface.h>

using namespace android;

//#include <camera/CameraHardwareInterface.h>

//namespace android {

#define ENABLE_ESD_PREVIEW_CHECK

#if defined(LOG_NDEBUG) && LOG_NDEBUG == 0
#define LOG_CAMERA LOGD
#define LOG_CAMERA_PREVIEW LOGD

#define LOG_TIME_DEFINE(n) \
    struct timeval time_start_##n, time_stop_##n; unsigned long log_time_##n = 0;

#define LOG_TIME_START(n) \
    gettimeofday(&time_start_##n, NULL);

#define LOG_TIME_END(n) \
    gettimeofday(&time_stop_##n, NULL); log_time_##n = measure_time(&time_start_##n, &time_stop_##n);

#define LOG_TIME(n) \
    log_time_##n

#else
#define LOG_CAMERA(...)
#define LOG_CAMERA_PREVIEW(...)
#define LOG_TIME_DEFINE(n)
#define LOG_TIME_START(n)
#define LOG_TIME_END(n)
#define LOG_TIME(n)
#endif

#define JOIN(x, y) JOIN_AGAIN(x, y)
#define JOIN_AGAIN(x, y) x ## y

#define FRONT_CAM VGA
#define BACK_CAM S5K4ECGX

#if !defined (FRONT_CAM) || !defined(BACK_CAM)
#error "Please define the Camera module"
#endif

#define S5K4ECGX_PREVIEW_WIDTH            720
#define S5K4ECGX_PREVIEW_HEIGHT           480
#define S5K4ECGX_SNAPSHOT_WIDTH           2560
#define S5K4ECGX_SNAPSHOT_HEIGHT          1920

#define S5K4ECGX_POSTVIEW_WIDTH           640
#define S5K4ECGX_POSTVIEW_WIDE_WIDTH      800
#define S5K4ECGX_POSTVIEW_HEIGHT          480
#define S5K4ECGX_POSTVIEW_BPP             16

#define S5K4ECGX_THUMBNAIL_WIDTH          320
#define S5K4ECGX_THUMBNAIL_HEIGHT         240
#define S5K4ECGX_THUMBNAIL_BPP            16

/* focal length of 3.43mm */
#define S5K4ECGX_FOCAL_LENGTH             343

#define VGA_PREVIEW_WIDTH               640
#define VGA_PREVIEW_HEIGHT              480
#define VGA_SNAPSHOT_WIDTH              640
#define VGA_SNAPSHOT_HEIGHT             480

#define VGA_THUMBNAIL_WIDTH             160
#define VGA_THUMBNAIL_HEIGHT            120
#define VGA_THUMBNAIL_BPP               16

/* focal length of 0.9mm */
#define VGA_FOCAL_LENGTH                90

#define MAX_BACK_CAMERA_PREVIEW_WIDTH       JOIN(BACK_CAM,_PREVIEW_WIDTH)
#define MAX_BACK_CAMERA_PREVIEW_HEIGHT      JOIN(BACK_CAM,_PREVIEW_HEIGHT)
#define MAX_BACK_CAMERA_SNAPSHOT_WIDTH      JOIN(BACK_CAM,_SNAPSHOT_WIDTH)
#define MAX_BACK_CAMERA_SNAPSHOT_HEIGHT     JOIN(BACK_CAM,_SNAPSHOT_HEIGHT)
#define BACK_CAMERA_POSTVIEW_WIDTH          JOIN(BACK_CAM,_POSTVIEW_WIDTH)
#define BACK_CAMERA_POSTVIEW_WIDE_WIDTH     JOIN(BACK_CAM,_POSTVIEW_WIDE_WIDTH)
#define BACK_CAMERA_POSTVIEW_HEIGHT         JOIN(BACK_CAM,_POSTVIEW_HEIGHT)
#define BACK_CAMERA_POSTVIEW_BPP            JOIN(BACK_CAM,_POSTVIEW_BPP)
#define BACK_CAMERA_THUMBNAIL_WIDTH         JOIN(BACK_CAM,_THUMBNAIL_WIDTH)
#define BACK_CAMERA_THUMBNAIL_HEIGHT        JOIN(BACK_CAM,_THUMBNAIL_HEIGHT)
#define BACK_CAMERA_THUMBNAIL_BPP           JOIN(BACK_CAM,_THUMBNAIL_BPP)

#define BACK_CAMERA_FOCAL_LENGTH            JOIN(BACK_CAM,_FOCAL_LENGTH)

#define MAX_FRONT_CAMERA_PREVIEW_WIDTH      JOIN(FRONT_CAM,_PREVIEW_WIDTH)
#define MAX_FRONT_CAMERA_PREVIEW_HEIGHT     JOIN(FRONT_CAM,_PREVIEW_HEIGHT)
#define MAX_FRONT_CAMERA_SNAPSHOT_WIDTH     JOIN(FRONT_CAM,_SNAPSHOT_WIDTH)
#define MAX_FRONT_CAMERA_SNAPSHOT_HEIGHT    JOIN(FRONT_CAM,_SNAPSHOT_HEIGHT)

#define FRONT_CAMERA_THUMBNAIL_WIDTH        JOIN(FRONT_CAM,_THUMBNAIL_WIDTH)
#define FRONT_CAMERA_THUMBNAIL_HEIGHT       JOIN(FRONT_CAM,_THUMBNAIL_HEIGHT)
#define FRONT_CAMERA_THUMBNAIL_BPP          JOIN(FRONT_CAM,_THUMBNAIL_BPP)
#define FRONT_CAMERA_FOCAL_LENGTH           JOIN(FRONT_CAM,_FOCAL_LENGTH)

#define DEFAULT_JPEG_THUMBNAIL_WIDTH        256
#define DEFAULT_JPEG_THUMBNAIL_HEIGHT       192

#define CAMERA_DEV_NAME   "/dev/video0"
#define CAMERA_DEV_NAME2  "/dev/video2"

#define CAMERA_DEV_NAME_TEMP "/data/videotmp_000"
#define CAMERA_DEV_NAME2_TEMP "/data/videotemp_002"

#define BPP             2
#define MIN(x, y)       (((x) < (y)) ? (x) : (y))
#define MAX_BUFFERS     8 //11

#define FIRST_AF_SEARCH_COUNT 80
#define SECOND_AF_SEARCH_COUNT 80
#define AF_PROGRESS 0x01
#define AF_SUCCESS 0x02
#define AF_DELAY 50000

/*
 * V 4 L 2   F I M C   E X T E N S I O N S
 *
 */
#define V4L2_CID_ROTATION                   (V4L2_CID_PRIVATE_BASE + 0)
#define V4L2_CID_PADDR_Y                    (V4L2_CID_PRIVATE_BASE + 1)
#define V4L2_CID_PADDR_CB                   (V4L2_CID_PRIVATE_BASE + 2)
#define V4L2_CID_PADDR_CR                   (V4L2_CID_PRIVATE_BASE + 3)
#define V4L2_CID_PADDR_CBCR                 (V4L2_CID_PRIVATE_BASE + 4)
#define V4L2_CID_STREAM_PAUSE               (V4L2_CID_PRIVATE_BASE + 53)

#define V4L2_CID_CAM_JPEG_MAIN_SIZE         (V4L2_CID_PRIVATE_BASE + 32)
#define V4L2_CID_CAM_JPEG_MAIN_OFFSET       (V4L2_CID_PRIVATE_BASE + 33)
#define V4L2_CID_CAM_JPEG_THUMB_SIZE        (V4L2_CID_PRIVATE_BASE + 34)
#define V4L2_CID_CAM_JPEG_THUMB_OFFSET      (V4L2_CID_PRIVATE_BASE + 35)
#define V4L2_CID_CAM_JPEG_POSTVIEW_OFFSET   (V4L2_CID_PRIVATE_BASE + 36)
#define V4L2_CID_CAM_JPEG_QUALITY           (V4L2_CID_PRIVATE_BASE + 37)

#define TPATTERN_COLORBAR           1
#define TPATTERN_HORIZONTAL         2
#define TPATTERN_VERTICAL           3

#define V4L2_PIX_FMT_YVYU           v4l2_fourcc('Y', 'V', 'Y', 'U')

/* FOURCC for FIMC specific */
#define V4L2_PIX_FMT_VYUY           v4l2_fourcc('V', 'Y', 'U', 'Y')
#define V4L2_PIX_FMT_NV16           v4l2_fourcc('N', 'V', '1', '6')
#define V4L2_PIX_FMT_NV61           v4l2_fourcc('N', 'V', '6', '1')
#define V4L2_PIX_FMT_NV12T          v4l2_fourcc('T', 'V', '1', '2')
/*
 * U S E R   D E F I N E D   T Y P E S
 *
 */

#define BACK_CAMERA_AUTO_FOCUS_DISTANCES_STR       "0.10,1.20,Infinity"
#define BACK_CAMERA_MACRO_FOCUS_DISTANCES_STR      "0.10,0.20,Infinity"
#define BACK_CAMERA_INFINITY_FOCUS_DISTANCES_STR   "0.10,1.20,Infinity"
#define FRONT_CAMERA_FOCUS_DISTANCES_STR           "0.20,0.25,Infinity"

static const int INITIAL_SKIP_FRAME = 3;
static const int EFFECT_SKIP_FRAME = 1;

struct fimc_buffer {
	void *start;
	size_t length;
};

struct yuv_fmt_list {
	const char *name;
	const char *desc;
	unsigned int fmt;
	int depth;
	int planes;
};

//s1 [Apply factory standard]
struct camsensor_date_info {
	unsigned int year;
	unsigned int month;
	unsigned int date;
};

enum CAMERA_ID {
	CAMERA_ID_BACK = 0, CAMERA_ID_FRONT = 1,
};

enum JPEG_QUALITY {
	JPEG_QUALITY_ECONOMY = 0,
	JPEG_QUALITY_NORMAL = 50,
	JPEG_QUALITY_SUPERFINE = 100,
	JPEG_QUALITY_MAX,
};

enum OBJECT_TRACKING {
	OBJECT_TRACKING_OFF, OBJECT_TRACKING_ON, OBJECT_TRACKING_MAX,
};

/*VT call*/
enum VT_MODE {
	VT_MODE_OFF, VT_MODE_ON, VT_MODE_MAX,
};

/*Camera sensor mode - Camcorder fix fps*/
enum SENSOR_MODE {
	SENSOR_MODE_CAMERA, SENSOR_MODE_MOVIE,
};

/*Camera Shot mode*/
enum SHOT_MODE {
	SHOT_MODE_SINGLE = 0,
	SHOT_MODE_CONTINUOUS = 1,
	SHOT_MODE_PANORAMA = 2,
	SHOT_MODE_SMILE = 3,
	SHOT_MODE_SELF = 6,
};

enum CHK_DATALINE {
	CHK_DATALINE_OFF, CHK_DATALINE_ON, CHK_DATALINE_MAX,
};

int m_touch_af_start_stop;

struct gps_info_latiude {
	unsigned int north_south;
	unsigned int dgree;
	unsigned int minute;
	unsigned int second;
} gpsInfoLatitude;
struct gps_info_longitude {
	unsigned int east_west;
	unsigned int dgree;
	unsigned int minute;
	unsigned int second;
} gpsInfoLongitude;
struct gps_info_altitude {
	unsigned int plus_minus;
	unsigned int dgree;
	unsigned int minute;
	unsigned int second;
} gpsInfoAltitude;

struct addrs_cap {
    unsigned int addr_y;
    unsigned int width;
    unsigned int height;
};

//	unsigned char* getJpeg(unsigned char *snapshot_data, int snapshot_size,
//			int *size);
//	unsigned char* yuv2Jpeg(unsigned char *raw_data, int raw_size,
//			int *jpeg_size, int width, int height, int pixel_format);
//

static double jpeg_ratio;
static int interleaveDataSize;
static int jpegLineLength;

v4l2_streamparm m_streamparm;
struct sec_cam_parm *m_params;
int m_flag_init;

int m_camera_id;

int m_cam_fd;

int m_cam_fd2;
struct pollfd m_events_c2;
struct pollfd m_events_c;
int m_flag_record_start;

int m_preview_v4lformat;
int m_preview_width;
int m_preview_height;
int m_preview_max_width;
int m_preview_max_height;

int m_snapshot_v4lformat;
int m_snapshot_width;
int m_snapshot_height;
int m_snapshot_max_width;
int m_snapshot_max_height;

int m_angle;
int m_anti_banding;
int m_wdr;
int m_anti_shake;
int m_zoom_level;
int m_object_tracking;
int m_smart_auto;
int m_beauty_shot;
int m_vintage_mode;
int m_face_detect;
int m_object_tracking_start_stop;
int m_recording_width;
int m_recording_height;
bool m_gps_enabled;
long m_gps_latitude;
long m_gps_longitude;
long m_gps_altitude;
long m_gps_timestamp;
int m_vtmode;
int m_sensor_mode; /*Camcorder fix fps */
int m_shot_mode; /* Shot mode */
int m_exif_orientation;
int m_blur_level;
int m_chk_dataline;
int m_video_gamma;
int m_slow_ae;
int m_caf_on_off;
int m_default_imei;
int m_camera_af_flag;

int m_flag_camera_start;

int m_jpeg_fd;
int m_jpeg_thumbnail_width;
int m_jpeg_thumbnail_height;
int m_jpeg_quality;

int *rgb = NULL;
int *ybuf = NULL;

int yuv_tbl_ready=0;
int y1192_tbl[256];
int v1634_tbl[256];
int v833_tbl[256];
int u400_tbl[256];
int u2066_tbl[256];

const __u8  *mCameraSensorName;

void        *mCallbackCookie = NULL;
data_callback       mDataCb = NULL;

int m_postview_offset;

int mSkipFrame = 0;
int32_t     mMsgEnabled = 0;

CameraParameters mParameters;
CameraParameters mInternalParameters;

sp<MemoryHeapBase>  mRawHeap;

Vector<Size> mSupportedPreviewSizes;

static void setJpegRatio(double ratio) {
	if ((ratio < 0) || (ratio > 1))
		return;

	jpeg_ratio = ratio;
}

static double getJpegRatio() {
	return jpeg_ratio;
}

static void setInterleaveDataSize(int x) {
	interleaveDataSize = x;
}

static int getInterleaveDataSize() {
	return interleaveDataSize;
}

static void setJpegLineLength(int x) {
	jpegLineLength = x;
}

static int getJpegLineLength() {
	return jpegLineLength;
}

#ifdef ENABLE_ESD_PREVIEW_CHECK
int m_esd_check_count;
#endif // ENABLE_ESD_PREVIEW_CHECK
exif_attribute_t mExifInfo;

struct fimc_buffer m_capture_buf;

struct fimc_buffer m_buffers_c[MAX_BUFFERS];

extern unsigned long measure_time(struct timeval *start, struct timeval *stop);

//}
//;
// namespace android

#endif