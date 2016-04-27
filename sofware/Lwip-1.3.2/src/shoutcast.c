/*
 * http client for RT-Thread
 */

//#include <dfs_posix.h>		//文件系统

#include "config.h"
#include "shoutcast.h"
#include "mem.h"
#include "netdb.h"
#include "inet.h"
#include "api.h" 
const char _http_get[] = "GET %s HTTP/1.0\r\nHost: %s:%d\r\nUser-Agent: RT-Thread HTTP Agent\r\n\r\n";
const char _shoutcast_get[] = "GET %s HTTP/1.0\r\nHost: %s:%d\r\nUser-Agent: RT-Thread HTTP Agent\r\nIcy-MetaData: 1\r\nConnection: close\r\n\r\n";

#define RT_NULL 				((void *)0)


//
// This function will parse the Content-Length header line and return the file size
//
int http_parse_content_length(char *mime_buf)
{
	char *line;

	line = strstr(mime_buf, "CONTENT-LENGTH:");
	line += strlen("CONTENT-LENGTH:");

	// Advance past any whitepace characters
	while((*line == ' ') || (*line == '\t')) line++;

	return (int)strtol(line, ((void *)0), 10);
}


//
// This function will parse the initial response header line and return 0 for a "200 OK",
// or return the error code in the event of an error (such as 404 - not found)
//
int http_is_error_header(char *mime_buf)
{
	char *line;
	int i;
	int code;

	line = strstr(mime_buf, "HTTP/1.");
	line += strlen("HTTP/1.");

	// Advance past minor protocol version number
	line++;

	// Advance past any whitespace characters
	while((*line == ' ') || (*line == '\t')) line++;

	// Terminate string after status code
	for(i = 0; ((line[i] != ' ') && (line[i] != '\t')); i++);
	line[i] = '\0';

	code = (int)strtol(line, RT_NULL, 10);
	if( code == 200 )
		return 0;
	else
		return code;
}

int shoutcast_is_error_header(char *mime_buf)
{
	char *line;
	int i;
	int code;

	line = strstr(mime_buf, "ICY");
	line += strlen("ICY");

	// Advance past minor protocol version number
	line++;

	// Advance past any whitespace characters
	while((*line == ' ') || (*line == '\t')) line++;

	// Terminate string after status code
	for(i = 0; ((line[i] != ' ') && (line[i] != '\t')); i++);
	line[i] = '\0';

	code = (int)strtol(line, RT_NULL, 10);
	if( code == 200 )
		return 0;
	else
		return code;
}

//
// When a request has been sent, we can expect mime headers to be
// before the data.  We need to read exactly to the end of the headers
// and no more data.  This readline reads a single char at a time.
//
int http_read_line( int socket, char * buffer, int size )
{
	char * ptr = buffer;
	int count = 0;
	int rc;

	// Keep reading until we fill the buffer.
	while ( count < size )
	{
		rc = recv( socket, ptr, 1, 0 );
		if ( rc <= 0 ) return rc;

		if ((*ptr == '\n'))
		{
			ptr ++;
			count++;
			break;
		}

		// increment after check for cr.  Don't want to count the cr.
		count++;
		ptr++;
	}

	// Terminate string
	*ptr = '\0';

	// return how many bytes read.
	return count;
}

/*
 * resolve server address
 * @param server the server sockaddress
 * @param url the input URL address, for example, http://www.rt-thread.org/index.html
 * @param host_addr the buffer pointer to save server host address
 * @param request the pointer to point the request url, for example, /index.html
 *
 * @return 0 on resolve server address OK, others failed
 */
int http_resolve_address(struct sockaddr_in *server, const char * url, char *host_addr, char** request)
{
	char *ptr;
	char port[6] = "80"; /* default port of 80(HTTP) */
	int i = 0, is_domain;
	struct hostent *hptr;

	/* strip http: */
	ptr = strchr(url, ':');
	if (ptr != NULL)
	{
		url = ptr + 1;
	}

	/* URL must start with double forward slashes. */
	if((url[0] != '/') || (url[1] != '/' )) return -1;

	url += 2; is_domain = 0;
	i = 0;
	/* allow specification of port in URL like http://www.server.net:8080/ */
	while (*url)
	{
		if (*url == '/') break;
		if (*url == ':')
		{
			unsigned char w;
			for (w = 0; w < 5 && url[w + 1] != '/' && url[w + 1] != '\0'; w ++)
				port[w] = url[w + 1];
			
			/* get port ok */
			port[w] = '\0';
			url += w + 1;
			break;
		}

		if ((*url < '0' || *url > '9') && *url != '.')
			is_domain = 1;
		host_addr[i++] = *url;
		url ++;
	}
	*request = (char*)url;

	/* get host addr ok. */
	host_addr[i] = '\0';

	if (is_domain)
	{
		/* resolve the host name. */
		hptr = lwip_gethostbyname(host_addr);   //netdb
		if(hptr == 0)
		{
			printf("HTTP: failed to resolve domain '%s'\n", host_addr);
			return -1;
		}
		memcpy(&server->sin_addr, *hptr->h_addr_list, sizeof(server->sin_addr));
	}
	else
	{
		inet_aton(host_addr, (struct in_addr*)&(server->sin_addr));	  //inet
	}
	/* set the port */
	server->sin_port = htons((int) strtol(port, NULL, 10));		 //
	server->sin_family = AF_INET;

	return 0;
}

signed long rt_snprintf(char *buf,  unsigned long size, const char *fmt, ...)
{
	signed long n;
	va_list args;

	va_start(args, fmt);
	n = vsnprintf(buf, size, fmt, args);
	va_end(args);
	return n;
}

unsigned long rt_strlen(const char *s)
{
	const char *sc;

	for (sc = s; *sc != '\0'; ++sc) /* nothing */
		;

	return sc - s;
}
void *rt_memcpy(void * dst, const void *src, unsigned long count)
{
#define UNALIGNED(X, Y) \
	(((signed long)X & (sizeof (signed long) - 1)) | ((signed long)Y & (sizeof (signed long) - 1)))
#define BIGBLOCKSIZE    (sizeof (signed long) << 2)
#define LITTLEBLOCKSIZE (sizeof (signed long))
#define TOO_SMALL(LEN)  ((LEN) < BIGBLOCKSIZE)

	char *dst_ptr = (char*)dst;
	char *src_ptr = (char*)src;
	signed long *aligned_dst;
	signed long *aligned_src;
	int len = count;

	/* If the size is small, or either SRC or DST is unaligned,
	then punt into the byte copy loop.  This should be rare.  */
	if (!TOO_SMALL(len) && !UNALIGNED (src_ptr, dst_ptr))
	{
		aligned_dst = (signed long*)dst_ptr;
		aligned_src = (signed long*)src_ptr;

		/* Copy 4X long words at a time if possible.  */
		while (len >= BIGBLOCKSIZE)
		{
			*aligned_dst++ = *aligned_src++;
			*aligned_dst++ = *aligned_src++;
			*aligned_dst++ = *aligned_src++;
			*aligned_dst++ = *aligned_src++;
			len -= BIGBLOCKSIZE;
		}

		/* Copy one long word at a time if possible.  */
		while (len >= LITTLEBLOCKSIZE)
		{
			*aligned_dst++ = *aligned_src++;
			len -= LITTLEBLOCKSIZE;
		}

		/* Pick up any residual with a byte copier.  */
		dst_ptr = (char*)aligned_dst;
		src_ptr = (char*)aligned_src;
	}

	while (len--)
		*dst_ptr++ = *src_ptr++;

	return dst;
}


char *rt_strdup(const char *s)
{
	unsigned long len = rt_strlen(s) + 1;
	char *tmp = (char *)mem_malloc(len);

	if(!tmp) return RT_NULL;

	rt_memcpy(tmp, s, len);
	return tmp;
}

//
// This is the main HTTP client connect work.  Makes the connection
// and handles the protocol and reads the return headers.  Needs
// to leave the stream at the start of the real data.
//
static int http_connect(struct http_session* session,
    struct sockaddr_in * server, char *host_addr, const char *url)
{
	int socket_handle;
	int peer_handle;
	int rc;
	char mimeBuffer[100];

	if((socket_handle = socket( PF_INET, SOCK_STREAM, IPPROTO_TCP )) < 0)
	{
		printf( "HTTP: SOCKET FAILED\n" );
		return -1;
	}

	peer_handle = connect( socket_handle, (struct sockaddr *) server, sizeof(*server));
	if ( peer_handle < 0 )
	{
		printf( "HTTP: CONNECT FAILED %i\n", peer_handle );
		return -1;
	}

	{
		char *buf;
		unsigned  long length;

		buf =(char *) mem_malloc (512);
		if (*url)
			length = rt_snprintf(buf, 512, _http_get, url, host_addr, server->sin_port);
		else
			length = rt_snprintf(buf, 512, _http_get, "/", host_addr, server->sin_port);
		
		rc = send(peer_handle, buf, length, 0);
		printf("HTTP request:\n%s", buf);
		
		/* release buffer */
		mem_free(buf);
	}

	// We now need to read the header information
	while ( 1 )
	{
		int i;

		// read a line from the header information.
		rc = http_read_line( peer_handle, mimeBuffer, 100 );
		printf(">> %s\n", mimeBuffer);

		if ( rc < 0 ) return rc;

		// End of headers is a blank line.  exit.
		if (rc == 0) break;
		if ((rc == 2) && (mimeBuffer[0] == '\r')) break;

		// Convert mimeBuffer to upper case, so we can do string comps
		for(i = 0; i < strlen(mimeBuffer); i++)
			mimeBuffer[i] = toupper(mimeBuffer[i]);

		if(strstr(mimeBuffer, "HTTP/1.")) // First line of header, contains status code. Check for an error code
		{
			rc = http_is_error_header(mimeBuffer);
			if(rc)
			{
				printf("HTTP: status code = %d!\n", rc);
				return -rc;
			}
		}

		if(strstr(mimeBuffer, "CONTENT-LENGTH:"))
		{
			session->size = http_parse_content_length(mimeBuffer);
			printf("size = %d\n", session->size);
		}
	}

	// We've sent the request, and read the headers.  SockHandle is
	// now at the start of the main data read for a file io read.
	return peer_handle;
}

struct http_session* http_session_open(char* url)
{
	int peer_handle = 0;
	struct sockaddr_in server;
	char *request, host_addr[32];
	struct http_session* session;

    session = (struct http_session*) mem_malloc(sizeof(struct http_session));
	if(session == RT_NULL) return RT_NULL;

	session->size = 0;
	session->position = 0;

	/* Check valid IP address and URL */
	if(http_resolve_address(&server, url, &host_addr[0], &request) != 0)
	{
		mem_free(session);
		return RT_NULL;
	}

	// Now we connect and initiate the transfer by sending a
	// request header to the server, and receiving the response header
	if((peer_handle = http_connect(session, &server, host_addr, request)) < 0)
	{
        printf("HTTP: failed to connect to '%s'!\n", host_addr);
		mem_free(session);
		return RT_NULL;
	}

	// http connect returns valid socket.  Save in handle list.
	session->socket = peer_handle;

	/* open successfully */
	return session;
}

unsigned  long http_session_read(struct http_session* session, unsigned  char *buffer, unsigned  long length)
{
	int bytesRead = 0;
	int totalRead = 0;
	int left = length;

	// Read until: there is an error, we've read "size" bytes or the remote
	//             side has closed the connection.
	do
	{
		bytesRead = recv(session->socket, buffer + totalRead, left, 0);
		if(bytesRead <= 0) break;

		left -= bytesRead;
		totalRead += bytesRead;
	} while(left);

	return totalRead;
}

unsigned  long http_session_seek(struct http_session* session, unsigned  long offset, int mode)
{
	switch(mode)
	{
	case SEEK_SET:
		session->position = offset;
		break;

	case SEEK_CUR:
		session->position += offset;
		break;

	case SEEK_END:
		session->position = session->size + offset;
		break;
	}

	return session->position;
}

int http_session_close(struct http_session* session)
{
   	lwip_close(session->socket);
	mem_free(session);

	return 0;
}

//
// This is the main HTTP client connect work.  Makes the connection
// and handles the protocol and reads the return headers.  Needs
// to leave the stream at the start of the real data.
//
static int shoutcast_connect(struct shoutcast_session* session,
    struct sockaddr_in* server, char* host_addr, const char* url)
{
	int socket_handle;
	int peer_handle;
	int rc;
	char mimeBuffer[256];

	if((socket_handle = socket( PF_INET, SOCK_STREAM, IPPROTO_TCP )) < 0)
	{
		printf( "ICY: SOCKET FAILED\n" );
		return -1;
	}

	peer_handle = connect( socket_handle, (struct sockaddr *) server, sizeof(*server));
	if ( peer_handle < 0 )
	{
		printf( "ICY: CONNECT FAILED %i\n", peer_handle );
		return -1;
	}

	{
		char *buf;
		unsigned  long length;

		buf =(char *) mem_malloc (512);
//		(char *)
		if (*url)
			length = rt_snprintf(buf, 512, _shoutcast_get, url, host_addr, ntohs(server->sin_port));
		else
			length = rt_snprintf(buf, 512, _shoutcast_get, "/", host_addr, ntohs(server->sin_port));

		rc = send(peer_handle, buf, length, 0);
		printf("SHOUTCAST request:\n%s", buf);
		
		/* release buffer */
		mem_free(buf);
	}

	/* read the header information */
	while ( 1 )
	{
		// read a line from the header information.
		rc = http_read_line(peer_handle, mimeBuffer, 100);
		printf(">>%s", mimeBuffer);

		if ( rc < 0 ) return rc;

		// End of headers is a blank line.  exit.
		if (rc == 0) break;
		if ((rc == 2) && (mimeBuffer[0] == '\r')) break;

		if(strstr(mimeBuffer, "ICY")) // First line of header, contains status code. Check for an error code
		{
			rc = shoutcast_is_error_header(mimeBuffer);
			if(rc)
			{
				printf("ICY: status code = %d!\n", rc);
				return -rc;
			}
		}
		
		if (strstr(mimeBuffer, "HTTP/1."))
		{
			rc = http_is_error_header(mimeBuffer);
			if(rc)
			{
				printf("HTTP: status code = %d!\n", rc);
				return -rc;
			}
		}

		if (strstr(mimeBuffer, "icy-name:"))
		{
			/* get name */
			char* name;

			name = mimeBuffer + strlen("icy-name:");
			session->station_name = rt_strdup(name);
			printf("station name: %s\n", session->station_name);
		}

		if (strstr(mimeBuffer, "icy-br:"))
		{
			/* get bitrate */
			session->bitrate = strtol(mimeBuffer + strlen("icy-br:"), RT_NULL, 10);
			printf("bitrate: %d\n", session->bitrate);
		}

		if (strstr(mimeBuffer, "icy-metaint:"))
		{
			/* get metaint */
			session->metaint = strtol(mimeBuffer + strlen("icy-metaint:"), RT_NULL, 10);
			printf("metaint: %d\n", session->metaint);
		}
		
		if (strstr(mimeBuffer, "content-type:"))
		{
			/* check content-type */
			if (strstr(mimeBuffer, "audio/mpeg") == RT_NULL)
			{
				printf("ICY content is not audio/mpeg.\n");
				return -1;
			}
		}

		if (strstr(mimeBuffer, "Content-Type:"))
		{
			/* check content-type */
			if (strstr(mimeBuffer, "audio/mpeg") == RT_NULL)
			{
				printf("ICY content is not audio/mpeg.\n");
				return -1;
			}
		}
	}

	// We've sent the request, and read the headers.  SockHandle is
	// now at the start of the main data read for a file io read.
	return peer_handle;
}

struct shoutcast_session* shoutcast_session_open(char* url)
{
	int peer_handle = 0;
	struct sockaddr_in server;
	char *request, host_addr[32];
	struct shoutcast_session* session;

    session = (struct shoutcast_session*) mem_malloc(sizeof(struct shoutcast_session));
	if(session == RT_NULL) return RT_NULL;

	session->metaint = 0;
	session->current_meta_chunk = 0;
	session->bitrate = 0;
	session->station_name = RT_NULL;

	/* Check valid IP address and URL */
	if(http_resolve_address(&server, url, &host_addr[0], &request) != 0)
	{
		mem_free(session);
		return RT_NULL;
	}

	printf("connect to: %s...\n", host_addr);

	// Now we connect and initiate the transfer by sending a
	// request header to the server, and receiving the response header
	if((peer_handle = shoutcast_connect(session, &server, host_addr, request)) < 0)
	{
        printf("SHOUTCAST: failed to connect to '%s'!\n", host_addr);
		if (session->station_name != RT_NULL)
			mem_free(session->station_name);
		mem_free(session);
		return RT_NULL;
	}

	// http connect returns valid socket.  Save in handle list.
	session->socket = peer_handle;

	/* open successfully */
	return session;
}

unsigned  long shoutcast_session_read(struct shoutcast_session* session, unsigned  char *buffer, unsigned  long length)
{
	int bytesRead = 0;
	int totalRead = 0;
	int left = length;
	static unsigned  long first_meta_size = 0;

	// Read until: there is an error, we've read "size" bytes or the remote
	//             side has closed the connection.
	do
	{
		bytesRead = recv(session->socket, buffer + totalRead, left, 0);
		if(bytesRead <= 0) 
		{
			printf("no data on recv, len %d,err %d\n", bytesRead, 
				session->socket);
			break;
		}

		left -= bytesRead;
		totalRead += bytesRead;
	} while(left);

	/* handle meta */
	if (first_meta_size > 0)
	{
		/* skip meta data */
		memmove(&buffer[0], &buffer[first_meta_size], totalRead - first_meta_size);

		// printf("remove meta: len %d\n", first_meta_size);

		totalRead = totalRead - first_meta_size;
		first_meta_size = 0;
		session->current_meta_chunk = totalRead;
	}
	else
	{
		if (session->current_meta_chunk + totalRead == session->metaint)
		{
			unsigned  char meta_data;
			recv(session->socket, &meta_data, 1, 0);
		
			/* remove meta data in next packet */
			first_meta_size = meta_data * 16;
			session->current_meta_chunk = 0;

			// printf("get meta: len %d\n", first_meta_size);
		}
		else if (session->current_meta_chunk + totalRead > session->metaint)
		{
			int meta_length, next_chunk_length;

			// printf("c: %d, total: %d, m: %d\n", session->current_meta_chunk, totalRead, session->metaint);

			/* get the length of meta data */
			meta_length = buffer[session->metaint - session->current_meta_chunk] * 16;
			next_chunk_length = totalRead - (session->metaint - session->current_meta_chunk) - 
				(meta_length + 1);

			// printf("l: %d, n: %d\n", meta_length, next_chunk_length);

			/* skip meta data */
			memmove(&buffer[session->metaint - session->current_meta_chunk], 
				&buffer[session->metaint - session->current_meta_chunk + meta_length + 1],
				next_chunk_length);

			/* set new current meta chunk */
			session->current_meta_chunk = next_chunk_length;
			totalRead = totalRead - (meta_length + 1);
		}
		else 
		{
			session->current_meta_chunk += totalRead;
		}
	}

	return totalRead;
}

unsigned  long shoutcast_session_seek(struct shoutcast_session* session, unsigned  long offset, int mode)
{
	/* not support seek yet */
	return 0;
}

int shoutcast_session_close(struct shoutcast_session* session)
{
   	lwip_close(session->socket);
	if (session->station_name != RT_NULL)
		mem_free(session->station_name);
	mem_free(session);

	return 0;
}
	  /*
#include <finsh.h>
void http_test(char* url)
{
	struct http_session* session;
	char buffer[80];
	unsigned  long length;

	session = http_session_open(url);
	if (session == RT_NULL)
	{
		printf("open http session failed\n");
		return;
	}

	do
	{
		rt_memset(buffer, 0, sizeof(buffer));
		length = http_session_read(session, (unsigned  char*)buffer, sizeof(buffer));

		printf(buffer);printf("\n");
	} while (length > 0);

	http_session_close(session);
}
FINSH_FUNCTION_EXPORT(http_test, http client test);

void shoutcast_test(char* url)
{
	struct shoutcast_session* session;

	session = shoutcast_session_open(url);
	if (session == RT_NULL)
	{
		printf("open shoutcast session failed\n");
		return;
	}

	shoutcast_session_close(session);
}
FINSH_FUNCTION_EXPORT(shoutcast_test, shoutcast client test);

	*/	

