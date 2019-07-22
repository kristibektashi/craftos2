#include "fs_handle.h"
#include "lib.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

int fs_handle_close(lua_State *L) {
    fclose((FILE*)lua_touserdata(L, lua_upvalueindex(1)));
    return 0;
}

char checkChar(char c) {
    if (c == 127) return '?';
    if ((c >= 32 && c < 127) || c == '\n' || c == '\t' || c == '\r') return c;
    else return '?';
}

int fs_handle_readAll(lua_State *L) {
    FILE * fp = (FILE*)lua_touserdata(L, lua_upvalueindex(1));
    if (feof(fp)) return 0;
    long pos = ftell(fp);
    fseek(fp, 0, SEEK_END);
    long size = ftell(fp) - pos;
    char * retval = (char*)malloc(size + 1);
    memset(retval, 0, size + 1);
    fseek(fp, pos, SEEK_SET);
    int i;
    for (i = 0; !feof(fp) && i < size; i++)
        retval[i] = checkChar(fgetc(fp));
    lua_pushstring(L, retval);
    free(retval);
    return 1;
}

int fs_handle_readLine(lua_State *L) {
    FILE * fp = (FILE*)lua_touserdata(L, lua_upvalueindex(1));
    if (feof(fp) || ferror(fp)) {
        lua_pushnil(L);
        return 1;
    }
    long size = 0;
    long lastpos = ftell(fp);
    while (fgetc(fp) != '\n' && !ferror(fp) && !feof(fp)) size++;
    long p = ftell(fp);
    fseek(fp, lastpos, SEEK_SET);
    char * retval = (char*)malloc(size);
    for (int i = 0; i < size; i++) retval[i] = checkChar(fgetc(fp));
    fgetc(fp);
    assert(ftell(fp) == p);
    lua_pushlstring(L, retval, size);
    free(retval);
    return 1;
}

int fs_handle_readChar(lua_State *L) {
    FILE * fp = (FILE*)lua_touserdata(L, lua_upvalueindex(1));
    if (feof(fp)) return 0;
    char retval[2];
    retval[0] = checkChar(fgetc(fp));
    lua_pushstring(L, retval);
    return 1;
}

int fs_handle_readByte(lua_State *L) {
    FILE * fp = (FILE*)lua_touserdata(L, lua_upvalueindex(1));
    if (feof(fp)) return 0;
    char retval = fgetc(fp);
    lua_pushinteger(L, retval);
    return 1;
}

int fs_handle_writeString(lua_State *L) {
    if (!lua_isstring(L, 1)) bad_argument(L, "string", 1);
    const char * str = lua_tostring(L, 1);
    FILE * fp = (FILE*)lua_touserdata(L, lua_upvalueindex(1));
    fwrite(str, strlen(str), 1, fp);
    return 0;
}

int fs_handle_writeLine(lua_State *L) {
    if (!lua_isstring(L, 1)) bad_argument(L, "string", 1);
    const char * str = lua_tostring(L, 1);
    FILE * fp = (FILE*)lua_touserdata(L, lua_upvalueindex(1));
    fwrite(str, strlen(str), 1, fp);
    fputc('\n', fp);
    return 0;
}

int fs_handle_writeByte(lua_State *L) {
    if (!lua_isnumber(L, 1)) bad_argument(L, "number", 1);
    const char b = lua_tointeger(L, 1) & 0xFF;
    FILE * fp = (FILE*)lua_touserdata(L, lua_upvalueindex(1));
    fputc(b, fp);
    return 0;
}

int fs_handle_flush(lua_State *L) {
    fflush((FILE*)lua_touserdata(L, lua_upvalueindex(1)));
    return 0;
}