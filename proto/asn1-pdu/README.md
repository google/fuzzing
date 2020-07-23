# ASN.1 PDU Protobuf
This [protobuf](https://developers.google.com/protocol-buffers) represents a
DER-encoded ASN.1 PDU, defined in X.690 (2015). When used with
[libprotobuf-mutator](https://github.com/google/libprotobuf-mutator), the protobuf can be used
to produce valid DER sequences, as well as syntactically invalid (e.g. BER) and structurally
invalid (e.g. random data) inputs for a fuzzer.

## How to use it
Example fuzz targets that use this proto can be seen here: https://github.com/google/oss-fuzz/pull/4179.
