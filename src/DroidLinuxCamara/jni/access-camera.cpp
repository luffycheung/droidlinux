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

	camera_device = open("/dev/video0", O_RDWR, 0);
	LOGI("access camera id %d", camera_device);
	// get camera capabilities and initialize the camera
	struct v4l2_capability cap;
	int ret = ioctl(camera_device, VIDIOC_QUERYCAP, &cap);
	if (ret == -1){
		LOGE("Query Capabilities Error!");
	}
	LOGI("capabilities 0x%x", cap.capabilities);

	if(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE){
		LOGI("Support video capture");
	}
	if(cap.capabilities & V4L2_CAP_VIDEO_OUTPUT){
		LOGI("Support video OUTPUT");
	}
	LOGI("Capability Informations:\n");
	LOGI(" driver: %s\n", cap.driver);
	LOGI(" card: %s\n", cap.card);
	LOGI(" bus_info: %s\n", cap.bus_info);
	LOGI(" version: %08X\n", cap.version);
	LOGI(" capabilities: %08X\n", cap.capabilities);

	struct v4l2_input input;
	input.index = 0;
	if (0 != ioctl(camera_device, VIDIOC_ENUMINPUT, &input)) {
		LOGE("VIDIOC_ENUM_INPUT");
	}
	LOGI("Current input index: %d\n", input.index);
	LOGI("Current input camera name: %s\n", input.name);
	LOGI("Current input camera status: %08X\n", input.status);


	if (-1 == ioctl(camera_device, VIDIOC_S_INPUT, input.index)) {
		LOGE("VIDIOC_S_INPUT error!");
	}
//
//	v4l2_std_id std;
//	ret = ioctl(camera_device, VIDIOC_QUERYSTD, &std);
//	if (ret == -1){
//		LOGE("Query Standard Error!");
//	}
//	switch (std) {
//	    case V4L2_STD_NTSC:
//	        LOGI("V4L2_STD_NTSC");
//	    case V4L2_STD_PAL:
//	        LOGI("V4L2_STD_PAL");
//	}
//
//	struct v4l2_fmtdesc fmtdesc;
//	fmtdesc.index = 0;
//	fmtdesc.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
//	ret = ioctl(camera_device, VIDIOC_ENUM_FMT, &fmtdesc);
//	if (ret == -1){
//		LOGE("Read FMT error!");
//	}
//	else {
//	        LOGI("{ pixelformat = ''%c%c%c%c'', description = ''%s'' }\n",
//	                  fmtdesc.pixelformat & 0xFF, (fmtdesc.pixelformat >> 8) & 0xFF, (fmtdesc.pixelformat >> 16) & 0xFF,
//	                  (fmtdesc.pixelformat >> 24) & 0xFF, fmtdesc.description);
//	}
//
//	struct v4l2_standard standard;
//
//	memset(&input, 0, sizeof(input));
//	if (-1 == ioctl(camera_device, VIDIOC_G_INPUT, &input.index)) {
//		LOGE("VIDIOC_G_INPUT");
//	}
//

//	memset(&standard, 0, sizeof(standard));
//	standard.index = 0;
//	while (0 == ioctl(camera_device, VIDIOC_ENUMSTD, &standard)) {
//		if (standard.id & input.std)
//		LOGI("%s\n", standard.name);
//
//		standard.index++;
//	}
	/* EINVAL indicates the end of the enumeration, which cannot be
	 empty unless this device falls under the USB exception. */

//	if (errno != EINVAL || standard.index == 0) {
//		LOGE("VIDIOC_ENUMSTD");
//	}
/*
	struct v4l2_queryctrl queryctrl;
	struct v4l2_querymenu querymenu;
	memset(&queryctrl, 0, sizeof(queryctrl));
	for (queryctrl.id = V4L2_CID_BASE; queryctrl.id < V4L2_CID_LASTP1;
			queryctrl.id++) {
		if (ioctl(camera_device, VIDIOC_QUERYCTRL, &queryctrl) == 0) {
			LOGI(
					"VIDIOC_QUERYCTRL(V4L2_CID_BASE+%d)\\n", queryctrl.id-V4L2_CID_BASE);
			LOGI("   id: %d\\n", queryctrl.id);
			switch (queryctrl.type) {
			case V4L2_CTRL_TYPE_INTEGER:
				LOGI("   type: INTEGER\\n");
				break;
			case V4L2_CTRL_TYPE_BOOLEAN:
				LOGI("   type: BOOLEAN\\n");
				break;
			case V4L2_CTRL_TYPE_MENU:
				LOGI("   type: MENU\\n");
				querymenu.id = queryctrl.id;
				for (querymenu.index = queryctrl.minimum;
						querymenu.index < queryctrl.maximum;
						querymenu.index++) {
					LOGI("      menu id: %d\\n", querymenu.index);
					LOGI("      menu name: %s\\n", querymenu.name);
				}
				break;
			case V4L2_CTRL_TYPE_BUTTON:
				LOGI("   type: BUTTON\\n");
				break;
			}
			LOGI("   name: %s\\n", queryctrl.name);
			LOGI("   minimum: %d\\n", queryctrl.minimum);
			LOGI("   maximum: %d\\n", queryctrl.maximum);
			LOGI("   step: %d\\n", queryctrl.step);
			LOGI("   default_value: %d\\n", queryctrl.default_value);
			LOGI("   flags: %d\\n", queryctrl.flags);
		} else {
			if (errno == EINVAL)
				continue;
			LOGE("VIDIOC_QUERYCTRL");
			break;
		}
	}
*/
//	LOGI("Prepare to build FMT!");
//	//set format
//	struct v4l2_format fmt;
//
//	fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
//	if (-1 == ioctl(camera_device, VIDIOC_G_FMT, &fmt)){
//		LOGE("VIDIOC_G_FMT");
//	}
//	else {
//		LOGI("VIDIOC_G_FMT");
//	}
//
//	CLEAR(fmt);
//	fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
//	fmt.fmt.pix.width = 640;
//	fmt.fmt.pix.height = 480;
//	fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_YUYV;
//	LOGI("FMT prepared!");
//	fmt.fmt.pix.field = V4L2_FIELD_INTERLACED;
//	if (-1 == ioctl(camera_device, VIDIOC_S_FMT, &fmt)) {
//		LOGE("VIDIOC_S_FMT");
//	}
//	LOGI("FMT sended!");
//	// calculate
//	int file_length = fmt.fmt.pix.bytesperline * fmt.fmt.pix.height;
//
//	struct v4l2_requestbuffers req;
//	CLEAR(req);
//	req.count = 4;
//	req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
//	req.memory = V4L2_MEMORY_MMAP;
//	if (-1 == ioctl(camera_device, VIDIOC_REQBUFS, &req)) { //allocate buffer
//        LOGE("VIDIOC_REQBUFS");
//	}
//	if(req.count < 2){
//		LOGE("Insufficient Memory");
//	}
//
//	LOGI("req.count %d", req.count);
//	struct buffer* buffers = NULL;
//	buffers = (struct buffer *) calloc(req.count, sizeof(*buffers));
//
//	int n_buffers = 0;
//	for(; n_buffers < req.count; ++ n_buffers){
//		struct v4l2_buffer buf;   //one frame in the driver
//		CLEAR(buf);
//		buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
//		buf.memory = V4L2_MEMORY_MMAP;
//		buf.index = n_buffers;
//		if (-1 == ioctl(camera_device, VIDIOC_QUERYBUF, &buf)) //map user space memory
//			LOGE("VIDIOC_QUERYBUF error\n");
//		buffers[n_buffers].length = buf.length;
//		buffers[n_buffers].start = mmap(
//				NULL /* start anywhere */,
//				buf.length, PROT_READ | PROT_WRITE /* required */,
//				MAP_SHARED /* recommended */, camera_device, buf.m.offset);
//		if (MAP_FAILED == buffers[n_buffers].start)
//			LOGE("mmap failed\n");
//	}
//    LOGI("Buffers");
//
//	for (i = 0; i < n_buffers; ++i) {
//		struct v4l2_buffer buf;
//		CLEAR(buf);
//		buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
//		buf.memory = V4L2_MEMORY_MMAP;
//		buf.index = i;
//		if (-1 == ioctl(camera_device, VIDIOC_QBUF, &buf))    //get into the camera queue
//			LOGE("VIDIOC_QBUF failed\n");
//	}
//
//	type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
//	if (-1 == ioctl(camera_device, VIDIOC_STREAMON, &type)) //begin picture capture
//			LOGE("VIDIOC_STREAMON failed\n");
//
//	for (;;){
//		fd_set fds;
//		struct timeval tv;
//		int r;
//		FD_ZERO(&fds);
//		FD_SET(camera_device, &fds);
//		/* Timeout. */
//		tv.tv_sec = 2;
//		tv.tv_usec = 0;
//		r = select(camera_device + 1, &fds, NULL, NULL, &tv); //camera ready? tv is the time out
//		if (-1 == r) {
//			if (EINTR == errno)
//				continue;
//			LOGE("select err\n");
//		}
//		if (0 == r) {
//			LOGE("select timeout\n");
//			return -1;
//		}
//		// Read buffer
//		struct v4l2_buffer buf;
//		unsigned int i;
//		CLEAR(buf);
//		buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
//		buf.memory = V4L2_MEMORY_MMAP;	//Using memory map
//		ioctl(camera_device, VIDIOC_DQBUF, &buf); //get the data when ready
//		assert(buf.index < n_buffers);
//		LOGI("buf.index dq is %d,\n", buf.index);
//		fwrite(buffers[buf.index].start, buffers[buf.index].length, 1, file_fd); //write to file
//		break;
//	}
//
//	unmap: for (i = 0; i < n_buffers; ++i)
//		if (-1 == munmap(buffers[i].start, buffers[i].length))
//			LOGE("munmap error");

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


