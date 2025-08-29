zig cc -g -O1 src/main.c -o main
zig cc -g -O1 src/solution-703.c -o solution-703

zig cc src/window.c \
  -framework Cocoa \
  -framework CoreFoundation \
  -lobjc \
  -o window
