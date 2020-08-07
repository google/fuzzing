#ifndef CERT_PROTO_CONVERTER_H_
#define CERT_PROTO_CONVERTER_H_

#include <stdint.h>

#include <vector>

#include "asn1_universal_types_to_der.h"
#include "x509_certificate.pb.h"

namespace x509_certificate {

// Encodes |X509_certificate| to DER, returning the encoded bytes in |der_|.
std::vector<uint8_t> X509CertificateToDER(
    const X509Certificate& X509_certificate);

// Encodes a |pdu| if |t| contains one; otherwise, encodes the value belonging
// to |t|.
template <typename T>
void Encode(const T& t, std::vector<uint8_t>& der) {
  if (t.has_pdu()) {
    Encode(t.pdu(), der);
    return;
  }
  Encode(t.value(), der);
}

// Encodes the |TYPE| found in X509 Certificates and writes the results to
// |der|.
#define DECLARE_ENCODE_FUNCTION(TYPE) \
  template <>                         \
  void Encode<TYPE>(const TYPE& val, std::vector<uint8_t>& der)

DECLARE_ENCODE_FUNCTION(TBSCertificateSequence);
DECLARE_ENCODE_FUNCTION(VersionNumber);
DECLARE_ENCODE_FUNCTION(ValiditySequence);
DECLARE_ENCODE_FUNCTION(TimeChoice);
DECLARE_ENCODE_FUNCTION(SubjectPublicKeyInfoSequence);
DECLARE_ENCODE_FUNCTION(AlgorithmIdentifierSequence);
DECLARE_ENCODE_FUNCTION(asn1_pdu::PDU);

}  // namespace x509_certificate

#endif