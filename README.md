# RSS

RSS is an interactive room scheduling system built using Ncurses in seek of room management and availability directly from the terminal

Compiling with windows: gcc -Isrc/utils -I/msys64/mingw64/include -o RSS src/about/about.c src/admin/admin.c src/global/global.c src/main/main.c src/user/user.c -lncurses -L/mingw64/bin -DNCURSES_STATIC