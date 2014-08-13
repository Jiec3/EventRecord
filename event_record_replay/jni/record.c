#include <stdio.h>
#include <fcntl.h> 
#include <poll.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <linux/input.h>
#include <linux/time.h>
#include <string.h>
#include "uinput.h"
#include "com_example_demo_native_event_JniHelper.h"
#include <jni.h>
#include <android/log.h>
#define LOG_TAG "debug"
#define LOGI(fmt, args...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, fmt, ##args)

const char *EV_PREFIX = "/dev/input/";
const char *OUT_FN = "/events/events.sh";
const char *IN_FN = "/events/events_temp.sh";

char * sdcardPath = "/sdcard";

/* NB event4 is the compass -- not required for tests. */
char *ev_devices[] = { "event0", "event1", "event2", "event3" /*, "event4" */};
#define NUM_DEVICES (sizeof(ev_devices) / sizeof(char *))

struct pollfd in_fds[NUM_DEVICES];
/*
 int out_fds[NUM_DEVICES];
 */
int out_fd;

int out_fds[NUM_DEVICES];
int num_events;
int in_fd;

/*if in recording*/
int in_record = 0;
/*if in replaying*/
int in_replay = 0;

/***/
int device_width = 0;
int device_height = 0;

int init()
{
	LOGI("init <<<<<<<<1");
	char buffer[256];
	int i, fd;

	sprintf(buffer, "%s%s", sdcardPath, OUT_FN);
	out_fd = open(buffer, O_WRONLY | O_CREAT | O_TRUNC);
	if (out_fd < 0)
	{
		LOGI("Couldn't open output file\n");
		return 1;
	}

	for (i = 0; i < NUM_DEVICES; i++)
	{
		sprintf(buffer, "%s%s", EV_PREFIX, ev_devices[i]);
		in_fds[i].events = POLLIN;
		in_fds[i].fd = open(buffer, O_RDONLY | O_NDELAY);
		if (in_fds[i].fd < 0)
		{
			LOGI("Couldn't open input device %s\n", ev_devices[i]);
			return 2;
		}
	}
	LOGI("init <<<<<<<<<<<<<<2");
	return 0;
}

int record()
{
	int i, numread;
	struct input_event event;
	char buffer[512];

	LOGI("start record");
	in_record = 1;

	while (in_record)
	{
		if (poll(in_fds, NUM_DEVICES, -1) < 0)
		{
			LOGI("Poll error\n");
			return 1;
		}
		for (i = 0; i < NUM_DEVICES; i++)
		{
			if (in_fds[i].revents & POLLIN)
			{
				/* Data available */
				numread = read(in_fds[i].fd, &event, sizeof(event));
				if (numread != sizeof(event))
				{
					LOGI("<<<<<<<<Read error\n");
					return 2;
				}

				if (write(out_fd, &i, sizeof(i)) != sizeof(i))
				{
					LOGI("<<<<<<<<<Write error\n");
					return 3;
				}
				if (write(out_fd, &event, sizeof(event)) != sizeof(event))
				{
					LOGI("<<<<<<<<<<<<Write error\n");
					return 4;
				}

				LOGI("input %d, time %ld.%06ld, type %d, code %d, value %d\n",
						i, event.time.tv_sec, event.time.tv_usec, event.type, event.code, event.value);
			}
		}
	}
	sprintf(buffer, "%s%s", sdcardPath, OUT_FN);
	char *oldname = buffer;
	char buffer2[64];
	sprintf(buffer2, "%s%s", sdcardPath, IN_FN);
	char *newname = buffer2;
	LOGI("<<<<<<<<rename %s, %s", oldname, newname);

	if (rename(oldname, newname) == 0) {
		LOGI("<<<<<<<<rename success");
	}

	close(out_fd);
	LOGI("<<<<<<<<<<close success");
	in_record = 0;
}

/****************play event script*******************/
int play_init()
{
	char buf[256];
	int i;
	struct stat statinfo;

	for (i = 0; i < NUM_DEVICES; i++)
	{
		sprintf(buf, "%s%s", EV_PREFIX, ev_devices[i]);
		out_fds[i] = open(buf, O_WRONLY | O_NDELAY);
		if (out_fds[i] < 0)
		{
			LOGI("Couldn't open output device\n");
			return 1;
		}
	}

	sprintf(buf, "%s%s", sdcardPath, IN_FN);
	if (stat(buf, &statinfo) == -1)
	{
		LOGI("Couldn't stat input\n");
		return 2;
	}

	num_events = statinfo.st_size / (sizeof(struct input_event) + sizeof(int));

	if ((in_fd = open(buf, O_RDONLY)) < 0)
	{
		LOGI("Couldn't open input\n");
		return 3;
	}

	// Hacky ioctl init
	ioctl(out_fds[3], UI_SET_EVBIT, EV_KEY);
	ioctl(out_fds[3], UI_SET_EVBIT, EV_REP);
	ioctl(out_fds[1], UI_SET_EVBIT, EV_ABS);

	LOGI("<<<<<<<replay init success");
	return 0;
}

int replay(JNIEnv * env) {
	struct timeval tdiff;
	struct input_event event;
	int i, outputdev;
	int record_width, record_height;

	char buff[512];

	in_replay = 1;
	timerclear(&tdiff);
	LOGI("<<<<<<start replay num_events = %d", num_events);

	for (i = 0; i < num_events; i++)
	{
		struct timeval now, tevent, tsleep;

		if (in_replay == 0)
		{
			LOGI("user force stop replay");
			close(in_fd);
			break;
		}

		if (read(in_fd, &outputdev, sizeof(outputdev)) != sizeof(outputdev))
		{
			LOGI("Input read error\n");
			return 1;
		}

		if (read(in_fd, &event, sizeof(event)) != sizeof(event))
		{
			LOGI("Input read error\n");
			return 2;
		}

		gettimeofday(&now, NULL);
		if (!timerisset(&tdiff))
		{
			timersub(&now, &event.time, &tdiff);
		}

		timeradd(&event.time, &tdiff, &tevent);
		timersub(&tevent, &now, &tsleep);
		if (tsleep.tv_sec > 0 || tsleep.tv_usec > 100)
			select(0, NULL, NULL, NULL, &tsleep);

		event.time = tevent;

		if (write(out_fds[outputdev], &event, sizeof(event)) != sizeof(event))
		{
			LOGI("Output write error\n");
			return 2;
		}

		LOGI("input %d, time %ld.%06ld, type %d, code %d, value %d\n", outputdev,
				event.time.tv_sec, event.time.tv_usec, event.type, event.code, event.value);
	}
	LOGI("<<<<<<<< replay finish");
	close(in_fd);
	in_replay = 0;
	return 0;
}

JNIEXPORT void JNICALL  Java_com_syouquan_script_ScriptEngine_nativeEventStartRecord
  (JNIEnv * env, jobject thiz )
{
	LOGI("---call native method---");

	if (in_record == 1)
	{
		LOGI("is in recoding");
		in_record = 0;
		return;
	}

	if (init() != 0)
	{
		printf("Init failed");
		return;
	}

	record();

}

JNIEXPORT void JNICALL  Java_com_syouquan_script_ScriptEngine_nativeEventStopRecord
  (JNIEnv * env, jobject thiz )
{
	LOGI("---call native method stop record---");

	in_record = 0;
}

JNIEXPORT void JNICALL Java_com_syouquan_script_ScriptEngine_nativeEventStartReplay
(JNIEnv * env, jobject thiz )
{
	LOGI("---call native method start replay---");

	if (in_replay == 1)
	{
		LOGI("is in replaying");
		return;
	}

	//sleep 1s
	sleep(1);

	in_record = 0;
	if(play_init() != 0)
	{
		LOGI("init failed\n");
		return;
	}

	if(replay(env) != 0)
	{
		LOGI("replay failed\n");
		return;
	}
}

JNIEXPORT void JNICALL  Java_com_syouquan_script_ScriptEngine_nativeEventStopReplay
  (JNIEnv * env, jobject thiz )
{
	LOGI("---call native method stop replay---");

	in_replay = 0;
}

