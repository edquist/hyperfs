# SRCS.program = prog.cpp progsrc2.cpp progsrc3.cpp ...

LIB.SRCS      = connor.c getrange.c cheddar.c \
                ymdhms.c month_idx.c date_parse.c drainf.c logger.c

SRCS.getrange = getrange-main.c $(LIB.SRCS)

SRCS.hyperfs  = hyperfs-main.c hyperfs-cache.c hyperfs-ops-getattr.c $(LIB.SRCS)

