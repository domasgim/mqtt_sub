#include "mqtt_sub.h"

int curl_send_email(char * email_group, char * recipient_email, char * topic, 
    char * json_name, cJSON * json_value, char * operator, char * comparison_val) {
    int user_group_iteration = uci_get_user_group_iteration(email_group);
    if (user_group_iteration == -1) {
        fprintf(stderr, "Error: %s email group was not found\n", email_group);
        return -1;
    }

    char secure_conn_command_buffer[64];
    char smtp_ip_command_buffer[64];
    char smtp_port_command_buffer[64];
    char username_command_buffer[64];
    char password_command_buffer[64];
    char senderemail_command_buffer[64];

    snprintf(secure_conn_command_buffer, 64, "user_groups.@email[%d].secure_conn", user_group_iteration);
    snprintf(smtp_ip_command_buffer, 64, "user_groups.@email[%d].smtp_ip", user_group_iteration);
    snprintf(smtp_port_command_buffer, 64, "user_groups.@email[%d].smtp_port", user_group_iteration);
    snprintf(username_command_buffer, 64, "user_groups.@email[%d].username", user_group_iteration);
    snprintf(password_command_buffer, 64, "user_groups.@email[%d].password", user_group_iteration);
    snprintf(senderemail_command_buffer, 64, "user_groups.@email[%d].senderemail", user_group_iteration);

    char * secure_conn = uci_get_config_entry_V2(secure_conn_command_buffer);
    char * smtp_ip = uci_get_config_entry_V2(smtp_ip_command_buffer);
    char * smtp_port = uci_get_config_entry_V2(smtp_port_command_buffer);
    char * username = uci_get_config_entry_V2(username_command_buffer);
    char * password = uci_get_config_entry_V2(password_command_buffer);
    char * senderemail = uci_get_config_entry_V2(senderemail_command_buffer);

    if(smtp_ip == NULL || smtp_port == NULL || secure_conn == NULL || senderemail == NULL) {
        fprintf(stderr, "Null values found in user_groups uci configuration\n");
        return -1;
    }

    if(cJSON_IsString(json_value)) {
        char * json_value_str = json_value->valuestring;
        create_email_file(senderemail, recipient_email, topic, json_name, json_value_str, operator, comparison_val);
    } else if (cJSON_IsNumber(json_value)) {
        char json_value_str[256];
        snprintf(json_value_str, sizeof(json_value_str), "%d", json_value->valueint);
        create_email_file(senderemail, recipient_email, topic, json_name, json_value_str, operator, comparison_val);
    }

    CURL *curl;
    CURLcode res = CURLE_OK;
    struct curl_slist *recipients = NULL;

    FILE * fPtr;
    fPtr = fopen(EMAIL_PATH, "r");
    if (fPtr == NULL) {
        fprintf(stderr, "Unable to create a file on /tmp/email_text.txt\n");
        return -1;
    }

    char * curl_url_string[64];
    char * curl_userpwd[1024];

    snprintf(curl_url_string, sizeof(curl_url_string), "smtps://%s:%s", smtp_ip, smtp_port);
    snprintf(curl_userpwd, sizeof(curl_userpwd), "%s:%s", username, password);

    curl = curl_easy_init();
    if(curl) {
        curl_easy_setopt(curl, CURLOPT_URL, curl_url_string);
        curl_easy_setopt(curl, CURLOPT_MAIL_FROM, senderemail);
        recipients = curl_slist_append(recipients, recipient_email);
        curl_easy_setopt(curl, CURLOPT_MAIL_RCPT, recipients);

        curl_easy_setopt(curl, CURLOPT_READDATA, fPtr);
        curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);
        curl_easy_setopt(curl, CURLOPT_USERPWD, curl_userpwd);
        curl_easy_setopt(curl, CURLOPT_USE_SSL, (long)CURLUSESSL_ALL);
        // curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);

        res = curl_easy_perform(curl);

        if(res != CURLE_OK)
        fprintf(stderr, "curl_easy_perform() failed: %s\n",
                curl_easy_strerror(res));

        curl_slist_free_all(recipients);

        curl_easy_cleanup(curl);
    }
    fclose(fPtr);
    return (int)res;
}