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
#include <linux/videodev2.h>
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
#include <linux/videodev2.h>
#include <assert.h>


#define LOG_TAG "NativeLib"
#define LOGV(...) __android_log_print(ANDROID_LOG_VERBOSE, LOG_TAG, __VA_ARGS__)
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__)
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGW(...) __android_log_print(ANDROID_LOG_WARN, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

#define CLEAR(x) memset (&(x), 0, sizeof (x))

struct buffer {
	void * start;
	size_t length;
};


#ifdef __cplusplus
extern "C"
{
#endif

static int camera_device;


JNIEXPORT jint JNICALL Java_com_example_droidlinuxcamara_MainActivity_openTestFile(){
	LOGI("calling Open File");
	FILE * file;
	file = fopen("file.txt", "a+");
	LOGI("Calling fopen()");
	//fprintf(file, "%s", "This is a test file");
	LOGI("write");
	fclose(file);
	LOGI("Closed");
}

JNIEXPORT jint JNICALL Java_com_example_droidlinuxcamara_MainActivity_getPicture(JNIEnv* env, jobject thiz)
{
	unsigned int i;
	enum v4l2_buf_type type;
	FILE* file_fd = fopen("test-mmap.jpg", "w");

	camera_device = open("/dev/video1", O_RDWR|O_NONBLOCK, 0);
	LOGI("access camera id %d", camera_device);
	// get camera capabilities and initialize the camera
	struct v4l2_capability cap;
	ioctl(camera_device, VIDIOC_QUERYCAP, &cap);

	LOGI("capabilities 0x%x", cap.capabilities);

	if(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE){
		LOGI("Support video capture");
	}
	if(cap.capabilities & V4L2_CAP_VIDEO_OUTPUT){
		LOGI("Support video OUTPUT");
	}

	//set format
	struct v4l2_format fmt;
	CLEAR(fmt);
	fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	fmt.fmt.pix.width = 640;
	fmt.fmt.pix.height = 480;
	fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_YUYV;
	fmt.fmt.pix.field = V4L2_FIELD_INTERLACED;
	ioctl(camera_device, VIDIOC_S_FMT, &fmt);

	// calculate
	int file_length = fmt.fmt.pix.bytesperline * fmt.fmt.pix.height;

	struct v4l2_requestbuffers req;
	CLEAR(req);
	req.count = 4;
	req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	req.memory = V4L2_MEMORY_MMAP;
	ioctl(camera_device, VIDIOC_REQBUFS, &req); //allocate buffer

	if(req.count < 2){
		LOGE("Insufficient Memory");
	}

	LOGI("req.count %d", req.count);
	struct buffer* buffers = NULL;
	buffers = (struct buffer *) calloc(req.count, sizeof(*buffers));

	int n_buffers = 0;
	for(; n_buffers < req.count; ++ n_buffers){
		struct v4l2_buffer buf;   //one frame in the driver
		CLEAR(buf);
		buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		buf.memory = V4L2_MEMORY_MMAP;
		buf.index = n_buffers;
		if (-1 == ioctl(camera_device, VIDIOC_QUERYBUF, &buf)) //map user space memory
			LOGE("VIDIOC_QUERYBUF error\n");
		buffers[n_buffers].length = buf.length;
		buffers[n_buffers].start = mmap(
				NULL /* start anywhere */,
				buf.length, PROT_READ | PROT_WRITE /* required */,
				MAP_SHARED /* recommended */, camera_device, buf.m.offset);
		if (MAP_FAILED == buffers[n_buffers].start)
			LOGE("mmap failed\n");
	}

	for (i = 0; i < n_buffers; ++i) {
		struct v4l2_buffer buf;
		CLEAR(buf);
		buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		buf.memory = V4L2_MEMORY_MMAP;
		buf.index = i;
		if (-1 == ioctl(camera_device, VIDIOC_QBUF, &buf))    //get into the camera queue
			LOGE("VIDIOC_QBUF failed\n");
	}

	type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	if (-1 == ioctl(camera_device, VIDIOC_STREAMON, &type)) //begin picture capture
			LOGE("VIDIOC_STREAMON failed\n");

	for (;;){
		fd_set fds;
		struct timeval tv;
		int r;
		FD_ZERO(&fds);
		FD_SET(camera_device, &fds);
		/* Timeout. */
		tv.tv_sec = 2;
		tv.tv_usec = 0;
		r = select(camera_device + 1, &fds, NULL, NULL, &tv); //camera ready? tv is the time out
		if (-1 == r) {
			if (EINTR == errno)
				continue;
			LOGE("select err\n");
		}
		if (0 == r) {
			LOGE("select timeout\n");
			return -1;
		}
		// Read buffer
		struct v4l2_buffer buf;
		unsigned int i;
		CLEAR(buf);
		buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		buf.memory = V4L2_MEMORY_MMAP;	//Using memory map
		ioctl(camera_device, VIDIOC_DQBUF, &buf); //get the data when ready
		assert(buf.index < n_buffers);
		LOGI("buf.index dq is %d,\n", buf.index);
		fwrite(buffers[buf.index].start, buffers[buf.index].length, 1, file_fd); //write to file
		break;
	}

	unmap: for (i = 0; i < n_buffers; ++i)
		if (-1 == munmap(buffers[i].start, buffers[i].length))
			LOGE("munmap error");

	close(camera_device);

	fclose(file_fd);
}

/* no mmap */
JNIEXPORT jint JNICALL Java_com_example_droidlinuxcamara_MainActivity_getPictureNoMmap(JNIEnv* env, jobject thiz){

	LOGI("getPictureNoMap native function");

	char * dev_name = "/dev/video1";
	int fd = -1;
	struct buffer * buffers = NULL;
	FILE *file_fd;
	unsigned long file_length;
	unsigned char *file_name;

	struct v4l2_capability cap;
	struct v4l2_format fmt;

	LOGI("before open test.jpg");

	file_fd = fopen("test.jpg", "w");
	fd = open(dev_name, O_RDWR /* required */| O_NONBLOCK, 0);
	ioctl(fd, VIDIOC_QUERYCAP, &cap);

	CLEAR(fmt);
	fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	fmt.fmt.pix.width = 640;
	fmt.fmt.pix.height = 480;
	fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_YUYV;
	fmt.fmt.pix.field = V4L2_FIELD_INTERLACED;
	ioctl(fd, VIDIOC_S_FMT, &fmt); // this is the problem

	file_length = fmt.fmt.pix.bytesperline * fmt.fmt.pix.height;
	buffers = (struct buffer*) calloc(1, sizeof(*buffers));
	buffers[0].length = file_length;
	buffers[0].start = malloc(file_length);

	return 1;

	for (;;) {
		fd_set fds;
		struct timeval tv;
		int r;
		FD_ZERO(&fds);
		FD_SET(fd, &fds);
		/* Timeout. */
		tv.tv_sec = 3;
		tv.tv_usec = 0;
		r = select(fd + 1, &fds, NULL, NULL, &tv);
		if (-1 == r) {
			if (EINTR == errno)
				continue;
			LOGI("select");
		}
		if (0 == r) {
			LOGE("select timeout\n");
			exit (EXIT_FAILURE);
		}
		if (read(fd, buffers[0].start, buffers[0].length))
			break;
	}

	LOGI("before writing buffer");

	fwrite(buffers[0].start, buffers[0].length, 1, file_fd);
	free(buffers[0].start);
	close(fd);
	fclose(file_fd);
	return 0;
}
#ifdef __cplusplus
}
#endif


