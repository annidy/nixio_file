/*
 * nixio - Linux I/O library for lua
 *
 *   Copyright (C) 2009 Steven Barth <steven@midlink.org>
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

#include "nixio.h"
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <signal.h>

#define VERSION 0.3


/* pushes nil, error number and errstring on the stack */
int nixio__perror(lua_State *L) {
	if (errno == EAGAIN || errno == EWOULDBLOCK) {
		lua_pushboolean(L, 0);
	} else {
		lua_pushnil(L);
	}
    lua_pushinteger(L, errno);
    lua_pushstring(L, strerror(errno));
    return 3;
}

/* pushes true, if operation succeeded, otherwise call nixio__perror */
int nixio__pstatus(lua_State *L, int condition) {
	if (condition) {
		lua_pushboolean(L, 1);
		return 1;
	} else {
		return nixio__perror(L);
	}
}

/* return any possible fp, otherwise error out */
FILE *nixio__checkfp(lua_State *L, int ud) {
	FILE *fp = nixio__tofp(L, ud);
	if (fp == NULL) luaL_argerror(L, ud, "invalid file descriptor");
	return fp; 
}

/* return any possible fp */
FILE *nixio__tofp(lua_State *L, int ud) {
	void *udata = lua_touserdata(L, ud);
	return *((FILE **)udata);
}

/* An empty iterator */
int nixio__nulliter(lua_State *L) {
	lua_pushnil(L);
	return 1;
}

static int nixio_errno(lua_State *L) {
	lua_pushinteger(L, errno);
	return 1;
}

static int nixio_strerror(lua_State *L) {
	lua_pushstring(L, strerror(luaL_checkinteger(L, 1)));
	return 1;
}

/* object table */
static const luaL_reg nixio_reg[] = {
	{"errno",		nixio_errno},
	{"strerror",	nixio_strerror},
	{NULL,			NULL}
};

/* entry point */
NIXIO_API int luaopen_nixio(lua_State *L) {

	/* register methods */
	nixio_open_file(L);

	return 1;
}
