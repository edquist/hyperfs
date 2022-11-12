#pragma once

struct ymd {
	int year;
	int month;
	int day;
};

struct hms {
	int hour;
	int min;
	int sec;
};

struct ymdhmsz {
	struct ymd dt;
	struct hms tm;
	int zz;
};

int daynum(struct ymd *dt);
int timeofday(struct hms *tm);
long epoch(struct ymdhmsz *ts);


/* vim: set noexpandtab sts=0 sw=8 ts=8: */

