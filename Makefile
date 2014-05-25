#########################################################################
#
# Copyright 2014 by Sean Conner.  All Rights Reserved.
#
# This library is free software; you can redistribute it and/or modify it
# under the terms of the GNU Lesser General Public License as published by
# the Free Software Foundation; either version 3 of the License, or (at your
# option) any later version.
#
# This library is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
# or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public
# License for more details.
#
# You should have received a copy of the GNU Lesser General Public License
# along with this library; if not, see <http://www.gnu.org/licenses/>.
#
# Comments, questions and criticisms can be sent to: sean@conman.org
#
########################################################################

CC = gcc -std=c99 -Wall -Wextra -pedantic
CFLAGS = -g
LDFLAGS = -g
LDLIBS = 

.PHONE: all clean

all : zipf zipw.so zipr.so mz.so
clean:
	$(RM) -rf zipf zipw.so zipr.so mz.so *~

zipf :

zipw.so : zipwlua.c lem.h
	$(CC) $(CFLAGS) -shared -fPIC -o $@ $<

zipr.so : ziprlua.c lem.h
	$(CC) $(CFLAGS) -shared -fPIC -o $@ $<

mz.so : mz.c
	$(CC) $(CFLAGS) -shared -fPIC -o $@ $< -lz
