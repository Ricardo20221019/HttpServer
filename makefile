CXX ?= g++

server: main.cpp ./myserver/myserver.cpp ./threadpool/threadpool.cpp ./event/myevent.cpp ./utils/utils.cpp
	$(CXX) -std=c++11  $^ -lpthread  -o server

clean:
	rm  -r server
