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
 */
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <linux/input.h>
#include <unistd.h>
#define NOKEY 0

int main(int argc, char *argv[])
{
	int keys_fd;	
	struct input_event t;

    setvbuf(stdout, (char *)NULL, _IONBF, 0);//disable stdio out buffer;

	if (2 != argc) {
		fprintf(stderr, "input: %s /dev/input/eventx\n", argv[0]);
		return -1;
	}

    keys_fd = open(argv[1], O_RDONLY);
	if(keys_fd<=0)
	{
        printf("open %s device error!\n", argv[1]);
		return 0;
	}

	while(1)
	{	
            if(read(keys_fd,&t,sizeof(t))==sizeof(t)) {
		    if(t.type==EV_KEY)
			if(t.value==0 || t.value==1)
			{
				printf("%d \n", t.code);
				switch(t.code)
				{
					case 103:
			    		printf("key103 key-up %s\n",(t.value)?"Presse":"Released");
			    	break;
			    	
			    	case 108:
			    		printf("key108 key-down %s\n",(t.value)?"Pressed":"Released");
			    	break;
			    	
			    	case 106:
			    		printf("key106 key-right %s\n",(t.value)?"pressed":"Released");
			    	break;  

					case 105:
						printf("key105 key-left %s\n",(t.value)?"Released":"Pressed");
					break;
			    	
			    	case 28:
			    		printf("key28 key-enter %s\n",(t.value)?"Pressed":"Released");
			    	break;
			    	
			    	case 1:
			    		printf("key1 key-esc %s\n",(t.value)?"Released":"Pressed");
			        break;

					case 2:
			    		printf("key2 key-ec11 %s\n",(t.value)?"Released":"Pressed");
			        break;
	    	
			    	default:
			    		break;
			    }
			}
		}
	}	
	close(keys_fd);
	
        return 0;
}
