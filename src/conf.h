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
	char default_filename[MAX_STRING]; //Ĭ���ļ���, ��������www.baidu.com/index/ ����ʱ����ҪĬ���ļ���
	char http_proxy[MAX_STRING]; //proxy����; ���HTTP_PROXY�������������õĻ�����Ҫ�������������
	char no_proxy[MAX_STRING]; //���ߴ����host;�����ļ����ö��ŷָ�,host����Ҫ��ȷƥ��
	uint16_t num_connections; //������;����������̫��,������4������
	int strip_cgi_parameters; //�Ƿ�CGI���������ڱ����ļ�����,Ĭ��1
	int save_state_interval; //����st�ļ���Ƶ��,Ĭ��ÿ��10s����.�������Ϊ0�򲻱��棻�����Ŀ����Ϊ��crash����
	int connection_timeout; //socket���ӳ�ʱʱ��,Ĭ��45s�����õ��Ƿ������׽���+select���Ƴ�ʱģʽ
	int reconnect_delay; //�������
	int max_redirect; //����ض������,Ĭ��20
	int buffer_size; //��һ��conn�е��ζ�ȡ������ֽ���;buffer���ý�Сʱ���ʿ��ƻ��Ϊ��ȷ�������߽������ô�һЩ
	int max_speed; //��������ٶ�, ����5%��ƫ��
	int verbose; //�Ƿ��ӡ������Ϣ������̨
	int alternate_output; //Ĭ��������wgetģʽ���,�������Ϊ0��ʹ��scpģʽ���
	int insecure;
	int no_clobber; //outputĿ¼��ͬ�������ļ����Ҳ���st�ļ�ʱ��no_clobberΪ1�������������ء�������Զ����ز�����Ϊ x.2���Ƹ�ʽ���ļ�

	if_t *interfaces; //����������,���һ�������ж������,��֧���г���Щ����,Ȼ��ͨ���������������ء�����Ĭ�����·�ɱ����õ�һ��ƥ��ı�������ip��

	sa_family_t ai_family;

	int search_timeout;
	int search_threads;
	int search_amount;
	int search_top;

	unsigned io_timeout;

	int add_header_count; //header����
	char add_header[MAX_ADD_HEADERS][MAX_STRING]; //header��ά����

	char user_agent[MAX_STRING]; //ua
} conf_t;

int conf_loadfile(conf_t *conf, char *file);
int conf_init(conf_t *conf);
void conf_free(conf_t *conf);

#endif				/* AXEL_CONF_H */
