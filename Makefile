.PHONY:	clean all

all: 
	g++  -g  Test/*.cpp Core/*.cxx  Core/*.cpp -lm -lpthread  -luuid -o securityToken

clean:
	rm -rf securityToken



