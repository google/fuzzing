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

#ifndef PROTO_ASN1_PDU_X509_CERTIFICATE_CHAIN_TO_DER_H
#define PROTO_ASN1_PDU_X509_CERTIFICATE_CHAIN_TO_DER_H

#include <stdint.h>

#include <vector>

#include "x509_certificate_chain.pb.h"

namespace x509_certificate_chain {

// DER encodes each |certificate| in |x509_certificate_chain| and returns
// the encoded certificates in |der|.
std::vector<std::vector<uint8_t>> X509CertificateChainToDER(
    const X509CertificateChain& x509_certificate_chain);

}  // namespace x509_certificate_chain

#endif  // PROTO_ASN1_PDU_X509_CERTIFICATE_CHAIN_TO_DER_H