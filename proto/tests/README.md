# Proto tests

## Setup

Clone and build `libprotobuf-mutator`:

```sh
$ git clone https://github.com/google/libprotobuf-mutator.git libprotobuf-mutator
$ cd libprotobuf-mutator
$ mkdir build && cd build
$ cmake .. -GNinja -DLIB_PROTO_MUTATOR_DOWNLOAD_PROTOBUF=ON -DLIB_PROTO_MUTATOR_TESTING=OFF -DCMAKE_BUILD_TYPE=Release
$ ninja
```

Clone `google-fuzzing` and build test files:

```
$ git clone https://github.com/google/fuzzing.git fuzzing
$ cd fuzzing/proto/tests
$ make
```

## Run

Generate python files:

```sh
$ protoc ../asn1-pdu/*.proto --python_out=../asn1-pdu
```

Generate a test certificate and convert it to the DER format:

```sh
$ openssl req -nodes -new -x509 -keyout key.pem -out cert.pem
$ openssl x509 -in cert.pem -out cert.der -outform DER
```

Convert it to protobuf:

```sh
$ ./x509_to_protobuf.py cert.pem cert.proto
```

Convert the protobuf back to DER:

```sh
$ ./protobuf_to_der.o cert.proto cert-2.der
```

Make sure OpenSSL can properly parse the certificate and compare it to the
initial one:

```
$ openssl x509 -inform DER -in cert-2.der -noout -text
$ openssl x509 -inform DER -in cert.der -noout -text
```
