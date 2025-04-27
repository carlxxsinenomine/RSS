srcs = src/about/about.c src/admin/admin.c src/global/global.c src/main/main.c src/user/user.c
I = -Isrc/utils

all:
	gcc $(I) -o RSS $(srcs) -lncurses
