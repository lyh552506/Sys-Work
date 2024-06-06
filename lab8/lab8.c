#include <bits/types/struct_timeval.h>
#include <fcntl.h>
#include <linux/videodev2.h>
#include <poll.h>
#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/poll.h>
#include <sys/select.h>
#include <time.h>
#include <unistd.h>
#define VIDEO "/dev/video0"
#define IMG_WIDTH 640
#define IMG_HEIGHT 480

int main() {
  int fdv4l = open(VIDEO, O_RDWR);
  struct v4l2_capability cap; // 设备属性
  struct v4l2_fmtdesc fmt;    // 获取当前视频设备支持的视频格式
  struct v4l2_format format;  // 设置视频设备的视频数据格式
  struct v4l2_requestbuffers reqbuf; // 请求缓冲区
  struct v4l2_frmsizeenum frmsize;
  struct v4l2_buffer buffs; // 查询缓冲区
  void *BuffAddr = NULL;
  int ret;
#ifdef DEBUG
  // 查询是否有解析视频能力
  ret = ioctl(fdv4l, VIDIOC_QUERYCAP, &cap);
  if (ret < 0) {
    perror("ioctl failed");
    return 0;
  }
  printf("DriverName:%s\nCard Name:%s\nBus info:%s\nDriverVersion:%u.%u.%u\n",
         cap.driver, cap.card, cap.bus_info, (cap.version >> 16) & 0XFF,
         (cap.version >> 8) & 0XFF, cap.version & 0XFF);
  // 查询支持格式
  memset(&fmt, 0, sizeof(fmt));
  fmt.index = 0;
  fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  while ((ret = ioctl(fdv4l, VIDIOC_ENUM_FMT, &fmt)) == 0) {
    fmt.index++;
    printf("{ pixelformat =''%c%c%c%c'', description = ''%s'' }\n",
           fmt.pixelformat & 0xFF, (fmt.pixelformat >> 8) & 0xFF,
           (fmt.pixelformat >> 16) & 0xFF, (fmt.pixelformat >> 24) & 0xFF,
           fmt.description);
  }

  frmsize.pixel_format = V4L2_PIX_FMT_YUYV;
  frmsize.index = 0;
  while (ioctl(fdv4l, VIDIOC_ENUM_FRAMESIZES, &frmsize) >= 0) {
    printf("Supported frame size: %dx%d\n", frmsize.discrete.width,
           frmsize.discrete.height);
    frmsize.index++;
  }
#endif
  // 设置视频格式
  format.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  format.fmt.pix.height = IMG_HEIGHT;
  format.fmt.pix.width = IMG_WIDTH;
  format.fmt.pix.pixelformat = V4L2_PIX_FMT_YUYV;
  format.fmt.pix.field = V4L2_FIELD_INTERLACED;
  ret = ioctl(fdv4l, VIDIOC_S_FMT, &format);
  if (ret < 0) {
    perror("ioctl failed");
    return 0;
  }
#ifdef DEBUG
  ret = ioctl(fdv4l, VIDIOC_G_FMT, &format);
  if (ret < 0) {
    printf("VIDIOC_G_FMT failed (%d)\n", ret);
    return ret;
  }
  // Print Stream Format
  printf("Stream Format Informations:\n");
  printf(" type: %d\n", format.type);
  printf(" width: %d\n", format.fmt.pix.width);
  printf(" height: %d\n", format.fmt.pix.height);
#endif
  // 请求缓冲区
  memset(&reqbuf, 0, sizeof(struct v4l2_requestbuffers));
  reqbuf.count = 1;                          // 申请1个缓冲区
  reqbuf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE; // 缓冲区类型是视频捕获缓冲区
  reqbuf.memory = V4L2_MEMORY_MMAP;
  ret = ioctl(fdv4l, VIDIOC_REQBUFS, &reqbuf);
  if (ret < 0) {
    perror("ioctl failed");
    return 0;
  }
  // 查询已经分配的缓冲区
  for (int ind = 0; ind < reqbuf.count; ind++) {
    memset(&buffs, 0, sizeof(struct v4l2_buffer));
    buffs.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    buffs.memory = V4L2_MEMORY_MMAP;
    buffs.index = ind;
    ret = ioctl(fdv4l, VIDIOC_QUERYBUF, &buffs);
    if (ret < 0) {
      perror("ioctl failed");
      return 0;
    }
    // 把内核空间缓冲区映射到用户空间缓冲区
    int length = buffs.length;
    BuffAddr = mmap(NULL, buffs.length, PROT_READ | PROT_WRITE, MAP_SHARED,
                    fdv4l, buffs.m.offset);
    // 投放空的视频缓冲区到视频缓冲区输入队列
    memset(&buffs, 0, sizeof(struct v4l2_buffer));
    buffs.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    buffs.memory = V4L2_MEMORY_MMAP;
    buffs.index=ind;
    ret = ioctl(fdv4l, VIDIOC_QBUF, &buffs);
    if (ret < 0) {
      perror("ioctl failed");
      return 0;
    }
    enum v4l2_buf_type v4l2ty = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    ret = ioctl(fdv4l, VIDIOC_STREAMON, &v4l2ty);
    if (ret < 0) {
      perror("ioctl failed");
      return 0;
    }
    fd_set fd; // 创建fd集合
    struct timeval tv;
    FD_ZERO(&fd); // init
    FD_SET(fdv4l, &fd);
    tv.tv_sec = 3, tv.tv_usec = 0; // 设置超时时间
    select(fdv4l + 1, &fd, NULL, NULL, &tv);
    // struct pollfd pfd;
    // pfd.fd = fdv4l;
    // pfd.events = POLLIN;
    // ret = poll(&pfd, 1, 10000);

    // 取出缓冲区队列
    ret = ioctl(fdv4l, VIDIOC_STREAMOFF, &v4l2ty);
    if (ret < 0) {
      perror("ioctl failed");
      return 0;
    }
    memset(&buffs, 0, sizeof(struct v4l2_buffer));
    buffs.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    buffs.memory = V4L2_MEMORY_MMAP;
    buffs.index=ind;
    ret = ioctl(fdv4l, VIDIOC_DQBUF, &buffs);
    if (ret < 0) {
      perror("ioctl failed");
      return 0;
    }
  }
}