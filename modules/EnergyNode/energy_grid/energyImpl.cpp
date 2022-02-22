// SPDX-License-Identifier: Apache-2.0
// Copyright 2022 - 2022 Pionix GmbH and Contributors to EVerest

#include "energyImpl.hpp"
#include <boost/uuid/random_generator.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <chrono>
#include <date/date.h>

namespace module {
namespace energy_grid {

std::string to_rfc3339(std::chrono::time_point<std::chrono::system_clock> t) {
    return date::format("%FT%TZ", std::chrono::time_point_cast<std::chrono::milliseconds>(t));
}

std::chrono::time_point<std::chrono::system_clock> from_rfc3339(std::string t) {
    std::istringstream infile{t};
    std::chrono::time_point<std::chrono::system_clock> tp;
    infile >> date::parse("%FT%T", tp);

    // std::cout <<"timepoint"<<" "<<t<<" "<< tp.time_since_epoch().count()<<std::endl;
    return tp;
}

void energyImpl::init() {
    energy_price = {};
    initializeEnergyObject();

    mod->r_energy_consumer->subscribe_energy([this](json e) {
        // Received new energy object from a child. Update in the cached object and republish.
        // FIXME: this will need to handle multiple childs once 1:N requirement support is in framework
        energy["children"] = {e};

        json schedule_entry;
        schedule_entry["timestamp"] = to_rfc3339(std::chrono::system_clock::now());
        schedule_entry["capabilities"]["limit_type"] = "Hard";
        schedule_entry["capabilities"]["ac_current_A"] = {{"max_current_A", mod->config.fuse_limit_A},
                                                   {"max_phase_count", mod->config.phase_count}};

        energy["schedule_import"] = json::array({schedule_entry});
        // std::cout << energy << std::endl;
        publish_complete_energy_object();
    });

    // FIXME this is optional
    mod->r_price_information->subscribe_energy_price_schedule([this](json p) {
        // std::cout << energy << std::endl;
        energy_price = p;
        publish_complete_energy_object();
    });

    // FIXME this is optional
    mod->r_powermeter->subscribe_powermeter([this](json p) {
        powermeter = p;
        publish_complete_energy_object();
    });
}

void energyImpl::publish_complete_energy_object() {
    // join the different schedules to the complete array (with resampling)

    json energy_complete = energy;
    // FIXME deal with non set properties!
    if (!energy["schedule_import"].is_null())
        energy_complete["schedule_import"] =
            merge_price_into_schedule(energy["schedule_import"], energy_price["schedule_import"]);

    if (!powermeter.is_null())
        energy_complete["energy_usage"] = powermeter;

    publish_energy(energy_complete);
}

json energyImpl::merge_price_into_schedule(json schedule, json price) {
    if (schedule.is_null())
        return json({});
    else if (price.is_null())
        return schedule;

    auto it_schedule = schedule.begin();
    auto it_price = price.begin();

    json joined_array = json::array();
    // The first element is already valid now even if the timestamp is in the future (per agreement)
    json next_entry_schedule = *it_schedule;
    json next_entry_price = *it_price;
    json currently_valid_entry_schedule = next_entry_schedule;
    json currently_valid_entry_price = next_entry_price;

    while (true) {
        if (it_schedule == schedule.end() && it_price == price.end())
            break;

        auto tp_schedule = from_rfc3339(next_entry_schedule["timestamp"]);
        auto tp_price = from_rfc3339(next_entry_price["timestamp"]);

        if (tp_schedule < tp_price && it_schedule != schedule.end() || it_price == price.end()) {
            currently_valid_entry_schedule = next_entry_schedule;
            json joined_entry = currently_valid_entry_schedule;

            joined_entry["price_per_kwh"] = currently_valid_entry_price["price_per_kwh"];
            joined_array.push_back(joined_entry);
            it_schedule++;
            if (it_schedule != schedule.end()) {
                next_entry_schedule = *it_schedule;
            }
            continue;
        }
        if (tp_price < tp_schedule && it_price != price.end() || it_schedule == schedule.end()) {
            currently_valid_entry_price = next_entry_price;
            json joined_entry = currently_valid_entry_schedule;
            joined_entry["price_per_kwh"] = currently_valid_entry_price["price_per_kwh"];
            joined_entry["timestamp"] = currently_valid_entry_price["timestamp"];
            joined_array.push_back(joined_entry);
            it_price++;
            if (it_price != price.end()) {
                next_entry_price = *it_price;
            }
            continue;
        }
    }
    // std::cout << joined_array.dump(4) << std::endl;
    return joined_array;
}

void energyImpl::ready() {
    // publish my own limits at least once
    publish_energy(energy);
}

void energyImpl::handle_enforce_limits(std::string& uuid, Object& limits_import, Object& limits_export,
                                       Array& schedule_import, Array& schedule_export) {
    // is it for me?
    if (uuid == energy["uuid"]) {
        // as a generic node we cannot do much about limits.
        EVLOG(error) << "EnergyNode cannot accept limits from EnergyManager";
    }
    // if not, route to children
    else
        mod->r_energy_consumer->call_enforce_limits(uuid, limits_import, limits_export, schedule_import,
                                                    schedule_export);
};

void energyImpl::initializeEnergyObject() {
    energy["children"] = json::array();

    energy["node_type"] = "Fuse"; // FIXME: node types need to be figured out
    
    // UUID must be unique also beyond this charging station
    energy["uuid"] = mod->info.id + "_" + boost::uuids::to_string(boost::uuids::random_generator()());
}

} // namespace energy_grid
} // namespace module
