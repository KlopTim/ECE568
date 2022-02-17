main: main.o Parser.o Request.o Response.o abstract.o
	g++ -o main main.o Parser.o Request.o Response.o abstract.o

main.o: main.cpp
	g++ -c main.cpp

Parser.o: Parser.cpp Parser.hpp
	g++ -c Parser.cpp

abstract.o: abstract.cpp abstract.hpp
	g++ -c abstract.cpp


Request.o: Request.cpp Request.hpp
	g++ -c Request.cpp

Response.o: Response.cpp Response.hpp
	g++ -c Response.cpp

.PHONY:
	clean
clean:
	rm -rf *.o main