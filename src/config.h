#pragma once

#include <string>
#include <map>
#include <playapi/util/config.h>
#include <playapi/checkin.h>

namespace playapi { struct device_info; class api; }

struct app_config {

    playapi::config config;

    std::string user_email, user_token;

    void load();

    void save();

};

struct device_config {

    playapi::config config;
    std::string path;

    playapi::checkin_result checkin_data;

    device_config(const std::string& path) : path(path) {
        load();
    }


    void load();

    void save();

    void load_device_info_data(playapi::device_info& dev);

    void set_device_info_data(const playapi::device_info& dev);

    void load_api_data(const std::string& email, playapi::api& api);

    void set_api_data(const std::string& email, const playapi::api& api);

};