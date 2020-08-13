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

DECLARE_ENCODE_FUNCTION(AuthorityKeyIdentifierSequence) {
  // // Save the current size in |tag_len_pos| to place sequence tag and length
  // // after the value is encoded.
  size_t tag_len_pos = der.size();

  if (val.has_key_identifier()) {
    Encode(val.key_identifier(), der);
  }
  if (val.has_authority_cert_issuer()) {
    Encode(val.authority_cert_issuer(), der);
  }
  if (val.has_authority_cert_serial_number()) {
    Encode(val.authority_cert_serial_number(), der);
  }

  // The fields of an AuthorityKeyIdentifier are wrapped around a sequence (RFC
  // 5280, 4.2.1.1).
  // The current size of |der| subtracted by |tag_len_pos|
  // equates to the size of the value of |AuthorityKeyIdentifierSequence|.
  EncodeTagAndLength(kAsn1Sequence, der.size() - tag_len_pos, tag_len_pos, der);
}

DECLARE_ENCODE_FUNCTION(AuthorityKeyIdentifier) {
  if (val.has_object_identifier()) {
    Encode(val.object_identifier(), der);
  } else {
    // RFC 5280, 4.2.1 & A.1: |AuthorityKeyIdentifier| OID is {2 5 29 35}.
    std::vector<uint8_t> aki_id = {(2 * 40) + 5, 29, 35};
    der.insert(der.end(), aki_id.begin(), aki_id.end());
  }
  Encode(val.critical(), der);
  Encode(val.aki_sequence(), der);
}

DECLARE_ENCODE_FUNCTION(RawExtension) {
  // Save the current size in |tag_len_pos| to place sequence tag and length
  // after the value is encoded.
  size_t tag_len_pos = der.size();

  Encode(val.object_identifier(), der);
  Encode(val.critical(), der);
  Encode(val.octet_string(), der);

  // The fields of an Extension are wrapped around a sequence (RFC
  // 5280, 4.1 & 4.1.2.9).
  // The current size of |der| subtracted by |tag_len_pos|
  // equates to the size of the value of |RawExtension|.
  EncodeTagAndLength(kAsn1Sequence, der.size() - tag_len_pos, tag_len_pos, der);
}

DECLARE_ENCODE_FUNCTION(Extension) {
  if (val.has_raw_extension()) {
    Encode(val.raw_extension(), der);
  } else if (val.has_authority_key_identifier()) {
    Encode(val.authority_key_identifier(), der);
  }
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
  std::vector<uint8_t> der_version = {kAsn1Constructed, 0x01,
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
  if (!val.extensions().empty()) {
    size_t pos_of_tag = der.size();
    // RFC 5280, 4.1: Extensions, if present, is made up one one ore more
    // Extension.
    for (auto extension : val.extensions()) {
      Encode(extension, der);
    }
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