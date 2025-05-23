// SPDX-License-Identifier: Apache-2.0
// Copyright Pionix GmbH and Contributors to EVerest

#include <evse_security_ocpp.hpp>

EvseSecurity::EvseSecurity(evse_securityIntf& r_security) : r_security(r_security) {
}

EvseSecurity::~EvseSecurity() {
}

ocpp::InstallCertificateResult EvseSecurity::install_ca_certificate(const std::string& certificate,
                                                                    const ocpp::CaCertificateType& certificate_type) {
    try {
        return conversions::to_ocpp(
            this->r_security.call_install_ca_certificate(certificate, conversions::from_ocpp(certificate_type)));
    } catch (const std::out_of_range& e) {
        EVLOG_warning << e.what();
        return ocpp::InstallCertificateResult::WriteError;
    }
}

ocpp::DeleteCertificateResult
EvseSecurity::delete_certificate(const ocpp::CertificateHashDataType& certificate_hash_data) {
    try {
        return conversions::to_ocpp(
            this->r_security.call_delete_certificate(conversions::from_ocpp(certificate_hash_data)));
    } catch (const std::out_of_range& e) {
        EVLOG_warning << e.what();
        return ocpp::DeleteCertificateResult::Failed;
    }
}

ocpp::InstallCertificateResult
EvseSecurity::update_leaf_certificate(const std::string& certificate_chain,
                                      const ocpp::CertificateSigningUseEnum& certificate_type) {
    try {
        return conversions::to_ocpp(
            this->r_security.call_update_leaf_certificate(certificate_chain, conversions::from_ocpp(certificate_type)));
    } catch (const std::out_of_range& e) {
        EVLOG_warning << e.what();
        return ocpp::InstallCertificateResult::WriteError;
    }
}

ocpp::CertificateValidationResult EvseSecurity::verify_certificate(const std::string& certificate_chain,
                                                                   const ocpp::LeafCertificateType& certificate_type) {
    try {
        return conversions::to_ocpp(
            this->r_security.call_verify_certificate(certificate_chain, {conversions::from_ocpp(certificate_type)}));
    } catch (const std::out_of_range& e) {
        EVLOG_warning << e.what();
        return ocpp::CertificateValidationResult::Unknown;
    }
}

ocpp::CertificateValidationResult
EvseSecurity::verify_certificate(const std::string& certificate_chain,
                                 const std::vector<ocpp::LeafCertificateType>& certificate_types) {
    std::vector<types::evse_security::LeafCertificateType> _certificate_types;

    for (const auto& certificate_type : certificate_types) {
        try {
            _certificate_types.push_back(conversions::from_ocpp(certificate_type));
        } catch (const std::out_of_range& e) {
            EVLOG_warning << e.what();
        }
    }

    try {
        return conversions::to_ocpp(this->r_security.call_verify_certificate(certificate_chain, _certificate_types));
    } catch (const std::out_of_range& e) {
        EVLOG_warning << e.what();
        return ocpp::CertificateValidationResult::Unknown;
    }
}

std::vector<ocpp::CertificateHashDataChain>
EvseSecurity::get_installed_certificates(const std::vector<ocpp::CertificateType>& certificate_types) {
    std::vector<ocpp::CertificateHashDataChain> result;

    std::vector<types::evse_security::CertificateType> _certificate_types;

    for (const auto& certificate_type : certificate_types) {
        try {
            _certificate_types.push_back(conversions::from_ocpp(certificate_type));
        } catch (const std::out_of_range& e) {
            EVLOG_warning << e.what();
        }
    }

    const auto installed_certificates = this->r_security.call_get_installed_certificates(_certificate_types);

    for (const auto& certificate_hash_data : installed_certificates.certificate_hash_data_chain) {
        try {
            result.push_back(conversions::to_ocpp(certificate_hash_data));
        } catch (const std::out_of_range& e) {
            EVLOG_warning << e.what();
        }
    }
    return result;
}

std::vector<ocpp::OCSPRequestData> EvseSecurity::get_v2g_ocsp_request_data() {
    std::vector<ocpp::OCSPRequestData> result;

    const auto ocsp_request_data = this->r_security.call_get_v2g_ocsp_request_data();
    for (const auto& ocsp_request_entry : ocsp_request_data.ocsp_request_data_list) {
        try {
            result.push_back(conversions::to_ocpp(ocsp_request_entry));
        } catch (const std::out_of_range& e) {
            EVLOG_warning << e.what();
        }
    }
    return result;
}

std::vector<ocpp::OCSPRequestData> EvseSecurity::get_mo_ocsp_request_data(const std::string& certificate_chain) {
    std::vector<ocpp::OCSPRequestData> result;

    const auto ocsp_request_data = this->r_security.call_get_mo_ocsp_request_data(certificate_chain);
    for (const auto& ocsp_request_entry : ocsp_request_data.ocsp_request_data_list) {
        try {
            result.push_back(conversions::to_ocpp(ocsp_request_entry));
        } catch (const std::out_of_range& e) {
            EVLOG_warning << e.what();
        }
    }
    return result;
}

void EvseSecurity::update_ocsp_cache(const ocpp::CertificateHashDataType& certificate_hash_data,
                                     const std::string& ocsp_response) {
    try {
        this->r_security.call_update_ocsp_cache(conversions::from_ocpp(certificate_hash_data), ocsp_response);
    } catch (const std::out_of_range& e) {
        EVLOG_warning << e.what();
    }
}

bool EvseSecurity::is_ca_certificate_installed(const ocpp::CaCertificateType& certificate_type) {
    try {
        return this->r_security.call_is_ca_certificate_installed(conversions::from_ocpp(certificate_type));
    } catch (const std::out_of_range& e) {
        EVLOG_warning << e.what();
        return false;
    }
}

ocpp::GetCertificateSignRequestResult
EvseSecurity::generate_certificate_signing_request(const ocpp::CertificateSigningUseEnum& certificate_type,
                                                   const std::string& country, const std::string& organization,
                                                   const std::string& common, bool use_tpm) {
    try {
        auto csr_response = this->r_security.call_generate_certificate_signing_request(
            conversions::from_ocpp(certificate_type), country, organization, common, use_tpm);

        ocpp::GetCertificateSignRequestResult result;

        result.status = conversions::to_ocpp(csr_response.status);
        if (csr_response.csr.has_value()) {
            result.csr = csr_response.csr;
        }

        return result;
    } catch (const std::out_of_range& e) {
        EVLOG_warning << e.what();
        ocpp::GetCertificateSignRequestResult result;
        result.status = ocpp::GetCertificateSignRequestStatus::GenerationError;
        return result;
    }
}

ocpp::GetCertificateInfoResult
EvseSecurity::get_leaf_certificate_info(const ocpp::CertificateSigningUseEnum& certificate_type, bool include_ocsp) {
    try {
        const auto info_response = this->r_security.call_get_leaf_certificate_info(
            conversions::from_ocpp(certificate_type), types::evse_security::EncodingFormat::PEM, include_ocsp);

        ocpp::GetCertificateInfoResult result;

        result.status = conversions::to_ocpp(info_response.status);
        if (info_response.info.has_value()) {
            result.info = conversions::to_ocpp(info_response.info.value());
        }

        return result;
    } catch (const std::out_of_range& e) {
        EVLOG_warning << e.what();
        ocpp::GetCertificateInfoResult result;
        result.status = ocpp::GetCertificateInfoStatus::Rejected;
        return result;
    }
}

bool EvseSecurity::update_certificate_links(const ocpp::CertificateSigningUseEnum& certificate_type) {
    // TODO: Implement if required
    return false;
}

std::string EvseSecurity::get_verify_file(const ocpp::CaCertificateType& certificate_type) {
    try {
        return this->r_security.call_get_verify_file(conversions::from_ocpp(certificate_type));
    } catch (const std::out_of_range& e) {
        EVLOG_warning << e.what();
        return {};
    }
}

std::string EvseSecurity::get_verify_location(const ocpp::CaCertificateType& certificate_type) {
    try {
        return this->r_security.call_get_verify_location(conversions::from_ocpp(certificate_type));
    } catch (std::out_of_range& e) {
        EVLOG_warning << e.what();
        return {};
    }
}

int EvseSecurity::get_leaf_expiry_days_count(const ocpp::CertificateSigningUseEnum& certificate_type) {
    try {
        return this->r_security.call_get_leaf_expiry_days_count(conversions::from_ocpp(certificate_type));
    } catch (const std::out_of_range& e) {
        EVLOG_warning << e.what();
        return 0;
    }
}

namespace conversions {

ocpp::CaCertificateType to_ocpp(types::evse_security::CaCertificateType other) {
    switch (other) {
    case types::evse_security::CaCertificateType::V2G:
        return ocpp::CaCertificateType::V2G;
    case types::evse_security::CaCertificateType::MO:
        return ocpp::CaCertificateType::MO;
    case types::evse_security::CaCertificateType::CSMS:
        return ocpp::CaCertificateType::CSMS;
    case types::evse_security::CaCertificateType::MF:
        return ocpp::CaCertificateType::MF;
    }
    throw std::out_of_range("Could not convert types::evse_security::CaCertificateType to ocpp::CaCertificateType");
}

ocpp::LeafCertificateType to_ocpp(types::evse_security::LeafCertificateType other) {
    switch (other) {
    case types::evse_security::LeafCertificateType::CSMS:
        return ocpp::LeafCertificateType::CSMS;
    case types::evse_security::LeafCertificateType::V2G:
        return ocpp::LeafCertificateType::V2G;
    case types::evse_security::LeafCertificateType::MF:
        return ocpp::LeafCertificateType::MF;
    case types::evse_security::LeafCertificateType::MO:
        return ocpp::LeafCertificateType::MO;
    }
    throw std::out_of_range(
        "Could not convert types::evse_security::LeafCertificateType to ocpp::CertificateSigningUseEnum");
}

ocpp::CertificateType to_ocpp(types::evse_security::CertificateType other) {
    switch (other) {
    case types::evse_security::CertificateType::V2GRootCertificate:
        return ocpp::CertificateType::V2GRootCertificate;
    case types::evse_security::CertificateType::MORootCertificate:
        return ocpp::CertificateType::MORootCertificate;
    case types::evse_security::CertificateType::CSMSRootCertificate:
        return ocpp::CertificateType::CSMSRootCertificate;
    case types::evse_security::CertificateType::V2GCertificateChain:
        return ocpp::CertificateType::V2GCertificateChain;
    case types::evse_security::CertificateType::MFRootCertificate:
        return ocpp::CertificateType::MFRootCertificate;
    }
    throw std::out_of_range("Could not convert types::evse_security::CertificateType to ocpp::CertificateType");
}

ocpp::HashAlgorithmEnumType to_ocpp(types::evse_security::HashAlgorithm other) {
    switch (other) {
    case types::evse_security::HashAlgorithm::SHA256:
        return ocpp::HashAlgorithmEnumType::SHA256;
    case types::evse_security::HashAlgorithm::SHA384:
        return ocpp::HashAlgorithmEnumType::SHA384;
    case types::evse_security::HashAlgorithm::SHA512:
        return ocpp::HashAlgorithmEnumType::SHA512;
    }
    throw std::out_of_range("Could not convert types::evse_security::HashAlgorithm to ocpp::HashAlgorithmEnumType");
}

ocpp::InstallCertificateResult to_ocpp(types::evse_security::InstallCertificateResult other) {
    switch (other) {
    case types::evse_security::InstallCertificateResult::InvalidSignature:
        return ocpp::InstallCertificateResult::InvalidSignature;
    case types::evse_security::InstallCertificateResult::InvalidCertificateChain:
        return ocpp::InstallCertificateResult::InvalidCertificateChain;
    case types::evse_security::InstallCertificateResult::InvalidFormat:
        return ocpp::InstallCertificateResult::InvalidFormat;
    case types::evse_security::InstallCertificateResult::InvalidCommonName:
        return ocpp::InstallCertificateResult::InvalidCommonName;
    case types::evse_security::InstallCertificateResult::NoRootCertificateInstalled:
        return ocpp::InstallCertificateResult::NoRootCertificateInstalled;
    case types::evse_security::InstallCertificateResult::Expired:
        return ocpp::InstallCertificateResult::Expired;
    case types::evse_security::InstallCertificateResult::CertificateStoreMaxLengthExceeded:
        return ocpp::InstallCertificateResult::CertificateStoreMaxLengthExceeded;
    case types::evse_security::InstallCertificateResult::WriteError:
        return ocpp::InstallCertificateResult::WriteError;
    case types::evse_security::InstallCertificateResult::Accepted:
        return ocpp::InstallCertificateResult::Accepted;
    }
    throw std::out_of_range(
        "Could not convert types::evse_security::InstallCertificateResult to ocpp::InstallCertificateResult");
}

ocpp::CertificateValidationResult to_ocpp(types::evse_security::CertificateValidationResult other) {
    switch (other) {
    case types::evse_security::CertificateValidationResult::Valid:
        return ocpp::CertificateValidationResult::Valid;
    case types::evse_security::CertificateValidationResult::InvalidSignature:
        return ocpp::CertificateValidationResult::InvalidSignature;
    case types::evse_security::CertificateValidationResult::IssuerNotFound:
        return ocpp::CertificateValidationResult::IssuerNotFound;
    case types::evse_security::CertificateValidationResult::InvalidLeafSignature:
        return ocpp::CertificateValidationResult::InvalidLeafSignature;
    case types::evse_security::CertificateValidationResult::InvalidChain:
        return ocpp::CertificateValidationResult::InvalidChain;
    case types::evse_security::CertificateValidationResult::Unknown:
        return ocpp::CertificateValidationResult::Unknown;
    case types::evse_security::CertificateValidationResult::Expired:
        return ocpp::CertificateValidationResult::Expired;
    }
    throw std::out_of_range("Could not convert types::evse_security::CertificateValidationResult to "
                            "ocpp::CertificateValidationResult");
}

ocpp::GetCertificateInfoStatus to_ocpp(types::evse_security::GetCertificateInfoStatus other) {
    switch (other) {
    case types::evse_security::GetCertificateInfoStatus::Accepted:
        return ocpp::GetCertificateInfoStatus::Accepted;
    case types::evse_security::GetCertificateInfoStatus::Rejected:
        return ocpp::GetCertificateInfoStatus::Rejected;
    case types::evse_security::GetCertificateInfoStatus::NotFound:
        return ocpp::GetCertificateInfoStatus::NotFound;
    case types::evse_security::GetCertificateInfoStatus::NotFoundValid:
        return ocpp::GetCertificateInfoStatus::NotFoundValid;
    case types::evse_security::GetCertificateInfoStatus::PrivateKeyNotFound:
        return ocpp::GetCertificateInfoStatus::PrivateKeyNotFound;
    }
    throw std::out_of_range("Could not convert types::evse_security::GetCertificateInfoStatus to "
                            "ocpp::GetCertificateInfoStatus");
}

ocpp::DeleteCertificateResult to_ocpp(types::evse_security::DeleteCertificateResult other) {
    switch (other) {
    case types::evse_security::DeleteCertificateResult::Accepted:
        return ocpp::DeleteCertificateResult::Accepted;
    case types::evse_security::DeleteCertificateResult::Failed:
        return ocpp::DeleteCertificateResult::Failed;
    case types::evse_security::DeleteCertificateResult::NotFound:
        return ocpp::DeleteCertificateResult::NotFound;
    }
    throw std::out_of_range(
        "Could not convert types::evse_security::DeleteCertificateResult to ocpp::DeleteCertificateResult");
}

ocpp::GetCertificateSignRequestStatus to_ocpp(types::evse_security::GetCertificateSignRequestStatus other) {
    switch (other) {
    case types::evse_security::GetCertificateSignRequestStatus::Accepted:
        return ocpp::GetCertificateSignRequestStatus::Accepted;
    case types::evse_security::GetCertificateSignRequestStatus::InvalidRequestedType:
        return ocpp::GetCertificateSignRequestStatus::InvalidRequestedType;
    case types::evse_security::GetCertificateSignRequestStatus::KeyGenError:
        return ocpp::GetCertificateSignRequestStatus::KeyGenError;
    case types::evse_security::GetCertificateSignRequestStatus::GenerationError:
        return ocpp::GetCertificateSignRequestStatus::GenerationError;
    }
    throw std::out_of_range("Could not convert types::evse_security::GetCertificateSignRequestStatus to "
                            "ocpp::GetCertificateSignRequestStatus");
}

ocpp::CertificateHashDataType to_ocpp(types::evse_security::CertificateHashData other) {
    ocpp::CertificateHashDataType lhs;
    lhs.hashAlgorithm = to_ocpp(other.hash_algorithm);
    lhs.issuerNameHash = other.issuer_name_hash;
    lhs.issuerKeyHash = other.issuer_key_hash;
    lhs.serialNumber = other.serial_number;
    return lhs;
}

ocpp::CertificateHashDataChain to_ocpp(types::evse_security::CertificateHashDataChain other) {
    ocpp::CertificateHashDataChain lhs;
    lhs.certificateType = to_ocpp(other.certificate_type);
    lhs.certificateHashData = to_ocpp(other.certificate_hash_data);
    if (other.child_certificate_hash_data.has_value()) {
        std::vector<ocpp::CertificateHashDataType> v;
        for (const auto& certificate_hash_data : other.child_certificate_hash_data.value()) {
            v.push_back(to_ocpp(certificate_hash_data));
        }
        lhs.childCertificateHashData = v;
    }
    return lhs;
}

ocpp::OCSPRequestData to_ocpp(types::evse_security::OCSPRequestData other) {
    ocpp::OCSPRequestData lhs;
    if (other.certificate_hash_data.has_value()) {
        lhs.issuerNameHash = other.certificate_hash_data.value().issuer_name_hash;
        lhs.issuerKeyHash = other.certificate_hash_data.value().issuer_key_hash;
        lhs.serialNumber = other.certificate_hash_data.value().serial_number;
        lhs.hashAlgorithm = to_ocpp(other.certificate_hash_data.value().hash_algorithm);
    }
    if (other.responder_url.has_value()) {
        lhs.responderUrl = other.responder_url.value();
    }
    return lhs;
}

ocpp::CertificateOCSP to_ocpp(types::evse_security::CertificateOCSP other) {
    ocpp::CertificateOCSP lhs;
    lhs.hash = to_ocpp(other.hash);

    if (other.ocsp_path.has_value()) {
        lhs.ocsp_path = other.ocsp_path.value();
    }

    return lhs;
}

ocpp::CertificateInfo to_ocpp(types::evse_security::CertificateInfo other) {
    ocpp::CertificateInfo lhs;
    lhs.certificate_path = other.certificate;
    lhs.certificate_single_path = other.certificate_single;
    lhs.key_path = other.key;
    lhs.password = other.password;
    lhs.certificate_count = other.certificate_count;

    if (other.ocsp.has_value()) {
        for (auto& ocsp_data : other.ocsp.value()) {
            lhs.ocsp.push_back(to_ocpp(ocsp_data));
        }
    }

    return lhs;
}

types::evse_security::CaCertificateType from_ocpp(ocpp::CaCertificateType other) {
    switch (other) {
    case ocpp::CaCertificateType::V2G:
        return types::evse_security::CaCertificateType::V2G;
    case ocpp::CaCertificateType::MO:
        return types::evse_security::CaCertificateType::MO;
    case ocpp::CaCertificateType::CSMS:
        return types::evse_security::CaCertificateType::CSMS;
    case ocpp::CaCertificateType::MF:
        return types::evse_security::CaCertificateType::MF;
    case ocpp::CaCertificateType::OEM:
        throw std::out_of_range("Could not convert ocpp::CaCertificateType::OEM to evse_security::CaCertificateType");
    }
    throw std::out_of_range("Could not convert ocpp::CaCertificateType to evse_security::CaCertificateType");
}

types::evse_security::LeafCertificateType from_ocpp(ocpp::CertificateSigningUseEnum other) {
    switch (other) {
    case ocpp::CertificateSigningUseEnum::ChargingStationCertificate:
        return types::evse_security::LeafCertificateType::CSMS;
    case ocpp::CertificateSigningUseEnum::V2GCertificate:
        return types::evse_security::LeafCertificateType::V2G;
    case ocpp::CertificateSigningUseEnum::ManufacturerCertificate:
        return types::evse_security::LeafCertificateType::MF;
    case ocpp::CertificateSigningUseEnum::V2G20Certificate:
        throw std::out_of_range("Could not convert ocpp::CertificateSigningUseEnum::V2G20Certificate to "
                                "evse_security::LeafCertificateType");
    }
    throw std::out_of_range(
        "Could not convert ocpp::CertificateSigningUseEnum to types::evse_security::LeafCertificateType");
}

types::evse_security::LeafCertificateType from_ocpp(ocpp::LeafCertificateType other) {
    switch (other) {
    case ocpp::LeafCertificateType::CSMS:
        return types::evse_security::LeafCertificateType::CSMS;
    case ocpp::LeafCertificateType::V2G:
        return types::evse_security::LeafCertificateType::V2G;
    case ocpp::LeafCertificateType::MF:
        return types::evse_security::LeafCertificateType::MF;
    case ocpp::LeafCertificateType::MO:
        return types::evse_security::LeafCertificateType::MO;
    }
    throw std::out_of_range(
        "Could not convert ocpp::CertificateSigningUseEnum to types::evse_security::LeafCertificateType");
}

types::evse_security::CertificateType from_ocpp(ocpp::CertificateType other) {
    switch (other) {
    case ocpp::CertificateType::V2GRootCertificate:
        return types::evse_security::CertificateType::V2GRootCertificate;
    case ocpp::CertificateType::MORootCertificate:
        return types::evse_security::CertificateType::MORootCertificate;
    case ocpp::CertificateType::CSMSRootCertificate:
        return types::evse_security::CertificateType::CSMSRootCertificate;
    case ocpp::CertificateType::V2GCertificateChain:
        return types::evse_security::CertificateType::V2GCertificateChain;
    case ocpp::CertificateType::MFRootCertificate:
        return types::evse_security::CertificateType::MFRootCertificate;
    case ocpp::CertificateType::OEMRootCertificate:
        throw std::out_of_range("Could not convert ocpp::CertificateType::OEMRootCertificate to "
                                "evse_security::CertificateType");
    }
    throw std::out_of_range("Could not convert ocpp::CertificateType to types::evse_security::CertificateType");
}

types::evse_security::HashAlgorithm from_ocpp(ocpp::HashAlgorithmEnumType other) {
    switch (other) {
    case ocpp::HashAlgorithmEnumType::SHA256:
        return types::evse_security::HashAlgorithm::SHA256;
    case ocpp::HashAlgorithmEnumType::SHA384:
        return types::evse_security::HashAlgorithm::SHA384;
    case ocpp::HashAlgorithmEnumType::SHA512:
        return types::evse_security::HashAlgorithm::SHA512;
    }
    throw std::out_of_range("Could not convert ocpp::HashAlgorithmEnumType to types::evse_security::HashAlgorithm");
}

types::evse_security::InstallCertificateResult from_ocpp(ocpp::InstallCertificateResult other) {
    switch (other) {
    case ocpp::InstallCertificateResult::InvalidSignature:
        return types::evse_security::InstallCertificateResult::InvalidSignature;
    case ocpp::InstallCertificateResult::InvalidCertificateChain:
        return types::evse_security::InstallCertificateResult::InvalidCertificateChain;
    case ocpp::InstallCertificateResult::InvalidFormat:
        return types::evse_security::InstallCertificateResult::InvalidFormat;
    case ocpp::InstallCertificateResult::InvalidCommonName:
        return types::evse_security::InstallCertificateResult::InvalidCommonName;
    case ocpp::InstallCertificateResult::NoRootCertificateInstalled:
        return types::evse_security::InstallCertificateResult::NoRootCertificateInstalled;
    case ocpp::InstallCertificateResult::Expired:
        return types::evse_security::InstallCertificateResult::Expired;
    case ocpp::InstallCertificateResult::CertificateStoreMaxLengthExceeded:
        return types::evse_security::InstallCertificateResult::CertificateStoreMaxLengthExceeded;
    case ocpp::InstallCertificateResult::WriteError:
        return types::evse_security::InstallCertificateResult::WriteError;
    case ocpp::InstallCertificateResult::Accepted:
        return types::evse_security::InstallCertificateResult::Accepted;
    }
    throw std::out_of_range(
        "Could not convert ocpp::InstallCertificateResult to types::evse_security::InstallCertificateResult");
}

types::evse_security::DeleteCertificateResult from_ocpp(ocpp::DeleteCertificateResult other) {
    switch (other) {
    case ocpp::DeleteCertificateResult::Accepted:
        return types::evse_security::DeleteCertificateResult::Accepted;
    case ocpp::DeleteCertificateResult::Failed:
        return types::evse_security::DeleteCertificateResult::Failed;
    case ocpp::DeleteCertificateResult::NotFound:
        return types::evse_security::DeleteCertificateResult::NotFound;
    }
    throw std::out_of_range(
        "Could not convert ocpp::DeleteCertificateResult to types::evse_security::DeleteCertificateResult");
}

types::evse_security::CertificateHashData from_ocpp(ocpp::CertificateHashDataType other) {
    types::evse_security::CertificateHashData lhs;
    lhs.hash_algorithm = from_ocpp(other.hashAlgorithm);
    lhs.issuer_name_hash = other.issuerNameHash;
    lhs.issuer_key_hash = other.issuerKeyHash;
    lhs.serial_number = other.serialNumber;
    return lhs;
}

types::evse_security::CertificateHashDataChain from_ocpp(ocpp::CertificateHashDataChain other) {
    types::evse_security::CertificateHashDataChain lhs;
    lhs.certificate_type = from_ocpp(other.certificateType);
    lhs.certificate_hash_data = from_ocpp(other.certificateHashData);
    if (other.childCertificateHashData.has_value()) {
        std::vector<types::evse_security::CertificateHashData> v;
        for (const auto& certificate_hash_data : other.childCertificateHashData.value()) {
            v.push_back(from_ocpp(certificate_hash_data));
        }
        lhs.child_certificate_hash_data = v;
    }
    return lhs;
}

types::evse_security::OCSPRequestData from_ocpp(ocpp::OCSPRequestData other) {
    types::evse_security::OCSPRequestData lhs;
    types::evse_security::CertificateHashData certificate_hash_data;
    certificate_hash_data.issuer_name_hash = other.issuerNameHash;
    certificate_hash_data.issuer_key_hash = other.issuerKeyHash;
    certificate_hash_data.serial_number = other.serialNumber;
    certificate_hash_data.hash_algorithm = from_ocpp(other.hashAlgorithm);
    lhs.certificate_hash_data = certificate_hash_data;
    lhs.responder_url = other.responderUrl;
    return lhs;
}

types::evse_security::CertificateInfo from_ocpp(ocpp::CertificateInfo other) {
    types::evse_security::CertificateInfo lhs;
    lhs.certificate = other.certificate_path;
    lhs.certificate_single = other.certificate_single_path;
    lhs.certificate_count = other.certificate_count;
    lhs.key = other.key_path;
    lhs.password = other.password;
    return lhs;
}

} // namespace conversions
