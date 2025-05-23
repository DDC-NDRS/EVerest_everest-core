// SPDX-License-Identifier: Apache-2.0
// Copyright Pionix GmbH and Contributors to EVerest

#include <conversions.hpp>
#include <generated/interfaces/ISO15118_charger/Implementation.hpp>
#include <ocpp/v2/messages/Authorize.hpp>

#include "auth_token_validatorImpl.hpp"

namespace module {
namespace auth_validator {

void auth_token_validatorImpl::init() {
}

void auth_token_validatorImpl::ready() {
}

types::authorization::ValidationResult
auth_token_validatorImpl::handle_validate_token(types::authorization::ProvidedIdToken& provided_token) {
    types::authorization::ValidationResult validation_result;
    try {
        const auto id_token = conversions::to_ocpp_id_token(provided_token.id_token);

        std::optional<ocpp::CiString<10000>> certificate_opt;
        if (provided_token.certificate.has_value()) {
            certificate_opt.emplace(provided_token.certificate.value());
        }
        std::optional<std::vector<ocpp::v2::OCSPRequestData>> ocsp_request_data_opt;
        if (provided_token.iso15118CertificateHashData.has_value()) {
            ocsp_request_data_opt =
                conversions::to_ocpp_ocsp_request_data_vector(provided_token.iso15118CertificateHashData.value());
        }

        // request response
        const auto response = this->mod->charge_point->validate_token(id_token, certificate_opt, ocsp_request_data_opt);
        validation_result = conversions::to_everest_validation_result(response);
    } catch (const ocpp::StringConversionException& e) {
        EVLOG_warning << "Error converting id token to validate: " << e.what();
        validation_result.authorization_status = types::authorization::AuthorizationStatus::Unknown;
    } catch (const std::exception& e) {
        EVLOG_warning << "Unknown error during validation of id token: " << e.what();
        validation_result.authorization_status = types::authorization::AuthorizationStatus::Unknown;
    }
    return validation_result;
};

} // namespace auth_validator
} // namespace module
