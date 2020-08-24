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

void ApplyOperation(
    const MutateSignatureOperation& operation,
    google::protobuf::RepeatedPtrField<X509Certificate>& chain) {
  // An |operation| on a certiciate cannot be executed if the |index| is greater
  // than or euqal to the |size| of the certificate chain.
  if (operation.index() >= chain.size()) {
    return;
  }

  auto* signature_value = chain[operation.index()].mutable_signature_value();
  signature_value->clear_pdu();
  signature_value->mutable_value()->set_unused_bits(
      asn1_universal_types::UnusedBits::VAL0);
  // Represent a valid signature value with 1 and invalid with 0.
  if (operation.valid()) {
    signature_value->mutable_value()->set_val("1");
  } else {
    signature_value->mutable_value()->set_val("0");
  }
}

void ApplyOperation(
    const Operation& operation,
    google::protobuf::RepeatedPtrField<X509Certificate>& chain) {
  switch (operation.types_case()) {
    case Operation::TypesCase::kMutateSignatureOperation:
      ApplyOperation(operation.mutate_signature_operation(), chain);
      break;
    case Operation::TypesCase::TYPES_NOT_SET:
      return;
  }
}

std::vector<std::vector<uint8_t>> MutatedChainToDER(
    const MutatedChain& mutated_chain) {
  auto chain = mutated_chain.chain();
  auto operations = mutated_chain.operations();

  // If the chain is empty, return immediately to not slow down the fuzzer.
  if (chain.empty()) {
    return {{}};
  }

  for (const auto& operation : operations) {
    ApplyOperation(operation, chain);
  }

  return EncodeChain(chain);
}

}  // namespace x509_certificate