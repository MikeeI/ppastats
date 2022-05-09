/*
 * Copyright (C) 2011-2015 jeanfi@gmail.com
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301 USA
 */

#ifndef _PPASTATS_LIST_H_
#define _PPASTATS_LIST_H_

/*
 * Convenience functions for manipulating null-terminated list of
 * pointers.
 */

/* Returns the number of items in the list. */
int list_length(void **list);

/*
 * Adds item into a list.
 *
 * Returns a new allocated list containing all items.
 */
void **list_add(void **list, void *new_item);

/*
 * Appends all items of two lists.
 *
 * Returns a new allocated list containing all items.
 */
void **list_append_list(void **list1, void **list2);

#endif
