#include <iostream>
#include <fstream>
#include <curl/curl.h>
#include <playdl/login.h>
#include <playdl/device_info.h>
#include <playdl/config.h>
#include <playdl/checkin.h>
#include "gsf.pb.h"
#include "config.h"

using namespace playdl;

app_config conf;

void do_interactive_auth(login_manager& login) {
    auth_select_method:
    std::cout << "We haven't authorized you yet. Please select your preferred login method:" << std::endl;
    std::cout << "1. via login and password" << std::endl;
    std::cout << "2. via access token" << std::endl;
    std::cout << "3. via master token" << std::endl;
    std::cout << "Please type the selected number and press enter: ";
    std::string method;
    std::getline(std::cin, method);
    std::cout << "\n";
    if (method == "1") {
        // via login & password
        do_login_pass_auth:
        std::cout << "Please enter your email: ";
        std::string email;
        std::getline(std::cin, email);
        std::cout << "Please enter password for your account: ";
        std::string password;
        std::getline(std::cin, password);
        std::cout << "Authenticating..." << std::endl;
        try {
            login.perform_login_using_password(email, password);
        } catch (std::runtime_error err) {
            std::cout << "Failed to login using the specified credentials: " << err.what() << std::endl;
            goto do_login_pass_auth;
        }
        std::cout << "Logged you in successfully." << std::endl;
        std::cout << std::endl;
    } else if (method == "2") {
        // via master token
        do_token_auth:
        std::cout << "Please enter your access token: ";
        std::string token;
        std::getline(std::cin, token);
        login.perform_login_using_access_token(token);
        try {
            login.perform_login();
        } catch (std::runtime_error err) {
            std::cout << "Failed to login using the specified token: " << err.what() << std::endl << std::endl;
            goto do_token_auth;
        }
        std::cout << std::endl;
    } else if (method == "3") {
        // via master token
        do_master_token_auth:
        std::cout << "Please enter your master token: ";
        std::string token;
        std::getline(std::cin, token);
        login.set_token("", token);
        login.perform_login();
        try {
            login.perform_login();
        } catch (std::runtime_error err) {
            std::cout << "Failed to login using the specified token: " << err.what() << std::endl << std::endl;
            goto do_master_token_auth;
        }
        std::cout << std::endl;
    } else {
        std::cout << "You have entered an invalid number." << std::endl << std::endl;
        goto auth_select_method;
    }

    if (login.has_token()) {
        std::cout << "We can save a token on your hard drive that will allow you to use this application"
                  << " without having to sign in again." << std::endl;
        std::cout << "Would you like to store the token? [Y/n]: ";
        std::string answ;
        std::getline(std::cin, answ);
        if (answ.length() == 0 || answ[0] == 'Y' || answ[0] == 'y') {
            conf.user_email = login.get_email();
            conf.user_token = login.get_token();
            conf.save();
            std::cout << "Saved the token." << std::endl;
        }
    } else {
        std::cout << "Something went wrong. Let's try again." << std::endl;
        goto auth_select_method;
    }
}
void do_auth_from_config(login_manager& login) {
    do_auth:
    login.set_token(conf.user_email, conf.user_token);
    try {
        login.perform_login();
    } catch (std::runtime_error err) {
        std::cout << "Failed to login using a saved token: " << err.what() << std::endl;
        std::cout << "Would you like to delete it and authenticate again? [y/N]";
        std::string answ;
        std::getline(std::cin, answ);
        if (answ[0] == 'Y' || answ[0] == 'y') {
            conf.user_email = "";
            conf.user_token = "";
            do_interactive_auth(login);
            return;
        }
        goto do_auth;
    }
}

int main() {
    curl_global_init(CURL_GLOBAL_ALL);
    GOOGLE_PROTOBUF_VERIFY_VERSION;

    conf.load();

    device_info device;

    checkin_handler checkin (device);
    if (conf.user_token.length() <= 0) {
        do_interactive_auth(checkin.get_login());
    } else {
        do_auth_from_config(checkin.get_login());
    }
    checkin.do_checkin();

    curl_global_cleanup();
    google::protobuf::ShutdownProtobufLibrary();
    return 0;
}