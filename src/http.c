/*
  Axel -- A lighter download accelerator for Linux and other Unices

  Copyright 2001-2007 Wilmer van der Gaast
  Copyright 2008      Y Giridhar Appaji Nag
  Copyright 2008-2009 Philipp Hagemeister
  Copyright 2015      Joao Eriberto Mota Filho
  Copyright 2016      Ivan Gimenez
  Copyright 2016      Phillip Berndt
  Copyright 2016      Sjjad Hashemian
  Copyright 2016      Stephen Thirlwall
  Copyright 2017      Antonio Quartulli
  Copyright 2017      David Polverari
  Copyright 2017      Ismael Luceno

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  as published by the Free Software Foundation; either version 2
  of the License, or (at your option) any later version.

  In addition, as a special exception, the copyright holders give
  permission to link the code of portions of this program with the
  OpenSSL library under certain conditions as described in each
  individual source file, and distribute linked combinations including
  the two.

  You must obey the GNU General Public License in all respects for all
  of the code used other than OpenSSL. If you modify file(s) with this
  exception, you may extend this exception to your version of the
  file(s), but you are not obligated to do so. If you do not wish to do
  so, delete this exception statement from your version. If you delete
  this exception statement from all source files in the program, then
  also delete it here.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

/* HTTP control file */

#include "axel.h"

inline static char
chain_next(const char ***p)
{
	while (**p && !***p)
		++(*p);
	return **p ? *(**p)++ : 0;
}

static void
http_auth_token(char *token, const char *user, const char *pass)
{
	const char base64_encode[64] =
	    "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
	    "abcdefghijklmnopqrstuvwxyz" "0123456789+/";
	const char *auth[] = { user, ":", pass, NULL };
	const char **p = auth;

	while (*p && **p) {
		char a = chain_next(&p);
		*token++ = base64_encode[a >> 2];
		char b = chain_next(&p);
		*token++ = base64_encode[((a & 3) << 4) | (b >> 4)];
		if (!b) {
			*token++ = '=';
			*token++ = '=';
			break;
		} else {
			char c = chain_next(&p);
			*token++ = base64_encode[((b & 15) << 2)
						 | (c >> 6)];
			if (!c) {
				*token++ = '=';
				break;
			} else
				*token++ = base64_encode[c & 63];
		}
	}
}

//建立tcp连接生成authenkey
int
http_connect(http_t *conn, int proto, char *proxy, char *host, int port,
	     char *user, char *pass, unsigned io_timeout)
{
	char *puser = NULL, *ppass = "";
	conn_t tconn[1];

	strncpy(conn->host, host, sizeof(conn->host) - 1);
	conn->port = port;
	conn->proto = proto;

	if (proxy != NULL) { //如果设置了proxy的话，覆盖连接建立相关的变量。
		if (*proxy != 0) {
			sprintf(conn->host, "%s:%i", host, port);
			if (!conn_set(tconn, proxy)) {
				/* We'll put the message in conn->headers, not in request */
				sprintf(conn->headers,
					_("Invalid proxy string: %s\n"), proxy);
				return 0;
			}
			host = tconn->host;
			port = tconn->port;
			proto = tconn->proto;
			puser = tconn->user;
			ppass = tconn->pass;
			conn->proxy = 1;
		} else {
			conn->proxy = 0;
		}
	}

	//建立tcp链接,fd保存在conn->tcp里面
	if (tcp_connect(&conn->tcp, host, port, PROTO_IS_SECURE(proto),
			conn->local_if, conn->headers, io_timeout) == -1)
		return 0;

	if (*user == 0) {
		*conn->auth = 0;
	} else {
		http_auth_token(conn->auth, user, pass);
	}

	if (!conn->proxy || !puser || *puser == 0) {
		*conn->proxy_auth = 0;
	} else {
		//base64加密。。细看逻辑？
		http_auth_token(conn->proxy_auth, puser, ppass);
	}

	return 1;
}

void
http_disconnect(http_t *conn)
{
	tcp_close(&conn->tcp);
}

//往http请求头部添加一些关键header，例如GET，HOST，Range，Authorization等
void
http_get(http_t *conn, char *lurl) //lurl = dir + file
{
	*conn->request = 0;
	if (conn->proxy) {
		const char *proto = scheme_from_proto(conn->proto);
		http_addheader(conn, "GET %s%s%s HTTP/1.0", proto, conn->host, lurl);
	} else {
		http_addheader(conn, "GET %s HTTP/1.0", lurl);
		if ((conn->proto == PROTO_HTTP &&
		     conn->port != PROTO_HTTP_PORT) ||
		    (conn->proto == PROTO_HTTPS &&
		     conn->port != PROTO_HTTPS_PORT))
			http_addheader(conn, "Host: %s:%i", conn->host,
				       conn->port);
		else
			http_addheader(conn, "Host: %s", conn->host);
	}
	if (*conn->auth)
		http_addheader(conn, "Authorization: Basic %s", conn->auth);
	if (*conn->proxy_auth)
		http_addheader(conn, "Proxy-Authorization: Basic %s", conn->proxy_auth);
	http_addheader(conn, "Accept: */*");
	if (conn->firstbyte) {
		if (conn->lastbyte)
			http_addheader(conn, "Range: bytes=%lld-%lld", conn->firstbyte, conn->lastbyte);
		else
			http_addheader(conn, "Range: bytes=%lld-", conn->firstbyte);
	}
}

void
http_addheader(http_t *conn, char *format, ...)
{
	char s[MAX_STRING];
	va_list params;

	va_start(params, format);
	vsnprintf(s, sizeof(s) - 3, format, params);
	strcat(s, "\r\n");
	va_end(params);

	strncat(conn->request, s, MAX_QUERY - strlen(conn->request) - 1);
}

//发送http请求和接收http返回
int
http_exec(http_t *conn)
{
	int i = 0;
	ssize_t nwrite = 0;
	char s[2] = {0}, *s2;

#ifdef DEBUG
	fprintf(stderr, "--- Sending request ---\n%s--- End of request ---\n", conn->request);
#endif

	http_addheader(conn, "");
	while (nwrite < strlen(conn->request)) {
		if ((i = tcp_write(&conn->tcp, conn->request + nwrite, strlen(conn->request) - nwrite)) < 0) {
			if (errno == EINTR || errno == EAGAIN)
				continue;
			/* We'll put the message in conn->headers, not in request */
			sprintf(conn->headers, _("Connection gone while writing.\n"));
			return 0;
		}
		nwrite += i;
	}

	*conn->headers = 0;
	/* Read the headers byte by byte to make sure we don't touch the actual data */
	while (1) {
		if (tcp_read(&conn->tcp, s, 1) <= 0) {
			/* We'll put the message in conn->headers, not in request */
			sprintf(conn->headers, _("Connection gone.\n"));
			return 0;
		}

		if (*s == '\r') {
			continue;
		} else if (*s == '\n') { //两个\r\n\r\n是http头部和body的分隔符，可以理解为header和body之间有两个空行。在这里只读header；读完以后这里文件指针已经指到body的开始地址了。
			if (i == 0)
				break;
			i = 0;
		} else {
			i++;
		}
		strncat(conn->headers, s, MAX_QUERY - strlen(conn->headers) - 1);
	}

#ifdef DEBUG
	fprintf(stderr, "--- Reply headers ---\n%s--- End of headers ---\n",
		conn->headers);
#endif

	sscanf(conn->headers, "%*s %3i", &conn->status);
	s2 = strchr(conn->headers, '\n');
	*s2 = 0;
	strcpy(conn->request, conn->headers);
	*s2 = '\n';

	return 1;
}

const char *
http_header(const http_t *conn, const char *header)
{
	const char *p = conn->headers;
	size_t hlen = strlen(header);

	do {
		if (strncasecmp(p, header, hlen) == 0)
			return p + hlen;
		while (*p != '\n' && *p)
			p++;
		if (*p == '\n')
			p++;
	} while (*p);

	return NULL;
}

//用来指明发送给接收方的消息主体的大小
long long int
http_size(http_t *conn)
{
	const char *i;
	long long int j;

	if ((i = http_header(conn, "Content-Length:")) == NULL)
		return -2;

	sscanf(i, "%lld", &j);
	return j;
}

//Content-Range: bytes 200-1000/67589  http resp header中Content-Range表示返回的数据包在整个文件中的位置. start=200, end=1000, 文件总大小67589
//从resp header中获取文件总大小
long long int
http_size_from_range(http_t *conn)
{
	const char *i;
	long long int j;

	if ((i = http_header(conn, "Content-Range:")) == NULL)
		return -2;

	i = strchr(i, '/');
	if (i == NULL)
		return -2;

	if (sscanf(i + 1, "%lld", &j) != 1)
		return -3;

	return j;
}

//从http resp header  Content-Disposition 中取出 文件名 // https://developer.mozilla.org/zh-CN/docs/Web/HTTP/Headers/Content-Disposition
void
http_filename(const http_t *conn, char *filename)
{
	const char *h;
	if ((h = http_header(conn, "Content-Disposition:")) != NULL) {
		sscanf(h, "%*s%*[ \t]filename%*[ \t=\"\'-]%254[^\n\"\' ]", filename);

		/* Replace common invalid characters in filename
		   https://en.wikipedia.org/wiki/Filename#Reserved_characters_and_words */
		char *i = filename;
		const char *invalid_characters = "/\\?%*:|<>";
		const char replacement = '_';
		while ((i = strpbrk(i, invalid_characters)) != NULL) {
			*i = replacement;
			i++;
		}
	}
}

inline static char
decode_nibble(char n)
{
	if (n <= '9')
		return n - '0';
	if (n >= 'a')
		n -= 'a' - 'A';
	return n - 'A' + 10;
}

inline static char
encode_nibble(char n)
{
	return n > 9 ? n + 'a' - 10 : n + '0';
}

inline static void
encode_byte(char dst[3], char n)
{
	*dst++ = '%';
	*dst++ = encode_nibble(n >> 4);
	*dst = encode_nibble(n & 15);
}

/* Decode%20a%20file%20name */
void
http_decode(char *s)
{
	for (; *s && *s != '%'; s++) ;
	if (!*s)
		return;

	char *p = s;
	do {
		if (!s[1] || !s[2])
			break;
		*p++ = (decode_nibble(s[1]) << 4) | decode_nibble(s[2]);
		s += 3;
		while (*s && *s != '%')
			*p++ = *s++;
	} while (*s == '%');
	*p = 0;
}

void
http_encode(char *s)
{
	char t[MAX_STRING];
	int i, j;

	for (i = j = 0; s[i]; i++, j++) {
		/* Fix buffer overflow */
		if (j >= MAX_STRING - 1) {
			break;
		}

		t[j] = s[i];
		if (s[i] <= 0x20 || s[i] >= 0x7f) {
			/* Fix buffer overflow */
			if (j >= MAX_STRING - 3) {
				break;
			}

			encode_byte(t + j, s[i]);
			j += 2;
		}
	}
	t[j] = 0;

	strcpy(s, t);
}
