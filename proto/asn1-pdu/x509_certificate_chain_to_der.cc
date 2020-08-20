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

#include "x509_certificate_chain_to_der.h"

#include "x509_certificate_to_der.h"

namespace x509_certificate_chain {

// Encodes |X509_certificate| to DER, returning the encoded bytes in |der_|.
std::vector<uint8_t> X509CertificateChainToDER(
    const X509CertificateChain& x509_certificate_chain) {
  // Contains DER encoded |X509CertificateChain|.
  std::vector<uint8_t> der;

  std::vector<uint8_t> der_cert = x509_certificate::X509CertificateToDER(
      x509_certificate_chain.certificate());
  der.insert(der.end(), der_cert.begin(), der_cert.end());

  for (const auto& cert : x509_certificate_chain.certificates()) {
    der_cert = x509_certificate::X509CertificateToDER(cert);
    der.insert(der.end(), der_cert.begin(), der_cert.end());
  }

  return der;
}

}  // namespace x509_certificate_chain