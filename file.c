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
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

static int nixio_open(lua_State *L) {
	const char *filename = luaL_checklstring(L, 1, NULL);
	const char *str = luaL_optstring(L, 2, "r");
	int *udata;
	FILE *fp;

	if ((fp = fopen(filename, str)) == 0) {
		return 0;
	}
	udata = lua_newuserdata(L, sizeof(int));
	if (!udata) {
		return 0;
	}

	*udata = (int)fp;

	luaL_getmetatable(L, NIXIO_FILE_META);
	lua_setmetatable(L, -2);

	return 1;
}

static int nixio_file_write(lua_State *L) {
	FILE *fp = nixio__checkfp(L, 1);
	size_t len;
	size_t sent = 0;
	const char *data = luaL_checklstring(L, 2, &len);


	do {
		sent += fwrite(data+sent, 1, len-sent, fp);
	} while(sent < len && errno == EINTR);

	if (sent >= 0) {
		lua_pushinteger(L, sent);
		return 1;
	} else {
		return 0;
	}
}

static int nixio_file_read(lua_State *L) {
	FILE *fp = nixio__checkfp(L, 1);
	static char *buffer = NULL;
	static size_t buflen = 0;

	unsigned int req = luaL_checkinteger(L, 2);
	size_t readc = 0;

	if (req > buflen) {
		buflen = req * 2;
		if (buffer) { free(buffer); }
		buffer = (char*)malloc(buflen);
	}

	do {
		readc += fread(buffer+readc, 1, req-readc, fp);
	} while(readc < req && errno == EINTR);

	if (readc <= 0) {
		return 0;
	} else {
		lua_pushlstring(L, buffer, readc);
		return 1;
	}
}


static int nixio_file_seek(lua_State *L) {
	FILE *fp = nixio__checkfp(L, 1);
	const char *whstr = luaL_optlstring(L, 2, "set", NULL);
	int64_t off = (int64_t)luaL_checknumber(L, 3);
	int64_t offr;
	int whence;
	if (!strcmp(whstr, "set")) {
		whence = SEEK_SET;
	} else if (!strcmp(whstr, "cur")) {
		whence = SEEK_CUR;
	} else if (!strcmp(whstr, "end")) {
		whence = SEEK_END;
	} else {
		return luaL_argerror(L, 3, "supported values: set, cur, end");
	}
	offr = _fseeki64(fp, off, whence);
	if (offr < 0) {
		return 0;
	} else {
		lua_pushnumber(L, _ftelli64(fp));
		return 1;
	}
}

static int nixio_file_tell(lua_State *L) {
	FILE *fp = nixio__checkfp(L, 1);
	int64_t pos = _ftelli64(fp);
	if (pos < 0) {
		return 0;
	} else {
		lua_pushnumber(L, pos);
		return 1;
	}
}

static int nixio_file_close(lua_State *L) {
	FILE **fpp = luaL_checkudata(L, 1, NIXIO_FILE_META);
	fclose(*fpp);
	*fpp = NULL;
	return 0;
}

static int nixio_file__gc(lua_State *L) {
	FILE **fpp = luaL_checkudata(L, 1, NIXIO_FILE_META);
	if (*fpp != NULL) {
		fclose(*fpp);
		*fpp = NULL;
	}
	return 0;
}

/**
 * string representation
 */
static int nixio_file__tostring(lua_State *L) {
	lua_pushfstring(L, "nixio file %d", nixio__tofp(L, 1));
	return 1;
}

/* method table */
static const luaL_reg M[] = {
	{"write",		nixio_file_write},
	{"read",		nixio_file_read},
	{"tell",		nixio_file_tell},
	{"seek",		nixio_file_seek},
	{"close",		nixio_file_close},
	{"__gc",		nixio_file__gc},
	{"__tostring",	nixio_file__tostring},
	{NULL,			NULL}
};

/* module table */
static const luaL_reg R[] = {
	{"open",		nixio_open},
	{NULL,			NULL}
};

void nixio_open_file(lua_State *L) {

	luaL_newmetatable(L, NIXIO_FILE_META);
	luaL_register(L, NULL, M);
	lua_pushvalue(L, -1);
	lua_setfield(L, -2, "__index");

	luaL_register(L, NULL, R);
}
