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

#include "mutated_x509_chain_to_der.h"
#include "x509_certificate_to_der.h"

#include <google/protobuf/text_format.h>

#include <stdlib.h>
#include <utility>

namespace x509_certificate {

// DER encodes each |certificate| in |chain| and returns
// the encoded certificates in |der|.
std::vector<std::vector<uint8_t>> EncodeChain(
    const google::protobuf::RepeatedPtrField<X509Certificate>& chain) {
  std::vector<std::vector<uint8_t>> der;

  for (const auto& cert : chain) {
    der.push_back(X509CertificateToDER(cert));
  }

  return der;
}

void SetTrust(
    std::vector<X509>& encoded_mutated_chain,
    google::protobuf::RepeatedPtrField<TrustParameter>& trust_parameters) {
  for (const auto& trust_parameter : trust_parameters) {
    if (trust_parameter.index() >= encoded_mutated_chain.size()) {
      continue;
    }

    encoded_mutated_chain[trust_parameter.index()].trusted =
        trust_parameter.trusted();
  }
}

void Mutate(const MutateSignature& mutation,
            google::protobuf::RepeatedPtrField<X509Certificate>& chain) {
  // An |operation| on a certiciate cannot be executed if the |index| is greater
  // than or euqal to the |size| of the certificate chain.
  if (mutation.index() >= chain.size()) {
    return;
  }

  auto* signature_value = chain[mutation.index()].mutable_signature_value();
  signature_value->clear_pdu();
  signature_value->mutable_value()->set_unused_bits(
      asn1_universal_types::UnusedBits::VAL0);
  // Represent a valid signature value with 1 and invalid with 0.
  if (mutation.valid()) {
    signature_value->mutable_value()->set_val("1");
  } else {
    signature_value->mutable_value()->set_val("0");
  }
}

void Mutate(const Mutation& mutation,
            google::protobuf::RepeatedPtrField<X509Certificate>& chain) {
  switch (mutation.types_case()) {
    case Mutation::TypesCase::kMutateSignature:
      Mutate(mutation.mutate_signature(), chain);
      break;
    case Mutation::TypesCase::TYPES_NOT_SET:
      return;
  }
}

std::vector<X509> MutatedChainToDER(const MutatedChain& mutated_chain) {
  auto chain = mutated_chain.chain();
  auto trust_parameters = mutated_chain.trust_parameters();
  auto mutations = mutated_chain.mutations();

  if (chain.empty()) {
    return {{}};
  }

  for (const auto& mutation : mutations) {
    Mutate(mutation, chain);
  }

  std::vector<X509> encoded_mutated_chain;
  for (const auto& encoded_cert : EncodeChain(chain)) {
    encoded_mutated_chain.push_back({encoded_cert, false});
  }
  SetTrust(encoded_mutated_chain, trust_parameters);

  return encoded_mutated_chain;
}

}  // namespace x509_certificate