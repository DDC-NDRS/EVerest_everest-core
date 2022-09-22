// SPDX-License-Identifier: Apache-2.0
// Copyright Pionix GmbH and Contributors to EVerest

#ifndef _AUTH_HANDLER_HPP_
#define _AUTH_HANDLER_HPP_

#include <condition_variable>
#include <list>
#include <map>
#include <memory>
#include <mutex>
#include <set>

#include <utils/types.hpp>

#include <generated/types/authorization.hpp>
#include <generated/types/evse_manager.hpp>
#include <generated/types/reservation.hpp>

#include <Connector.hpp>
#include <ReservationHandler.hpp>

using namespace types::evse_manager;
using namespace types::authorization;
using namespace types::reservation;

namespace module {

/**
 * @brief This class handles authorization and reservation requests. It keeps track of the state of each connector and
 * validates incoming token and reservation requests accordingly.
 *
 */
class AuthHandler {

public:
    AuthHandler(const SelectionAlgorithm& selection_algorithm, const int connection_timeout,
                bool prioritize_authorization_over_stopping_transaction);
    virtual ~AuthHandler();

    /**
     * @brief Initializes the connector with the given \p connector_id and the given \p evse_id . It instantiates new
     * connector objects and fills data sturctures of the class.
     *
     * @param connector_id
     * @param evse_index
     */
    void init_connector(const int connector_id, const int evse_index);

    /**
     * @brief Handler for a new incoming \p provided_token
     *
     * @param provided_token
     */
    void on_token(const ProvidedIdToken& provided_token);

    /**
     * @brief Handler for new incoming \p reservation for the given \p connector . Places the reservation if possible.
     *
     * @param connector_id
     * @param reservation
     * @return types::reservation::ReservationResult
     */
    types::reservation::ReservationResult handle_reservation(int connector_id, const Reservation& reservation);

    /**
     * @brief Handler for incoming cancel reservation request for the given \p reservation_id .
     *
     * @param reservation_id
     * @return int Returns -1 if the reservation could not been cancelled, else the id of the connector.
     */
    int handle_cancel_reservation(int reservation_id);

    /**
     * @brief Callback to signal EvseManager that the given \p connector_id has been reserved with the given \p
     * reservation_id .
     *
     * @param connector_id
     * @param reservation_id
     */
    void call_reserved(const int& connector_id, const int reservation_id);

    /**
     * @brief Callback to signal EvseManager that the reservation for the given \p connector_id has been cancelled.
     *
     * @param connector_id
     */
    void call_reservation_cancelled(const int& connector_id);

    /**
     * @brief Handler for the given \p events at the given \p connector . Submits events to the state machine of the
     * handler.
     *
     * @param connector_id
     * @param events
     */
    void handle_session_event(const int connector_id, const SessionEvent& events);

    /**
     * @brief Set the connection timeout of the handler.
     *
     * @param connection_timeout
     */
    void set_connection_timeout(const int connection_timeout);

    /**
     * @brief Set the prioritize authorization over stopping transaction flag of the handler.
     *
     * @param b
     */
    void set_prioritize_authorization_over_stopping_transaction(bool b);

    /**
     * @brief Registers the given \p callback to provide authorization.
     *
     * @param callback
     */
    void register_authorize_callback(
        const std::function<void(const int evse_index, const std::string& id_token)>& callback);

    /**
     * @brief Registers the given \p callback to withdraw authorization.
     *
     * @param callback
     */
    void register_withdraw_authorization_callback(const std::function<void(const int evse_index)>& callback);

    /**
     * @brief Registers the given \p callback to validate a token.
     *
     * @param callback
     */
    void register_validate_token_callback(
        const std::function<std::vector<ValidationResult>(const std::string& id_token)>& callback);

    /**
     * @brief Registers the given \p callback to stop a transaction at an EvseManager.
     *
     * @param callback
     */
    void register_stop_transaction_callback(
        const std::function<void(const int evse_index, const StopTransactionReason& reason)>& callback);

    /**
     * @brief Registers the given \p callback to signal a reservation to an EvseManager.
     *
     * @param callback
     */
    void register_reserved_callback(
        const std::function<void(const int& evse_index, const int& reservation_id)>& callback);

    /**
     * @brief Registers the given \p callback to signal a reservation has been cancelled to the EvseManager.
     *
     * @param callback
     */
    void register_reservation_cancelled_callback(const std::function<void(const int& evse_index)>& callback);

private:
    SelectionAlgorithm selection_algorithm;
    int connection_timeout;
    bool prioritize_authorization_over_stopping_transaction;
    ReservationHandler reservation_handler;

    std::map<int, std::unique_ptr<ConnectorContext>> connectors;

    std::mutex timer_mutex;
    std::list<int> plug_in_queue;
    std::mutex auth_queue_mutex;
    std::set<std::string> tokens_in_process;
    std::mutex token_in_process_mutex;
    std::condition_variable cv;

    // callbacks
    std::function<void(const int evse_index, const std::string& id_token)> authorize_callback;
    std::function<void(const int evse_index)> withdraw_authorization_callback;
    std::function<std::vector<ValidationResult>(const std::string& id_token)> validate_token_callback;
    std::function<void(const int evse_index, const StopTransactionReason& reason)> stop_transaction_callback;
    std::function<void(const Array& reservations)> reservation_update_callback;
    std::function<void(const int& evse_index, const int& reservation_id)> reserved_callback;
    std::function<void(const int& evse_index)> reservation_cancelled_callback;

    std::vector<int> get_referenced_connectors(const ProvidedIdToken& provided_token);
    int used_for_transaction(const std::vector<int>& connectors, const std::string& id_token);
    bool is_token_already_in_process(const std::string& id_token);
    bool any_connector_available(const std::vector<int>& connectors);

    void handle_token(const ProvidedIdToken& provided_token);

    /**
     * @brief Method selects a connector based on the configured selection algorithm. It might block until an event
     * occurs that can be used to determine a connector.
     *
     * @param connectors
     * @return int
     */
    int select_connector(const std::vector<int>& connectors);

    void lock_referenced_connectors(const std::vector<int>& connectors);
    void unlock_referenced_connectors(const std::vector<int>& connectors);
    int get_latest_plugin(const std::vector<int>& connectors);
    void authorize_evse(int connector, const Identifier& identifier);
    Identifier get_identifier(const ValidationResult& validation_result, const std::string& id_token);
};

} // namespace module

#endif //_AUTH_HANDLER_HPP_