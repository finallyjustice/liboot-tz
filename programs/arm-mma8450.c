// This is the program to test the accelerometer

// arm-none-linux-gnueabi-gcc arm-mma8450.c -o arm-mma8450

// EV_SYN 0x0
// EV_ABS 0x3

#include <linux/input.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>

int main(int argc, char **argv)
{
	int fd;
	struct input_event event;

	fd = open("/dev/event2", O_RDONLY | O_NDELAY);
	if(fd == -1)
	{
		printf("Unable to open /dev/event2");
		return 0;
    }
    
    ssize_t ret;
    while(ret=read(fd, &event, sizeof(struct input_event)))
    {
		if(ret != -1)
		{
			//if(event.type == EV_ABS)
			//{
				//if(event.code = 0)
				//	printf("ax=%hd\n", event.value);
				//if(event.code = 1)
				//	printf("ay=%hd\n", event.value);
				//if(event.code = 2)
				//	printf("az=%hd\n", event.value);
				//else
				if(event.type == 0x3)
					printf("type: %d, code: %d, value: %hd\n", event.type, event.code, event.value);
			//}
			//else
			//{
				//then it is a SYNC
			//}
		}

	}

	return 0;
}
