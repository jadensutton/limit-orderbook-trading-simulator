#ifndef Server_H
#define Server_H

#define SERVER_MQUEUE 1234
#define MAX_TEXT 128

struct MsgSt {
	long int type;
	char text[MAX_TEXT];
};
#endif
