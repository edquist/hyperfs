# SRCS.program = prog.cpp progsrc2.cpp progsrc3.cpp ...

LIB.SRCS      = connor.c getrange.c cheddar.c escape.c \
                ymdhms.c month_idx.c date_parse.c drainf.c loggo.c

SRCS.getrange = getrange-main.c $(LIB.SRCS)

SRCS.hyperfs  = hyperfs-main.c hyperfs-cache.c hyperfs-ops.c $(LIB.SRCS) \
                hyperfs-ops-getattr.c hyperfs-ops-open.c hyperfs-ops-read.c \
                hyperfs-ops-readlink.c hyperfs-ops-readdir.c hyperfs-get.c

SRCS.escape   = escape-main.c escape.c
SRCS.unescape = unescape-main.c escape.c

