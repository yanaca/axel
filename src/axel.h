/*
  Axel -- A lighter download accelerator for Linux and other Unices

  Copyright 2001-2007 Wilmer van der Gaast
  Copyright 2008      Y Giridhar Appaji Nag
  Copyright 2008-2009 Philipp Hagemeister
  Copyright 2015-2017 Joao Eriberto Mota Filho
  Copyright 2016      Stephen Thirlwall
  Copyright 2017      Antonio Quartulli
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

/* Main include file */

#ifndef AXEL_AXEL_H
#define AXEL_AXEL_H

#include "config.h"

#include <time.h>
#include <ctype.h>
#include <fcntl.h>
#include <errno.h>
#include <stdio.h>
#ifndef	NOGETOPTLONG
#define _GNU_SOURCE
#include <getopt.h>
#endif
#include <limits.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <stdarg.h>
#include <stdbool.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <netinet/in_systm.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <pthread.h>

/* Internationalization */
#ifdef ENABLE_NLS
#define _(x)			gettext(x)
#include <libintl.h>
#include <locale.h>
#else
#define _(x)			(x)
#endif

/* Compiled-in settings */
#define MAX_STRING		1024
#define MAX_ADD_HEADERS	10
#define MAX_REDIRECT		20
#define DEFAULT_IO_TIMEOUT	120
#define DEFAULT_USER_AGENT	"Axel/" VERSION " (" ARCH ")"

typedef struct {
	void *next;
	char text[MAX_STRING];
} message_t;

typedef message_t url_t;
typedef message_t if_t;

#include "conf.h"
#include "tcp.h"
#include "ftp.h"
#include "http.h"
#include "conn.h"
#include "ssl.h"
#include "search.h"

#define min(a, b)		((a) < (b) ? (a) : (b))
#define max(a, b)		((a) > (b) ? (a) : (b))

typedef struct {
	conn_t *conn;	//conn_t数组,保存了所有连接的信息
	conf_t *conf; 	//conf_t数组,保存了所有连接的配置
	char filename[MAX_STRING];	//输出文件的文件名,多线程同事写一个文件(lseek+write)加锁
	double start_time;	//开始时间
	int next_state; //下一次记录state文件的时间
	int finish_time; //按照当前下载速度计算出来的预计下载完成时间
	long long bytes_done, start_byte, size; //已经下载的字节数,开始字节,总字节数
	int bytes_per_second; //每秒下载的字节数
	struct timespec delay_time;
	int outfd; //输出文件的fd
	int ready; //是否完成,0未完成,1已下载完成,-1出现异常下载中断
	message_t *message, *last_message; //errmsg
	url_t *url; //下载的url信息
} axel_t;

axel_t *axel_new(conf_t *conf, int count, const void *url); //会发起一次http调用将conn需要的信息都填补完整
int axel_open(axel_t *axel);
void axel_start(axel_t *axel);
void axel_do(axel_t *axel);
void axel_close(axel_t *axel);
void print_messages(axel_t *axel);

double gettime();

#endif				/* AXEL_AXEL_H */
