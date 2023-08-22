/* Copyright (c) 2023, Canaan Bright Sight Co., Ltd
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * 1. Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND
 * CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
 * INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <linux/input.h>

int main(int argc, const char *argv[])
{
    struct input_event event_mouse ;
    int fd    = -1 ;
	char *filename = "/dev/input/event1";

	if (argc > 1) {
        filename = argv[1];
    }

    fd = open(filename, O_RDONLY);
    if(-1 == fd)
    {
        printf("open mouse event fair!\n");
        return -1 ;
    }
    while(1)
    {
        read(fd, &event_mouse, sizeof(event_mouse));
        switch(event_mouse.type)
		{
            case EV_KEY:
			{
				switch(event_mouse.code)
				{
					case BTN_LEFT:
					{
						printf("the left %d!\n", event_mouse.value);
					}
					break;
					case BTN_RIGHT:
					{
						printf("the right %d!\n", event_mouse.value);
					}
					break;
					case BTN_MIDDLE:
					{
                        printf("the middle %d!\n", event_mouse.value);
					}
					break;
				}
			}
			break;
			case EV_REL:
			{
				switch(event_mouse.code)
				{
					case REL_X:
					{
						if(event_mouse.value>0)
							printf("X slip is right!\n");
                        else if(event_mouse.value<0)
                            printf("X slip is left!\n");
					}
					break;
					case REL_Y:
					{
						if(event_mouse.value<0)
							printf("Y slip is up!\n");
                        else if(event_mouse.value>0)
                            printf("Y slip is down!\n");
					}
                    case REL_WHEEL:
					{
						if(event_mouse.value<0)
							printf("wheel slip is up!\n");
                        else if(event_mouse.value>0)
                            printf("wheel slip is down!\n");
					}
					break;
				}
			
			}
			break;
		}
    }
    close(fd);
    return 0 ;
}