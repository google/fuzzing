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

namespace mutated_x509_chain {

std::vector<std::vector<uint8_t>> EncodeX509Chain(const X509Chain& x509_chain) {
  std::vector<std::vector<uint8_t>> der;

  for (const auto& cert : x509_chain.certificates()) {
    der.push_back(X509CertificateToDER(cert));
  }

  return der;
}

DECLARE_APPLY_OPERATION_FUNCTION(MutateSignatureOperation) {
  auto* signature_value = x509.mutable_signature_value();
  signature_value->clear_pdu();
  signature_value->mutable_value()->set_unused_bits(
      asn1_universal_types::UnusedBits::VAL0);
  // Represent a valid signature value with 1 and invalid with 0.
  if (operation.is_valid()) {
    signature_value->mutable_value()->set_val("1");
  } else {
    signature_value->mutable_value()->set_val("0");
  }
}

DECLARE_APPLY_OPERATION_FUNCTION(Operation) {
  switch (operation.types_case()) {
    case Operation::TypesCase::kMutateSignatureOperation:
      ApplyOperation(x509, operation.mutate_signature_operation());
      break;
    case Operation::TypesCase::TYPES_NOT_SET:
      return;
  }
}

// An |index| of a certiciate is valid if it's less than the |size| of the
// certificate chain.
inline bool isValidIndex(uint8_t index, size_t size) {
  return index < size;
}

std::vector<std::vector<uint8_t>> MutatedChainToDER(
    const MutatedX509Chain& mutated_x509_chain) {
  auto x509_chain = mutated_x509_chain.x509_chain().certificates();
  auto operations = mutated_x509_chain.operations();

  if (x509_chain.empty()) {
    return {{}};
  }

  for (const auto& operation : operations) {
    if (isValidIndex(operation.index(), x509_chain.size())) {
      ApplyOperation(x509_chain[operation.index()], operation);
    }
  }

  return EncodeX509Chain(mutated_x509_chain.x509_chain());
}

}  // namespace mutated_x509_chain