#ifndef __RC_COMMAND_H__
#define __RC_COMMAND_H__

enum __command
{
	RC_CMD_INVALID = 0,
	RC_CMD_EXEC,
	RC_CMD_NUM
};

struct rc_request {
	int cmd;
	unsigned int id;
	unsigned long long request[0];
};

struct rc_response {
	int cmd;
	unsigned int id;
	unsigned long long response[0];
};

struct rc_request_exec
{
	int len;
	char cmd[128];
};

struct rc_response_exec
{
	int ret;
};

#endif
