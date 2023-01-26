#include <fstream>
#include <openssl/evp.h>
#include "x509_certificate.pb.h"

#include "asn1_pdu_to_der.h"
#include "x509_certificate_to_der.h"

std::string base64_encode(const unsigned char *buf, int len) {
    const auto expected_size = 4 * ((len + 2) / 3);
    auto output = reinterpret_cast<char *>(calloc(expected_size + 1, 1));
    const auto actual_size = EVP_EncodeBlock(reinterpret_cast<unsigned char *>(output), buf, len);
    if (expected_size != actual_size) {
        std::cerr << "Wrong base64 output length: expected " << expected_size << " but got " << actual_size << ".\n";
    }
    return output;
}

std::string readFile(const char* path) {
    printf("Reading file at %s\n", path);
    std::ifstream file(path, std::ios::in | std::ios::binary);
    std::string str = std::string((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    printf("Read %ld bytes\n\n", str.length());
    return str;
}

void writeFile(const char* path, std::vector<uint8_t> data) {
    printf("Writing %ld bytes to %s\n", data.size(), path);
    std::ofstream file(path, std::ios::out | std::ios::binary);
    std::copy(data.begin(), data.end(), std::ostreambuf_iterator<char>(file));
}

int main(int argc, char **argv) {
    if (argc != 3) {
        printf("Usage: ./protobuf_to_der.o <in file> <out file>\n");
        return 1;
    }

    std::string protobuf = readFile(argv[1]);
    std::string b64Protobuf = base64_encode((unsigned char *)protobuf.c_str(), protobuf.length());
    printf("Protobuf: %s\n\n", b64Protobuf.c_str());

    x509_certificate::X509Certificate input;
    bool parse_ok = input.ParseFromArray(protobuf.c_str(), protobuf.length());
    if (!parse_ok) {
        printf("ParseFromArray failed!\n");
        return 1;
    }

    std::string serialized;
    input.SerializeToString(&serialized);
    std::string b64Serialized = base64_encode((unsigned char *)serialized.c_str(), serialized.length());
    printf("Re-serialized protobuf: %s\n\n", b64Serialized.c_str());

    std::vector<uint8_t> asn1 = x509_certificate::X509CertificateToDER(input);

    std::string b64asn1 = base64_encode(asn1.data(), asn1.size());
    printf("ASN.1: %s\n", b64asn1.c_str());

    writeFile(argv[2], asn1);

    return 0;
}
