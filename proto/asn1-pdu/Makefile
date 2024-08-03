LPM_DIR=../../../libprotobuf-mutator
PROTOBUF_DIR=$(LPM_DIR)/build/external.protobuf
FLAGS=-O2 -Wall -Wextra -D_FORTIFY_SOURCE=2 -fstack-protector-strong -fstack-clash-protection -fPIE -fPIC -Wl,-z,relro -Wl,-z,now -Wl,-z,noexecstack -Wl,-z,separate-code

all: proto cpp

proto: *.proto
	protoc *.proto --python_out=. --cpp_out=.

cpp: *.h *.cc
	$(CXX) -c $(FLAGS) *.cc -I$(LPM_DIR) -I$(PROTOBUF_DIR)/include

clean:
	$(RM) *.o
