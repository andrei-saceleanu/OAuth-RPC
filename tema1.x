enum response_code {
    OK = 0,
    USER_NOT_FOUND = 1,
    REQUEST_DENIED = 2,
    PERMISSION_DENIED = 3,
    TOKEN_EXPIRED = 4,
    RESOURCE_NOT_FOUND = 5,
    OPERATION_NOT_PERMITTED = 6,
    PERMISSION_GRANTED = 7
};

struct auth_response{
    string token<>;
    response_code code;
};

struct access_response{
    string token<>;
    string refresh_token<>;
    int availability;
    response_code code;
};

struct access_params{
    string id<>;
    string token<>;
    int refresh;
};

struct refresh_params{
    string access_token<>;
    string refresh_token<>;
    int refresh;
};

struct valid_action_params{
    string op<>;
    string resource<>;
    string access_token<>;
};

program TEMA1_PROG{
    version TEMA1_VERS{
        auth_response request_auth_token(string) = 1;
        access_response request_access_token(struct access_params) = 2;
        int validate_delegated_action(struct valid_action_params) = 3;
        string approve_auth_token(string) = 4;
        access_response get_fresh_token(struct refresh_params) = 5;
    } = 1;
} = 0x12345678;