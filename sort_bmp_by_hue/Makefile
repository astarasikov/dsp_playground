APPNAME=sort_colors

all:
	make clean
	clang++ -O2 -Wall -Wextra -Wpedantic -o $(APPNAME) -std=c++11 sort_colors.cpp
	./$(APPNAME) B1.bmp out.bmp
	open out.bmp || xdg-open out.bmp

clean:
	rm $(APPNAME) *.o out.bmp || true
