// Copyright 2020 Google Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
////////////////////////////////////////////////////////////////////////////////

#ifndef PROTO_ASN1_PDU_X509_CERTIFICATE_TO_DER_H_
#define PROTO_ASN1_PDU_X509_CERTIFICATE_TO_DER_H_

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
DECLARE_ENCODE_FUNCTION(Extension);
DECLARE_ENCODE_FUNCTION(RawExtension);
DECLARE_ENCODE_FUNCTION(AuthorityKeyIdentifier);
DECLARE_ENCODE_FUNCTION(AuthorityKeyIdentifierSequence);
DECLARE_ENCODE_FUNCTION(SubjectPublicKeyInfoSequence);
DECLARE_ENCODE_FUNCTION(AlgorithmIdentifierSequence);
DECLARE_ENCODE_FUNCTION(asn1_pdu::PDU);

}  // namespace x509_certificate

#endif  // PROTO_ASN1_PDU_X509_CERTIFICATE_TO_DER_H_