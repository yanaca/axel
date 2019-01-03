/*
  Axel -- A lighter download accelerator for Linux and other Unices

  Copyright 2001-2007 Wilmer van der Gaast
  Copyright 2008      Philipp Hagemeister
  Copyright 2008      Y Giridhar Appaji Nag
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

/* Configuration handling include file */

#ifndef AXEL_CONF_H
#define AXEL_CONF_H

typedef struct {
	char default_filename[MAX_STRING]; //默认文件名, 例如下载www.baidu.com/index/ 这种时候需要默认文件名
	char http_proxy[MAX_STRING]; //proxy数组; 如果HTTP_PROXY环境变量有设置的话则不需要再设置这个配置
	char no_proxy[MAX_STRING]; //不走代理的host;配置文件中用逗号分隔,host不需要精确匹配
	uint16_t num_connections; //连接数;不建议设置太高,理论上4个即可
	int strip_cgi_parameters; //是否将CGI参数保留在本地文件名中,默认1
	int save_state_interval; //保存st文件的频率,默认每隔10s保存.如果设置为0则不保存；保存的目的是为了crash续传
	int connection_timeout; //socket连接超时时间,默认45s。采用的是非阻塞套接字+select控制超时模式
	int reconnect_delay; //重连间隔
	int max_redirect; //最多重定向次数,默认20
	int buffer_size; //从一个conn中单次读取的最大字节数;buffer设置较小时速率控制会较为精确但是作者建议设置大一些
	int max_speed; //最大下载速度, 正负5%的偏差
	int verbose; //是否打印更多信息到控制台
	int alternate_output; //默认是类似wget模式输出,如果设置为0则使用scp模式输出
	int insecure;
	int no_clobber; //output目录有同名下载文件且找不到st文件时，no_clobber为1则跳过本次下载。否则会自动下载并保存为 x.2类似格式的文件

	if_t *interfaces; //网卡名数组,如果一个机器有多个网卡,则支持列出这些网卡,然后通过多网卡进行下载。否则默认则从路由表中拿第一个匹配的本地网卡ip。

	sa_family_t ai_family;

	int search_timeout;
	int search_threads;
	int search_amount;
	int search_top;

	unsigned io_timeout;

	int add_header_count; //header数量
	char add_header[MAX_ADD_HEADERS][MAX_STRING]; //header二维数组

	char user_agent[MAX_STRING]; //ua
} conf_t;

int conf_loadfile(conf_t *conf, char *file);
int conf_init(conf_t *conf);
void conf_free(conf_t *conf);

#endif				/* AXEL_CONF_H */
