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

#include "x509_certificate_to_der.h"

#include "asn1_pdu_to_der.h"
#include "common.h"

namespace x509_certificate {

DECLARE_ENCODE_FUNCTION(asn1_pdu::PDU) {
  // Used to encode PDU for fields that contain them.
  asn1_pdu::ASN1PDUToDER pdu_to_der;
  std::vector<uint8_t> der_pdu = pdu_to_der.PDUToDER(val);
  der.insert(der.end(), der_pdu.begin(), der_pdu.end());
}

DECLARE_ENCODE_FUNCTION(AlgorithmIdentifierSequence) {
  // Save the current size in |tag_len_pos| to place sequence tag and length
  // after the value is encoded.
  size_t tag_len_pos = der.size();

  Encode(val.object_identifier(), der);
  Encode(val.parameters(), der);

  // The fields of |algorithm_identifier| are wrapped around a sequence (RFC
  // 5280, 4.1.1.2).
  // The current size of |der| subtracted by |tag_len_pos|
  // equates to the size of the value of |algorithm_identifier|.
  EncodeTagAndLength(kAsn1Sequence, der.size() - tag_len_pos, tag_len_pos, der);
}

DECLARE_ENCODE_FUNCTION(ExtendedKeyUsage) {
  // RFC 5280, 4.2.1.9: |ExtendedKeyUsage| is a sequence of (1..MAX)
  // |key_purpose_id|.
  // Save the current size in |tag_len_pos| to place sequence
  // tag and length after the values are encoded.
  size_t tag_len_pos = der.size();

  // First |key_purpose_id| is set by protobuf and always encoded to comply
  // with spec.
  Encode(val.key_purpose_id(), der);
  for (const auto& key_purpose_id : val.key_purpose_ids()) {
    Encode(key_purpose_id, der);
  }

  // The current size of |der| subtracted by |tag_len_pos|
  // equates to the size of the sequence |ExtendedKeyUsage|.
  EncodeTagAndLength(kAsn1Sequence, der.size() - tag_len_pos, tag_len_pos, der);
}

DECLARE_ENCODE_FUNCTION(BasicConstraints) {
  // RFC 5280, 4.2.1.9: |BasicConstraints| is a sequence of |ca| and
  // |path_len_constraint|.
  // Save the current size in |tag_len_pos| to place sequence tag and length
  // after the values are encoded.
  size_t tag_len_pos = der.size();

  // RFC 5280, 4.2.1.9: |ca| is BOOLEAN DEFAULT FALSE.
  // (X.690 (2015), 11.5): DEFAULT value in a sequence field is not encoded.
  if (val.ca().val()) {
    Encode(val.ca(), der);
  }

  // RFC 5280, 4.2.1.9: |path_len_constrant| is OPTIONAL. Encode only if
  // present.
  if (val.has_path_len_constraint()) {
    Encode(val.path_len_constraint(), der);
  }

  // The current size of |der| subtracted by |tag_len_pos|
  // equates to the size of the sequence |BasicConstraints|.
  EncodeTagAndLength(kAsn1Sequence, der.size() - tag_len_pos, tag_len_pos, der);
}

DECLARE_ENCODE_FUNCTION(KeyUsage) {
  constexpr struct Mask {
    using Fn = bool (KeyUsage::*)(void) const;
    Fn key_usage_set;
    uint32_t key_usage_value;
  } kMasks[] = {
      {&KeyUsage::digital_signature, 0x01},
      {&KeyUsage::non_repudation, 0x02},
      {&KeyUsage::key_encipherment, 0x04},
      {&KeyUsage::data_encipherment, 0x08},
      {&KeyUsage::key_agreement, 0x10},
      {&KeyUsage::key_cert_sign, 0x20},
      {&KeyUsage::crl_sign, 0x40},
      {&KeyUsage::encipher_only, 0x80},
      {&KeyUsage::decipher_only, 0x100},
  };
  uint16_t key_usage = 0x00;
  for (const auto& mask : kMasks) {
    if ((val.*(mask.key_usage_set))()) {
      key_usage |= mask.key_usage_value;
    }
  }
  // RFC 5280, 4.2.1.3: KeyUsage ::= BIT STRING.
  // Save the current size in |tag_len_pos| to place BitString tag and length
  // after |key_usage| is encoded.
  size_t tag_len_pos = der.size();
  InsertVariableIntBase256(key_usage, der.size(), der);
  EncodeTagAndLength(kAsn1BitString, der.size() - tag_len_pos, tag_len_pos,
                     der);
}

DECLARE_ENCODE_FUNCTION(SubjectKeyIdentifier) {
  Encode(val.key_identifier(), der);
}

DECLARE_ENCODE_FUNCTION(AuthorityKeyIdentifier) {
  // RFC 5280, 4.2.1.1: |AuthorityKeyIdentifier)| is a sequence of
  // |key_identifier|, |authority_cert_issuer|, and
  // |authority_cert_serial_number|.
  // Save the current size in |tag_len_pos| to place sequence tag and length
  // after the value is encoded.
  size_t tag_len_pos = der.size();

  if (val.has_key_identifier()) {
    size_t pos_of_tag = der.size();
    Encode(val.key_identifier(), der);
    // |key_identifier| is Context-specific with tag number 0 (RFC
    // 5280, 4.2.1.1).
    ReplaceTag(kAsn1ContextSpecific | 0x00, pos_of_tag, der);
  }
  if (val.has_authority_cert_issuer()) {
    size_t pos_of_tag = der.size();
    Encode(val.authority_cert_issuer(), der);
    // |authority_cert_issuer| is Context-specific with tag number 1 (RFC
    // 5280, 4.2.1.1).
    ReplaceTag(kAsn1ContextSpecific | 0x01, pos_of_tag, der);
  }
  if (val.has_authority_cert_serial_number()) {
    size_t pos_of_tag = der.size();
    Encode(val.authority_cert_serial_number(), der);
    // |authority_cert_serial_number| is Context-specific with tag number 2 (RFC
    // 5280, 4.2.1.1).
    ReplaceTag(kAsn1ContextSpecific | 0x02, pos_of_tag, der);
  }

  // The current size of |der| subtracted by |tag_len_pos|
  // equates to the size of the |AuthorityKeyIdentifier|.
  EncodeTagAndLength(kAsn1Sequence, der.size() - tag_len_pos, tag_len_pos, der);
}

DECLARE_ENCODE_FUNCTION(RawExtension) {
  if (val.has_pdu()) {
    // Save the current size in |tag_len_pos| to place octet string tag and
    // length after the value is encoded.
    size_t tag_len_pos = der.size();
    Encode(val.pdu(), der);
    EncodeTagAndLength(kAsn1OctetString, der.size() - tag_len_pos, tag_len_pos,
                       der);
  } else {
    Encode(val.extn_value(), der);
  }
}

void EncodeExtensionValue(const Extension& val, std::vector<uint8_t>& der) {
  switch (val.types_case()) {
    case Extension::TypesCase::kAuthorityKeyIdentifier:
      Encode(val.authority_key_identifier(), der);
      break;
    case Extension::TypesCase::kSubjectKeyIdentifier:
      Encode(val.subject_key_identifier(), der);
      break;
    case Extension::TypesCase::kBasicConstraints:
      Encode(val.basic_constraints(), der);
      break;
    case Extension::TypesCase::kExtendedKeyUsage:
      Encode(val.extended_key_usage(), der);
      break;
    case Extension::TypesCase::kKeyUsage:
      Encode(val.key_usage(), der);
      break;
    case Extension::TypesCase::TYPES_NOT_SET:
      Encode(val.raw_extension(), der);
      break;
  }
}

void EncodeExtensionID(const Extension& val, std::vector<uint8_t>& der) {
  if (val.has_extn_id()) {
    Encode(val.extn_id(), der);
    return;
  }

  std::vector<uint8_t> encoded_oid;
  switch (val.types_case()) {
    case Extension::TypesCase::kAuthorityKeyIdentifier:
      // RFC 5280, 4.2.1.1: |AuthorityKeyIdentifier| OID is {2 5 29 35}.
      encoded_oid = {(2 * 40) + 5, 29, 35};
      break;
    case Extension::TypesCase::kSubjectKeyIdentifier:
      // RFC 5280, 4.2.1.2: |SubjectKeyIdentifier| OID is {2 5 29 14}.
      encoded_oid = {(2 * 40) + 5, 29, 14};
      break;
    case Extension::TypesCase::kKeyUsage:
      // RFC 5280, 4.2.1.3: |KeyUsage| OID is {2 5 29 15}.
      encoded_oid = {(2 * 40) + 5, 29, 15};
      break;
    case Extension::TypesCase::kBasicConstraints:
      // RFC 5280, 4.2.1.9: |BasicConstraints| OID is {2 5 29 14}.
      encoded_oid = {(2 * 40) + 5, 29, 19};
      break;
    case Extension::TypesCase::kExtendedKeyUsage:
      // RFC 5280, 4.2.1.12: |ExtendedKeyUsage| OID is {2 5 29 37}.
      encoded_oid = {(2 * 40) + 5, 29, 37};
      break;
    case Extension::TypesCase::TYPES_NOT_SET:
      Encode(val.raw_extension().extn_id(), der);
      return;
  }

  der.insert(der.end(), encoded_oid.begin(), encoded_oid.end());
}

DECLARE_ENCODE_FUNCTION(Extension) {
  // Save the current size in |tag_len_pos| to place sequence tag and length
  // after the value is encoded.
  size_t tag_len_pos = der.size();

  EncodeExtensionID(val, der);

  // RFC 5280, 4.1: |critical| is DEFAULT false. Furthermore,
  // (X.690 (2015), 11.5): DEFAULT value in a sequence field is not encoded.
  if (val.critical().val()) {
    Encode(val.critical(), der);
  }

  EncodeExtensionValue(val, der);

  // The fields of an |Extension| are wrapped around a sequence (RFC 5280, 4.1).
  // The current size of |der| subtracted by |tag_len_pos|
  // equates to the size of the |Extension|.
  EncodeTagAndLength(kAsn1Sequence, der.size() - tag_len_pos, tag_len_pos, der);
}

DECLARE_ENCODE_FUNCTION(ExtensionSequence) {
  // RFC 5280, 4.2.1.9: |ExtensionSequence| is a sequence of (1..MAX) Extension.
  // Save the current size in |tag_len_pos| to place sequence tag and length
  // after the value is encoded.
  size_t tag_len_pos = der.size();

  // First |extension| is set by protobuf and always encoded to comply
  // with spec.
  Encode(val.extension(), der);
  for (const auto& extension : val.extensions()) {
    Encode(extension, der);
  }

  // The current size of |der| subtracted by |tag_len_pos|
  // equates to the size of the |ExtensionSequence|.
  EncodeTagAndLength(kAsn1Sequence, der.size() - tag_len_pos, tag_len_pos, der);
}

DECLARE_ENCODE_FUNCTION(SubjectPublicKeyInfoSequence) {
  // Save the current size in |tag_len_pos| to place sequence tag and length
  // after the value is encoded.
  size_t tag_len_pos = der.size();

  Encode(val.algorithm_identifier(), der);
  Encode(val.subject_public_key(), der);

  // The fields of |subject_public_key_info| are wrapped around a sequence (RFC
  // 5280, 4.1 & 4.1.2.5).
  // The current size of |der| subtracted by |tag_len_pos|
  // equates to the size of the value of |subject_public_key_info|.
  EncodeTagAndLength(kAsn1Sequence, der.size() - tag_len_pos, tag_len_pos, der);
}

DECLARE_ENCODE_FUNCTION(TimeChoice) {
  // The |Time| field either has an UTCTime or a GeneralizedTime (RFC 5280, 4.1
  // & 4.1.2.5).
  if (val.has_utc_time()) {
    return Encode(val.utc_time(), der);
  }
  return Encode(val.generalized_time(), der);
}

DECLARE_ENCODE_FUNCTION(ValiditySequence) {
  // Save the current size in |tag_len_pos| to place sequence tag and length
  // after the value is encoded.
  size_t tag_len_pos = der.size();

  Encode(val.not_before().value(), der);
  Encode(val.not_after().value(), der);

  // The fields of |Validity| are wrapped around a sequence (RFC
  // 5280, 4.1 & 4.1.2.5).
  // The current size of |der| subtracted by |tag_len_pos|
  // equates to the size of the value of |validity|.
  EncodeTagAndLength(kAsn1Sequence, der.size() - tag_len_pos, tag_len_pos, der);
}

DECLARE_ENCODE_FUNCTION(VersionNumber) {
  // |version| is Context-specific with tag number 0 (RFC 5280, 4.1 & 4.1.2.1).
  // Takes on values 0, 1 and 2, so only require length of 1 to
  // encode it (RFC 5280, 4.1 & 4.1.2.1).
  std::vector<uint8_t> der_version = {kAsn1ContextSpecific | 0x00, 0x01,
                                      static_cast<uint8_t>(val)};
  der.insert(der.end(), der_version.begin(), der_version.end());
}

DECLARE_ENCODE_FUNCTION(TBSCertificateSequence) {
  // Save the current size in |tag_len_pos| to place sequence tag and length
  // after the value is encoded.
  size_t tag_len_pos = der.size();

  Encode(val.version(), der);
  Encode(val.serial_number(), der);
  Encode(val.signature_algorithm(), der);
  Encode(val.issuer(), der);
  Encode(val.validity(), der);
  Encode(val.subject(), der);
  Encode(val.subject_public_key_info(), der);

  // RFC 5280, 4.1: |issuer_unique_id| and |subject_unique_id|
  // are only set for v2 and v3 and |extensions| only set for v3.
  // However, set |issuer_unique_id|, |subject_unique_id|, and |extensions|
  // independently of the version number for interesting inputs.
  if (val.has_issuer_unique_id()) {
    size_t pos_of_tag = der.size();
    Encode(val.issuer_unique_id(), der);
    // |issuer_unqiue_id| is Context-specific with tag number 1 (RFC 5280, 4.1
    // & 4.1.2.8).
    ReplaceTag(kAsn1ContextSpecific | 0x01, pos_of_tag, der);
  }
  if (val.has_subject_unique_id()) {
    size_t pos_of_tag = der.size();
    Encode(val.subject_unique_id(), der);
    // |subject_unqiue_id| is Context-specific with tag number 2 (RFC 5280, 4.1
    // & 4.1.2.8).
    ReplaceTag(kAsn1ContextSpecific | 0x02, pos_of_tag, der);
  }
  if (val.has_extensions()) {
    size_t pos_of_tag = der.size();
    Encode(val.extensions(), der);
    // |extensions| is Context-specific with tag number 3 (RFC 5280, 4.1
    // & 4.1.2.8).
    ReplaceTag(kAsn1ContextSpecific | 0x03, pos_of_tag, der);
  }

  // The fields of |tbs_certificate| are wrapped around a sequence (RFC
  // 5280, 4.1 & 4.1.2.5).
  // The current size of |der| subtracted by |tag_len_pos|
  // equates to the size of the value of |tbs_certificate|.
  EncodeTagAndLength(kAsn1Sequence, der.size() - tag_len_pos, tag_len_pos, der);
}

std::vector<uint8_t> X509CertificateToDER(
    const X509Certificate& X509_certificate) {
  // Contains DER encoded X509 Certificate.
  std::vector<uint8_t> der;

  // Save the current size in |tag_len_pos| to place sequence tag and length
  // after the value is encoded.
  size_t tag_len_pos = der.size();

  Encode(X509_certificate.tbs_certificate(), der);
  Encode(X509_certificate.signature_algorithm(), der);
  Encode(X509_certificate.signature_value(), der);

  // The fields of |X509_certificate| are wrapped around a sequence (RFC
  // 5280, 4.1 & 4.1.2.5).
  // The current size of |der| subtracted by |tag_len_pos|
  // equates to the size of the value of |X509_certificate|.
  EncodeTagAndLength(kAsn1Sequence, der.size() - tag_len_pos, tag_len_pos, der);
  return der;
}

}  // namespace x509_certificate