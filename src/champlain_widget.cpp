/*
 * Copyright (C) 2008 Pierre-Luc Beaudoin <pierre-luc@squidy.info>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "config.h"

#include "champlain.h"
#include "champlain_widget.h"

#include <stdio.h>

int main(int argc, char* argv[])
{
   printf("Hello, world!\n");

   return 0;
}

enum {
    /* normal signals */
    TBD,
    LAST_SIGNAL
};

enum {
    PROP_0,

    PROP_TBD
};

static guint champlain_widget_signals[LAST_SIGNAL] = { 0, };

G_DEFINE_TYPE(ChamplainWidget, champlain_widget, GTK_TYPE_CONTAINER)

