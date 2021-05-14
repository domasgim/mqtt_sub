#include "curl_helper.h"
#include "uci_option_list.h"

extern int curl_send_email(event_t event, char *json_item, char *topic) {
    create_email_file(event, json_item, topic);

    CURL *curl;
    CURLcode res = CURLE_OK;
    struct curl_slist *recipients = NULL;

    FILE * fPtr;
    fPtr = fopen(EMAIL_PATH, "r");
    if (fPtr == NULL) {
        fprintf(stderr, "Unable to read a file on %s\n", EMAIL_PATH);
        return -1;
    }

    char * curl_url_string[64];
    char * curl_userpwd[1024];

    snprintf(curl_url_string, sizeof(curl_url_string), "smtps://%s:%s", event.smtp_ip, event.smtp_port);
    snprintf(curl_userpwd, sizeof(curl_userpwd), "%s:%s", event.username, event.password);

    curl = curl_easy_init();
    if(curl) {
        curl_easy_setopt(curl, CURLOPT_URL, curl_url_string);
        curl_easy_setopt(curl, CURLOPT_MAIL_FROM, event.senderemail);
        recipients = curl_slist_append(recipients, event.recip_email);
        curl_easy_setopt(curl, CURLOPT_MAIL_RCPT, recipients);

        curl_easy_setopt(curl, CURLOPT_READDATA, fPtr);
        curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);
        curl_easy_setopt(curl, CURLOPT_USERPWD, curl_userpwd);
        curl_easy_setopt(curl, CURLOPT_USE_SSL, (long)CURLUSESSL_ALL);

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

static int create_email_file(event_t event, char *json_item, char *topic) {
    FILE * fPtr;
    fPtr = fopen(EMAIL_PATH, "w");
    if (fPtr == NULL) {
        fprintf(stderr, "Unable to create a file on %s\n", EMAIL_PATH);
        return -1;
    }

    char payload_buffer[2048];
    snprintf(payload_buffer, sizeof(payload_buffer), "From: <%s>\nTo: <%s>\nSubject: MQTT event\nMQTT subscriber received a JSON value and met the condition\n%s - %s: %s %s %s\n",
        event.senderemail, event.recip_email, topic, event.json_val, json_item, event.operator_str, event.comparison_val);

    fputs(payload_buffer, fPtr);
    fclose(fPtr);
    return 0;
}