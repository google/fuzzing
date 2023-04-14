LPM_DIR=../../../libprotobuf-mutator
PROTOBUF_DIR=$(LPM_DIR)/build/external.protobuf
ASN1_PROTOBUF_DIR=../asn1-pdu
FLAGS=-O2 -Wall -Wextra -D_FORTIFY_SOURCE=2 -fstack-protector-strong -fstack-clash-protection -fPIE -fPIC -Wl,-z,relro -Wl,-z,now -Wl,-z,noexecstack -Wl,-z,separate-code

all: protobuf tests

protobuf:
	$(MAKE) -C $(ASN1_PROTOBUF_DIR) all

tests: protobuf_to_der.cpp
	$(CXX) $(FLAGS) -o protobuf_to_der.o protobuf_to_der.cpp $(ASN1_PROTOBUF_DIR)/*.o -I$(PROTOBUF_DIR)/include -I$(LPM_DIR) -I$(ASN1_PROTOBUF_DIR) $(LPM_DIR)/build/src/libfuzzer/libprotobuf-mutator-libfuzzer.a $(LPM_DIR)/build/src/libprotobuf-mutator.a $(PROTOBUF_DIR)/lib/libprotobuf.a -lcrypto

clean:
	$(RM) *.o
	$(MAKE) -C $(ASN1_PROTOBUF_DIR) clean
