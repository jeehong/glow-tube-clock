#ifndef __SHOUTCAST_H__
#define __SHOUTCAST_H__



struct http_session
{
    char* user_agent;
	int   socket;

    /* size of http file */
    unsigned long size;
    unsigned long  position;
};

struct shoutcast_session
{
	int   socket;

	/* shoutcast name and bitrate */
	char* station_name;
	int   bitrate;

	/* size of meta data */
	unsigned long metaint;
	unsigned long current_meta_chunk;
};

struct http_session* http_session_open(char* url);
unsigned long http_session_read(struct http_session* session, unsigned  char *buffer, unsigned long length);
unsigned long http_session_seek(struct http_session* session, unsigned long offset, int mode);
int http_session_close(struct http_session* session);

struct shoutcast_session* shoutcast_session_open(char* url);
unsigned long shoutcast_session_read(struct shoutcast_session* session, unsigned  char *buffer, unsigned long length);
unsigned long shoutcast_session_seek(struct shoutcast_session* session, unsigned long offset, int mode);
int shoutcast_session_close(struct shoutcast_session* session);

#endif
