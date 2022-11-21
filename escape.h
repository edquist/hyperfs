#pragma once

//int needs_escape(unsigned char c)
int string_needs_escape(const char *s);
char *escape_raw(char *dest, const char *raw);
char *decode_escaped(char *dest, const char *s);


/* vim: set noexpandtab sts=0 sw=8 ts=8: */

