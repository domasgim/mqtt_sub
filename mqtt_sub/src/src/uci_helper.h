#ifndef UCI_HELPER_H
#define UCI_HELPER_H

#include <uci.h>

/**
 * @brief Get UCI configuration entry object
 * 
 * @param path UCI configuration file name
 * @param option UCI configuration entry name
 * @return char* option
 */
extern char * uci_get_config_entry(char *path);

/**
 * @brief Find the user_group UCI configuration section iteration according to the specified email group. 
 * Used to later specify this number while getting user_group information
 * 
 * @param email_group Email group's name which will be used to search in user_group UCI configuration
 * @return int iteration number, -1 for error
 */
extern int uci_get_user_group_iteration(char * email_group);

/**
 * @brief Find the section ID of a specifed topic. Used to later read the topi's event information
 * 
 * @param topic topic name
 * @return char* section ID or NULL if nothing was found
 */
extern char * uci_get_topic_section_id(char * topic);

#endif