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

#ifndef PROTO_ASN1_PDU_MUTATE_X509_CHAIN_H_
#define PROTO_ASN1_PDU_MUTATE_X509_CHAIN_H_

#include <stdint.h>

#include <vector>

#include "mutated_x509_chain.pb.h"

namespace mutated_x509_chain {

// Applies |operations| to |x509_chain| and returns the DER encoded chain.
std::vector<std::vector<uint8_t>> MutatedChainToDER(
    const MutatedX509Chain& mutated_x509_chain);

// Applies the |TYPE| operation to |x509| in order to create meaningful
// certificate chains.
#define DECLARE_APPLY_OPERATION_FUNCTION(TYPE)                 \
  void ApplyOperation(x509_certificate::X509Certificate& x509, \
                      const TYPE& operation)

DECLARE_APPLY_OPERATION_FUNCTION(Operation);
DECLARE_APPLY_OPERATION_FUNCTION(MutateSignatureOperation);

// DER encodes each |certificate| in |x509_chain| and returns
// the encoded certificates in |der|.
std::vector<std::vector<uint8_t>> EncodeX509Chain(const X509Chain& x509_chain);

}  // namespace mutated_x509_chain

#endif  // PROTO_ASN1_PDU_X509_CERTIFICATE_TO_DER_H_