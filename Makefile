all: mkdirectory run

FLAGS = -Wshadow -Winit-self -Wredundant-decls -Wcast-align -Wundef -Wfloat-equal -Winline -Wunreachable-code -Wmissing-declarations 		\
		-Wmissing-include-dirs -Wswitch-enum -Wswitch-default -Weffc++ -Wmain -Wextra -Wall -g -pipe -fexceptions -Wcast-qual -Wconversion	\
		-Wctor-dtor-privacy -Wempty-body -Wformat-security -Wformat=2 -Wignored-qualifiers -Wlogical-op -Wmissing-field-initializers		\
		-Wnon-virtual-dtor -Woverloaded-virtual -Wpointer-arith -Wsign-promo -Wstack-usage=8192 -Wstrict-aliasing -Wstrict-null-sentinel  	\
		-Wtype-limits -Wwrite-strings -D_DEBUG -D_EJUDGE_CLIENT_SIDE

SFML_FLAGS = -lsfml-graphics -lsfml-window -lsfml-system -lsfml-audio


run: 	 obj/generals.o obj/log_errors.o obj/alphaBlender.o obj/main.o obj/draw.o
	g++  obj/generals.o obj/log_errors.o obj/alphaBlender.o obj/main.o obj/draw.o -o run  $(SFML_FLAGS)


obj/main.o: main.cpp config.h
		g++ main.cpp -c -o obj/main.o $(FLAGS)


obj/alphaBlender.o: config.h alphaBlender/alphaBlender.h alphaBlender/alphaBlender.cpp  
				g++ alphaBlender/alphaBlender.cpp -c -o obj/alphaBlender.o -O2 -mavx2 -msse4 $(FLAGS)


obj/log_errors.o: src/log_info/log_errors.h src/log_info/log_errors.cpp
			  g++ src/log_info/log_errors.cpp -c -o obj/log_errors.o $(FLAGS)

obj/generals.o: src/generals_func/generals.cpp src/generals_func/generals.h
			g++ src/generals_func/generals.cpp -c -o obj/generals.o $(FLAGS)


obj/draw.o: src/draw/draw.cpp src/draw/draw.h
		g++ src/draw/draw.cpp -c -o obj/draw.o $(FLAGS)


.PHONY: cleanup mkdirectory

mkdirectory:
	mkdir -p obj;
	mkdir -p temp;
	

cleanup:
	rm obj/*.o