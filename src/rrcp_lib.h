/*
    This file is part of openrrcp

    OpenRRCP is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    OpenRRCP is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with OpenRRCP; if not, write to the Free Software
    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

    ---

    Some support can be found at: http://openrrcp.org.ru/
*/

struct t_str_number_list {
   const char *list;
   const char *cur_p;
   int last_val;
   int range_max;
};

int str_portlist_to_array(char *list,unsigned short int *arr,unsigned int arrlen);

int str_number_list_init(const char *str, struct t_str_number_list *list);
int str_number_list_get_next(struct t_str_number_list *list, int *val);

int parse_switch_id(const char *str);

