/*
 * In this project we want to investigate if we can take a picture
 * when another app is taking picture using the /dev/video devices.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <jni.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <android/log.h>
#include <getopt.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <malloc.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <asm/types.h>
#include <binder/MemoryBase.h>
#include <binder/MemoryHeapBase.h>
#include "videodev2.h"
#include "videodev2_samsung.h"
#include <assert.h>
#include <sys/poll.h>
#include "access-camera.h"
#include <properties.h>
#include <android/bitmap.h>

using namespace android;

#define JPEG_FROM_SENSOR
//#define DUMP_YUV

#define LOG_TAG "NativeLib"
//#define LOGI(...) __android_log_print(ANDROID_LOG_VERBOSE, LOG_TAG, __VA_ARGS__)
//#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__)
//#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
//#define LOGW(...) __android_log_print(ANDROID_LOG_WARN, LOG_TAG, __VA_ARGS__)
//#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

//static int CAMERA_MSG_RAW_IMAGE = 0x080;
//static int CAMERA_MSG_COMPRESSED_IMAGE = 0x100;

#define VIDEO_COMMENT_MARKER_H          0xFFBE
#define VIDEO_COMMENT_MARKER_L          0xFFBF
#define VIDEO_COMMENT_MARKER_LENGTH     4
#define JPEG_EOI_MARKER                 0xFFD9
#define HIBYTE(x) (((x) >> 8) & 0xFF)
#define LOBYTE(x) ((x) & 0xFF)

#define ALIGN_TO_32B(x)   ((((x) + (1 <<  5) - 1) >>  5) <<  5)
#define ALIGN_TO_128B(x)  ((((x) + (1 <<  7) - 1) >>  7) <<  7)
#define ALIGN_TO_8KB(x)   ((((x) + (1 << 13) - 1) >> 13) << 13)

#define CLEAR(x) memset (&(x), 0, sizeof (x))

struct buffer {
	void * start;
	size_t length;
};

#ifdef __cplusplus
extern "C" {
#endif

static int camera_device;

JNIEXPORT jint JNICALL Java_com_example_droidlinuxcamara_MainActivity_openTestFile() {
	LOGI("calling Open File");
	FILE * file;
	file = fopen("file.txt", "a+");
	LOGI("Calling fopen()");
	//fprintf(file, "%s", "This is a test file");
	LOGI("write");
	fclose(file);
	LOGI("Closed");
}

#define V4L2_CTRL_CLASS_CAMERA 0x009a0000  /* Camera class controls */

#define CHECK(return_value)                                          \
    if (return_value < 0) {                                          \
        LOGE("%s::%d fail. errno: %s, m_camera_id = %d\n",           \
             __func__, __LINE__, strerror(errno), m_camera_id);      \
        return -1;                                                   \
    }

#define CHECK_PTR(return_value)                                      \
    if (return_value < 0) {                                          \
        LOGE("%s::%d fail, errno: %s, m_camera_id = %d\n",           \
             __func__,__LINE__, strerror(errno), m_camera_id);       \
        return NULL;                                                 \
    }

static int get_pixel_depth(unsigned int fmt) {
	int depth = 0;

	switch (fmt) {
	case V4L2_PIX_FMT_NV12:
		depth = 12;
		break;
	case V4L2_PIX_FMT_NV12T:
		depth = 12;
		break;
	case V4L2_PIX_FMT_NV21:
		depth = 12;
		break;
	case V4L2_PIX_FMT_YUV420:
		depth = 12;
		break;

	case V4L2_PIX_FMT_RGB565:
	case V4L2_PIX_FMT_YUYV:
	case V4L2_PIX_FMT_YVYU:
	case V4L2_PIX_FMT_UYVY:
	case V4L2_PIX_FMT_VYUY:
	case V4L2_PIX_FMT_NV16:
	case V4L2_PIX_FMT_NV61:
	case V4L2_PIX_FMT_YUV422P:
		depth = 16;
		break;

	case V4L2_PIX_FMT_RGB32:
		depth = 32;
		break;
	}

	return depth;
}

inline int m_frameSize(int format, int width, int height) {
	int size = 0;

	switch (format) {
	case V4L2_PIX_FMT_YUV420:
	case V4L2_PIX_FMT_NV12:
	case V4L2_PIX_FMT_NV21:
		size = (width * height * 3 / 2);
		break;

	case V4L2_PIX_FMT_NV12T:
		size = ALIGN_TO_8KB(ALIGN_TO_128B(width) * ALIGN_TO_32B(height)) +
		ALIGN_TO_8KB(ALIGN_TO_128B(width) * ALIGN_TO_32B(height / 2));
		break;

	case V4L2_PIX_FMT_YUV422P:
	case V4L2_PIX_FMT_YUYV:
	case V4L2_PIX_FMT_UYVY:
		size = (width * height * 2);
		break;

	default:
		LOGE("ERR(%s):Invalid V4L2 pixel format(%d)\n", __func__, format);
	case V4L2_PIX_FMT_RGB565:
		size = (width * height * BPP);
		break;
	}

	return size;
}

#define ALIGN_W(x)      (((x) + 0x7F) & (~0x7F))    // Set as multiple of 128
#define ALIGN_H(x)      (((x) + 0x1F) & (~0x1F))    // Set as multiple of 32
#define ALIGN_BUF(x)    (((x) + 0x1FFF)& (~0x1FFF)) // Set as multiple of 8K
static int init_preview_buffers(struct fimc_buffer *buffers, int width,
		int height, unsigned int fmt) {
	int i, len;

	if (fmt == V4L2_PIX_FMT_NV12T) {
		len = ALIGN_BUF(ALIGN_W(width) * ALIGN_H(height))
				+ ALIGN_BUF(ALIGN_W(width) * ALIGN_H(height / 2));
	} else {
		len = (width * height * get_pixel_depth(fmt)) / 8;
	}

	for (i = 0; i < MAX_BUFFERS; i++) {
		buffers[i].length = len;
	}

	return 0;
}

static int fimc_poll(struct pollfd *events) {
	int ret;

	/* 10 second delay is because sensor can take a long time
	 * to do auto focus and capture in dark settings
	 */
	ret = poll(events, 1, 10000);
	if (ret < 0) {
		LOGE("ERR(%s):poll error\n", __func__);
		return ret;
	}

	if (ret == 0) {
		LOGE("ERR(%s):No data in 10 secs..\n", __func__);
		return ret;
	}

	return ret;
}

int previewPoll(bool preview) {
	int ret;

	if (preview) {

		ret = poll(&m_events_c, 1, 1000);
	} else {
		ret = poll(&m_events_c2, 1, 1000);
	}

	if (ret < 0) {
		LOGE("ERR(%s):poll error\n", __func__);
		return ret;
	}

	if (ret == 0) {
		LOGE("ERR(%s):No data in 1 secs.. Camera Device Reset \n", __func__);
		return ret;
	}

	return ret;
}

static int fimc_v4l2_querycap(int fp) {
	struct v4l2_capability cap;
	int ret = 0;

	ret = ioctl(fp, VIDIOC_QUERYCAP, &cap);

	if (ret < 0) {
		LOGE("ERR(%s):VIDIOC_QUERYCAP failed\n", __func__);
		return -1;
	}
	LOGI("VIDIOC_QUERYCAP supported!");
	if (!(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE)) {
		LOGE("ERR(%s):no capture devices\n", __func__);
		return -1;
	}

	return ret;
}

static const __u8* fimc_v4l2_enuminput(int fp, int index)
{
	static struct v4l2_input input;

	input.index = index;
	if (ioctl(fp, VIDIOC_ENUMINPUT, &input) != 0) {
		LOGE("ERR(%s):No matching index found\n", __func__);
		return NULL;
	}
	LOGI("VIDIOC_ENUMINPUT supported!");
	LOGI("Name of input channel[%d] is %s\n", input.index, input.name);

	return input.name;
}

static int fimc_v4l2_s_input(int fp, int index)
{
	struct v4l2_input input;
	int ret;

	input.index = index;

	ret = ioctl(fp, VIDIOC_S_INPUT, &input);
	if (ret < 0) {
		LOGE("ERR(%s):VIDIOC_S_INPUT failed\n", __func__);
		return ret;
	}
	LOGI("VIDIOC_S_INPUT supported!");

	return ret;
}

static int fimc_v4l2_s_fmt(int fp, int width, int height, unsigned int fmt, int flag_capture)
{
	struct v4l2_format v4l2_fmt;
	struct v4l2_pix_format pixfmt;
	int ret;

	v4l2_fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

	memset(&pixfmt, 0, sizeof(pixfmt));

	pixfmt.width = width;
	pixfmt.height = height;
	pixfmt.pixelformat = fmt;

	pixfmt.sizeimage = (width * height * get_pixel_depth(fmt)) / 8;

	pixfmt.field = V4L2_FIELD_NONE;

	v4l2_fmt.fmt.pix = pixfmt;

	/* Set up for capture */
	ret = ioctl(fp, VIDIOC_S_FMT, &v4l2_fmt);
	if (ret < 0) {
		LOGE("ERR(%s):VIDIOC_S_FMT failed\n", __func__);
		return -1;
	}
	LOGI("VIDIOC_S_FMT supported!");
	return 0;
}

static int fimc_v4l2_s_fmt_cap(int fp, int width, int height, unsigned int fmt)
{
	struct v4l2_format v4l2_fmt;
	struct v4l2_pix_format pixfmt;
	int ret;

	memset(&pixfmt, 0, sizeof(pixfmt));

	v4l2_fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

	pixfmt.width = width;
	pixfmt.height = height;
	pixfmt.pixelformat = fmt;
	if (fmt == V4L2_PIX_FMT_JPEG) {
		pixfmt.colorspace = V4L2_COLORSPACE_JPEG;
	}

	pixfmt.sizeimage = (width * height * get_pixel_depth(fmt)) / 8;

	v4l2_fmt.fmt.pix = pixfmt;

	//LOGE("ori_w %d, ori_h %d, w %d, h %d\n", width, height, v4l2_fmt.fmt.pix.width, v4l2_fmt.fmt.pix.height);

	/* Set up for capture */
	ret = ioctl(fp, VIDIOC_S_FMT, &v4l2_fmt);
	if (ret < 0) {
		LOGE("ERR(%s):VIDIOC_S_FMT failed\n", __func__);
		return ret;
	}

	LOGI("VIDIOC_S_FMT supported!");

	return ret;
}

static int fimc_v4l2_enum_fmt(int fp, unsigned int fmt)
{
	struct v4l2_fmtdesc fmtdesc;
	int found = 0;

	fmtdesc.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	fmtdesc.index = 0;

	while (ioctl(fp, VIDIOC_ENUM_FMT, &fmtdesc) == 0) {
		LOGI("VIDIOC_ENUM_FMT supported!");
		if (fmtdesc.pixelformat == fmt) {
			LOGI("passed fmt = %#x found pixel format[%d]: %s\n", fmt, fmtdesc.index, fmtdesc.description);
			found = 1;
			break;
		}

		fmtdesc.index++;
	}

	if (!found) {
		LOGE("unsupported pixel format\n");
		return -1;
	}

	return 0;
}

static int fimc_v4l2_reqbufs(int fp, enum v4l2_buf_type type, int nr_bufs)
{
	struct v4l2_requestbuffers req;
	int ret;

	req.count = nr_bufs;
	req.type = type;
	req.memory = V4L2_MEMORY_MMAP;

	ret = ioctl(fp, VIDIOC_REQBUFS, &req);
	if (ret < 0) {
		LOGE("ERR(%s):VIDIOC_REQBUFS failed\n", __func__);
		return -1;
	}
	LOGI("VIDIOC_REQBUFS supported!");

	return req.count;
}
static int fimc_v4l2_querybuf(int fp, struct fimc_buffer *buffers, enum v4l2_buf_type type, int nr_frames)
{
	struct v4l2_buffer v4l2_buf;
	int i, ret;

	for (i = 0; i < nr_frames; i++) {
		v4l2_buf.type = type;
		v4l2_buf.memory = V4L2_MEMORY_MMAP;
		v4l2_buf.index = i;

		ret = ioctl(fp , VIDIOC_QUERYBUF, &v4l2_buf);
		if (ret < 0) {
			LOGE("ERR(%s):VIDIOC_QUERYBUF failed\n", __func__);
			return -1;
		}
		LOGI("VIDIOC_QUERYBUF supported!");

		if (nr_frames == 1) {
			buffers[i].length = v4l2_buf.length;
			if ((buffers[i].start = (char *)mmap(0, v4l2_buf.length, PROT_READ | PROT_WRITE, MAP_SHARED,
									fp, v4l2_buf.m.offset)) < 0) {
				LOGE("%s %d] mmap() failed\n",__func__, __LINE__);
				return -1;
			}

			//LOGI("buffers[%d].start = %p v4l2_buf.length = %d", i, buffers[i].start, v4l2_buf.length);
		} else {

#if defined DUMP_YUV || defined (SEND_YUV_RECORD_DATA)
			buffers[i].length = v4l2_buf.length;
			if ((buffers[i].start = (char *)mmap(0, v4l2_buf.length, PROT_READ | PROT_WRITE, MAP_SHARED,
									fp, v4l2_buf.m.offset)) < 0) {
				LOGE("%s %d] mmap() failed\n",__func__, __LINE__);
				return -1;
			}

			//LOGI("buffers[%d].start = %p v4l2_buf.length = %d", i, buffers[i].start, v4l2_buf.length);
#endif
		}
	}

	return 0;
}

static int fimc_v4l2_streamon(int fp)
{
	enum v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	int ret;

	ret = ioctl(fp, VIDIOC_STREAMON, &type);
	if (ret < 0) {
		LOGE("ERR(%s):VIDIOC_STREAMON failed\n", __func__);
		return ret;
	}
	LOGI("VIDIOC_STREAMON supported!");

	return ret;
}

static int fimc_v4l2_streamoff(int fp)
{
	enum v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	int ret;

	LOGI("%s :", __func__);
	ret = ioctl(fp, VIDIOC_STREAMOFF, &type);
	if (ret < 0) {
		LOGE("ERR(%s):VIDIOC_STREAMOFF failed\n", __func__);
		return ret;
	}

	LOGI("VIDIOC_STREAMOFF supported!");

	return ret;
}

static int fimc_v4l2_qbuf(int fp, int index)
{
	struct v4l2_buffer v4l2_buf;
	int ret;

	v4l2_buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	v4l2_buf.memory = V4L2_MEMORY_MMAP;
	v4l2_buf.index = index;

	ret = ioctl(fp, VIDIOC_QBUF, &v4l2_buf);
	if (ret < 0) {
		LOGE("ERR(%s):VIDIOC_QBUF failed\n", __func__);
		return ret;
	}

	LOGI("VIDIOC_QBUF supported!");
	return 0;
}

static int fimc_v4l2_dqbuf(int fp)
{
	struct v4l2_buffer v4l2_buf;
	int ret;

	v4l2_buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	v4l2_buf.memory = V4L2_MEMORY_MMAP;

	ret = ioctl(fp, VIDIOC_DQBUF, &v4l2_buf);
	if (ret < 0) {
		LOGE("ERR(%s):VIDIOC_DQBUF failed, dropped frame\n", __func__);
		return ret;
	}

	LOGI("VIDIOC_DQBUF supported!");
	return v4l2_buf.index;
}

static int fimc_v4l2_g_ctrl(int fp, unsigned int id)
{
	struct v4l2_control ctrl;
	int ret;

	ctrl.id = id;

	ret = ioctl(fp, VIDIOC_G_CTRL, &ctrl);
	if (ret < 0) {
		LOGE("ERR(%s): VIDIOC_G_CTRL(id = 0x%x (%d)) failed, ret = %d\n",
				__func__, id, id-V4L2_CID_PRIVATE_BASE, ret);
		return ret;
	}

	LOGI("VIDIOC_G_CTRL supported!");
	return ctrl.value;
}

static int fimc_v4l2_s_ctrl(int fp, unsigned int id, unsigned int value)
{
	struct v4l2_control ctrl;
	int ret;

	ctrl.id = id;
	ctrl.value = value;

	ret = ioctl(fp, VIDIOC_S_CTRL, &ctrl);
	if (ret < 0) {
		LOGE("ERR(%s):VIDIOC_S_CTRL(id = %#x (%d), value = %d) failed ret = %d\n",
				__func__, id, id-V4L2_CID_PRIVATE_BASE, value, ret);

		return ret;
	}
	LOGI("VIDIOC_S_CTRL supported!");

	return ctrl.value;
}

static int fimc_v4l2_s_ext_ctrl(int fp, unsigned int id, void *value)
{
	struct v4l2_ext_controls ctrls;
	struct v4l2_ext_control ctrl;
	int ret;

	ctrl.id = id;
//	ctrl.reserved = value;

	ctrls.ctrl_class = V4L2_CTRL_CLASS_CAMERA;
	ctrls.count = 1;
	ctrls.controls = &ctrl;

	ret = ioctl(fp, VIDIOC_S_EXT_CTRLS, &ctrls);
	if (ret < 0) {
		LOGE("ERR(%s):VIDIOC_S_EXT_CTRLS failed\n", __func__);
	}

	LOGI("VIDIOC_S_EXT_CTRLS supported!");
	return ret;
}

static int fimc_v4l2_g_parm(int fp, struct v4l2_streamparm *streamparm)
{
	int ret;

	streamparm->type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

	ret = ioctl(fp, VIDIOC_G_PARM, streamparm);
	if (ret < 0) {
		LOGE("ERR(%s):VIDIOC_G_PARM failed\n", __func__);
		return -1;
	}

	LOGI("VIDIOC_G_PARM supported!");

	LOGI("%s : timeperframe: numerator %d, denominator %d\n", __func__,
			streamparm->parm.capture.timeperframe.numerator,
			streamparm->parm.capture.timeperframe.denominator);

	return 0;
}

static int fimc_v4l2_s_parm(int fp, struct v4l2_streamparm *streamparm)
{
	int ret;

	streamparm->type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

	ret = ioctl(fp, VIDIOC_S_PARM, streamparm);
	if (ret < 0) {
		LOGE("ERR(%s):VIDIOC_S_PARM failed\n", __func__);
		return ret;
	}

	LOGI("VIDIOC_S_PARM supported!");

	return 0;
}

void init() {
	m_flag_init = 0;
	m_camera_id = CAMERA_ID_BACK;
	m_cam_fd = -1;
	m_cam_fd2 = -1;
	m_preview_v4lformat = V4L2_PIX_FMT_NV21;
	m_preview_width = 0;
	m_preview_height = 0;
	m_preview_max_width = MAX_BACK_CAMERA_PREVIEW_WIDTH;
	m_preview_max_height = MAX_BACK_CAMERA_PREVIEW_HEIGHT;
	m_snapshot_v4lformat = -1;
	m_snapshot_width = 0;
	m_snapshot_height = 0;
	m_snapshot_max_width = MAX_BACK_CAMERA_SNAPSHOT_WIDTH;
	m_snapshot_max_height = MAX_BACK_CAMERA_SNAPSHOT_HEIGHT;
	m_angle= -1;
	m_anti_banding = -1;
	m_wdr = -1;
	m_anti_shake = -1;
	m_zoom_level = -1;
	m_object_tracking = -1;
	m_smart_auto = -1;
	m_beauty_shot = -1;
	m_vintage_mode = -1;
	m_face_detect = -1;
	m_gps_enabled = false;
	m_gps_latitude = -1;
	m_gps_longitude = -1;
	m_gps_altitude = -1;
	m_gps_timestamp = -1;
	m_vtmode = 0;
	m_sensor_mode = -1;
	m_shot_mode = -1;
	m_exif_orientation = -1;
	m_blur_level = -1;
	m_chk_dataline = -1;
	m_video_gamma = -1;
	m_slow_ae = -1;
	m_camera_af_flag = -1;
	m_flag_camera_start = 0;
	m_jpeg_thumbnail_width =0;
	m_jpeg_thumbnail_height =0;
	m_jpeg_quality =100;
	m_params = (struct sec_cam_parm*)&m_streamparm.parm.raw_data;
	struct v4l2_captureparm capture;
	m_params->capture.timeperframe.numerator = 1;
	m_params->capture.timeperframe.denominator = 0;
	m_params->contrast = -1;
	m_params->effects = -1;
	m_params->brightness = -1;
	m_params->flash_mode = -1;
	m_params->focus_mode = -1;
	m_params->iso = -1;
	m_params->metering = -1;
	m_params->saturation = -1;
	m_params->scene_mode = -1;
	m_params->sharpness = -1;
	m_params->white_balance = -1;

	int rawHeapSize = sizeof(struct addrs_cap);
	LOGI("mRawHeap : MemoryHeapBase(previewHeapSize(%d))", rawHeapSize);
//	mRawHeap = new MemoryHeapBase(rawHeapSize);
//	if (mRawHeap->getHeapID() < 0) {
//		LOGE("ERR(%s): Raw heap creation fail", __func__);
//		mRawHeap.clear();
//	}

//	memset(&m_capture_buf, 0, sizeof(m_capture_buf));
}

const __u8* getCameraSensorName(void)
{
	LOGI("%s", __func__);

	return fimc_v4l2_enuminput(m_cam_fd, m_camera_id);
}

void setExifFixedAttribute() {
	char property[PROPERTY_VALUE_MAX];

	//2 0th IFD TIFF Tags
	//3 Maker
//    property_get("ro.product.brand", property, EXIF_DEF_MAKER);
	strncpy((char *) mExifInfo.maker, property, sizeof(mExifInfo.maker) - 1);
	mExifInfo.maker[sizeof(mExifInfo.maker) - 1] = '\0';
	//3 Model
//    property_get("ro.product.model", property, EXIF_DEF_MODEL);
	strncpy((char *) mExifInfo.model, property, sizeof(mExifInfo.model) - 1);
	mExifInfo.model[sizeof(mExifInfo.model) - 1] = '\0';
	//3 Software
//    property_get("ro.build.id", property, EXIF_DEF_SOFTWARE);
	strncpy((char *) mExifInfo.software, property,
			sizeof(mExifInfo.software) - 1);
	mExifInfo.software[sizeof(mExifInfo.software) - 1] = '\0';

	//3 YCbCr Positioning
	mExifInfo.ycbcr_positioning = EXIF_DEF_YCBCR_POSITIONING;

	//2 0th IFD Exif Private Tags
	//3 F Number
	mExifInfo.fnumber.num = EXIF_DEF_FNUMBER_NUM;
	mExifInfo.fnumber.den = EXIF_DEF_FNUMBER_DEN;
	//3 Exposure Program
	mExifInfo.exposure_program = EXIF_DEF_EXPOSURE_PROGRAM;
	//3 Exif Version
	memcpy(mExifInfo.exif_version, EXIF_DEF_EXIF_VERSION,
			sizeof(mExifInfo.exif_version));
	//3 Aperture
	uint32_t av =
	APEX_FNUM_TO_APERTURE((double)mExifInfo.fnumber.num/mExifInfo.fnumber.den);
	mExifInfo.aperture.num = av * EXIF_DEF_APEX_DEN;
	mExifInfo.aperture.den = EXIF_DEF_APEX_DEN;
	//3 Maximum lens aperture
	mExifInfo.max_aperture.num = mExifInfo.aperture.num;
	mExifInfo.max_aperture.den = mExifInfo.aperture.den;
	//3 Lens Focal Length
	if (m_camera_id == CAMERA_ID_BACK)
	mExifInfo.focal_length.num = BACK_CAMERA_FOCAL_LENGTH;
	else
	mExifInfo.focal_length.num = FRONT_CAMERA_FOCAL_LENGTH;

	mExifInfo.focal_length.den = EXIF_DEF_FOCAL_LEN_DEN;
	//3 User Comments
	strcpy((char *) mExifInfo.user_comment, EXIF_DEF_USERCOMMENTS);
	//3 Color Space information
	mExifInfo.color_space = EXIF_DEF_COLOR_SPACE;
	//3 Exposure Mode
	mExifInfo.exposure_mode = EXIF_DEF_EXPOSURE_MODE;

	//2 0th IFD GPS Info Tags
	unsigned char gps_version[4] = {0x02, 0x02, 0x00, 0x00};
	memcpy(mExifInfo.gps_version_id, gps_version, sizeof(gps_version));

	//2 1th IFD TIFF Tags
	mExifInfo.compression_scheme = EXIF_DEF_COMPRESSION;
	mExifInfo.x_resolution.num = EXIF_DEF_RESOLUTION_NUM;
	mExifInfo.x_resolution.den = EXIF_DEF_RESOLUTION_DEN;
	mExifInfo.y_resolution.num = EXIF_DEF_RESOLUTION_NUM;
	mExifInfo.y_resolution.den = EXIF_DEF_RESOLUTION_DEN;
	mExifInfo.resolution_unit = EXIF_DEF_RESOLUTION_UNIT;
}

int initCamera(int index) {
	LOGI("initCamera() called.");
	LOGI("%s :", __func__);
	int ret = 0;

	if (!m_flag_init) {
		/* Arun C
		 * Reset the lense position only during camera starts; don't do
		 * reset between shot to shot
		 */
		m_camera_af_flag = -1;

		m_cam_fd = open(CAMERA_DEV_NAME, O_RDWR);
		if (m_cam_fd < 0) {
			LOGE("ERR(%s):Cannot open %s (error : %s)\n", __func__,
					CAMERA_DEV_NAME, strerror(errno));
			return -1;
		}
		LOGI("%s: open(%s) --> m_cam_fd %d", __FUNCTION__, CAMERA_DEV_NAME, m_cam_fd);

		LOGE("initCamera: m_cam_fd(%d), m_jpeg_fd(%d)", m_cam_fd, m_jpeg_fd);

		ret = fimc_v4l2_querycap(m_cam_fd);
		CHECK(ret);
		if (!fimc_v4l2_enuminput(m_cam_fd, index)) {
			return -1;
		}
		ret = fimc_v4l2_s_input(m_cam_fd, index);
		CHECK(ret);

		m_cam_fd2 = open(CAMERA_DEV_NAME2, O_RDWR);
		LOGI("%s: open(%s) --> m_cam_fd2 = %d", __FUNCTION__, CAMERA_DEV_NAME2, m_cam_fd2);
		if (m_cam_fd2 < 0) {
			LOGE("ERR(%s):Cannot open %s (error : %s)\n", __func__,
					CAMERA_DEV_NAME2, strerror(errno));
			return -1;
		}

		LOGE("initCamera: m_cam_fd2(%d)", m_cam_fd2);

		ret = fimc_v4l2_querycap(m_cam_fd2);
		CHECK(ret);
		if (!fimc_v4l2_enuminput(m_cam_fd2, index)) {
			return -1;
		}
		ret = fimc_v4l2_s_input(m_cam_fd2, index);
		CHECK(ret);

		m_camera_id = index;

		switch (m_camera_id) {
			case CAMERA_ID_FRONT:
			m_preview_max_width = MAX_FRONT_CAMERA_PREVIEW_WIDTH;
			m_preview_max_height = MAX_FRONT_CAMERA_PREVIEW_HEIGHT;
			m_snapshot_max_width = MAX_FRONT_CAMERA_SNAPSHOT_WIDTH;
			m_snapshot_max_height = MAX_FRONT_CAMERA_SNAPSHOT_HEIGHT;
			break;

			case CAMERA_ID_BACK:
			m_preview_max_width = MAX_BACK_CAMERA_PREVIEW_WIDTH;
			m_preview_max_height = MAX_BACK_CAMERA_PREVIEW_HEIGHT;
			m_snapshot_max_width = MAX_BACK_CAMERA_SNAPSHOT_WIDTH;
			m_snapshot_max_height = MAX_BACK_CAMERA_SNAPSHOT_HEIGHT;
			break;
		}

		setExifFixedAttribute();

		m_flag_init = 1;
		LOGI("%s : initialized", __FUNCTION__);
	}
	return 0;
}

int setFlashMode(int flash_mode) {
	LOGI("%s(flash_mode(%d))", __func__, flash_mode);

	if (flash_mode <= FLASH_MODE_BASE || FLASH_MODE_MAX <= flash_mode) {
		LOGE("ERR(%s):Invalid flash_mode (%d)", __func__, flash_mode);
		return -1;
	}

	if (m_params->flash_mode != flash_mode) {
		m_params->flash_mode = flash_mode;
		if (m_flag_camera_start) {
			if (fimc_v4l2_s_ctrl(m_cam_fd, V4L2_CID_CAMERA_FLASH_MODE,
							flash_mode) < 0) {
				LOGE("ERR(%s):Fail on V4L2_CID_CAMERA_FLASH_MODE", __func__);
				return -1;
			}
		}
	}

	return 0;
}

int setPreviewSize(int width, int height, int pixel_format)
{
	LOGI("%s(width(%d), height(%d), format(%d))", __func__, width, height, pixel_format);

	int v4lpixelformat = pixel_format;

	if (v4lpixelformat == V4L2_PIX_FMT_YUV420)
	LOGI("PreviewFormat:V4L2_PIX_FMT_YUV420");
	else if (v4lpixelformat == V4L2_PIX_FMT_NV12)
	LOGI("PreviewFormat:V4L2_PIX_FMT_NV12");
	else if (v4lpixelformat == V4L2_PIX_FMT_NV12T)
	LOGI("PreviewFormat:V4L2_PIX_FMT_NV12T");
	else if (v4lpixelformat == V4L2_PIX_FMT_NV21)
	LOGI("PreviewFormat:V4L2_PIX_FMT_NV21");
	else if (v4lpixelformat == V4L2_PIX_FMT_YUV422P)
	LOGI("PreviewFormat:V4L2_PIX_FMT_YUV422P");
	else if (v4lpixelformat == V4L2_PIX_FMT_YUYV)
	LOGI("PreviewFormat:V4L2_PIX_FMT_YUYV");
	else if (v4lpixelformat == V4L2_PIX_FMT_RGB565)
	LOGI("PreviewFormat:V4L2_PIX_FMT_RGB565");
	else
	LOGI("PreviewFormat:UnknownFormat");
	m_preview_width = width;
	m_preview_height = height;
	m_preview_v4lformat = v4lpixelformat;

	return 0;
}

int getPreviewMaxSize(int *width, int *height)
{
	*width = m_preview_max_width;
	*height = m_preview_max_height;

	return 0;
}

int getSnapshotMaxSize(int *width, int *height)
{
	switch (m_camera_id) {
		case CAMERA_ID_FRONT:
		m_snapshot_max_width = MAX_FRONT_CAMERA_SNAPSHOT_WIDTH;
		m_snapshot_max_height = MAX_FRONT_CAMERA_SNAPSHOT_HEIGHT;
		break;

		default:
		case CAMERA_ID_BACK:
		m_snapshot_max_width = MAX_BACK_CAMERA_SNAPSHOT_WIDTH;
		m_snapshot_max_height = MAX_BACK_CAMERA_SNAPSHOT_HEIGHT;
		break;
	}

	*width = m_snapshot_max_width;
	*height = m_snapshot_max_height;

	return 0;
}

int setISO(int iso_value)
{
	LOGI("%s(iso_value(%d))", __func__, iso_value);
	if (iso_value < ISO_AUTO || ISO_MAX <= iso_value) {
		LOGE("ERR(%s):Invalid iso_value (%d)", __func__, iso_value);
		return -1;
	}

	if (m_params->iso != iso_value) {
		m_params->iso = iso_value;
		if (m_flag_camera_start) {
			if (fimc_v4l2_s_ctrl(m_cam_fd, V4L2_CID_CAMERA_ISO, iso_value) < 0) {
				LOGE("ERR(%s):Fail on V4L2_CID_CAMERA_ISO", __func__);
				return -1;
			}
		}
	}

	return 0;
}

int setMetering(int metering_value)
{
	LOGI("%s(metering (%d))", __func__, metering_value);

	if (metering_value <= METERING_BASE || METERING_MAX <= metering_value) {
		LOGE("ERR(%s):Invalid metering_value (%d)", __func__, metering_value);
		return -1;
	}

	if (m_params->metering != metering_value) {
		m_params->metering = metering_value;
		if (m_flag_camera_start) {
			if (fimc_v4l2_s_ctrl(m_cam_fd, V4L2_CID_CAMERA_METERING, metering_value) < 0) {
				LOGE("ERR(%s):Fail on V4L2_CID_CAMERA_METERING", __func__);
				return -1;
			}
		}
	}

	return 0;
}

int setSharpness(int sharpness_value)
{
	LOGI("%s(sharpness_value(%d))", __func__, sharpness_value);

	if (sharpness_value < SHARPNESS_MINUS_2 || SHARPNESS_MAX <= sharpness_value) {
		LOGE("ERR(%s):Invalid sharpness_value (%d)", __func__, sharpness_value);
		return -1;
	}

	if (m_params->sharpness != sharpness_value) {
		m_params->sharpness = sharpness_value;
		if (m_flag_camera_start) {
			if (fimc_v4l2_s_ctrl(m_cam_fd, V4L2_CID_CAMERA_SHARPNESS, sharpness_value) < 0) {
				LOGE("ERR(%s):Fail on V4L2_CID_CAMERA_SHARPNESS", __func__);
				return -1;
			}
		}
	}

	return 0;
}

int setContrast(int contrast_value)
{
	LOGI("%s(contrast_value(%d))", __func__, contrast_value);

	if (contrast_value < CONTRAST_MINUS_2 || CONTRAST_MAX <= contrast_value) {
		LOGE("ERR(%s):Invalid contrast_value (%d)", __func__, contrast_value);
		return -1;
	}

	if (m_params->contrast != contrast_value) {
		m_params->contrast = contrast_value;
		if (m_flag_camera_start) {
			if (fimc_v4l2_s_ctrl(m_cam_fd, V4L2_CID_CAMERA_CONTRAST, contrast_value) < 0) {
				LOGE("ERR(%s):Fail on V4L2_CID_CAMERA_CONTRAST", __func__);
				return -1;
			}
		}
	}

	return 0;
}

int setSaturation(int saturation_value)
{
	LOGI("%s(saturation_value(%d))", __func__, saturation_value);

	if (saturation_value <SATURATION_MINUS_2 || SATURATION_MAX<= saturation_value) {
		LOGE("ERR(%s):Invalid saturation_value (%d)", __func__, saturation_value);
		return -1;
	}

	if (m_params->saturation != saturation_value) {
		m_params->saturation = saturation_value;
		if (m_flag_camera_start) {
			if (fimc_v4l2_s_ctrl(m_cam_fd, V4L2_CID_CAMERA_SATURATION, saturation_value) < 0) {
				LOGE("ERR(%s):Fail on V4L2_CID_CAMERA_SATURATION", __func__);
				return -1;
			}
		}
	}

	return 0;
}

int setFrameRate(int frame_rate)
{
	LOGI("%s(FrameRate(%d))", __func__, frame_rate);

	if (frame_rate < FRAME_RATE_AUTO || FRAME_RATE_MAX < frame_rate )
	LOGE("ERR(%s):Invalid frame_rate(%d)", __func__, frame_rate);

	if (m_params->capture.timeperframe.denominator != (unsigned)frame_rate) {
		m_params->capture.timeperframe.denominator = frame_rate;
		if (m_flag_camera_start) {
			if (fimc_v4l2_s_ctrl(m_cam_fd, V4L2_CID_CAMERA_FRAME_RATE, frame_rate) < 0) {
				LOGE("ERR(%s):Fail on V4L2_CID_CAMERA_FRAME_RATE", __func__);
				return -1;
			}
		}
	}

	return 0;
}

int getSnapshotPixelFormat(void)
{
	return m_snapshot_v4lformat;
}

bool isSupportedPreviewSize(const int width, const int height)
{
	unsigned int i;

	for (i = 0; i < mSupportedPreviewSizes.size(); i++) {
		if (mSupportedPreviewSizes[i].width == width &&
				mSupportedPreviewSizes[i].height == height)
		return true;
	}

	return false;
}

int setSnapshotSize(int width, int height)
{
	LOGI("%s(width(%d), height(%d))", __func__, width, height);

	m_snapshot_width = width;
	m_snapshot_height = height;

	return 0;
}

int setSnapshotPixelFormat(int pixel_format)
{
	int v4lpixelformat= pixel_format;

	if (m_snapshot_v4lformat != v4lpixelformat) {
		m_snapshot_v4lformat = v4lpixelformat;
	}

#if defined(LOG_NDEBUG) && LOG_NDEBUG == 0
	if (m_snapshot_v4lformat == V4L2_PIX_FMT_YUV420)
	LOGE("%s : SnapshotFormat:V4L2_PIX_FMT_YUV420", __func__);
	else if (m_snapshot_v4lformat == V4L2_PIX_FMT_NV12)
	LOGD("%s : SnapshotFormat:V4L2_PIX_FMT_NV12", __func__);
	else if (m_snapshot_v4lformat == V4L2_PIX_FMT_NV12T)
	LOGD("%s : SnapshotFormat:V4L2_PIX_FMT_NV12T", __func__);
	else if (m_snapshot_v4lformat == V4L2_PIX_FMT_NV21)
	LOGD("%s : SnapshotFormat:V4L2_PIX_FMT_NV21", __func__);
	else if (m_snapshot_v4lformat == V4L2_PIX_FMT_YUV422P)
	LOGD("%s : SnapshotFormat:V4L2_PIX_FMT_YUV422P", __func__);
	else if (m_snapshot_v4lformat == V4L2_PIX_FMT_YUYV)
	LOGD("%s : SnapshotFormat:V4L2_PIX_FMT_YUYV", __func__);
	else if (m_snapshot_v4lformat == V4L2_PIX_FMT_UYVY)
	LOGD("%s : SnapshotFormat:V4L2_PIX_FMT_UYVY", __func__);
	else if (m_snapshot_v4lformat == V4L2_PIX_FMT_RGB565)
	LOGD("%s : SnapshotFormat:V4L2_PIX_FMT_RGB565", __func__);
	else
	LOGD("SnapshotFormat:UnknownFormat");
#endif
	return 0;
}

int setJpegQuality(int jpeg_quality)
{
	LOGI("%s(jpeg_quality (%d))", __func__, jpeg_quality);

	if (jpeg_quality < JPEG_QUALITY_ECONOMY || JPEG_QUALITY_MAX <= jpeg_quality) {
		LOGE("ERR(%s):Invalid jpeg_quality (%d)", __func__, jpeg_quality);
		return -1;
	}

	if (m_jpeg_quality != jpeg_quality) {
		m_jpeg_quality = jpeg_quality;
		if (m_flag_camera_start && (m_camera_id == CAMERA_ID_BACK)) {
			if (fimc_v4l2_s_ctrl(m_cam_fd, V4L2_CID_CAM_JPEG_QUALITY, jpeg_quality) < 0) {
				LOGE("ERR(%s):Fail on V4L2_CID_CAM_JPEG_QUALITY", __func__);
				return -1;
			}
		}
	}

	return 0;
}

int setJpegThumbnailSize(int width, int height)
{
	LOGI("%s(width(%d), height(%d))", __func__, width, height);

	m_jpeg_thumbnail_width = width;
	m_jpeg_thumbnail_height = height;

	return 0;
}

int setExifOrientationInfo(int orientationInfo)
{
	LOGI("%s(orientationInfo(%d))", __func__, orientationInfo);

	if (orientationInfo < 0) {
		LOGE("ERR(%s):Invalid orientationInfo (%d)", __func__, orientationInfo);
		return -1;
	}
	m_exif_orientation = orientationInfo;

	return 0;
}

int setBrightness(int brightness)
{
	LOGI("%s(brightness(%d))", __func__, brightness);

	brightness += EV_DEFAULT;

	if (brightness < EV_MINUS_4 || EV_PLUS_4 < brightness) {
		LOGE("ERR(%s):Invalid brightness(%d)", __func__, brightness);
		return -1;
	}

	if (m_params->brightness != brightness) {
		m_params->brightness = brightness;
		if (m_flag_camera_start) {
			if (fimc_v4l2_s_ctrl(m_cam_fd, V4L2_CID_CAMERA_BRIGHTNESS, brightness) < 0) {
				LOGE("ERR(%s):Fail on V4L2_CID_CAMERA_BRIGHTNESS", __func__);
				return -1;
			}
		}
	}

	return 0;
}

int setWhiteBalance(int white_balance)
{
	LOGI("%s(white_balance(%d))", __func__, white_balance);

	if (white_balance <= WHITE_BALANCE_BASE || WHITE_BALANCE_MAX <= white_balance) {
		LOGE("ERR(%s):Invalid white_balance(%d)", __func__, white_balance);
		return -1;
	}

	if (m_params->white_balance != white_balance) {
		m_params->white_balance = white_balance;
		if (m_flag_camera_start) {
			if (fimc_v4l2_s_ctrl(m_cam_fd, V4L2_CID_CAMERA_WHITE_BALANCE, white_balance) < 0) {
				LOGE("ERR(%s):Fail on V4L2_CID_CAMERA_WHITE_BALANCE", __func__);
				return -1;
			}
		}
	}

	return 0;
}

int setFocusMode(int focus_mode)
{
	LOGI("%s(focus_mode(%d))", __func__, focus_mode);

	if (FOCUS_MODE_MAX <= focus_mode) {
		LOGE("ERR(%s):Invalid focus_mode (%d)", __func__, focus_mode);
		return -1;
	}

	if (m_params->focus_mode != focus_mode) {
		m_params->focus_mode = focus_mode;

		if (m_flag_camera_start) {
			if (fimc_v4l2_s_ctrl(m_cam_fd, V4L2_CID_CAMERA_FOCUS_MODE, focus_mode) < 0) {
				LOGE("ERR(%s):Fail on V4L2_CID_CAMERA_FOCUS_MODE", __func__);
				return -1;
			}
		}
	}

	return 0;
}

int setSceneMode(int scene_mode)
{
	LOGI("%s(scene_mode(%d))", __func__, scene_mode);

	if (scene_mode <= SCENE_MODE_BASE || SCENE_MODE_MAX <= scene_mode) {
		LOGE("ERR(%s):Invalid scene_mode (%d)", __func__, scene_mode);
		return -1;
	}

	if (m_params->scene_mode != scene_mode) {
		m_params->scene_mode = scene_mode;
		if (m_flag_camera_start) {
			if (fimc_v4l2_s_ctrl(m_cam_fd, V4L2_CID_CAMERA_SCENE_MODE, scene_mode) < 0) {
				LOGE("ERR(%s):Fail on V4L2_CID_CAMERA_SCENE_MODE", __func__);
				return -1;
			}
		}
	}

	return 0;
}

int setImageEffect(int image_effect)
{
	LOGI("%s(image_effect(%d))", __func__, image_effect);

	if (image_effect <= IMAGE_EFFECT_BASE || IMAGE_EFFECT_MAX <= image_effect) {
		LOGE("ERR(%s):Invalid image_effect(%d)", __func__, image_effect);
		return -1;
	}

	if (m_params->effects != image_effect) {
		m_params->effects = image_effect;
		if (m_flag_camera_start) {
			if (fimc_v4l2_s_ctrl(m_cam_fd, V4L2_CID_CAMERA_EFFECT, image_effect) < 0) {
				LOGE("ERR(%s):Fail on V4L2_CID_CAMERA_EFFECT", __func__);
				return -1;
			}
		}
	}

	return 0;
}

void setSkipFrame(int frame)
{
	if (frame < mSkipFrame)
	return;
	mSkipFrame = frame;
}

int setVTmode(int vtmode)
{
	LOGI("%s(vtmode (%d))", __func__, vtmode);

	if (vtmode < VT_MODE_OFF || VT_MODE_MAX <= vtmode) {
		LOGE("ERR(%s):Invalid vtmode (%d)", __func__, vtmode);
		return -1;
	}

	if (m_vtmode != vtmode) {
		m_vtmode = vtmode;
	}

	return 0;
}

int setWDR(int wdr_value)
{
	LOGI("%s(wdr_value(%d))", __func__, wdr_value);

	if (wdr_value < WDR_OFF || WDR_MAX <= wdr_value) {
		LOGE("ERR(%s):Invalid wdr_value (%d)", __func__, wdr_value);
		return -1;
	}

	if (m_wdr != wdr_value) {
		m_wdr = wdr_value;
		if (m_flag_camera_start) {
			if (fimc_v4l2_s_ctrl(m_cam_fd, V4L2_CID_CAMERA_WDR, wdr_value) < 0) {
				LOGE("ERR(%s):Fail on V4L2_CID_CAMERA_WDR", __func__);
				return -1;
			}
		}
	}

	return 0;
}

int setAntiShake(int anti_shake)
{
	LOGI("%s(anti_shake(%d))", __func__, anti_shake);

	if (anti_shake < ANTI_SHAKE_OFF || ANTI_SHAKE_MAX <= anti_shake) {
		LOGE("ERR(%s):Invalid anti_shake (%d)", __func__, anti_shake);
		return -1;
	}

	if (m_anti_shake != anti_shake) {
		m_anti_shake = anti_shake;
		if (m_flag_camera_start) {
			if (fimc_v4l2_s_ctrl(m_cam_fd, V4L2_CID_CAMERA_ANTI_SHAKE, anti_shake) < 0) {
				LOGE("ERR(%s):Fail on V4L2_CID_CAMERA_ANTI_SHAKE", __func__);
				return -1;
			}
		}
	}

	return 0;
}

int setGPSLatitude(const char *gps_latitude)
{
	LOGI("%s(gps_latitude(%s))", __func__, gps_latitude);
	if (gps_latitude == NULL)
	m_gps_enabled = false;
	else {
		m_gps_enabled = true;
		m_gps_latitude = lround(strtod(gps_latitude, NULL) * 10000000);
	}

	LOGI("%s(m_gps_latitude(%ld))", __func__, m_gps_latitude);
	return 0;
}

int setGPSLongitude(const char *gps_longitude)
{
	LOGI("%s(gps_longitude(%s))", __func__, gps_longitude);
	if (gps_longitude == NULL)
	m_gps_enabled = false;
	else {
		m_gps_enabled = true;
		m_gps_longitude = lround(strtod(gps_longitude, NULL) * 10000000);
	}

	LOGI("%s(m_gps_longitude(%ld))", __func__, m_gps_longitude);
	return 0;
}

int setGPSAltitude(const char *gps_altitude)
{
	LOGI("%s(gps_altitude(%s))", __func__, gps_altitude);
	if (gps_altitude == NULL)
	m_gps_altitude = 0;
	else {
		m_gps_altitude = lround(strtod(gps_altitude, NULL) * 100);
	}

	LOGI("%s(m_gps_altitude(%ld))", __func__, m_gps_altitude);
	return 0;
}

int setGPSTimeStamp(const char *gps_timestamp)
{
	LOGI("%s(gps_timestamp(%s))", __func__, gps_timestamp);
	if (gps_timestamp == NULL)
	m_gps_timestamp = 0;
	else
	m_gps_timestamp = atol(gps_timestamp);

	LOGI("%s(m_gps_timestamp(%ld))", __func__, m_gps_timestamp);
	return 0;
}

int setGPSProcessingMethod(const char *gps_processing_method)
{
	LOGI("%s(gps_processing_method(%s))", __func__, gps_processing_method);
	memset(mExifInfo.gps_processing_method, 0, sizeof(mExifInfo.gps_processing_method));
	if (gps_processing_method != NULL) {
		size_t len = strlen(gps_processing_method);
		if (len > sizeof(mExifInfo.gps_processing_method)) {
			len = sizeof(mExifInfo.gps_processing_method);
		}
		memcpy(mExifInfo.gps_processing_method, gps_processing_method, len);
	}
	return 0;
}

int setRecordingSize(int width, int height)
{
	LOGI("%s(width(%d), height(%d))", __func__, width, height);

	m_recording_width = width;
	m_recording_height = height;

	return 0;
}

int setGamma(int gamma)
{
	LOGI("%s(gamma(%d))", __func__, gamma);

	if (gamma < GAMMA_OFF || GAMMA_MAX <= gamma) {
		LOGE("ERR(%s):Invalid gamma (%d)", __func__, gamma);
		return -1;
	}

	if (m_video_gamma != gamma) {
		m_video_gamma = gamma;
		if (m_flag_camera_start) {
			if (fimc_v4l2_s_ctrl(m_cam_fd, V4L2_CID_CAMERA_SET_GAMMA, gamma) < 0) {
				LOGE("ERR(%s):Fail on V4L2_CID_CAMERA_SET_GAMMA", __func__);
				return -1;
			}
		}
	}

	return 0;
}

int setSlowAE(int slow_ae)
{
	LOGI("%s(slow_ae(%d))", __func__, slow_ae);

	if (slow_ae < GAMMA_OFF || GAMMA_MAX <= slow_ae) {
		LOGE("ERR(%s):Invalid slow_ae (%d)", __func__, slow_ae);
		return -1;
	}

	if (m_slow_ae!= slow_ae) {
		m_slow_ae = slow_ae;
		if (m_flag_camera_start) {
			if (fimc_v4l2_s_ctrl(m_cam_fd, V4L2_CID_CAMERA_SET_SLOW_AE, slow_ae) < 0) {
				LOGE("ERR(%s):Fail on V4L2_CID_CAMERA_SET_SLOW_AE", __func__);
				return -1;
			}
		}
	}

	return 0;
}

int setSensorMode(int sensor_mode)
{
	LOGI("%s(sensor_mode (%d))", __func__, sensor_mode);

	if (sensor_mode < SENSOR_MODE_CAMERA || SENSOR_MODE_MOVIE < sensor_mode) {
		LOGE("ERR(%s):Invalid sensor mode (%d)", __func__, sensor_mode);
		return -1;
	}

	if (m_sensor_mode != sensor_mode) {
		m_sensor_mode = sensor_mode;
	}

	return 0;
}

int setShotMode(int shot_mode)
{
	LOGI("%s(shot_mode (%d))", __func__, shot_mode);
	if (shot_mode < SHOT_MODE_SINGLE || SHOT_MODE_SELF < shot_mode) {
		LOGE("ERR(%s):Invalid shot_mode (%d)", __func__, shot_mode);
		return -1;
	}
	m_shot_mode = shot_mode;

	return 0;
}

int setBlur(int blur_level)
{
	LOGI("%s(level (%d))", __func__, blur_level);

	if (blur_level < BLUR_LEVEL_0 || BLUR_LEVEL_MAX <= blur_level) {
		LOGE("ERR(%s):Invalid level (%d)", __func__, blur_level);
		return -1;
	}

	if (m_blur_level != blur_level) {
		m_blur_level = blur_level;
		if (m_flag_camera_start) {
			if (fimc_v4l2_s_ctrl(m_cam_fd, V4L2_CID_CAMERA_VGA_BLUR, blur_level) < 0) {
				LOGE("ERR(%s):Fail on V4L2_CID_CAMERA_VGA_BLUR", __func__);
				return -1;
			}
		}
	}
	return 0;
}

int setDataLineCheck(int chk_dataline)
{
	LOGI("%s(chk_dataline (%d))", __func__, chk_dataline);

	if (chk_dataline < CHK_DATALINE_OFF || CHK_DATALINE_MAX <= chk_dataline) {
		LOGE("ERR(%s):Invalid chk_dataline (%d)", __func__, chk_dataline);
		return -1;
	}

	if (m_chk_dataline != chk_dataline) {
		m_chk_dataline = chk_dataline;
	}

	return 0;
}

bool isSupportedParameter(const char * const parm, const char * const supported_parm)
{
	const char *pStart;
	const char *pEnd;

	if (!parm || !supported_parm)
	return false;

	pStart = supported_parm;

	while (true) {
		pEnd = strchr(pStart, ',');
		if (!pEnd) {
			if (!strcmp(parm, pStart))
			return true;
			else
			return false;
		}
		if (!strncmp(parm, pStart, pEnd - pStart)) {
			return true;
		}
		pStart = pEnd + 1;
	}
	/* NOTREACHED */
}

status_t setParameters(const CameraParameters& params)
{
	LOGI("%s :", __func__);
	LOGI("setParameters");

	status_t ret = NO_ERROR;

	/* if someone calls us while picture thread is running, it could screw
	 * up the sensor quite a bit so return error.  we can't wait because
	 * that would cause deadlock with the callbacks
	 */

	// preview size
	int new_preview_width = 0;
	int new_preview_height = 0;
	params.getPreviewSize(&new_preview_width, &new_preview_height);
	const char *new_str_preview_format = params.getPreviewFormat();
	LOGI("%s : new_preview_width x new_preview_height = %dx%d, format = %s",
			__func__, new_preview_width, new_preview_height, new_str_preview_format);

	if (0 < new_preview_width && 0 < new_preview_height && new_str_preview_format != NULL) {
		int new_preview_format = 0;

		if (!strcmp(new_str_preview_format,
						CameraParameters::PIXEL_FORMAT_RGB565))
		new_preview_format = V4L2_PIX_FMT_RGB565;
		else if (!strcmp(new_str_preview_format,
						CameraParameters::PIXEL_FORMAT_YUV420SP))
		new_preview_format = V4L2_PIX_FMT_NV21;
		else if (!strcmp(new_str_preview_format, "yuv420sp_custom"))
		new_preview_format = V4L2_PIX_FMT_NV12T;
		else if (!strcmp(new_str_preview_format, "yuv420p"))
		new_preview_format = V4L2_PIX_FMT_YUV420;
		else if (!strcmp(new_str_preview_format, "yuv422i"))
		new_preview_format = V4L2_PIX_FMT_YUYV;
		else if (!strcmp(new_str_preview_format, "yuv422p"))
		new_preview_format = V4L2_PIX_FMT_YUV422P;
		else
		new_preview_format = V4L2_PIX_FMT_NV21; //for 3rd party

		if (setPreviewSize(new_preview_width, new_preview_height, new_preview_format) < 0) {
			LOGE("ERR(%s):Fail on mSecCamera->setPreviewSize(width(%d), height(%d), format(%d))",
					__func__, new_preview_width, new_preview_height, new_preview_format);
			ret = UNKNOWN_ERROR;
		} else {
			mParameters.setPreviewSize(new_preview_width, new_preview_height);
			mParameters.setPreviewFormat(new_str_preview_format);
		}
#if defined(BOARD_USES_OVERLAY)
		if (mUseOverlay == true && mOverlay != 0) {
			if (mOverlay->setCrop(0, 0, new_preview_width, new_preview_height) != NO_ERROR) {
				LOGE("ERR(%s)::(mOverlay->setCrop(0, 0, %d, %d) fail",
						__func__, new_preview_width, new_preview_height);
			}
		}
#endif
	}

	int new_picture_width = 0;
	int new_picture_height = 0;

	params.getPictureSize(&new_picture_width, &new_picture_height);
	LOGI("%s : new_picture_width x new_picture_height = %dx%d", __func__, new_picture_width, new_picture_height);
	if (0 < new_picture_width && 0 < new_picture_height) {
		if (setSnapshotSize(new_picture_width, new_picture_height) < 0) {
			LOGE("ERR(%s):Fail on mSecCamera->setSnapshotSize(width(%d), height(%d))",
					__func__, new_picture_width, new_picture_height);
			ret = UNKNOWN_ERROR;
		} else {
			mParameters.setPictureSize(new_picture_width, new_picture_height);
		}
	}

	// picture format
	const char *new_str_picture_format = params.getPictureFormat();
	LOGI("%s : new_str_picture_format %s", __func__, new_str_picture_format);
	if (new_str_picture_format != NULL) {
		int new_picture_format = 0;

		if (!strcmp(new_str_picture_format, CameraParameters::PIXEL_FORMAT_RGB565))
		new_picture_format = V4L2_PIX_FMT_RGB565;
		else if (!strcmp(new_str_picture_format, CameraParameters::PIXEL_FORMAT_YUV420SP))
		new_picture_format = V4L2_PIX_FMT_NV21;
		else if (!strcmp(new_str_picture_format, "yuv420sp_custom"))
		new_picture_format = V4L2_PIX_FMT_NV12T;
		else if (!strcmp(new_str_picture_format, "yuv420p"))
		new_picture_format = V4L2_PIX_FMT_YUV420;
		else if (!strcmp(new_str_picture_format, "yuv422i"))
		new_picture_format = V4L2_PIX_FMT_YUYV;
		else if (!strcmp(new_str_picture_format, "uyv422i_custom")) //Zero copy UYVY format
		new_picture_format = V4L2_PIX_FMT_UYVY;
		else if (!strcmp(new_str_picture_format, "uyv422i"))//Non-zero copy UYVY format
		new_picture_format = V4L2_PIX_FMT_UYVY;
		else if (!strcmp(new_str_picture_format, CameraParameters::PIXEL_FORMAT_JPEG))
#ifdef JPEG_FROM_SENSOR
		new_picture_format = V4L2_PIX_FMT_YUYV;
#endif
		else if (!strcmp(new_str_picture_format, "yuv422p"))
		new_picture_format = V4L2_PIX_FMT_YUV422P;
		else
		new_picture_format = V4L2_PIX_FMT_NV21; //for 3rd party

		if (setSnapshotPixelFormat(new_picture_format) < 0) {
			LOGE("ERR(%s):Fail on mSecCamera->setSnapshotPixelFormat(format(%d))", __func__, new_picture_format);
			ret = UNKNOWN_ERROR;
		} else {
			mParameters.setPictureFormat(new_str_picture_format);
		}
	}

#ifdef SWP1_CAMERA_ADD_ADVANCED_FUNCTION
	//JPEG image quality
	int new_jpeg_quality = params.getInt(CameraParameters::KEY_JPEG_QUALITY);
	LOGI("%s : new_jpeg_quality %d", __func__, new_jpeg_quality);
	/* we ignore bad values */
	if (new_jpeg_quality >=1 && new_jpeg_quality <= 100) {
		if (mSecCamera->setJpegQuality(new_jpeg_quality) < 0) {
			LOGE("ERR(%s):Fail on mSecCamera->setJpegQuality(quality(%d))", __func__, new_jpeg_quality);
			ret = UNKNOWN_ERROR;
		} else {
			mParameters.set(CameraParameters::KEY_JPEG_QUALITY, new_jpeg_quality);
		}
	}
#else
	//JPEG image quality
	int new_jpeg_quality = params.getInt(CameraParameters::KEY_JPEG_QUALITY);
	LOGI("%s : new_jpeg_quality %d", __func__, new_jpeg_quality);
	if (new_jpeg_quality < 0) {
		LOGW("JPEG-image quality is not specified or is negative, defaulting to 100");
		new_jpeg_quality = 100;
		mParameters.set(CameraParameters::KEY_JPEG_QUALITY, "100");
	}
	setJpegQuality(new_jpeg_quality);
#endif

	// JPEG thumbnail size
	int new_jpeg_thumbnail_width = params.getInt(CameraParameters::KEY_JPEG_THUMBNAIL_WIDTH);
	int new_jpeg_thumbnail_height= params.getInt(CameraParameters::KEY_JPEG_THUMBNAIL_HEIGHT);
	if (0 <= new_jpeg_thumbnail_width && 0 <= new_jpeg_thumbnail_height) {
		if (setJpegThumbnailSize(new_jpeg_thumbnail_width, new_jpeg_thumbnail_height) < 0) {
			LOGE("ERR(%s):Fail on mSecCamera->setJpegThumbnailSize(width(%d), height(%d))", __func__, new_jpeg_thumbnail_width, new_jpeg_thumbnail_height);
			ret = UNKNOWN_ERROR;
		} else {
			mParameters.set(CameraParameters::KEY_JPEG_THUMBNAIL_WIDTH, new_jpeg_thumbnail_width);
			mParameters.set(CameraParameters::KEY_JPEG_THUMBNAIL_HEIGHT, new_jpeg_thumbnail_height);
		}
	}

	// frame rate
	int new_frame_rate = params.getPreviewFrameRate();
	/* ignore any fps request, we're determine fps automatically based
	 * on scene mode.  don't return an error because it causes CTS failure.
	 */
	if (new_frame_rate != mParameters.getPreviewFrameRate()) {
		LOGW("WARN(%s): request for preview frame %d not allowed, != %d\n",
				__func__, new_frame_rate, mParameters.getPreviewFrameRate());
	}

	// rotation
	int new_rotation = params.getInt(CameraParameters::KEY_ROTATION);
	LOGI("%s : new_rotation %d", __func__, new_rotation);
	if (0 <= new_rotation) {
		LOGI("%s : set orientation:%d\n", __func__, new_rotation);
		if (setExifOrientationInfo(new_rotation) < 0) {
			LOGE("ERR(%s):Fail on mSecCamera->setExifOrientationInfo(%d)", __func__, new_rotation);
			ret = UNKNOWN_ERROR;
		} else {
			mParameters.set(CameraParameters::KEY_ROTATION, new_rotation);
		}
	}

	// brightness
	int new_exposure_compensation = params.getInt(CameraParameters::KEY_EXPOSURE_COMPENSATION);
	int max_exposure_compensation = params.getInt(CameraParameters::KEY_MAX_EXPOSURE_COMPENSATION);
	int min_exposure_compensation = params.getInt(CameraParameters::KEY_MIN_EXPOSURE_COMPENSATION);
	LOGI("%s : new_exposure_compensation %d", __func__, new_exposure_compensation);
	if ((min_exposure_compensation <= new_exposure_compensation) &&
			(max_exposure_compensation >= new_exposure_compensation)) {
		if (setBrightness(new_exposure_compensation) < 0) {
			LOGE("ERR(%s):Fail on mSecCamera->setBrightness(brightness(%d))", __func__, new_exposure_compensation);
			ret = UNKNOWN_ERROR;
		} else {
			mParameters.set(CameraParameters::KEY_EXPOSURE_COMPENSATION, new_exposure_compensation);
		}
	}

	// whitebalance
	const char *new_white_str = params.get(CameraParameters::KEY_WHITE_BALANCE);
	LOGI("%s : new_white_str %s", __func__, new_white_str);
	if (new_white_str != NULL) {
		int new_white = -1;

		if (!strcmp(new_white_str, CameraParameters::WHITE_BALANCE_AUTO))
		new_white = WHITE_BALANCE_AUTO;
		else if (!strcmp(new_white_str,
						CameraParameters::WHITE_BALANCE_DAYLIGHT))
		new_white = WHITE_BALANCE_SUNNY;
		else if (!strcmp(new_white_str,
						CameraParameters::WHITE_BALANCE_CLOUDY_DAYLIGHT))
		new_white = WHITE_BALANCE_CLOUDY;
		else if (!strcmp(new_white_str,
						CameraParameters::WHITE_BALANCE_FLUORESCENT))
		new_white = WHITE_BALANCE_FLUORESCENT;
		else if (!strcmp(new_white_str,
						CameraParameters::WHITE_BALANCE_INCANDESCENT))
		new_white = WHITE_BALANCE_TUNGSTEN;
		else {
			LOGE("ERR(%s):Invalid white balance(%s)", __func__, new_white_str); //twilight, shade, warm_flourescent
			ret = UNKNOWN_ERROR;
		}

		if (0 <= new_white) {
			if (setWhiteBalance(new_white) < 0) {
				LOGE("ERR(%s):Fail on mSecCamera->setWhiteBalance(white(%d))", __func__, new_white);
				ret = UNKNOWN_ERROR;
			} else {
				mParameters.set(CameraParameters::KEY_WHITE_BALANCE, new_white_str);
			}
		}
	}

	// scene mode
	const char *new_scene_mode_str = params.get(CameraParameters::KEY_SCENE_MODE);
	const char *current_scene_mode_str = mParameters.get(CameraParameters::KEY_SCENE_MODE);

	// fps range
	int new_min_fps = 0;
	int new_max_fps = 0;
	int current_min_fps, current_max_fps;
	params.getPreviewFpsRange(&new_min_fps, &new_max_fps);
	mParameters.getPreviewFpsRange(&current_min_fps, &current_max_fps);
	/* our fps range is determined by the sensor, reject any request
	 * that isn't exactly what we're already at.
	 * but the check is performed when requesting only changing fps range
	 */
	if (new_scene_mode_str && current_scene_mode_str) {
		if (!strcmp(new_scene_mode_str, current_scene_mode_str)) {
			if ((new_min_fps != current_min_fps) || (new_max_fps != current_max_fps)) {
				LOGW("%s : requested new_min_fps = %d, new_max_fps = %d not allowed",
						__func__, new_min_fps, new_max_fps);
				LOGE("%s : current_min_fps = %d, current_max_fps = %d",
						__func__, current_min_fps, current_max_fps);
				ret = UNKNOWN_ERROR;
			}
		}
	} else {
		/* Check basic validation if scene mode is different */
		if ((new_min_fps > new_max_fps) ||
				(new_min_fps < 0) || (new_max_fps < 0))
		ret = UNKNOWN_ERROR;
	}

	if (new_scene_mode_str != NULL) {
		int new_scene_mode = -1;

		const char *new_flash_mode_str = params.get(CameraParameters::KEY_FLASH_MODE);
		const char *new_focus_mode_str;

		new_focus_mode_str = params.get(CameraParameters::KEY_FOCUS_MODE);
		// fps range is (15000,30000) by default.
		mParameters.set(CameraParameters::KEY_SUPPORTED_PREVIEW_FPS_RANGE, "(15000,30000)");
		mParameters.set(CameraParameters::KEY_PREVIEW_FPS_RANGE,
				"15000,30000");

		if (!strcmp(new_scene_mode_str, CameraParameters::SCENE_MODE_AUTO)) {
			new_scene_mode = SCENE_MODE_NONE;
		} else {
			// defaults for non-auto scene modes
			if (m_camera_id == CAMERA_ID_BACK) {
				new_focus_mode_str = CameraParameters::FOCUS_MODE_AUTO;
			}
			new_flash_mode_str = CameraParameters::FLASH_MODE_OFF;

			if (!strcmp(new_scene_mode_str,
							CameraParameters::SCENE_MODE_PORTRAIT)) {
				new_scene_mode = SCENE_MODE_PORTRAIT;
				new_flash_mode_str = CameraParameters::FLASH_MODE_AUTO;
			} else if (!strcmp(new_scene_mode_str,
							CameraParameters::SCENE_MODE_LANDSCAPE)) {
				new_scene_mode = SCENE_MODE_LANDSCAPE;
			} else if (!strcmp(new_scene_mode_str,
							CameraParameters::SCENE_MODE_SPORTS)) {
				new_scene_mode = SCENE_MODE_SPORTS;
			} else if (!strcmp(new_scene_mode_str,
							CameraParameters::SCENE_MODE_PARTY)) {
				new_scene_mode = SCENE_MODE_PARTY_INDOOR;
				new_flash_mode_str = CameraParameters::FLASH_MODE_AUTO;
			} else if ((!strcmp(new_scene_mode_str,
									CameraParameters::SCENE_MODE_BEACH)) ||
					(!strcmp(new_scene_mode_str,
									CameraParameters::SCENE_MODE_SNOW))) {
				new_scene_mode = SCENE_MODE_BEACH_SNOW;
			} else if (!strcmp(new_scene_mode_str,
							CameraParameters::SCENE_MODE_SUNSET)) {
				new_scene_mode = SCENE_MODE_SUNSET;
			} else if (!strcmp(new_scene_mode_str,
							CameraParameters::SCENE_MODE_NIGHT)) {
				new_scene_mode = SCENE_MODE_NIGHTSHOT;
				mParameters.set(CameraParameters::KEY_SUPPORTED_PREVIEW_FPS_RANGE, "(4000,30000)");
				mParameters.set(CameraParameters::KEY_PREVIEW_FPS_RANGE,
						"4000,30000");
			} else if (!strcmp(new_scene_mode_str,
							CameraParameters::SCENE_MODE_FIREWORKS)) {
				new_scene_mode = SCENE_MODE_FIREWORKS;
			} else if (!strcmp(new_scene_mode_str,
							CameraParameters::SCENE_MODE_CANDLELIGHT)) {
				new_scene_mode = SCENE_MODE_CANDLE_LIGHT;
			} else {
				LOGE("%s::unmatched scene_mode(%s)",
						__func__, new_scene_mode_str); //action, night-portrait, theatre, steadyphoto
				ret = UNKNOWN_ERROR;
			}
		}

		// focus mode
		if (new_focus_mode_str != NULL) {
			int new_focus_mode = -1;

			if (!strcmp(new_focus_mode_str,
							CameraParameters::FOCUS_MODE_AUTO)) {
				new_focus_mode = FOCUS_MODE_AUTO;
				mParameters.set(CameraParameters::KEY_FOCUS_DISTANCES,
						BACK_CAMERA_AUTO_FOCUS_DISTANCES_STR);
			}
			else if (!strcmp(new_focus_mode_str,
							CameraParameters::FOCUS_MODE_MACRO)) {
				new_focus_mode = FOCUS_MODE_MACRO;
				mParameters.set(CameraParameters::KEY_FOCUS_DISTANCES,
						BACK_CAMERA_MACRO_FOCUS_DISTANCES_STR);
			}
			else if (!strcmp(new_focus_mode_str,
							CameraParameters::FOCUS_MODE_INFINITY)) {
				new_focus_mode = FOCUS_MODE_INFINITY;
				mParameters.set(CameraParameters::KEY_FOCUS_DISTANCES,
						BACK_CAMERA_INFINITY_FOCUS_DISTANCES_STR);
			}
			else {
				LOGE("%s::unmatched focus_mode(%s)", __func__, new_focus_mode_str);
				ret = UNKNOWN_ERROR;
			}

			if (0 <= new_focus_mode) {
				if (setFocusMode(new_focus_mode) < 0) {
					LOGE("%s::mSecCamera->setFocusMode(%d) fail", __func__, new_focus_mode);
					ret = UNKNOWN_ERROR;
				} else {
					mParameters.set(CameraParameters::KEY_FOCUS_MODE, new_focus_mode_str);
				}
			}
		}

		// flash..
		if (new_flash_mode_str != NULL) {
			int new_flash_mode = -1;

			if (!strcmp(new_flash_mode_str, CameraParameters::FLASH_MODE_OFF))
			new_flash_mode = FLASH_MODE_OFF;
			else if (!strcmp(new_flash_mode_str, CameraParameters::FLASH_MODE_AUTO))
			new_flash_mode = FLASH_MODE_AUTO;
			else if (!strcmp(new_flash_mode_str, CameraParameters::FLASH_MODE_ON))
			new_flash_mode = FLASH_MODE_ON;
			else if (!strcmp(new_flash_mode_str, CameraParameters::FLASH_MODE_TORCH))
			new_flash_mode = FLASH_MODE_TORCH;
			else {
				LOGE("%s::unmatched flash_mode(%s)", __func__, new_flash_mode_str); //red-eye
				ret = UNKNOWN_ERROR;
			}
			if (0 <= new_flash_mode) {
				if (setFlashMode(new_flash_mode) < 0) {
					LOGE("%s::mSecCamera->setFlashMode(%d) fail", __func__, new_flash_mode);
					ret = UNKNOWN_ERROR;
				} else {
					mParameters.set(CameraParameters::KEY_FLASH_MODE, new_flash_mode_str);
				}
			}
		}

		//  scene..
		if (0 <= new_scene_mode) {
			if (setSceneMode(new_scene_mode) < 0) {
				LOGE("%s::mSecCamera->setSceneMode(%d) fail", __func__, new_scene_mode);
				ret = UNKNOWN_ERROR;
			} else {
				mParameters.set(CameraParameters::KEY_SCENE_MODE, new_scene_mode_str);
			}
		}
	}

	// ---------------------------------------------------------------------------

	// image effect
	const char *new_image_effect_str = params.get(CameraParameters::KEY_EFFECT);
	if (new_image_effect_str != NULL) {

		int new_image_effect = -1;

		if (!strcmp(new_image_effect_str, CameraParameters::EFFECT_NONE))
		new_image_effect = IMAGE_EFFECT_NONE;
		else if (!strcmp(new_image_effect_str, CameraParameters::EFFECT_MONO))
		new_image_effect = IMAGE_EFFECT_BNW;
		else if (!strcmp(new_image_effect_str, CameraParameters::EFFECT_SEPIA))
		new_image_effect = IMAGE_EFFECT_SEPIA;
		else if (!strcmp(new_image_effect_str, CameraParameters::EFFECT_AQUA))
		new_image_effect = IMAGE_EFFECT_AQUA;
		else if (!strcmp(new_image_effect_str, CameraParameters::EFFECT_NEGATIVE))
		new_image_effect = IMAGE_EFFECT_NEGATIVE;
		else {
			//posterize, whiteboard, blackboard, solarize
			LOGE("ERR(%s):Invalid effect(%s)", __func__, new_image_effect_str);
			ret = UNKNOWN_ERROR;
		}

		if (new_image_effect >= 0) {
			if (setImageEffect(new_image_effect) < 0) {
				LOGE("ERR(%s):Fail on mSecCamera->setImageEffect(effect(%d))", __func__, new_image_effect);
				ret = UNKNOWN_ERROR;
			} else {
				const char *old_image_effect_str = mParameters.get(CameraParameters::KEY_EFFECT);

				if (old_image_effect_str) {
					if (strcmp(old_image_effect_str, new_image_effect_str)) {
						setSkipFrame(EFFECT_SKIP_FRAME);
					}
				}

				mParameters.set(CameraParameters::KEY_EFFECT, new_image_effect_str);
			}
		}
	}

	//vt mode
	int new_vtmode = mInternalParameters.getInt("vtmode");
	if (0 <= new_vtmode) {
		if (setVTmode(new_vtmode) < 0) {
			LOGE("ERR(%s):Fail on mSecCamera->setVTMode(%d)", __func__, new_vtmode);
			ret = UNKNOWN_ERROR;
		}
	}

	//contrast
	int new_contrast = mInternalParameters.getInt("contrast");

	if (0 <= new_contrast) {
		if (setContrast(new_contrast) < 0) {
			LOGE("ERR(%s):Fail on mSecCamera->setContrast(%d)", __func__, new_contrast);
			ret = UNKNOWN_ERROR;
		}
	}

	//WDR
	int new_wdr = mInternalParameters.getInt("wdr");

	if (0 <= new_wdr) {
		if (setWDR(new_wdr) < 0) {
			LOGE("ERR(%s):Fail on mSecCamera->setWDR(%d)", __func__, new_wdr);
			ret = UNKNOWN_ERROR;
		}
	}

	//anti shake
	int new_anti_shake = mInternalParameters.getInt("anti-shake");

	if (0 <= new_anti_shake) {
		if (setAntiShake(new_anti_shake) < 0) {
			LOGE("ERR(%s):Fail on mSecCamera->setWDR(%d)", __func__, new_anti_shake);
			ret = UNKNOWN_ERROR;
		}
	}

	// gps latitude
	const char *new_gps_latitude_str = params.get(CameraParameters::KEY_GPS_LATITUDE);
	if (setGPSLatitude(new_gps_latitude_str) < 0) {
		LOGE("%s::mSecCamera->setGPSLatitude(%s) fail", __func__, new_gps_latitude_str);
		ret = UNKNOWN_ERROR;
	} else {
		if (new_gps_latitude_str) {
			mParameters.set(CameraParameters::KEY_GPS_LATITUDE, new_gps_latitude_str);
		} else {
			mParameters.remove(CameraParameters::KEY_GPS_LATITUDE);
		}
	}

	// gps longitude
	const char *new_gps_longitude_str = params.get(CameraParameters::KEY_GPS_LONGITUDE);

	if (setGPSLongitude(new_gps_longitude_str) < 0) {
		LOGE("%s::mSecCamera->setGPSLongitude(%s) fail", __func__, new_gps_longitude_str);
		ret = UNKNOWN_ERROR;
	} else {
		if (new_gps_longitude_str) {
			mParameters.set(CameraParameters::KEY_GPS_LONGITUDE, new_gps_longitude_str);
		} else {
			mParameters.remove(CameraParameters::KEY_GPS_LONGITUDE);
		}
	}

	// gps altitude
	const char *new_gps_altitude_str = params.get(CameraParameters::KEY_GPS_ALTITUDE);

	if (setGPSAltitude(new_gps_altitude_str) < 0) {
		LOGE("%s::mSecCamera->setGPSAltitude(%s) fail", __func__, new_gps_altitude_str);
		ret = UNKNOWN_ERROR;
	} else {
		if (new_gps_altitude_str) {
			mParameters.set(CameraParameters::KEY_GPS_ALTITUDE, new_gps_altitude_str);
		} else {
			mParameters.remove(CameraParameters::KEY_GPS_ALTITUDE);
		}
	}

	// gps timestamp
	const char *new_gps_timestamp_str = params.get(CameraParameters::KEY_GPS_TIMESTAMP);

	if (setGPSTimeStamp(new_gps_timestamp_str) < 0) {
		LOGE("%s::mSecCamera->setGPSTimeStamp(%s) fail", __func__, new_gps_timestamp_str);
		ret = UNKNOWN_ERROR;
	} else {
		if (new_gps_timestamp_str) {
			mParameters.set(CameraParameters::KEY_GPS_TIMESTAMP, new_gps_timestamp_str);
		} else {
			mParameters.remove(CameraParameters::KEY_GPS_TIMESTAMP);
		}
	}

	// gps processing method
	const char *new_gps_processing_method_str = params.get(CameraParameters::KEY_GPS_PROCESSING_METHOD);

	if (setGPSProcessingMethod(new_gps_processing_method_str) < 0) {
		LOGE("%s::mSecCamera->setGPSProcessingMethod(%s) fail", __func__, new_gps_processing_method_str);
		ret = UNKNOWN_ERROR;
	} else {
		if (new_gps_processing_method_str) {
			mParameters.set(CameraParameters::KEY_GPS_PROCESSING_METHOD, new_gps_processing_method_str);
		} else {
			mParameters.remove(CameraParameters::KEY_GPS_PROCESSING_METHOD);
		}
	}

	// Recording size
	int new_recording_width = mInternalParameters.getInt("recording-size-width");
	int new_recording_height= mInternalParameters.getInt("recording-size-height");

	if (0 < new_recording_width && 0 < new_recording_height) {
		if (setRecordingSize(new_recording_width, new_recording_height) < 0) {
			LOGE("ERR(%s):Fail on mSecCamera->setRecordingSize(width(%d), height(%d))", __func__, new_recording_width, new_recording_height);
			ret = UNKNOWN_ERROR;
		}
	} else {
		if (setRecordingSize(new_preview_width, new_preview_height) < 0) {
			LOGE("ERR(%s):Fail on mSecCamera->setRecordingSize(width(%d), height(%d))", __func__, new_preview_width, new_preview_height);
			ret = UNKNOWN_ERROR;
		}
	}

	//gamma
	const char *new_gamma_str = mInternalParameters.get("video_recording_gamma");

	if (new_gamma_str != NULL) {
		int new_gamma = -1;
		if (!strcmp(new_gamma_str, "off"))
		new_gamma = GAMMA_OFF;
		else if (!strcmp(new_gamma_str, "on"))
		new_gamma = GAMMA_ON;
		else {
			LOGE("%s::unmatched gamma(%s)", __func__, new_gamma_str);
			ret = UNKNOWN_ERROR;
		}

		if (0 <= new_gamma) {
			if (setGamma(new_gamma) < 0) {
				LOGE("%s::mSecCamera->setGamma(%d) fail", __func__, new_gamma);
				ret = UNKNOWN_ERROR;
			}
		}
	}

	//slow ae
	const char *new_slow_ae_str = mInternalParameters.get("slow_ae");

	if (new_slow_ae_str != NULL) {
		int new_slow_ae = -1;

		if (!strcmp(new_slow_ae_str, "off"))
		new_slow_ae = SLOW_AE_OFF;
		else if (!strcmp(new_slow_ae_str, "on"))
		new_slow_ae = SLOW_AE_ON;
		else {
			LOGE("%s::unmatched slow_ae(%s)", __func__, new_slow_ae_str);
			ret = UNKNOWN_ERROR;
		}

		if (0 <= new_slow_ae) {
			if (setSlowAE(new_slow_ae) < 0) {
				LOGE("%s::mSecCamera->setSlowAE(%d) fail", __func__, new_slow_ae);
				ret = UNKNOWN_ERROR;
			}
		}
	}

	/*Camcorder fix fps*/
	int new_sensor_mode = mInternalParameters.getInt("cam_mode");

	if (0 <= new_sensor_mode) {
		if (setSensorMode(new_sensor_mode) < 0) {
			LOGE("ERR(%s):Fail on mSecCamera->setSensorMode(%d)", __func__, new_sensor_mode);
			ret = UNKNOWN_ERROR;
		}
	} else {
		new_sensor_mode=0;
	}

	/*Shot mode*/
	int new_shot_mode = mInternalParameters.getInt("shot_mode");

	if (0 <= new_shot_mode) {
		if (setShotMode(new_shot_mode) < 0) {
			LOGE("ERR(%s):Fail on mSecCamera->setShotMode(%d)", __func__, new_shot_mode);
			ret = UNKNOWN_ERROR;
		}
	} else {
		new_shot_mode=0;
	}

	//blur for Video call
	int new_blur_level = mInternalParameters.getInt("blur");

	if (0 <= new_blur_level) {
		if (setBlur(new_blur_level) < 0) {
			LOGE("ERR(%s):Fail on mSecCamera->setBlur(%d)", __func__, new_blur_level);
			ret = UNKNOWN_ERROR;
		}
	}

	// chk_dataline
	int new_dataline = mInternalParameters.getInt("chk_dataline");

	if (0 <= new_dataline) {
		if (setDataLineCheck(new_dataline) < 0) {
			LOGE("ERR(%s):Fail on mSecCamera->setDataLineCheck(%d)", __func__, new_dataline);
			ret = UNKNOWN_ERROR;
		}
	}
	LOGI("%s return ret = %d", __func__, ret);

	return ret;

//	status_t ret = NO_ERROR;
//
//	/* if someone calls us while picture thread is running, it could screw
//	 * up the sensor quite a bit so return error.
//	 */
//
//	// preview size
//	int new_preview_width = 0;
//	int new_preview_height = 0;
//	params.getPreviewSize(&new_preview_width, &new_preview_height);
//	const char *new_str_preview_format = params.getPreviewFormat();
//	LOGI("%s : new_preview_width x new_preview_height = %dx%d, format = %s",
//			__func__, new_preview_width, new_preview_height, new_str_preview_format);
//	if (strcmp(new_str_preview_format, CameraParameters::PIXEL_FORMAT_YUV420SP) &&
//			strcmp(new_str_preview_format, CameraParameters::PIXEL_FORMAT_YUV420P)) {
//		LOGE("Unsupported preview color format: %s", new_str_preview_format);
//		return BAD_VALUE;
//	}
//
//	if (0 < new_preview_width && 0 < new_preview_height &&
//			new_str_preview_format != NULL &&
//			isSupportedPreviewSize(new_preview_width, new_preview_height)) {
//		int new_preview_format = V4L2_PIX_FMT_YUV420;
//
//		int current_preview_width, current_preview_height, current_frame_size;
//		current_preview_width = m_preview_width;
//	    current_preview_height = m_preview_height;
//		current_frame_size = m_frameSize(m_preview_v4lformat, m_preview_width, m_preview_height);
//		int current_pixel_format = m_preview_v4lformat;
//
//		if (current_preview_width != new_preview_width ||
//				current_preview_height != new_preview_height ||
//				current_pixel_format != new_preview_format) {
//			if (setPreviewSize(new_preview_width, new_preview_height,
//							new_preview_format) < 0) {
//				LOGE("ERR(%s):Fail on mSecCamera->setPreviewSize(width(%d), height(%d), format(%d))",
//						__func__, new_preview_width, new_preview_height, new_preview_format);
//				ret = UNKNOWN_ERROR;
//			} else {
///*				if (mPreviewWindow) {
//					if (mPreviewRunning && !mPreviewStartDeferred) {
//						LOGE("ERR(%s): preview is running, cannot change size and format!",
//								__func__);
//						ret = INVALID_OPERATION;
//					}
//
//					LOGI("%s: mPreviewWindow (%p) set_buffers_geometry", __func__, mPreviewWindow);
//					LOGI("%s: mPreviewWindow->set_buffers_geometry (%p)", __func__,
//							mPreviewWindow->set_buffers_geometry);
//					mPreviewWindow->set_buffers_geometry(mPreviewWindow,
//							new_preview_width, new_preview_height,
//							new_preview_format);
//					LOGI("%s: DONE mPreviewWindow (%p) set_buffers_geometry", __func__, mPreviewWindow);
//				}*/
//				mParameters.setPreviewSize(new_preview_width, new_preview_height);
//				mParameters.setPreviewFormat(new_str_preview_format);
//			}
//		}
//		else LOGI("%s: preview size and format has not changed", __func__);
//	} else {
//		LOGE("%s: Invalid preview size(%dx%d)",
//				__func__, new_preview_width, new_preview_height);
//
//		ret = INVALID_OPERATION;
//	}
//
//	int new_picture_width = 0;
//	int new_picture_height = 0;
//
//	params.getPictureSize(&new_picture_width, &new_picture_height);
//	LOGI("%s : new_picture_width x new_picture_height = %dx%d", __func__, new_picture_width, new_picture_height);
//	if (0 < new_picture_width && 0 < new_picture_height) {
//		LOGI("%s: setSnapshotSize", __func__);
//		if (setSnapshotSize(new_picture_width, new_picture_height) < 0) {
//			LOGE("ERR(%s):Fail on mSecCamera->setSnapshotSize(width(%d), height(%d))",
//					__func__, new_picture_width, new_picture_height);
//			ret = UNKNOWN_ERROR;
//		} else {
//			mParameters.setPictureSize(new_picture_width, new_picture_height);
//		}
//	}
//
//	// picture format
//	const char *new_str_picture_format = params.getPictureFormat();
//	LOGI("%s : new_str_picture_format %s", __func__, new_str_picture_format);
//	if (new_str_picture_format != NULL) {
//		int new_picture_format = 0;
//
//		if (!strcmp(new_str_picture_format, CameraParameters::PIXEL_FORMAT_RGB565))
//		new_picture_format = V4L2_PIX_FMT_RGB565;
//		else if (!strcmp(new_str_picture_format, CameraParameters::PIXEL_FORMAT_RGBA8888))
//		new_picture_format = V4L2_PIX_FMT_RGB32;
//		else if (!strcmp(new_str_picture_format, CameraParameters::PIXEL_FORMAT_YUV420SP))
//		new_picture_format = V4L2_PIX_FMT_NV21;
//		else if (!strcmp(new_str_picture_format, "yuv420sp_custom"))
//		new_picture_format = V4L2_PIX_FMT_NV12T;
//		else if (!strcmp(new_str_picture_format, "yuv420p"))
//		new_picture_format = V4L2_PIX_FMT_YUV420;
//		else if (!strcmp(new_str_picture_format, "yuv422i"))
//		new_picture_format = V4L2_PIX_FMT_YUYV;
//		else if (!strcmp(new_str_picture_format, "uyv422i_custom")) //Zero copy UYVY format
//		new_picture_format = V4L2_PIX_FMT_UYVY;
//		else if (!strcmp(new_str_picture_format, "uyv422i"))//Non-zero copy UYVY format
//		new_picture_format = V4L2_PIX_FMT_UYVY;
//		else if (!strcmp(new_str_picture_format, CameraParameters::PIXEL_FORMAT_JPEG))
//		new_picture_format = V4L2_PIX_FMT_YUYV;
//		else if (!strcmp(new_str_picture_format, "yuv422p"))
//		new_picture_format = V4L2_PIX_FMT_YUV422P;
//		else
//		new_picture_format = V4L2_PIX_FMT_NV21;//for 3rd party
//
//		if (setSnapshotPixelFormat(new_picture_format) < 0) {
//			LOGE("ERR(%s):Fail on mSecCamera->setSnapshotPixelFormat(format(%d))", __func__, new_picture_format);
//			ret = UNKNOWN_ERROR;
//		} else {
//			mParameters.setPictureFormat(new_str_picture_format);
//		}
//	}
//
//	//JPEG image quality
//	int new_jpeg_quality = params.getInt(CameraParameters::KEY_JPEG_QUALITY);
//	LOGI("%s : new_jpeg_quality %d", __func__, new_jpeg_quality);
//	/* we ignore bad values */
//	if (new_jpeg_quality >=1 && new_jpeg_quality <= 100) {
//		if (setJpegQuality(new_jpeg_quality) < 0) {
//			LOGE("ERR(%s):Fail on mSecCamera->setJpegQuality(quality(%d))", __func__, new_jpeg_quality);
//			ret = UNKNOWN_ERROR;
//		} else {
//			mParameters.set(CameraParameters::KEY_JPEG_QUALITY, new_jpeg_quality);
//		}
//	}
//
//	// JPEG thumbnail size
//	int new_jpeg_thumbnail_width = params.getInt(CameraParameters::KEY_JPEG_THUMBNAIL_WIDTH);
//	int new_jpeg_thumbnail_height= params.getInt(CameraParameters::KEY_JPEG_THUMBNAIL_HEIGHT);
//	if (0 <= new_jpeg_thumbnail_width && 0 <= new_jpeg_thumbnail_height) {
//		if (setJpegThumbnailSize(new_jpeg_thumbnail_width, new_jpeg_thumbnail_height) < 0) {
//			LOGE("ERR(%s):Fail on mSecCamera->setJpegThumbnailSize(width(%d), height(%d))", __func__, new_jpeg_thumbnail_width, new_jpeg_thumbnail_height);
//			ret = UNKNOWN_ERROR;
//		} else {
//			mParameters.set(CameraParameters::KEY_JPEG_THUMBNAIL_WIDTH, new_jpeg_thumbnail_width);
//			mParameters.set(CameraParameters::KEY_JPEG_THUMBNAIL_HEIGHT, new_jpeg_thumbnail_height);
//		}
//	}
//
//	// frame rate
//	int new_frame_rate = params.getPreviewFrameRate();
//	/* ignore any fps request, we're determine fps automatically based
//	 * on scene mode.  don't return an error because it causes CTS failure.
//	 */
//	if (new_frame_rate != mParameters.getPreviewFrameRate()) {
//		LOGW("WARN(%s): request for preview frame %d not allowed, != %d\n",
//				__func__, new_frame_rate, mParameters.getPreviewFrameRate());
//	}
//
//	// rotation
//	int new_rotation = params.getInt(CameraParameters::KEY_ROTATION);
//	LOGI("%s : new_rotation %d", __func__, new_rotation);
//	if (0 <= new_rotation) {
//		LOGI("%s : set orientation:%d\n", __func__, new_rotation);
//		if (setExifOrientationInfo(new_rotation) < 0) {
//			LOGE("ERR(%s):Fail on mSecCamera->setExifOrientationInfo(%d)", __func__, new_rotation);
//			ret = UNKNOWN_ERROR;
//		} else {
//			mParameters.set(CameraParameters::KEY_ROTATION, new_rotation);
//		}
//	}
//
//	// brightness
//	int new_exposure_compensation = params.getInt(CameraParameters::KEY_EXPOSURE_COMPENSATION);
//	int max_exposure_compensation = params.getInt(CameraParameters::KEY_MAX_EXPOSURE_COMPENSATION);
//	int min_exposure_compensation = params.getInt(CameraParameters::KEY_MIN_EXPOSURE_COMPENSATION);
//	LOGI("%s : new_exposure_compensation %d", __func__, new_exposure_compensation);
//	if ((min_exposure_compensation <= new_exposure_compensation) &&
//			(max_exposure_compensation >= new_exposure_compensation)) {
//		if (setBrightness(new_exposure_compensation) < 0) {
//			LOGE("ERR(%s):Fail on mSecCamera->setBrightness(brightness(%d))", __func__, new_exposure_compensation);
//			ret = UNKNOWN_ERROR;
//		} else {
//			mParameters.set(CameraParameters::KEY_EXPOSURE_COMPENSATION, new_exposure_compensation);
//		}
//	}
//
//	// whitebalance
//	const char *new_white_str = params.get(CameraParameters::KEY_WHITE_BALANCE);
//	LOGI("%s : new_white_str %s", __func__, new_white_str);
//	if (new_white_str != NULL) {
//		int new_white = -1;
//
//		if (!strcmp(new_white_str, CameraParameters::WHITE_BALANCE_AUTO))
//		new_white = WHITE_BALANCE_AUTO;
//		else if (!strcmp(new_white_str,
//						CameraParameters::WHITE_BALANCE_DAYLIGHT))
//		new_white = WHITE_BALANCE_SUNNY;
//		else if (!strcmp(new_white_str,
//						CameraParameters::WHITE_BALANCE_CLOUDY_DAYLIGHT))
//		new_white = WHITE_BALANCE_CLOUDY;
//		else if (!strcmp(new_white_str,
//						CameraParameters::WHITE_BALANCE_FLUORESCENT))
//		new_white = WHITE_BALANCE_FLUORESCENT;
//		else if (!strcmp(new_white_str,
//						CameraParameters::WHITE_BALANCE_INCANDESCENT))
//		new_white = WHITE_BALANCE_TUNGSTEN;
//		else {
//			LOGE("ERR(%s):Invalid white balance(%s)", __func__, new_white_str); //twilight, shade, warm_flourescent
//			ret = UNKNOWN_ERROR;
//		}
//
//		if (0 <= new_white) {
//			if (setWhiteBalance(new_white) < 0) {
//				LOGE("ERR(%s):Fail on mSecCamera->setWhiteBalance(white(%d))", __func__, new_white);
//				ret = UNKNOWN_ERROR;
//			} else {
//				mParameters.set(CameraParameters::KEY_WHITE_BALANCE, new_white_str);
//			}
//		}
//	}
//
//	// scene mode
//	const char *new_scene_mode_str = params.get(CameraParameters::KEY_SCENE_MODE);
//	const char *current_scene_mode_str = mParameters.get(CameraParameters::KEY_SCENE_MODE);
//
//	// fps range
//	int new_min_fps = 0;
//	int new_max_fps = 0;
//	int current_min_fps, current_max_fps;
//	params.getPreviewFpsRange(&new_min_fps, &new_max_fps);
//	mParameters.getPreviewFpsRange(&current_min_fps, &current_max_fps);
//	/* our fps range is determined by the sensor, reject any request
//	 * that isn't exactly what we're already at.
//	 * but the check is performed when requesting only changing fps range
//	 */
//	if (new_scene_mode_str && current_scene_mode_str) {
//		if (!strcmp(new_scene_mode_str, current_scene_mode_str)) {
//			if ((new_min_fps != current_min_fps) || (new_max_fps != current_max_fps)) {
//				LOGW("%s : requested new_min_fps = %d, new_max_fps = %d not allowed",
//						__func__, new_min_fps, new_max_fps);
//				LOGE("%s : current_min_fps = %d, current_max_fps = %d",
//						__func__, current_min_fps, current_max_fps);
//				ret = UNKNOWN_ERROR;
//			}
//		}
//	} else {
//		/* Check basic validation if scene mode is different */
//		if ((new_min_fps > new_max_fps) ||
//				(new_min_fps < 0) || (new_max_fps < 0))
//		ret = UNKNOWN_ERROR;
//	}
//
//	const char *new_focus_mode_str = params.get(CameraParameters::KEY_FOCUS_MODE);
//	if (m_camera_id == CAMERA_ID_BACK) {
//		int new_scene_mode = -1;
//
//		const char *new_flash_mode_str = params.get(CameraParameters::KEY_FLASH_MODE);
//
//		// fps range is (15000,30000) by default.
//		mParameters.set(CameraParameters::KEY_SUPPORTED_PREVIEW_FPS_RANGE, "(15000,30000)");
//		mParameters.set(CameraParameters::KEY_PREVIEW_FPS_RANGE,
//				"15000,30000");
//
//		if (!strcmp(new_scene_mode_str, CameraParameters::SCENE_MODE_AUTO)) {
//			new_scene_mode = SCENE_MODE_NONE;
//			mParameters.set(CameraParameters::KEY_SUPPORTED_FLASH_MODES, "on,off,auto,torch");
//		} else {
//			// defaults for non-auto scene modes
//			if (m_camera_id == CAMERA_ID_BACK) {
//				new_focus_mode_str = CameraParameters::FOCUS_MODE_AUTO;
//			}
//			new_flash_mode_str = CameraParameters::FLASH_MODE_OFF;
//
//			if (!strcmp(new_scene_mode_str,
//							CameraParameters::SCENE_MODE_PORTRAIT)) {
//				new_scene_mode = SCENE_MODE_PORTRAIT;
//				new_flash_mode_str = CameraParameters::FLASH_MODE_AUTO;
//				mParameters.set(CameraParameters::KEY_SUPPORTED_FLASH_MODES, "auto");
//			} else if (!strcmp(new_scene_mode_str,
//							CameraParameters::SCENE_MODE_LANDSCAPE)) {
//				new_flash_mode_str = CameraParameters::FLASH_MODE_OFF;
//				mParameters.set(CameraParameters::KEY_SUPPORTED_FLASH_MODES, "off");
//				new_scene_mode = SCENE_MODE_LANDSCAPE;
//			} else if (!strcmp(new_scene_mode_str,
//							CameraParameters::SCENE_MODE_SPORTS)) {
//				new_scene_mode = SCENE_MODE_SPORTS;
//				new_flash_mode_str = CameraParameters::FLASH_MODE_OFF;
//				mParameters.set(CameraParameters::KEY_SUPPORTED_FLASH_MODES, "off");
//			} else if (!strcmp(new_scene_mode_str,
//							CameraParameters::SCENE_MODE_PARTY)) {
//				new_scene_mode = SCENE_MODE_PARTY_INDOOR;
//				new_flash_mode_str = CameraParameters::FLASH_MODE_AUTO;
//				mParameters.set(CameraParameters::KEY_SUPPORTED_FLASH_MODES, "auto");
//			} else if ((!strcmp(new_scene_mode_str,
//									CameraParameters::SCENE_MODE_BEACH)) ||
//					(!strcmp(new_scene_mode_str,
//									CameraParameters::SCENE_MODE_SNOW))) {
//				new_scene_mode = SCENE_MODE_BEACH_SNOW;
//				new_flash_mode_str = CameraParameters::FLASH_MODE_OFF;
//				mParameters.set(CameraParameters::KEY_SUPPORTED_FLASH_MODES, "off");
//			} else if (!strcmp(new_scene_mode_str,
//							CameraParameters::SCENE_MODE_SUNSET)) {
//				new_scene_mode = SCENE_MODE_SUNSET;
//				new_flash_mode_str = CameraParameters::FLASH_MODE_OFF;
//				mParameters.set(CameraParameters::KEY_SUPPORTED_FLASH_MODES, "off");
//			} else if (!strcmp(new_scene_mode_str,
//							CameraParameters::SCENE_MODE_NIGHT)) {
//				new_scene_mode = SCENE_MODE_NIGHTSHOT;
//				mParameters.set(CameraParameters::KEY_SUPPORTED_PREVIEW_FPS_RANGE, "(4000,30000)");
//				mParameters.set(CameraParameters::KEY_PREVIEW_FPS_RANGE,
//						"4000,30000");
//				new_flash_mode_str = CameraParameters::FLASH_MODE_OFF;
//				mParameters.set(CameraParameters::KEY_SUPPORTED_FLASH_MODES, "off");
//
//			} else if (!strcmp(new_scene_mode_str,
//							CameraParameters::SCENE_MODE_FIREWORKS)) {
//				new_scene_mode = SCENE_MODE_FIREWORKS;
//				new_flash_mode_str = CameraParameters::FLASH_MODE_OFF;
//				mParameters.set(CameraParameters::KEY_SUPPORTED_FLASH_MODES, "off");
//			} else if (!strcmp(new_scene_mode_str,
//							CameraParameters::SCENE_MODE_CANDLELIGHT)) {
//				new_scene_mode = SCENE_MODE_CANDLE_LIGHT;
//				new_flash_mode_str = CameraParameters::FLASH_MODE_OFF;
//				mParameters.set(CameraParameters::KEY_SUPPORTED_FLASH_MODES, "off");
//			} else {
//				LOGE("%s::unmatched scene_mode(%s)",
//						__func__, new_scene_mode_str); //action, night-portrait, theatre, steadyphoto
//				ret = UNKNOWN_ERROR;
//			}
//		}
//
//		// focus mode
//		if (new_focus_mode_str != NULL) {
//			int new_focus_mode = -1;
//
//			if (!strcmp(new_focus_mode_str,
//							CameraParameters::FOCUS_MODE_AUTO)) {
//				new_focus_mode = FOCUS_MODE_AUTO;
//				mParameters.set(CameraParameters::KEY_FOCUS_DISTANCES,
//						BACK_CAMERA_AUTO_FOCUS_DISTANCES_STR);
//			}
//			else if (!strcmp(new_focus_mode_str,
//							CameraParameters::FOCUS_MODE_MACRO)) {
//				new_focus_mode = FOCUS_MODE_MACRO;
//				mParameters.set(CameraParameters::KEY_FOCUS_DISTANCES,
//						BACK_CAMERA_MACRO_FOCUS_DISTANCES_STR);
//			}
//			else if (!strcmp(new_focus_mode_str,
//							CameraParameters::FOCUS_MODE_INFINITY)) {
//				new_focus_mode = FOCUS_MODE_INFINITY;
//				mParameters.set(CameraParameters::KEY_FOCUS_DISTANCES,
//						BACK_CAMERA_INFINITY_FOCUS_DISTANCES_STR);
//			}
//			else {
//				LOGE("%s::unmatched focus_mode(%s)", __func__, new_focus_mode_str);
//				ret = UNKNOWN_ERROR;
//			}
//
//			if (0 <= new_focus_mode) {
//				if (setFocusMode(new_focus_mode) < 0) {
//					LOGE("%s::mSecCamera->setFocusMode(%d) fail", __func__, new_focus_mode);
//					ret = UNKNOWN_ERROR;
//				} else {
//					mParameters.set(CameraParameters::KEY_FOCUS_MODE, new_focus_mode_str);
//				}
//			}
//		}
//
//		// flash..
//		if (new_flash_mode_str != NULL) {
//			int new_flash_mode = -1;
//
//			if (!strcmp(new_flash_mode_str, CameraParameters::FLASH_MODE_OFF))
//			new_flash_mode = FLASH_MODE_OFF;
//			else if (!strcmp(new_flash_mode_str, CameraParameters::FLASH_MODE_AUTO))
//			new_flash_mode = FLASH_MODE_AUTO;
//			else if (!strcmp(new_flash_mode_str, CameraParameters::FLASH_MODE_ON))
//			new_flash_mode = FLASH_MODE_ON;
//			else if (!strcmp(new_flash_mode_str, CameraParameters::FLASH_MODE_TORCH))
//			new_flash_mode = FLASH_MODE_TORCH;
//			else {
//				LOGE("%s::unmatched flash_mode(%s)", __func__, new_flash_mode_str); //red-eye
//				ret = UNKNOWN_ERROR;
//			}
//			if (0 <= new_flash_mode) {
//				if (setFlashMode(new_flash_mode) < 0) {
//					LOGE("%s::mSecCamera->setFlashMode(%d) fail", __func__, new_flash_mode);
//					ret = UNKNOWN_ERROR;
//				} else {
//					mParameters.set(CameraParameters::KEY_FLASH_MODE, new_flash_mode_str);
//				}
//			}
//		}
//
//		//  scene..
//		if (0 <= new_scene_mode) {
//			if (setSceneMode(new_scene_mode) < 0) {
//				LOGE("%s::mSecCamera->setSceneMode(%d) fail", __func__, new_scene_mode);
//				ret = UNKNOWN_ERROR;
//			} else {
//				mParameters.set(CameraParameters::KEY_SCENE_MODE, new_scene_mode_str);
//			}
//		}
//	} else {
//		if (!isSupportedParameter(new_focus_mode_str,
//						mParameters.get(CameraParameters::KEY_SUPPORTED_FOCUS_MODES))) {
//			LOGE("%s: Unsupported focus mode: %s", __func__, new_focus_mode_str);
//			ret = UNKNOWN_ERROR;
//		}
//	}
//
//	// ---------------------------------------------------------------------------
//
//	// image effect
//	const char *new_image_effect_str = params.get(CameraParameters::KEY_EFFECT);
//	if (new_image_effect_str != NULL) {
//
//		int new_image_effect = -1;
//
//		if (!strcmp(new_image_effect_str, CameraParameters::EFFECT_NONE))
//		new_image_effect = IMAGE_EFFECT_NONE;
//		else if (!strcmp(new_image_effect_str, CameraParameters::EFFECT_MONO))
//		new_image_effect = IMAGE_EFFECT_BNW;
//		else if (!strcmp(new_image_effect_str, CameraParameters::EFFECT_SEPIA))
//		new_image_effect = IMAGE_EFFECT_SEPIA;
//		else if (!strcmp(new_image_effect_str, CameraParameters::EFFECT_AQUA))
//		new_image_effect = IMAGE_EFFECT_AQUA;
//		else if (!strcmp(new_image_effect_str, CameraParameters::EFFECT_NEGATIVE))
//		new_image_effect = IMAGE_EFFECT_NEGATIVE;
//		else {
//			//posterize, whiteboard, blackboard, solarize
//			LOGE("ERR(%s):Invalid effect(%s)", __func__, new_image_effect_str);
//			ret = UNKNOWN_ERROR;
//		}
//
//		if (new_image_effect >= 0) {
//			if (setImageEffect(new_image_effect) < 0) {
//				LOGE("ERR(%s):Fail on mSecCamera->setImageEffect(effect(%d))", __func__, new_image_effect);
//				ret = UNKNOWN_ERROR;
//			} else {
//				const char *old_image_effect_str = mParameters.get(CameraParameters::KEY_EFFECT);
//
//				if (old_image_effect_str) {
//					if (strcmp(old_image_effect_str, new_image_effect_str)) {
//						setSkipFrame(EFFECT_SKIP_FRAME);
//					}
//				}
//
//				mParameters.set(CameraParameters::KEY_EFFECT, new_image_effect_str);
//			}
//		}
//	}
//
//	//vt mode
//	int new_vtmode = mInternalParameters.getInt("vtmode");
//	if (0 <= new_vtmode) {
//		if (setVTmode(new_vtmode) < 0) {
//			LOGE("ERR(%s):Fail on mSecCamera->setVTMode(%d)", __func__, new_vtmode);
//			ret = UNKNOWN_ERROR;
//		}
//	}
//
//	//contrast
//	int new_contrast = mInternalParameters.getInt("contrast");
//
//	if (0 <= new_contrast) {
//		if (setContrast(new_contrast) < 0) {
//			LOGE("ERR(%s):Fail on mSecCamera->setContrast(%d)", __func__, new_contrast);
//			ret = UNKNOWN_ERROR;
//		}
//	}
//
//	//WDR
//	int new_wdr = mInternalParameters.getInt("wdr");
//
//	if (0 <= new_wdr) {
//		if (setWDR(new_wdr) < 0) {
//			LOGE("ERR(%s):Fail on mSecCamera->setWDR(%d)", __func__, new_wdr);
//			ret = UNKNOWN_ERROR;
//		}
//	}
//
//	//anti shake
//	int new_anti_shake = mInternalParameters.getInt("anti-shake");
//
//	if (0 <= new_anti_shake) {
//		if (setAntiShake(new_anti_shake) < 0) {
//			LOGE("ERR(%s):Fail on mSecCamera->setWDR(%d)", __func__, new_anti_shake);
//			ret = UNKNOWN_ERROR;
//		}
//	}
//
//	// gps latitude
//	const char *new_gps_latitude_str = params.get(CameraParameters::KEY_GPS_LATITUDE);
//	if (setGPSLatitude(new_gps_latitude_str) < 0) {
//		LOGE("%s::mSecCamera->setGPSLatitude(%s) fail", __func__, new_gps_latitude_str);
//		ret = UNKNOWN_ERROR;
//	} else {
//		if (new_gps_latitude_str) {
//			mParameters.set(CameraParameters::KEY_GPS_LATITUDE, new_gps_latitude_str);
//		} else {
//			mParameters.remove(CameraParameters::KEY_GPS_LATITUDE);
//		}
//	}
//
//	// gps longitude
//	const char *new_gps_longitude_str = params.get(CameraParameters::KEY_GPS_LONGITUDE);
//
//	if (setGPSLongitude(new_gps_longitude_str) < 0) {
//		LOGE("%s::mSecCamera->setGPSLongitude(%s) fail", __func__, new_gps_longitude_str);
//		ret = UNKNOWN_ERROR;
//	} else {
//		if (new_gps_longitude_str) {
//			mParameters.set(CameraParameters::KEY_GPS_LONGITUDE, new_gps_longitude_str);
//		} else {
//			mParameters.remove(CameraParameters::KEY_GPS_LONGITUDE);
//		}
//	}
//
//	// gps altitude
//	const char *new_gps_altitude_str = params.get(CameraParameters::KEY_GPS_ALTITUDE);
//
//	if (setGPSAltitude(new_gps_altitude_str) < 0) {
//		LOGE("%s::mSecCamera->setGPSAltitude(%s) fail", __func__, new_gps_altitude_str);
//		ret = UNKNOWN_ERROR;
//	} else {
//		if (new_gps_altitude_str) {
//			mParameters.set(CameraParameters::KEY_GPS_ALTITUDE, new_gps_altitude_str);
//		} else {
//			mParameters.remove(CameraParameters::KEY_GPS_ALTITUDE);
//		}
//	}
//
//	// gps timestamp
//	const char *new_gps_timestamp_str = params.get(CameraParameters::KEY_GPS_TIMESTAMP);
//
//	if (setGPSTimeStamp(new_gps_timestamp_str) < 0) {
//		LOGE("%s::mSecCamera->setGPSTimeStamp(%s) fail", __func__, new_gps_timestamp_str);
//		ret = UNKNOWN_ERROR;
//	} else {
//		if (new_gps_timestamp_str) {
//			mParameters.set(CameraParameters::KEY_GPS_TIMESTAMP, new_gps_timestamp_str);
//		} else {
//			mParameters.remove(CameraParameters::KEY_GPS_TIMESTAMP);
//		}
//	}
//
//	// gps processing method
//	const char *new_gps_processing_method_str = params.get(CameraParameters::KEY_GPS_PROCESSING_METHOD);
//
//	if (setGPSProcessingMethod(new_gps_processing_method_str) < 0) {
//		LOGE("%s::mSecCamera->setGPSProcessingMethod(%s) fail", __func__, new_gps_processing_method_str);
//		ret = UNKNOWN_ERROR;
//	} else {
//		if (new_gps_processing_method_str) {
//			mParameters.set(CameraParameters::KEY_GPS_PROCESSING_METHOD, new_gps_processing_method_str);
//		} else {
//			mParameters.remove(CameraParameters::KEY_GPS_PROCESSING_METHOD);
//		}
//	}
//
//	// Recording size
//	int new_recording_width = mInternalParameters.getInt("recording-size-width");
//	int new_recording_height= mInternalParameters.getInt("recording-size-height");
//
//	if (0 < new_recording_width && 0 < new_recording_height) {
//		if (setRecordingSize(new_recording_width, new_recording_height) < 0) {
//			LOGE("ERR(%s):Fail on mSecCamera->setRecordingSize(width(%d), height(%d))", __func__, new_recording_width, new_recording_height);
//			ret = UNKNOWN_ERROR;
//		}
//	} else {
//		if (setRecordingSize(new_preview_width, new_preview_height) < 0) {
//			LOGE("ERR(%s):Fail on mSecCamera->setRecordingSize(width(%d), height(%d))", __func__, new_preview_width, new_preview_height);
//			ret = UNKNOWN_ERROR;
//		}
//	}
//
//	//gamma
//	const char *new_gamma_str = mInternalParameters.get("video_recording_gamma");
//
//	if (new_gamma_str != NULL) {
//		int new_gamma = -1;
//		if (!strcmp(new_gamma_str, "off"))
//		new_gamma = GAMMA_OFF;
//		else if (!strcmp(new_gamma_str, "on"))
//		new_gamma = GAMMA_ON;
//		else {
//			LOGE("%s::unmatched gamma(%s)", __func__, new_gamma_str);
//			ret = UNKNOWN_ERROR;
//		}
//
//		if (0 <= new_gamma) {
//			if (setGamma(new_gamma) < 0) {
//				LOGE("%s::mSecCamera->setGamma(%d) fail", __func__, new_gamma);
//				ret = UNKNOWN_ERROR;
//			}
//		}
//	}
//
//	//slow ae
//	const char *new_slow_ae_str = mInternalParameters.get("slow_ae");
//
//	if (new_slow_ae_str != NULL) {
//		int new_slow_ae = -1;
//
//		if (!strcmp(new_slow_ae_str, "off"))
//		new_slow_ae = SLOW_AE_OFF;
//		else if (!strcmp(new_slow_ae_str, "on"))
//		new_slow_ae = SLOW_AE_ON;
//		else {
//			LOGE("%s::unmatched slow_ae(%s)", __func__, new_slow_ae_str);
//			ret = UNKNOWN_ERROR;
//		}
//
//		if (0 <= new_slow_ae) {
//			if (setSlowAE(new_slow_ae) < 0) {
//				LOGE("%s::mSecCamera->setSlowAE(%d) fail", __func__, new_slow_ae);
//				ret = UNKNOWN_ERROR;
//			}
//		}
//	}
//
//	/*Camcorder fix fps*/
//	int new_sensor_mode = mInternalParameters.getInt("cam_mode");
//
//	if (0 <= new_sensor_mode) {
//		if (setSensorMode(new_sensor_mode) < 0) {
//			LOGE("ERR(%s):Fail on mSecCamera->setSensorMode(%d)", __func__, new_sensor_mode);
//			ret = UNKNOWN_ERROR;
//		}
//	} else {
//		new_sensor_mode=0;
//	}
//
//	/*Shot mode*/
//	int new_shot_mode = mInternalParameters.getInt("shot_mode");
//
//	if (0 <= new_shot_mode) {
//		if (setShotMode(new_shot_mode) < 0) {
//			LOGE("ERR(%s):Fail on mSecCamera->setShotMode(%d)", __func__, new_shot_mode);
//			ret = UNKNOWN_ERROR;
//		}
//	} else {
//		new_shot_mode=0;
//	}
//
//	//blur for Video call
//	int new_blur_level = mInternalParameters.getInt("blur");
//
//	if (0 <= new_blur_level) {
//		if (setBlur(new_blur_level) < 0) {
//			LOGE("ERR(%s):Fail on mSecCamera->setBlur(%d)", __func__, new_blur_level);
//			ret = UNKNOWN_ERROR;
//		}
//	}
//
//	// chk_dataline
//	int new_dataline = mInternalParameters.getInt("chk_dataline");
//
//	if (0 <= new_dataline) {
//		if (setDataLineCheck(new_dataline) < 0) {
//			LOGE("ERR(%s):Fail on mSecCamera->setDataLineCheck(%d)", __func__, new_dataline);
//			ret = UNKNOWN_ERROR;
//		}
//	}
//	LOGI("%s return ret = %d", __func__, ret);
//
//	return ret;
}

static int init_yuv_buffers(struct fimc_buffer *buffers, int width, int height, unsigned int fmt)
{
	int i, len;

	len = (width * height * get_pixel_depth(fmt)) / 8;

	for (i = 0; i < MAX_BUFFERS; i++) {
		if (fmt==V4L2_PIX_FMT_NV12T) {
			buffers[i].start = NULL;
			buffers[i].length = ALIGN_BUF(ALIGN_W(width) * ALIGN_H(height)) +
			ALIGN_BUF(ALIGN_W(width) * ALIGN_H(height / 2));
		} else {
			buffers[i].start = NULL;
			buffers[i].length = len;
		}
	}

	return 0;
}

static int close_buffers(struct fimc_buffer *buffers)
{
	int i;

	for (i = 0; i < MAX_BUFFERS; i++) {
		if (buffers[i].start) {
			munmap(buffers[i].start, buffers[i].length);
			//LOGI("munmap():virt. addr[%d]: 0x%x size = %d\n", i, (unsigned int) buffers[i].start, buffers[i].length);
			buffers[i].start = NULL;
		}
	}

	return 0;
}

void DeinitCamera()
{
	LOGI("%s :", __func__);

	if (m_flag_init) {

//        stopRecord();

		/* close m_cam_fd after stopRecord() because stopRecord()
		 * uses m_cam_fd to change frame rate
		 */
		LOGI("DeinitCamera: m_cam_fd(%d)", m_cam_fd);
		if (m_cam_fd > -1) {
			close(m_cam_fd);
			m_cam_fd = -1;
		}

		LOGI("DeinitCamera: m_cam_fd2(%d)", m_cam_fd2);
		if (m_cam_fd2 > -1) {
			close(m_cam_fd2);
			m_cam_fd2 = -1;
		}

		m_flag_init = 0;
	}
	else LOGI("%s : already deinitialized", __FUNCTION__);

}

void resetCamera()
{
	LOGI("%s :", __func__);
	DeinitCamera();
	initCamera(m_camera_id);
}

int setCameraId(int camera_id)
{
	if ((camera_id != CAMERA_ID_FRONT) && (camera_id != CAMERA_ID_BACK)) {
		LOGE("ERR(%s)::Invalid camera id(%d)\n", __func__, camera_id);
		return -1;
	}
	if (m_camera_id == camera_id)
	return 0;

	LOGI("%s(camera_id(%d))", __func__, camera_id);

	switch (camera_id) {
		case CAMERA_ID_FRONT:
		m_preview_max_width = MAX_FRONT_CAMERA_PREVIEW_WIDTH;
		m_preview_max_height = MAX_FRONT_CAMERA_PREVIEW_HEIGHT;
		m_snapshot_max_width = MAX_FRONT_CAMERA_SNAPSHOT_WIDTH;
		m_snapshot_max_height = MAX_FRONT_CAMERA_SNAPSHOT_HEIGHT;
		break;

		case CAMERA_ID_BACK:
		m_preview_max_width = MAX_BACK_CAMERA_PREVIEW_WIDTH;
		m_preview_max_height = MAX_BACK_CAMERA_PREVIEW_HEIGHT;
		m_snapshot_max_width = MAX_BACK_CAMERA_SNAPSHOT_WIDTH;
		m_snapshot_max_height = MAX_BACK_CAMERA_SNAPSHOT_HEIGHT;
		break;
	}

	m_camera_id = camera_id;

	resetCamera();

	return 0;
}

void initDefaultParameters(int cameraId)
{
    LOGI("initDefaultParameters");

    CameraParameters p;
    CameraParameters ip;

    mCameraSensorName = getCameraSensorName();
    LOGI("CameraSensorName: %s", mCameraSensorName);

    int preview_max_width   = 0;
    int preview_max_height  = 0;
    int snapshot_max_width  = 0;
    int snapshot_max_height = 0;

    /* set camera ID & reset camera */
    setCameraId(cameraId);
    if (cameraId == CAMERA_ID_BACK) {
        p.set(CameraParameters::KEY_SUPPORTED_PREVIEW_SIZES,
              "720x480,640x480,352x288,176x144");
        p.set(CameraParameters::KEY_SUPPORTED_PICTURE_SIZES,
              "2560x1920,2048x1536,1600x1200,1280x960,640x480");
    } else {
        p.set(CameraParameters::KEY_SUPPORTED_PREVIEW_SIZES,
              "640x480");
        p.set(CameraParameters::KEY_SUPPORTED_PICTURE_SIZES,
              "640x480");
    }

    // If these fail, then we are using an invalid cameraId and we'll leave the
    // sizes at zero to catch the error.
    if (getPreviewMaxSize(&preview_max_width,
                                      &preview_max_height) < 0)
        LOGE("getPreviewMaxSize fail (%d / %d) \n",
             preview_max_width, preview_max_height);
    if (getSnapshotMaxSize(&snapshot_max_width,
                                       &snapshot_max_height) < 0)
        LOGE("getSnapshotMaxSize fail (%d / %d) \n",
             snapshot_max_width, snapshot_max_height);

#ifdef PREVIEW_USING_MMAP
    p.setPreviewFormat(CameraParameters::PIXEL_FORMAT_YUV420SP);
#else
    p.setPreviewFormat("yuv420sp_custom");
#endif
    p.setPreviewSize(preview_max_width, preview_max_height);

    p.setPictureFormat(CameraParameters::PIXEL_FORMAT_JPEG);
    p.setPictureSize(snapshot_max_width, snapshot_max_height);
    p.set(CameraParameters::KEY_JPEG_QUALITY, "100"); // maximum quality
#ifdef SWP1_CAMERA_ADD_ADVANCED_FUNCTION
    p.set(CameraParameters::KEY_SUPPORTED_PREVIEW_FORMATS,
          CameraParameters::PIXEL_FORMAT_YUV420SP);
    p.set(CameraParameters::KEY_SUPPORTED_PICTURE_FORMATS,
          CameraParameters::PIXEL_FORMAT_JPEG);
    p.set(CameraParameters::KEY_VIDEO_FRAME_FORMAT,
          CameraParameters::PIXEL_FORMAT_YUV420SP);

    String8 parameterString;

    if (cameraId == SecCamera::CAMERA_ID_BACK) {
        parameterString = CameraParameters::FOCUS_MODE_AUTO;
        parameterString.append(",");
        parameterString.append(CameraParameters::FOCUS_MODE_INFINITY);
        parameterString.append(",");
        parameterString.append(CameraParameters::FOCUS_MODE_MACRO);
        p.set(CameraParameters::KEY_SUPPORTED_FOCUS_MODES,
              parameterString.string());
        p.set(CameraParameters::KEY_FOCUS_MODE,
              CameraParameters::FOCUS_MODE_AUTO);
        p.set(CameraParameters::KEY_FOCUS_DISTANCES,
              BACK_CAMERA_AUTO_FOCUS_DISTANCES_STR);
        p.set(CameraParameters::KEY_SUPPORTED_JPEG_THUMBNAIL_SIZES,
              "320x240,0x0");
        p.set(CameraParameters::KEY_JPEG_THUMBNAIL_WIDTH, "320");
        p.set(CameraParameters::KEY_JPEG_THUMBNAIL_HEIGHT, "240");
        p.set(CameraParameters::KEY_SUPPORTED_PREVIEW_FRAME_RATES, "30");
        p.setPreviewFrameRate(30);
    } else {
        parameterString = CameraParameters::FOCUS_MODE_FIXED;
        p.set(CameraParameters::KEY_SUPPORTED_FOCUS_MODES,
              parameterString.string());
        p.set(CameraParameters::KEY_FOCUS_MODE,
              CameraParameters::FOCUS_MODE_FIXED);
        p.set(CameraParameters::KEY_FOCUS_DISTANCES,
              FRONT_CAMERA_FOCUS_DISTANCES_STR);
        p.set(CameraParameters::KEY_SUPPORTED_JPEG_THUMBNAIL_SIZES,
              "160x120,0x0");
        p.set(CameraParameters::KEY_JPEG_THUMBNAIL_WIDTH, "160");
        p.set(CameraParameters::KEY_JPEG_THUMBNAIL_HEIGHT, "120");
        p.set(CameraParameters::KEY_SUPPORTED_PREVIEW_FRAME_RATES, "15");
        p.setPreviewFrameRate(15);
    }

    parameterString = CameraParameters::EFFECT_NONE;
    parameterString.append(",");
    parameterString.append(CameraParameters::EFFECT_MONO);
    parameterString.append(",");
    parameterString.append(CameraParameters::EFFECT_NEGATIVE);
    parameterString.append(",");
    parameterString.append(CameraParameters::EFFECT_SEPIA);
    p.set(CameraParameters::KEY_SUPPORTED_EFFECTS, parameterString.string());

    if (cameraId == CAMERA_ID_BACK) {
        parameterString = CameraParameters::FLASH_MODE_ON;
        parameterString.append(",");
        parameterString.append(CameraParameters::FLASH_MODE_OFF);
        parameterString.append(",");
        parameterString.append(CameraParameters::FLASH_MODE_AUTO);
        parameterString.append(",");
        parameterString.append(CameraParameters::FLASH_MODE_TORCH);
        p.set(CameraParameters::KEY_SUPPORTED_FLASH_MODES,
              parameterString.string());
        p.set(CameraParameters::KEY_FLASH_MODE,
              CameraParameters::FLASH_MODE_OFF);

        parameterString = CameraParameters::SCENE_MODE_AUTO;
        parameterString.append(",");
        parameterString.append(CameraParameters::SCENE_MODE_PORTRAIT);
        parameterString.append(",");
        parameterString.append(CameraParameters::SCENE_MODE_LANDSCAPE);
        parameterString.append(",");
        parameterString.append(CameraParameters::SCENE_MODE_NIGHT);
        parameterString.append(",");
        parameterString.append(CameraParameters::SCENE_MODE_BEACH);
        parameterString.append(",");
        parameterString.append(CameraParameters::SCENE_MODE_SNOW);
        parameterString.append(",");
        parameterString.append(CameraParameters::SCENE_MODE_SUNSET);
        parameterString.append(",");
        parameterString.append(CameraParameters::SCENE_MODE_FIREWORKS);
        parameterString.append(",");
        parameterString.append(CameraParameters::SCENE_MODE_SPORTS);
        parameterString.append(",");
        parameterString.append(CameraParameters::SCENE_MODE_PARTY);
        parameterString.append(",");
        parameterString.append(CameraParameters::SCENE_MODE_CANDLELIGHT);
        p.set(CameraParameters::KEY_SUPPORTED_SCENE_MODES,
              parameterString.string());
        p.set(CameraParameters::KEY_SCENE_MODE,
              CameraParameters::SCENE_MODE_AUTO);

        /* we have two ranges, 4-30fps for night mode and
         * 15-30fps for all others
         */
        p.set(CameraParameters::KEY_SUPPORTED_PREVIEW_FPS_RANGE, "(15000,30000)");
        p.set(CameraParameters::KEY_PREVIEW_FPS_RANGE, "15000,30000");

        p.set(CameraParameters::KEY_FOCAL_LENGTH, "3.43");
    } else {
        p.set(CameraParameters::KEY_SUPPORTED_PREVIEW_FPS_RANGE, "(7500,30000)");
        p.set(CameraParameters::KEY_PREVIEW_FPS_RANGE, "7500,30000");

        p.set(CameraParameters::KEY_FOCAL_LENGTH, "0.9");
    }

    parameterString = CameraParameters::WHITE_BALANCE_AUTO;
    parameterString.append(",");
    parameterString.append(CameraParameters::WHITE_BALANCE_INCANDESCENT);
    parameterString.append(",");
    parameterString.append(CameraParameters::WHITE_BALANCE_FLUORESCENT);
    parameterString.append(",");
    parameterString.append(CameraParameters::WHITE_BALANCE_DAYLIGHT);
    parameterString.append(",");
    parameterString.append(CameraParameters::WHITE_BALANCE_CLOUDY_DAYLIGHT);
    p.set(CameraParameters::KEY_SUPPORTED_WHITE_BALANCE,
          parameterString.string());

    ip.set("sharpness-min", 0);
    ip.set("sharpness-max", 4);
    ip.set("saturation-min", 0);
    ip.set("saturation-max", 4);
    ip.set("contrast-min", 0);
    ip.set("contrast-max", 4);
#endif

    p.set(CameraParameters::KEY_JPEG_THUMBNAIL_QUALITY, "100");

    p.set(CameraParameters::KEY_ROTATION, 0);
    p.set(CameraParameters::KEY_WHITE_BALANCE, CameraParameters::WHITE_BALANCE_AUTO);
#ifdef SWP1_CAMERA_ADD_ADVANCED_FUNCTION
    p.set(CameraParameters::KEY_EFFECT, CameraParameters::EFFECT_NONE);

    ip.set("sharpness", SHARPNESS_DEFAULT);
    ip.set("contrast", CONTRAST_DEFAULT);
    ip.set("saturation", SATURATION_DEFAULT);
    ip.set("iso", "auto");
    ip.set("metering", "center");

    ip.set("wdr", 0);
    ip.set("chk_dataline", 0);
    if (cameraId == SecCamera::CAMERA_ID_FRONT) {
        ip.set("vtmode", 0);
        ip.set("blur", 0);
    }
#else
    ip.set("image-effects", "original");
#endif


    p.set(CameraParameters::KEY_HORIZONTAL_VIEW_ANGLE, "51.2");
    p.set(CameraParameters::KEY_VERTICAL_VIEW_ANGLE, "39.4");

    p.set(CameraParameters::KEY_EXPOSURE_COMPENSATION, "0");
    p.set(CameraParameters::KEY_MAX_EXPOSURE_COMPENSATION, "4");
    p.set(CameraParameters::KEY_MIN_EXPOSURE_COMPENSATION, "-4");
    p.set(CameraParameters::KEY_EXPOSURE_COMPENSATION_STEP, "0.5");

    mParameters = p;
    mInternalParameters = ip;

    /* make sure mSecCamera has all the settings we do.  applications
     * aren't required to call setParameters themselves (only if they
     * want to change something.
     */
    setParameters(p);
    setISO(ISO_AUTO);
    setMetering(METERING_CENTER);
    setContrast(CONTRAST_DEFAULT);
    setSharpness(SHARPNESS_DEFAULT);
    setSaturation(SATURATION_DEFAULT);
    if (cameraId == CAMERA_ID_BACK)
        setFrameRate(30);
    else
        setFrameRate(15);

}

int startPreview(void) {

	v4l2_streamparm streamparm;
	struct sec_cam_parm *parms;
	parms = (struct sec_cam_parm*) &streamparm.parm.raw_data;
	LOGI("%s :", __func__);
	/*
	 // aleady started
	 if (m_flag_camera_start > 0) {
	 LOGE("ERR(%s):Preview was already started\n", __func__);
	 return 0;
	 }
	 */
	if (m_cam_fd <= 0) {
		LOGE("ERR(%s):Camera was closed\n", __func__);
		return -1;
	}

	memset(&m_events_c, 0, sizeof(m_events_c));
	m_events_c.fd = m_cam_fd;
	m_events_c.events = POLLIN | POLLERR;

	/* enum_fmt, s_fmt sample */
	int ret = fimc_v4l2_enum_fmt(m_cam_fd, m_preview_v4lformat);
	CHECK(ret);
	ret = fimc_v4l2_s_fmt(m_cam_fd, m_preview_width, m_preview_height,
			m_preview_v4lformat, 0);
	CHECK(ret);

	init_yuv_buffers(m_buffers_c, m_preview_width, m_preview_height, m_preview_v4lformat);
	ret = fimc_v4l2_reqbufs(m_cam_fd, V4L2_BUF_TYPE_VIDEO_CAPTURE, MAX_BUFFERS);
	CHECK(ret);

	LOGI("%s : m_preview_width: %d m_preview_height: %d m_angle: %d\n",
			__func__, m_preview_width, m_preview_height, m_angle);
	ret = fimc_v4l2_querybuf(m_cam_fd, m_buffers_c, V4L2_BUF_TYPE_VIDEO_CAPTURE, MAX_BUFFERS);
	CHECK(ret);

//	ret = fimc_v4l2_s_ctrl(m_cam_fd,
//			V4L2_CID_CAMERA_CHECK_DATALINE, m_chk_dataline);
//	CHECK(ret);

	if (m_camera_id == CAMERA_ID_FRONT) {
		/* VT mode setting */
		ret = fimc_v4l2_s_ctrl(m_cam_fd, V4L2_CID_CAMERA_VT_MODE, m_vtmode);
		CHECK(ret);
	}

	/* start with all buffers in queue */
	for (int i = 0; i < MAX_BUFFERS; i++) {
		ret = fimc_v4l2_qbuf(m_cam_fd, i);
		CHECK(ret);
	}

	ret = fimc_v4l2_streamon(m_cam_fd);
	CHECK(ret);

	m_flag_camera_start = 1;

	ret = fimc_v4l2_s_parm(m_cam_fd, &m_streamparm);
	CHECK(ret);

	if (m_camera_id == CAMERA_ID_FRONT) {
		/* Blur setting */
		LOGI("m_blur_level = %d", m_blur_level);
		ret = fimc_v4l2_s_ctrl(m_cam_fd, V4L2_CID_CAMERA_VGA_BLUR,
				m_blur_level);
		CHECK(ret);
	}

	// It is a delay for a new frame, not to show the previous bigger ugly picture frame.
	ret = fimc_poll(&m_events_c);
	CHECK(ret);
	ret = fimc_v4l2_s_ctrl(m_cam_fd, V4L2_CID_CAMERA_RETURN_FOCUS, 0);
	CHECK(ret);

	LOGI("%s: got the first frame of the preview\n", __func__);

	return 0;
}

int stopPreview(void) {
	int ret;

	LOGI("%s :", __func__);

	close_buffers(m_buffers_c);

	if (m_flag_camera_start == 0) {
		LOGW("%s: doing nothing because m_flag_camera_start is zero", __func__);
		return 0;
	}

	if (m_params->flash_mode == FLASH_MODE_TORCH)
	setFlashMode(FLASH_MODE_OFF);

	if (m_cam_fd <= 0) {
		LOGE("ERR(%s):Camera was closed\n", __func__);
		return -1;
	}

	ret = fimc_v4l2_streamoff(m_cam_fd);
	CHECK(ret);

	m_flag_camera_start = 0;

	return ret;
}

void setExifChangedAttribute()
{
	//2 0th IFD TIFF Tags
	//3 Width
	mExifInfo.width = m_snapshot_width;
	//3 Height
	mExifInfo.height = m_snapshot_height;
	//3 Orientation
	switch (m_exif_orientation) {
		case 0:
		mExifInfo.orientation = EXIF_ORIENTATION_UP;
		break;
		case 90:
		mExifInfo.orientation = EXIF_ORIENTATION_90;
		break;
		case 180:
		mExifInfo.orientation = EXIF_ORIENTATION_180;
		break;
		case 270:
		mExifInfo.orientation = EXIF_ORIENTATION_270;
		break;
		default:
		mExifInfo.orientation = EXIF_ORIENTATION_UP;
		break;
	}
	//3 Date time
	time_t rawtime;
	struct tm *timeinfo;
	time(&rawtime);
	timeinfo = localtime(&rawtime);
	strftime((char *)mExifInfo.date_time, 20, "%Y:%m:%d %H:%M:%S", timeinfo);

	//2 0th IFD Exif Private Tags
	//3 Exposure Time
	int shutterSpeed = fimc_v4l2_g_ctrl(m_cam_fd,
			V4L2_CID_CAMERA_GET_SHT_TIME);
	/* TBD - front camera needs to be fixed to support this g_ctrl,
	 it current returns a negative err value, so avoid putting
	 odd value into exif for now */
	if (shutterSpeed < 0) {
		LOGE("%s: error %d getting shutterSpeed, camera_id = %d, using 100",
				__func__, shutterSpeed, m_camera_id);
		shutterSpeed = 100;
	}
	mExifInfo.exposure_time.num = 1;
	mExifInfo.exposure_time.den = 1000.0 / shutterSpeed; /* ms -> sec */

	//3 ISO Speed Rating
	int iso = fimc_v4l2_g_ctrl(m_cam_fd, V4L2_CID_CAMERA_GET_ISO);
	/* TBD - front camera needs to be fixed to support this g_ctrl,
	 it current returns a negative err value, so avoid putting
	 odd value into exif for now */
	if (iso < 0) {
		LOGE("%s: error %d getting iso, camera_id = %d, using 100",
				__func__, iso, m_camera_id);
		iso = ISO_100;
	}
	switch(iso) {
		case ISO_50:
		mExifInfo.iso_speed_rating = 50;
		break;
		case ISO_100:
		mExifInfo.iso_speed_rating = 100;
		break;
		case ISO_200:
		mExifInfo.iso_speed_rating = 200;
		break;
		case ISO_400:
		mExifInfo.iso_speed_rating = 400;
		break;
		case ISO_800:
		mExifInfo.iso_speed_rating = 800;
		break;
		case ISO_1600:
		mExifInfo.iso_speed_rating = 1600;
		break;
		default:
		mExifInfo.iso_speed_rating = 100;
		break;
	}

	uint32_t av, tv, bv, sv, ev;
	av = APEX_FNUM_TO_APERTURE((double)mExifInfo.fnumber.num / mExifInfo.fnumber.den);
	tv = APEX_EXPOSURE_TO_SHUTTER((double)mExifInfo.exposure_time.num / mExifInfo.exposure_time.den);
	sv = APEX_ISO_TO_FILMSENSITIVITY(mExifInfo.iso_speed_rating);
	bv = av + tv - sv;
	ev = av + tv;
	LOGD("Shutter speed=%d us, iso=%d\n", shutterSpeed, mExifInfo.iso_speed_rating);
	LOGD("AV=%d, TV=%d, SV=%d\n", av, tv, sv);

	//3 Shutter Speed
	mExifInfo.shutter_speed.num = tv*EXIF_DEF_APEX_DEN;
	mExifInfo.shutter_speed.den = EXIF_DEF_APEX_DEN;
	//3 Brightness
	mExifInfo.brightness.num = bv*EXIF_DEF_APEX_DEN;
	mExifInfo.brightness.den = EXIF_DEF_APEX_DEN;
	//3 Exposure Bias
	if (m_params->scene_mode == SCENE_MODE_BEACH_SNOW) {
		mExifInfo.exposure_bias.num = EXIF_DEF_APEX_DEN;
		mExifInfo.exposure_bias.den = EXIF_DEF_APEX_DEN;
	} else {
		mExifInfo.exposure_bias.num = 0;
		mExifInfo.exposure_bias.den = 0;
	}
	//3 Metering Mode
	switch (m_params->metering) {
		case METERING_SPOT:
		mExifInfo.metering_mode = EXIF_METERING_SPOT;
		break;
		case METERING_MATRIX:
		mExifInfo.metering_mode = EXIF_METERING_AVERAGE;
		break;
		case METERING_CENTER:
		mExifInfo.metering_mode = EXIF_METERING_CENTER;
		break;
		default :
		mExifInfo.metering_mode = EXIF_METERING_AVERAGE;
		break;
	}

	//3 Flash
	int flash = fimc_v4l2_g_ctrl(m_cam_fd, V4L2_CID_CAMERA_GET_FLASH_ONOFF);
	if (flash < 0)
	mExifInfo.flash = EXIF_DEF_FLASH;
	else
	mExifInfo.flash = flash;

	//3 White Balance
	if (m_params->white_balance == WHITE_BALANCE_AUTO)
	mExifInfo.white_balance = EXIF_WB_AUTO;
	else
	mExifInfo.white_balance = EXIF_WB_MANUAL;
	//3 Scene Capture Type
	switch (m_params->scene_mode) {
		case SCENE_MODE_PORTRAIT:
		mExifInfo.scene_capture_type = EXIF_SCENE_PORTRAIT;
		break;
		case SCENE_MODE_LANDSCAPE:
		mExifInfo.scene_capture_type = EXIF_SCENE_LANDSCAPE;
		break;
		case SCENE_MODE_NIGHTSHOT:
		mExifInfo.scene_capture_type = EXIF_SCENE_NIGHT;
		break;
		default:
		mExifInfo.scene_capture_type = EXIF_SCENE_STANDARD;
		break;
	}

	//2 0th IFD GPS Info Tags
	if (m_gps_latitude != 0 && m_gps_longitude != 0) {
		if (m_gps_latitude > 0)
		strcpy((char *)mExifInfo.gps_latitude_ref, "N");
		else
		strcpy((char *)mExifInfo.gps_latitude_ref, "S");

		if (m_gps_longitude > 0)
		strcpy((char *)mExifInfo.gps_longitude_ref, "E");
		else
		strcpy((char *)mExifInfo.gps_longitude_ref, "W");

		if (m_gps_altitude > 0)
		mExifInfo.gps_altitude_ref = 0;
		else
		mExifInfo.gps_altitude_ref = 1;

		double latitude = fabs(m_gps_latitude / 10000.0);
		double longitude = fabs(m_gps_longitude / 10000.0);
		double altitude = fabs(m_gps_altitude / 100.0);

		mExifInfo.gps_latitude[0].num = (uint32_t)latitude;
		mExifInfo.gps_latitude[0].den = 1;
		mExifInfo.gps_latitude[1].num = (uint32_t)((latitude - mExifInfo.gps_latitude[0].num) * 60);
		mExifInfo.gps_latitude[1].den = 1;
		mExifInfo.gps_latitude[2].num = (uint32_t)((((latitude - mExifInfo.gps_latitude[0].num) * 60)
						- mExifInfo.gps_latitude[1].num) * 60);
		mExifInfo.gps_latitude[2].den = 1;

		mExifInfo.gps_longitude[0].num = (uint32_t)longitude;
		mExifInfo.gps_longitude[0].den = 1;
		mExifInfo.gps_longitude[1].num = (uint32_t)((longitude - mExifInfo.gps_longitude[0].num) * 60);
		mExifInfo.gps_longitude[1].den = 1;
		mExifInfo.gps_longitude[2].num = (uint32_t)((((longitude - mExifInfo.gps_longitude[0].num) * 60)
						- mExifInfo.gps_longitude[1].num) * 60);
		mExifInfo.gps_longitude[2].den = 1;

		mExifInfo.gps_altitude.num = (uint32_t)altitude;
		mExifInfo.gps_altitude.den = 1;

		struct tm tm_data;
		gmtime_r(&m_gps_timestamp, &tm_data);
		mExifInfo.gps_timestamp[0].num = tm_data.tm_hour;
		mExifInfo.gps_timestamp[0].den = 1;
		mExifInfo.gps_timestamp[1].num = tm_data.tm_min;
		mExifInfo.gps_timestamp[1].den = 1;
		mExifInfo.gps_timestamp[2].num = tm_data.tm_sec;
		mExifInfo.gps_timestamp[2].den = 1;
		snprintf((char*)mExifInfo.gps_datestamp, sizeof(mExifInfo.gps_datestamp),
				"%04d:%02d:%02d", tm_data.tm_year, tm_data.tm_mon, tm_data.tm_mday);

		mExifInfo.enableGps = true;
	} else {
		mExifInfo.enableGps = false;
	}

	//2 1th IFD TIFF Tags
	mExifInfo.widthThumb = m_jpeg_thumbnail_width;
	mExifInfo.heightThumb = m_jpeg_thumbnail_height;
}

unsigned int getPhyAddrY(int index)
{
	unsigned int addr_y;

	addr_y = fimc_v4l2_s_ctrl(m_cam_fd, V4L2_CID_PADDR_Y, index);
	CHECK((int)addr_y);
	return addr_y;
}

void getPostViewConfig(int *width, int *height, int *size)
{
	if (m_preview_width == 1024) {
		*width = BACK_CAMERA_POSTVIEW_WIDE_WIDTH;
		*height = BACK_CAMERA_POSTVIEW_HEIGHT;
		*size = BACK_CAMERA_POSTVIEW_WIDE_WIDTH * BACK_CAMERA_POSTVIEW_HEIGHT * BACK_CAMERA_POSTVIEW_BPP / 8;
	} else {
		*width = BACK_CAMERA_POSTVIEW_WIDTH;
		*height = BACK_CAMERA_POSTVIEW_HEIGHT;
		*size = BACK_CAMERA_POSTVIEW_WIDTH * BACK_CAMERA_POSTVIEW_HEIGHT * BACK_CAMERA_POSTVIEW_BPP / 8;
	}
	LOGI("[5B] m_preview_width : %d, mPostViewWidth = %d mPostViewHeight = %d mPostViewSize = %d",
			m_preview_width, *width, *height, *size);
}

void getThumbnailConfig(int *width, int *height, int *size)
{
	if (m_camera_id == CAMERA_ID_BACK) {
		*width = BACK_CAMERA_THUMBNAIL_WIDTH;
		*height = BACK_CAMERA_THUMBNAIL_HEIGHT;
		*size = BACK_CAMERA_THUMBNAIL_WIDTH * BACK_CAMERA_THUMBNAIL_HEIGHT
		* BACK_CAMERA_THUMBNAIL_BPP / 8;
	} else {
		*width = FRONT_CAMERA_THUMBNAIL_WIDTH;
		*height = FRONT_CAMERA_THUMBNAIL_HEIGHT;
		*size = FRONT_CAMERA_THUMBNAIL_WIDTH * FRONT_CAMERA_THUMBNAIL_HEIGHT
		* FRONT_CAMERA_THUMBNAIL_BPP / 8;
	}
}

int getSnapshotSize(int *width, int *height, int *frame_size)
{
	*width = m_snapshot_width;
	*height = m_snapshot_height;

	int frame = 0;

	frame = m_frameSize(m_snapshot_v4lformat, m_snapshot_width, m_snapshot_height);

	// set it big.
	if (frame == 0)
	frame = m_snapshot_width * m_snapshot_height * BPP;

	*frame_size = frame;

	return 0;
}

void yuyv422toABGRY(unsigned char *src, int width, int height)
{

	int frameSize =width*height*2;

	int i;

	if((!rgb || !ybuf)) {
		return;
	}
	int *lrgb = NULL;
	int *lybuf = NULL;

	lrgb = &rgb[0];
	lybuf = &ybuf[0];

	if(yuv_tbl_ready==0) {
		for(i=0; i<256; i++) {
			y1192_tbl[i] = 1192*(i-16);
			if(y1192_tbl[i]<0) {
				y1192_tbl[i]=0;
			}

			v1634_tbl[i] = 1634*(i-128);
			v833_tbl[i] = 833*(i-128);
			u400_tbl[i] = 400*(i-128);
			u2066_tbl[i] = 2066*(i-128);
		}
		yuv_tbl_ready=1;
	}

	for(i=0; i<frameSize; i+=4) {
		unsigned char y1, y2, u, v;
		y1 = src[i];
		u = src[i+1];
		y2 = src[i+2];
		v = src[i+3];

		int y1192_1=y1192_tbl[y1];
		int r1 = (y1192_1 + v1634_tbl[v])>>10;
		int g1 = (y1192_1 - v833_tbl[v] - u400_tbl[u])>>10;
		int b1 = (y1192_1 + u2066_tbl[u])>>10;

		int y1192_2=y1192_tbl[y2];
		int r2 = (y1192_2 + v1634_tbl[v])>>10;
		int g2 = (y1192_2 - v833_tbl[v] - u400_tbl[u])>>10;
		int b2 = (y1192_2 + u2066_tbl[u])>>10;

		r1 = r1>255 ? 255 : r1<0 ? 0 : r1;
		g1 = g1>255 ? 255 : g1<0 ? 0 : g1;
		b1 = b1>255 ? 255 : b1<0 ? 0 : b1;
		r2 = r2>255 ? 255 : r2<0 ? 0 : r2;
		g2 = g2>255 ? 255 : g2<0 ? 0 : g2;
		b2 = b2>255 ? 255 : b2<0 ? 0 : b2;

		*lrgb++ = 0xff000000 | b1<<16 | g1<<8 | r1;
		*lrgb++ = 0xff000000 | b2<<16 | g2<<8 | r2;

		if(lybuf!=NULL) {
			*lybuf++ = y1;
			*lybuf++ = y2;
		}
	}

}

static int save_yuv(struct fimc_buffer *m_buffers_c, int width, int height, int depth, int index, int frame_count)
{
	FILE *yuv_fp = NULL, *rgb_fp = NULL, *ybuf_fp = NULL;
	char filename[100], *buffer = NULL;
	int bufferLength = sizeof(int) * (width * height);

	/* file create/open, note to "wb" */
	yuv_fp = fopen("/sdcard/camtest/main.yuv", "wb");
	if (yuv_fp == NULL) {
		LOGE("Save YUV] file open error");
		return -1;
	}

	rgb_fp = fopen("/sdcard/camtest/rgb.byte", "wb");
	if (rgb_fp == NULL) {
		LOGE("Save rgb] file open error");
		return -1;
	}

	ybuf_fp = fopen("/sdcard/camtest/ybuf.byte", "wb");
	if (rgb_fp == NULL) {
		LOGE("Save ybuf] file open error");
		return -1;
	}

	buffer = (char *) malloc(m_buffers_c[index].length);
	if (buffer == NULL) {
		LOGE("Save YUV] buffer alloc failed");
		if (yuv_fp) fclose(yuv_fp);
		return -1;
	}

	int *rgbBuffer = NULL, *yBuffer = NULL;

	rgbBuffer = (int *) malloc(bufferLength);
	if (rgbBuffer == NULL) {
		LOGE("Save rgb buffer alloc failed");
		if (yuv_fp) fclose(yuv_fp);
		return -1;
	}
	yBuffer = (int *) malloc(bufferLength);
	if (yBuffer == NULL) {
		LOGE("Save ybuf buffer alloc failed");
		if (yuv_fp) fclose(yuv_fp);
		return -1;
	}

	memcpy(buffer, m_buffers_c[index].start, m_buffers_c[index].length);
	memcpy(rgbBuffer, rgb, bufferLength);
	memcpy(yBuffer, ybuf, bufferLength);

	fflush(stdout);

	fwrite(buffer, 1, m_buffers_c[index].length, yuv_fp);

	fflush(yuv_fp);

	fwrite(rgbBuffer, sizeof(int), (width * height), rgb_fp);

	fflush(rgb_fp);

	fwrite(yBuffer, sizeof(int), (width * height), ybuf_fp);

	fflush(ybuf_fp);

	if (yuv_fp)
	fclose(yuv_fp);
	if (yuv_fp)
	fclose(rgb_fp);
	if (yuv_fp)
	fclose(ybuf_fp);
	if (buffer)
	free(buffer);
	if (buffer)
	free(rgbBuffer);
	if (buffer)
	free(yBuffer);

	return 0;
}

int getSnapshotAndJpeg()
{
	LOGI("%s :", __func__);

	int index;
	//unsigned int addr;
	unsigned char *addr;
	int ret = 0;

//    LOG_TIME_DEFINE(0)
//    LOG_TIME_DEFINE(1)
//    LOG_TIME_DEFINE(2)
//    LOG_TIME_DEFINE(3)
//    LOG_TIME_DEFINE(4)
//    LOG_TIME_DEFINE(5)

	//fimc_v4l2_streamoff(m_cam_fd); [zzangdol] remove - it is separate in HWInterface with camera_id

	if (m_cam_fd <= 0) {
		LOGE("ERR(%s):Camera was closed\n", __func__);
		return -1;
	}

//    if (m_flag_camera_start > 0) {
//        LOG_TIME_START(0)
//        stopPreview();
//        LOG_TIME_END(0)
//    }

	memset(&m_events_c, 0, sizeof(m_events_c));
	m_events_c.fd = m_cam_fd;
	m_events_c.events = POLLIN | POLLERR;

	if (m_snapshot_v4lformat == V4L2_PIX_FMT_YUV420)
	LOGI("SnapshotFormat:V4L2_PIX_FMT_YUV420");
	else if (m_snapshot_v4lformat == V4L2_PIX_FMT_NV12)
	LOGI("SnapshotFormat:V4L2_PIX_FMT_NV12");
	else if (m_snapshot_v4lformat == V4L2_PIX_FMT_NV12T)
	LOGI("SnapshotFormat:V4L2_PIX_FMT_NV12T");
	else if (m_snapshot_v4lformat == V4L2_PIX_FMT_NV21)
	LOGI("SnapshotFormat:V4L2_PIX_FMT_NV21");
	else if (m_snapshot_v4lformat == V4L2_PIX_FMT_YUV422P)
	LOGI("SnapshotFormat:V4L2_PIX_FMT_YUV422P");
	else if (m_snapshot_v4lformat == V4L2_PIX_FMT_YUYV)
	LOGI("SnapshotFormat:V4L2_PIX_FMT_YUYV");
	else if (m_snapshot_v4lformat == V4L2_PIX_FMT_UYVY)
	LOGI("SnapshotFormat:V4L2_PIX_FMT_UYVY");
	else if (m_snapshot_v4lformat == V4L2_PIX_FMT_RGB565)
	LOGI("SnapshotFormat:V4L2_PIX_FMT_RGB565");
	else
	LOGI("SnapshotFormat:UnknownFormat");

	LOG_TIME_START(1)// prepare
	int nframe = 1;

	ret = fimc_v4l2_enum_fmt(m_cam_fd,m_snapshot_v4lformat);
	CHECK(ret);

	if(m_camera_id == CAMERA_ID_BACK)
	{
		ret = fimc_v4l2_s_fmt_cap(m_cam_fd, m_snapshot_width, m_snapshot_height, V4L2_PIX_FMT_JPEG);
	}
	else {
		ret = fimc_v4l2_s_fmt_cap(m_cam_fd, m_snapshot_width, m_snapshot_height, m_snapshot_v4lformat);
	}
	CHECK(ret);
	init_yuv_buffers(m_buffers_c, m_snapshot_width, m_snapshot_height, m_snapshot_v4lformat);

	rgb = (int *)malloc(sizeof(int) * (m_snapshot_width*m_snapshot_height));
	ybuf = (int *)malloc(sizeof(int) * (m_snapshot_width*m_snapshot_height));

	ret = fimc_v4l2_reqbufs(m_cam_fd, V4L2_BUF_TYPE_VIDEO_CAPTURE, nframe);
	CHECK(ret);
	ret = fimc_v4l2_querybuf(m_cam_fd, m_buffers_c, V4L2_BUF_TYPE_VIDEO_CAPTURE, nframe);
	CHECK(ret);

	ret = fimc_v4l2_qbuf(m_cam_fd, 0);
	CHECK(ret);

	ret = fimc_v4l2_streamon(m_cam_fd);
	CHECK(ret);
//    LOG_TIME_END(1)

//    LOGI("%s",(unsigned char*)m_buffers_c[index].start);

	LOG_TIME_START(2)// capture
	fimc_poll(&m_events_c);
	index = fimc_v4l2_dqbuf(m_cam_fd);
	fimc_v4l2_s_ctrl(m_cam_fd, V4L2_CID_STREAM_PAUSE, 0);
	LOGI("\nsnapshot dequeued buffer = %d snapshot_width = %d snapshot_height = %d\n\n",
			index, m_snapshot_width, m_snapshot_height);

	yuyv422toABGRY((unsigned char*)m_buffers_c[index].start, m_snapshot_width, m_snapshot_height);
#ifdef DUMP_YUV
//    save_yuv(m_buffers_c, m_snapshot_width, m_snapshot_height, 16, index, 0);
#endif
//    LOG_TIME_END(2)

//    memcpy(yuv_buf, (unsigned char*)m_buffers_c[index].start, m_snapshot_width * m_snapshot_height * 2);
//    LOG_TIME_START(5) // post
	fimc_v4l2_streamoff(m_cam_fd);

//    LOGI("%s",yuv_buf);
#ifdef DUMP_YUV
	close_buffers(m_buffers_c);
#endif
//    LOG_TIME_END(5)

//    LOG_CAMERA("getSnapshotAndJpeg intervals : stopPreview(%lu), prepare(%lu),"
//                " capture(%lu), memcpy(%lu), yuv2Jpeg(%lu), post(%lu)  us",
//                    LOG_TIME(0), LOG_TIME(1), LOG_TIME(2), LOG_TIME(3), LOG_TIME(4), LOG_TIME(5));
	/* JPEG encoding */
	/*    JpegEncoder jpgEnc;
	 int inFormat = JPG_MODESEL_YCBCR;
	 int outFormat = JPG_422;

	 switch (m_snapshot_v4lformat) {
	 case V4L2_PIX_FMT_NV12:
	 case V4L2_PIX_FMT_NV21:
	 case V4L2_PIX_FMT_NV12T:
	 case V4L2_PIX_FMT_YUV420:
	 outFormat = JPG_420;
	 break;
	 case V4L2_PIX_FMT_YUYV:
	 case V4L2_PIX_FMT_UYVY:
	 case V4L2_PIX_FMT_YUV422P:
	 default:
	 outFormat = JPG_422;
	 break;
	 }

	 if (jpgEnc.setConfig(JPEG_SET_ENCODE_IN_FORMAT, inFormat) != JPG_SUCCESS)
	 LOGE("[JPEG_SET_ENCODE_IN_FORMAT] Error\n");

	 if (jpgEnc.setConfig(JPEG_SET_SAMPING_MODE, outFormat) != JPG_SUCCESS)
	 LOGE("[JPEG_SET_SAMPING_MODE] Error\n");

	 image_quality_type_t jpegQuality;
	 if (m_jpeg_quality >= 90)
	 jpegQuality = JPG_QUALITY_LEVEL_1;
	 else if (m_jpeg_quality >= 80)
	 jpegQuality = JPG_QUALITY_LEVEL_2;
	 else if (m_jpeg_quality >= 70)
	 jpegQuality = JPG_QUALITY_LEVEL_3;
	 else
	 jpegQuality = JPG_QUALITY_LEVEL_4;

	 if (jpgEnc.setConfig(JPEG_SET_ENCODE_QUALITY, jpegQuality) != JPG_SUCCESS)
	 LOGE("[JPEG_SET_ENCODE_QUALITY] Error\n");
	 if (jpgEnc.setConfig(JPEG_SET_ENCODE_WIDTH, m_snapshot_width) != JPG_SUCCESS)
	 LOGE("[JPEG_SET_ENCODE_WIDTH] Error\n");

	 if (jpgEnc.setConfig(JPEG_SET_ENCODE_HEIGHT, m_snapshot_height) != JPG_SUCCESS)
	 LOGE("[JPEG_SET_ENCODE_HEIGHT] Error\n");

	 unsigned int snapshot_size = m_snapshot_width * m_snapshot_height * 2;
	 unsigned char *pInBuf = (unsigned char *)jpgEnc.getInBuf(snapshot_size);

	 if (pInBuf == NULL) {
	 LOGE("JPEG input buffer is NULL!!\n");
	 return -1;
	 }
	 memcpy(pInBuf, yuv_buf, snapshot_size);

	 setExifChangedAttribute();
	 jpgEnc.encode(output_size, &mExifInfo);

	 uint64_t outbuf_size;
	 unsigned char *pOutBuf = (unsigned char *)jpgEnc.getOutBuf(&outbuf_size);

	 if (pOutBuf == NULL) {
	 LOGE("JPEG output buffer is NULL!!\n");
	 return -1;
	 }

	 memcpy(jpeg_buf, pOutBuf, outbuf_size);
	 */
	return 0;
}

int setSnapshotCmd(void)
{
	LOGI("%s :", __func__);
	LOGI("setSnapshotCmd");

	int ret = 0;

	LOG_TIME_DEFINE(0)
	LOG_TIME_DEFINE(1)

	if (m_cam_fd <= 0) {
		LOGE("ERR(%s):Camera was closed\n", __func__);
		return 0;
	}

//    if (m_flag_camera_start > 0) {
//        LOG_TIME_START(0)
//        stopPreview();
//        LOG_TIME_END(0)
//    }

	memset(&m_events_c, 0, sizeof(m_events_c));
	m_events_c.fd = m_cam_fd;
	m_events_c.events = POLLIN | POLLERR;

	LOG_TIME_START(1)// prepare
	int nframe = 1;

	ret = fimc_v4l2_enum_fmt(m_cam_fd,m_snapshot_v4lformat);
	CHECK(ret);
	ret = fimc_v4l2_s_fmt_cap(m_cam_fd, m_snapshot_width, m_snapshot_height, V4L2_PIX_FMT_JPEG);
	CHECK(ret);
	init_yuv_buffers(m_buffers_c, m_snapshot_width, m_snapshot_height, m_snapshot_v4lformat);

	ret = fimc_v4l2_reqbufs(m_cam_fd, V4L2_BUF_TYPE_VIDEO_CAPTURE, nframe);
	CHECK(ret);
	ret = fimc_v4l2_querybuf(m_cam_fd, m_buffers_c, V4L2_BUF_TYPE_VIDEO_CAPTURE, nframe);
	CHECK(ret);

	ret = fimc_v4l2_qbuf(m_cam_fd, 0);
	CHECK(ret);

	ret = fimc_v4l2_streamon(m_cam_fd);
	CHECK(ret);
//    LOG_TIME_END(1)

	return 0;
}

unsigned char* getJpeg(int *jpeg_size, unsigned int *phyaddr)
{
	LOGI("%s :", __func__);

	int index, ret = 0;
	unsigned char *addr;

	LOG_TIME_DEFINE(2)

#ifdef SWP1_CAMERA_ADD_ADVANCED_FUNCTION
	// capture
	ret = fimc_poll(&m_events_c);
	CHECK_PTR(ret);
	index = fimc_v4l2_dqbuf(m_cam_fd);
	if (!(0 <= index && index < MAX_BUFFERS)) {
		LOGE("ERR(%s):wrong index = %d\n", __func__, index);
		return NULL;
	}
#endif

	*jpeg_size = fimc_v4l2_g_ctrl(m_cam_fd, V4L2_CID_CAM_JPEG_MAIN_SIZE);
	CHECK_PTR(*jpeg_size);

	int main_offset = fimc_v4l2_g_ctrl(m_cam_fd, V4L2_CID_CAM_JPEG_MAIN_OFFSET);
	CHECK_PTR(main_offset);
	m_postview_offset = fimc_v4l2_g_ctrl(m_cam_fd, V4L2_CID_CAM_JPEG_POSTVIEW_OFFSET);
	CHECK_PTR(m_postview_offset);

	ret = fimc_v4l2_s_ctrl(m_cam_fd, V4L2_CID_STREAM_PAUSE, 0);
	CHECK_PTR(ret);
	LOGI("\nsnapshot dqueued buffer = %d snapshot_width = %d snapshot_height = %d, size = %d\n\n",
			index, m_snapshot_width, m_snapshot_height, *jpeg_size);

	addr = (unsigned char*)(m_buffers_c[index].start) + main_offset;
	*phyaddr = getPhyAddrY(index) + m_postview_offset;

	LOG_TIME_START(2) // post
	ret = fimc_v4l2_streamoff(m_cam_fd);
	CHECK_PTR(ret);
//    LOG_TIME_END(2)

	return addr;
}

bool CheckVideoStartMarker(unsigned char *pBuf)
{
	if (!pBuf) {
		LOGE("CheckVideoStartMarker() => pBuf is NULL\n");
		return false;
	}

	if (HIBYTE(VIDEO_COMMENT_MARKER_H) == * pBuf && LOBYTE(VIDEO_COMMENT_MARKER_H) == *(pBuf + 1) &&
			HIBYTE(VIDEO_COMMENT_MARKER_L) == *(pBuf + 2) && LOBYTE(VIDEO_COMMENT_MARKER_L) == *(pBuf + 3))
	return true;

	return false;
}

bool CheckEOIMarker(unsigned char *pBuf)
{
	if (!pBuf) {
		LOGE("CheckEOIMarker() => pBuf is NULL\n");
		return false;
	}

	// EOI marker [FF D9]
	if (HIBYTE(JPEG_EOI_MARKER) == *pBuf && LOBYTE(JPEG_EOI_MARKER) == *(pBuf + 1))
	return true;

	return false;
}

bool FindEOIMarkerInJPEG(unsigned char *pBuf, int dwBufSize, int *pnJPEGsize)
{
	if (NULL == pBuf || 0 >= dwBufSize) {
		LOGE("FindEOIMarkerInJPEG() => There is no contents.");
		return false;
	}

	unsigned char *pBufEnd = pBuf + dwBufSize;

	while (pBuf < pBufEnd) {
		if (CheckEOIMarker(pBuf++))
		return true;

		(*pnJPEGsize)++;
	}

	return false;
}

bool SplitFrame(unsigned char *pFrame, int dwSize,
		int dwJPEGLineLength, int dwVideoLineLength, int dwVideoHeight,
		void *pJPEG, int *pdwJPEGSize,
		void *pVideo, int *pdwVideoSize)
{
	LOGI("===========SplitFrame Start==============");

	if (NULL == pFrame || 0 >= dwSize) {
		LOGE("There is no contents.\r\n");
		return false;
	}

	if (0 == dwJPEGLineLength || 0 == dwVideoLineLength) {
		LOGE("There in no input information for decoding interleaved jpeg");
		return false;
	}

	unsigned char *pSrc = pFrame;
	unsigned char *pSrcEnd = pFrame + dwSize;

	unsigned char *pJ = (unsigned char *)pJPEG;
	int dwJSize = 0;
	unsigned char *pV = (unsigned char *)pVideo;
	int dwVSize = 0;

	bool bRet = false;
	bool isFinishJpeg = false;

	while (pSrc < pSrcEnd) {
		// Check video start marker
		if (CheckVideoStartMarker(pSrc)) {
			int copyLength;

			if (pSrc + dwVideoLineLength <= pSrcEnd)
			copyLength = dwVideoLineLength;
			else
			copyLength = pSrcEnd - pSrc - VIDEO_COMMENT_MARKER_LENGTH;

			// Copy video data
			if (pV) {
				memcpy(pV, pSrc + VIDEO_COMMENT_MARKER_LENGTH, copyLength);
				pV += copyLength;
				dwVSize += copyLength;
			}

			pSrc += copyLength + VIDEO_COMMENT_MARKER_LENGTH;
		} else {
			// Copy pure JPEG data
			int size = 0;
			int dwCopyBufLen = dwJPEGLineLength <= pSrcEnd-pSrc ? dwJPEGLineLength : pSrcEnd - pSrc;

			if (FindEOIMarkerInJPEG((unsigned char *)pSrc, dwCopyBufLen, &size)) {
				isFinishJpeg = true;
				size += 2;  // to count EOF marker size
			} else {
				if ((dwCopyBufLen == 1) && (pJPEG < pJ)) {
					unsigned char checkBuf[2] = {*(pJ - 1), *pSrc};

					if (CheckEOIMarker(checkBuf))
					isFinishJpeg = true;
				}
				size = dwCopyBufLen;
			}

			memcpy(pJ, pSrc, size);

			dwJSize += size;

			pJ += dwCopyBufLen;
			pSrc += dwCopyBufLen;
		}
		if (isFinishJpeg)
		break;
	}

	if (isFinishJpeg) {
		bRet = true;
		if(pdwJPEGSize)
		*pdwJPEGSize = dwJSize;
		if(pdwVideoSize)
		*pdwVideoSize = dwVSize;
	} else {
		LOGE("DecodeInterleaveJPEG_WithOutDT() => Can not find EOI");
		bRet = false;
		if(pdwJPEGSize)
		*pdwJPEGSize = 0;
		if(pdwVideoSize)
		*pdwVideoSize = 0;
	}
	LOGI("===========SplitFrame end==============");

	return bRet;
}

int decodeInterleaveData(unsigned char *pInterleaveData,
		int interleaveDataSize,
		int yuvWidth,
		int yuvHeight,
		int *pJpegSize,
		void *pJpegData,
		void *pYuvData)
{
	if (pInterleaveData == NULL)
	return false;

	bool ret = true;
	unsigned int *interleave_ptr = (unsigned int *)pInterleaveData;
	unsigned char *jpeg_ptr = (unsigned char *)pJpegData;
	unsigned char *yuv_ptr = (unsigned char *)pYuvData;
	unsigned char *p;
	int jpeg_size = 0;
	int yuv_size = 0;

	int i = 0;

	LOGI("decodeInterleaveData Start~~~");
	while (i < interleaveDataSize) {
		if ((*interleave_ptr == 0xFFFFFFFF) || (*interleave_ptr == 0x02FFFFFF) ||
				(*interleave_ptr == 0xFF02FFFF)) {
			// Padding Data
//            LOGE("%d(%x) padding data\n", i, *interleave_ptr);
			interleave_ptr++;
			i += 4;
		}
		else if ((*interleave_ptr & 0xFFFF) == 0x05FF) {
			// Start-code of YUV Data
//            LOGE("%d(%x) yuv data\n", i, *interleave_ptr);
			p = (unsigned char *)interleave_ptr;
			p += 2;
			i += 2;

			// Extract YUV Data
			if (pYuvData != NULL) {
				memcpy(yuv_ptr, p, yuvWidth * 2);
				yuv_ptr += yuvWidth * 2;
				yuv_size += yuvWidth * 2;
			}
			p += yuvWidth * 2;
			i += yuvWidth * 2;

			// Check End-code of YUV Data
			if ((*p == 0xFF) && (*(p + 1) == 0x06)) {
				interleave_ptr = (unsigned int *)(p + 2);
				i += 2;
			} else {
				ret = false;
				break;
			}
		} else {
			// Extract JPEG Data
//            LOGE("%d(%x) jpg data, jpeg_size = %d bytes\n", i, *interleave_ptr, jpeg_size);
			if (pJpegData != NULL) {
				memcpy(jpeg_ptr, interleave_ptr, 4);
				jpeg_ptr += 4;
				jpeg_size += 4;
			}
			interleave_ptr++;
			i += 4;
		}
	}
	if (ret) {
		if (pJpegData != NULL) {
			// Remove Padding after EOI
			for (i = 0; i < 3; i++) {
				if (*(--jpeg_ptr) != 0xFF) {
					break;
				}
				jpeg_size--;
			}
			*pJpegSize = jpeg_size;

		}
		// Check YUV Data Size
		if (pYuvData != NULL) {
			if (yuv_size != (yuvWidth * yuvHeight * 2)) {
				ret = false;
			}
		}
	}
	LOGI("decodeInterleaveData End~~~");
	return ret;
}

bool scaleDownYuv422(char *srcBuf, uint32_t srcWidth, uint32_t srcHeight,
		char *dstBuf, uint32_t dstWidth, uint32_t dstHeight)
{
	int32_t step_x, step_y;
	int32_t iXsrc, iXdst;
	int32_t x, y, src_y_start_pos, dst_pos, src_pos;

	if (dstWidth % 2 != 0 || dstHeight % 2 != 0) {
		LOGE("scale_down_yuv422: invalid width, height for scaling");
		return false;
	}

	step_x = srcWidth / dstWidth;
	step_y = srcHeight / dstHeight;

	dst_pos = 0;
	for (uint32_t y = 0; y < dstHeight; y++) {
		src_y_start_pos = (y * step_y * (srcWidth * 2));

		for (uint32_t x = 0; x < dstWidth; x += 2) {
			src_pos = src_y_start_pos + (x * (step_x * 2));

			dstBuf[dst_pos++] = srcBuf[src_pos ];
			dstBuf[dst_pos++] = srcBuf[src_pos + 1];
			dstBuf[dst_pos++] = srcBuf[src_pos + 2];
			dstBuf[dst_pos++] = srcBuf[src_pos + 3];
		}
	}

	return true;
}

int getExif(unsigned char *pExifDst, unsigned char *pThumbSrc)
{
	JpegEncoder jpgEnc;
#if ADD_THUMB_IMG
	LOGI("%s : m_jpeg_thumbnail_width = %d, height = %d",
			__func__, m_jpeg_thumbnail_width, m_jpeg_thumbnail_height);
	if ((m_jpeg_thumbnail_width > 0) && (m_jpeg_thumbnail_height > 0)) {
		int inFormat = JPG_MODESEL_YCBCR;
		int outFormat = JPG_422;
		switch (m_snapshot_v4lformat) {
			case V4L2_PIX_FMT_NV12:
			case V4L2_PIX_FMT_NV21:
			case V4L2_PIX_FMT_NV12T:
			case V4L2_PIX_FMT_YUV420:
			outFormat = JPG_420;
			break;
			case V4L2_PIX_FMT_YUYV:
			case V4L2_PIX_FMT_UYVY:
			case V4L2_PIX_FMT_YUV422P:
			outFormat = JPG_422;
			break;
		}

		if (jpgEnc.setConfig(JPEG_SET_ENCODE_IN_FORMAT, inFormat) != JPG_SUCCESS)
		return -1;

		if (jpgEnc.setConfig(JPEG_SET_SAMPING_MODE, outFormat) != JPG_SUCCESS)
		return -1;

		if (jpgEnc.setConfig(JPEG_SET_ENCODE_QUALITY, JPG_QUALITY_LEVEL_2) != JPG_SUCCESS)
		return -1;

		int thumbWidth, thumbHeight, thumbSrcSize;
		getThumbnailConfig(&thumbWidth, &thumbHeight, &thumbSrcSize);
		if (jpgEnc.setConfig(JPEG_SET_ENCODE_WIDTH, thumbWidth) != JPG_SUCCESS)
		return -1;

		if (jpgEnc.setConfig(JPEG_SET_ENCODE_HEIGHT, thumbHeight) != JPG_SUCCESS)
		return -1;

		char *pInBuf = (char *)jpgEnc.getInBuf(thumbSrcSize);
		if (pInBuf == NULL)
		return -1;
		memcpy(pInBuf, pThumbSrc, thumbSrcSize);

		unsigned int thumbSize;

		jpgEnc.encode(&thumbSize, NULL);

		LOGI("%s : enableThumb set to true", __func__);
		mExifInfo.enableThumb = true;
	} else {
		LOGI("%s : enableThumb set to false", __func__);
		mExifInfo.enableThumb = false;
	}
#else
	mExifInfo.enableThumb = false;
#endif

	unsigned int exifSize;

	setExifChangedAttribute();

	LOGI("%s: calling jpgEnc.makeExif, mExifInfo.width set to %d, height to %d\n",
			__func__, mExifInfo.width, mExifInfo.height);

	jpgEnc.makeExif(pExifDst, &mExifInfo, &exifSize, true);

	return exifSize;
}

int pictureThread()
{
	LOGI("%s :", __func__);

	int jpeg_size = 0;
	int ret = NO_ERROR;
//    unsigned char *jpeg_data = NULL;
	int postview_offset = 0;
	unsigned char *postview_data = NULL;

//    unsigned char *addr = NULL;
	int mPostViewWidth, mPostViewHeight, mPostViewSize;
	int mThumbWidth, mThumbHeight, mThumbSize;
	int cap_width, cap_height, cap_frame_size;

	unsigned int output_size = 0;

	getPostViewConfig(&mPostViewWidth, &mPostViewHeight, &mPostViewSize);
	getThumbnailConfig(&mThumbWidth, &mThumbHeight, &mThumbSize);
	int postviewHeapSize = mPostViewSize;
	getSnapshotSize(&cap_width, &cap_height, &cap_frame_size);
	int mJpegHeapSize;
	if (m_camera_id == CAMERA_ID_BACK)
	mJpegHeapSize = cap_frame_size * 1;//getJpegRatio();
	else
	mJpegHeapSize = cap_frame_size;

//#ifdef DIRECT_DELIVERY_OF_POSTVIEW_DATA
//    sp<MemoryBase> buffer = new MemoryBase(mRawHeap, 0, mPostViewSize + 8);
//#else
////    LOGI("%d", sizeof(struct addrs_cap));
//    sp<MemoryBase> buffer = new MemoryBase(mRawHeap, 0, sizeof(struct addrs_cap));
//#endif
//
//    struct addrs_cap *addrs = (struct addrs_cap *)mRawHeap->base();
//
//#ifdef JPEG_FROM_SENSOR
//    addrs[0].width = mPostViewWidth;
//    addrs[0].height = mPostViewHeight;
//    LOGI("[5B] mPostViewWidth = %d mPostViewHeight = %d\n",mPostViewWidth,mPostViewHeight);
//#else
//    mParameters.getPictureSize((int*)&addrs[0].width, (int*)&addrs[0].height);
//#endif

//    LOGI("%d, %d, %d", mJpegHeapSize, mPostViewSize, mThumbSize);
//    sp<MemoryHeapBase> JpegHeap = new MemoryHeapBase(mJpegHeapSize);
//    sp<MemoryHeapBase> PostviewHeap = new MemoryHeapBase(mPostViewSize);
//    sp<MemoryHeapBase> ThumbnailHeap = new MemoryHeapBase(mThumbSize);

	if (1) {  //mMsgEnabled & CAMERA_MSG_RAW_IMAGE) {

		int picture_size, picture_width, picture_height;
		getSnapshotSize(&picture_width, &picture_height, &picture_size);
		int picture_format = getSnapshotPixelFormat();

		unsigned int phyAddr;
#ifdef JPEG_FROM_SENSOR
		// Modified the shutter sound timing for Jpeg capture
//        if (m_camera_id == CAMERA_ID_BACK)
//            setSnapshotCmd();
#ifdef SWP1_CAMERA_ADD_ADVANCED_FUNCTION
		if (mMsgEnabled & CAMERA_MSG_SHUTTER) {
			mNotifyCb(CAMERA_MSG_SHUTTER, 0, 0, mCallbackCookie);
		}
#endif

//        if (m_camera_id == CAMERA_ID_BACK){
//            jpeg_data = getJpeg(&jpeg_size, &phyAddr);
//            if (jpeg_data == NULL) {
//                LOGE("ERR(%s):Fail on SecCamera->getJpeg()", __func__);
//                ret = UNKNOWN_ERROR;
//            }
//        } else {
		if (getSnapshotAndJpeg() < 0) {
			return UNKNOWN_ERROR;
		}
		LOGI("snapshotandjpeg done\n");
//       }
#else
		phyAddr = getSnapshotAndJpeg();
		if (phyAddr < 0) {
			LOGI("Error here in pictureThread()")
			return UNKNOWN_ERROR;
		}

		jpeg_data = yuv2Jpeg((unsigned char*)phyAddr, 0, &jpeg_size,
				picture_width, picture_height, picture_format);
#endif
//
//#ifndef DIRECT_DELIVERY_OF_POSTVIEW_DATA
//
//        addrs[0].addr_y = phyAddr;
//#endif
//
// //       LOG_TIME_END(1)
//        LOG_CAMERA("getSnapshotAndJpeg interval: %lu us", LOG_TIME(1));
	}
//
//    /* the capture is done at this point so we can allow sensor commands
//     * again, we still need to do jpeg and thumbnail processing, but the
//     * sensor is available for something else
//     */
////    mStateLock.lock();
////    mCaptureInProgress = false;
////    mStateLock.unlock();
//
//    int JpegImageSize, JpegExifSize;
//    bool isLSISensor = false;
//
//    if (m_camera_id == CAMERA_ID_BACK) {
//        isLSISensor = !strncmp((const char*)mCameraSensorName, "S5K4ECGX", 8);
//        if(isLSISensor) {
//            LOGI("== Camera Sensor Detect %s - Samsung LSI SOC 5M ==\n", mCameraSensorName);
//            // LSI 5M SOC
//            if (!SplitFrame(jpeg_data, getInterleaveDataSize(),
//                       getJpegLineLength(),
//                       mPostViewWidth * 2, mPostViewWidth,
//                       JpegHeap->base(), &JpegImageSize,
//                       PostviewHeap->base(), &mPostViewSize))
//                return UNKNOWN_ERROR;
//        } else {
//            LOGI("== Camera Sensor Detect %s Sony SOC 5M ==\n", mCameraSensorName);
//            decodeInterleaveData(jpeg_data, getInterleaveDataSize(), mPostViewWidth, mPostViewHeight,
//                                &JpegImageSize, JpegHeap->base(), PostviewHeap->base());
//
//        }
//    } else {
//        JpegImageSize = static_cast<int>(output_size);
//    }
//    scaleDownYuv422((char *)PostviewHeap->base(), mPostViewWidth, mPostViewHeight,
//                    (char *)ThumbnailHeap->base(), mThumbWidth, mThumbHeight);
//
//#ifdef POSTVIEW_CALLBACK
//    sp<MemoryBase> postview = new MemoryBase(PostviewHeap, 0, postviewHeapSize);
//#endif
//    memcpy(mRawHeap->base(),PostviewHeap->base(), postviewHeapSize);
//
//#if defined(BOARD_USES_OVERLAY)
//   /* Put postview image to Overlay */
//    unsigned int index = 0;
//    unsigned int offset = ((mPostViewWidth*mPostViewHeight*3/2) + 16) * index;
//    unsigned int overlay_header[4];
//
//    // Only show postview image if size is VGA since sensor cannot deliver
//    // any other sizes.
//    int previewWidth, previewHeight, previewSize;
//    mSecCamera->getPreviewSize(&previewWidth, &previewHeight, &previewSize);
//    if ((previewWidth != 640) || (previewHeight != 480))
//        goto PostviewOverlayEnd;
//
//    mOverlayBufferIdx ^= 1;
//    overlay_header[0]= mSecCamera->getPhyAddrY(index);
//    overlay_header[1]= overlay_header[0] + mPostViewWidth*mPostViewHeight;
//    overlay_header[2]= mOverlayBufferIdx;
//
//    YUY2toNV21(mRawHeap->base(), (void*)(static_cast<unsigned char *>(mPreviewHeap->base()) + offset),
//                mPostViewWidth, mPostViewHeight);
//
//    memcpy(static_cast<unsigned char*>(mPreviewHeap->base()) + offset + (mPostViewWidth*mPostViewHeight * 3 / 2),
//            overlay_header, 16);
//
//    ret = mOverlay->queueBuffer((void*)(static_cast<unsigned char *>(mPreviewHeap->base()) + offset +
//                                (mPostViewWidth*mPostViewHeight * 3 / 2)));
//
//    if (ret == ALL_BUFFERS_FLUSHED) {
//        goto PostviewOverlayEnd;
//    } else if (ret == -1) {
//        LOGE("ERR(%s):overlay queueBuffer fail", __func__);
//        goto PostviewOverlayEnd;
//    }
//
//    overlay_buffer_t overlay_buffer;
//    ret = mOverlay->dequeueBuffer(&overlay_buffer);
//
//    if (ret == ALL_BUFFERS_FLUSHED) {
//        goto PostviewOverlayEnd;
//    } else if (ret == -1) {
//        LOGE("ERR(%s):overlay dequeueBuffer fail", __func__);
//        goto PostviewOverlayEnd;
//    }
//
//PostviewOverlayEnd:
//#endif
//    if (mMsgEnabled & CAMERA_MSG_RAW_IMAGE) {
//        mDataCb(CAMERA_MSG_RAW_IMAGE, buffer, mCallbackCookie);
//    }
//#ifdef POSTVIEW_CALLBACK
//    if (mMsgEnabled & CAMERA_MSG_POSTVIEW_FRAME) {
//        int postviewHeapSize = mPostViewSize;
//        sp<MemoryHeapBase> mPostviewHeap = new MemoryHeapBase(postviewHeapSize);
//
//        postview_data = jpeg_data + postview_offset;
//        sp<MemoryBase> postview = new MemoryBase(mPostviewHeap, 0, postviewHeapSize);
//
//        if (postview_data != NULL)
//            memcpy(mPostviewHeap->base(), postview_data, postviewHeapSize);
//
//        mDataCb(CAMERA_MSG_POSTVIEW_FRAME, postview, mCallbackCookie);
//    }
//#endif
//    if (mMsgEnabled & CAMERA_MSG_COMPRESSED_IMAGE) {
//        sp<MemoryHeapBase> ExifHeap = new MemoryHeapBase(EXIF_FILE_SIZE + JPG_STREAM_BUF_SIZE);
//        JpegExifSize = getExif((unsigned char *)ExifHeap->base(),
//                (unsigned char *)ThumbnailHeap->base());
//
//        LOGI("JpegExifSize=%d", JpegExifSize);
//
//        if (JpegExifSize < 0) {
//            return UNKNOWN_ERROR;
//        }
//
//        unsigned char *ExifStart = (unsigned char *)JpegHeap->base() + 2;
//        unsigned char *ImageStart = ExifStart + JpegExifSize;
//
//        memmove(ImageStart, ExifStart, JpegImageSize - 2);
//        memcpy(ExifStart, ExifHeap->base(), JpegExifSize);
//        sp<MemoryBase> mem = new MemoryBase(JpegHeap, 0, JpegImageSize + JpegExifSize);
//
//        mDataCb(CAMERA_MSG_COMPRESSED_IMAGE, mem, mCallbackCookie);
//    }

//    LOG_TIME_END(0)
//    LOG_CAMERA("pictureThread interval: %lu us", LOG_TIME(0));

	LOGI("%s : pictureThread end", __func__);
	return ret;
}

JNIEXPORT jint JNICALL Java_com_example_droidlinuxcamara_MainActivity_getPicture(
		JNIEnv* env, jobject thiz) {
	LOGI("getPicture() called.");
	init();
	initCamera(1);
	m_flag_camera_start = 1;
	initDefaultParameters(1);
//	startPreview();
//	stopPreview();
	pictureThread();
	DeinitCamera();
}

void
Java_com_example_droidlinuxcamara_MainActivity_pixeltobmp( JNIEnv* env,jobject thiz,jobject bitmap) {

	LOGI("pixeltobmp");

	jboolean bo;

	AndroidBitmapInfo info;
	void* pixels;
	int ret;
	int i;
	int *colors;

	int width=0;
	int height=0;

	if ((ret = AndroidBitmap_getInfo(env, bitmap, &info)) < 0) {
		LOGE("AndroidBitmap_getInfo() failed ! error=%d", ret);
		return;
	}

	width = info.width;
	height = info.height;

	if(!rgb || !ybuf) return;

	if (info.format != ANDROID_BITMAP_FORMAT_RGBA_8888) {
		LOGE("Bitmap format is not RGBA_8888 !");
		return;
	}

	if ((ret = AndroidBitmap_lockPixels(env, bitmap, &pixels)) < 0) {
		LOGE("AndroidBitmap_lockPixels() failed ! error=%d", ret);
	}

	colors = (int*)pixels;
	int *lrgb =NULL;
	lrgb = &rgb[0];

	for(i=0; i<width*height; i++) {
		*colors++ = *lrgb++;
	}

	AndroidBitmap_unlockPixels(env, bitmap);

}

#ifdef __cplusplus
}
#endif

