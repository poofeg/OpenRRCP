/*
    This file is part of openrrcp

    pcapsipdump is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    pcapsipdump is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Foobar; if not, write to the Free Software
    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

    ---

    You can send your updates, patches and suggestions on this software
    to it's original author, Andrew Chernyak (nording@yandex.ru)
    This would be appreciated, however not required.
*/

//chip types
#define unknown 0
#define rtl8316b 1
#define rtl8326 2

struct switchtype_t {
	char		*vendor;
	char		*model;
	char		*hw_rev;
	char		*chip_name;
	unsigned int	chip_id;
	unsigned int	num_ports;
	unsigned int	port_order[27];
};

struct t_rtl83xx_port_link_status {
	unsigned char	speed:2,
  		duplex:1,
  		reserved:1,
  		link:1,
  		flow_control:1,
  		asy_pause:1,
  		auto_negotiation:1;
};

extern struct switchtype_t switchtypes[5];

extern unsigned int switchtype;