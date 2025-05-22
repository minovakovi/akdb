/**
@file observable_custom.c Implementation file that provides custom observable pattern implementations
 */
/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor Boston, MA 02110-1301,  USA
 */

#include "observable.h"
#include "observable_custom.h"
#include <stdio.h>

/**
 * @author unknown, updated by Vilim Trakoštanec
 * @brief Method for retrieving message from observable custom type
 * @param self Pointer to AK_TypeObservable instance
 * @return char* Message string if available, NULL if error occurs or no message set
 */
char * AK_get_message(AK_TypeObservable *self) {
    char *ret;
    AK_PRO;
    
    if (self == NULL) {
        AK_dbg_messg(LOW, GLOBAL, "ERROR: NULL observable passed to AK_get_message");
        AK_EPI;
        return NULL;
    }

    if (self->notifyDetails == NULL) {
        AK_dbg_messg(LOW, GLOBAL, "ERROR: notifyDetails not set in observable");
        AK_EPI;
        return NULL;
    }

    ret = self->notifyDetails->message;
    if (ret == NULL) {
        AK_dbg_messg(LOW, GLOBAL, "WARNING: Empty message in notifyDetails");
    }
    
    AK_EPI;
    return ret;
}

/**
 * @author unknown, updated by Vilim Trakoštanec
 * @brief Method for registering an observer to custom observable type
 * @param self Pointer to AK_TypeObservable instance
 * @param observer Pointer to AK_observer instance to register
 * @return int Registration status (OBSERVER_REGISTER_SUCCESS on success, NOT_OK on failure)
 */
int AK_custom_register_observer(AK_TypeObservable* self, AK_observer* observer) {
    int result;
    AK_PRO;

    if (self == NULL || observer == NULL) {
        AK_dbg_messg(LOW, GLOBAL, "ERROR: NULL pointer passed to AK_custom_register_observer");
        AK_EPI;
        return NOT_OK;
    }

    if (self->observable == NULL) {
        AK_dbg_messg(LOW, GLOBAL, "ERROR: Observable not initialized in AK_custom_register_observer");
        AK_EPI;
        return NOT_OK;
    }

    result = self->observable->AK_register_observer(self->observable, observer);
    if (result == OBSERVER_REGISTER_SUCCESS) {
        AK_dbg_messg(LOW, GLOBAL, "Observer successfully registered to custom observable");
    } else {
        AK_dbg_messg(LOW, GLOBAL, "Failed to register observer to custom observable");
    }

    AK_EPI;
    return result;
}

/**
 * @author unknown, updated by Vilim Trakoštanec
 * @brief Method for unregistering an observer from custom observable type
 * @param self Pointer to AK_TypeObservable instance
 * @param observer Pointer to AK_observer instance to unregister
 * @return int Unregistration status (OBSERVER_UNREGISTER_SUCCESS on success, NOT_OK on failure)
 */
int AK_custom_unregister_observer(AK_TypeObservable * self, AK_observer* observer) {
    int result;
    AK_PRO;

    if (self == NULL || observer == NULL) {
        AK_dbg_messg(LOW, GLOBAL, "ERROR: NULL pointer passed to AK_custom_unregister_observer");
        AK_EPI;
        return NOT_OK;
    }

    if (self->observable == NULL) {
        AK_dbg_messg(LOW, GLOBAL, "ERROR: Observable not initialized in AK_custom_unregister_observer");
        AK_EPI;
        return NOT_OK;
    }

    result = self->observable->AK_unregister_observer(self->observable, observer);
    if (result == OBSERVER_UNREGISTER_SUCCESS) {
        AK_dbg_messg(LOW, GLOBAL, "Observer successfully unregistered from custom observable");
    } else {
        AK_dbg_messg(LOW, GLOBAL, "Failed to unregister observer from custom observable");
    }

    AK_EPI;
    return result;
}

/**
 * @author unknown, updated by Vilim Trakoštanec
 * @brief Helper method for setting notification details in observable
 * @param self Pointer to AK_TypeObservable instance
 * @param type Type of notification (ERROR, WARNING, or INFO)
 * @param message Message string to be set in notification details
 */
void AK_set_notify_info_details(AK_TypeObservable *self, NotifyType type, char *message) {
    NotifyDetails *notifyDetails;
    AK_PRO;

    if (self == NULL) {
        AK_dbg_messg(LOW, GLOBAL, "ERROR: NULL observable passed to AK_set_notify_info_details");
        AK_EPI;
        return;
    }

    if (message == NULL) {
        AK_dbg_messg(LOW, GLOBAL, "WARNING: NULL message passed to AK_set_notify_info_details");
    }

    // Validate notify type
    if (type < ERROR || type > WARNING) {
        AK_dbg_messg(LOW, GLOBAL, "ERROR: Invalid notify type: %d", type);
        AK_EPI;
        return;
    }

    notifyDetails = AK_calloc(1, sizeof(NotifyDetails));
    if (notifyDetails == NULL) {
        AK_dbg_messg(LOW, GLOBAL, "ERROR: Memory allocation failed for notifyDetails");
        AK_EPI;
        return;
    }

    notifyDetails->message = message;
    notifyDetails->type = type;

    // Free previous notifyDetails if it exists
    if (self->notifyDetails != NULL) {
        AK_free(self->notifyDetails);
    }
    
    self->notifyDetails = notifyDetails;
    AK_dbg_messg(LOW, GLOBAL, "Notify details set successfully with type: %d", type);
    AK_EPI;
}

/**
 * @author unknown, updated by Vilim Trakoštanec
 * @brief Custom action implementation for observable pattern
 * @param data Optional data parameter (currently unused)
 * @return int Operation status (OK on success)
 */
int AK_custom_action(void *data) {
    AK_PRO;
    
    // Even though we don't use data parameter currently, we should validate it
    if (data != NULL) {
        AK_dbg_messg(LOW, GLOBAL, "WARNING: Non-NULL data passed to AK_custom_action but not used");
    }
    
    printf("THIS IS SOME CUSTOM FUNCTION!\n");
    AK_dbg_messg(LOW, GLOBAL, "Custom action executed successfully");
    
    AK_EPI;
    return OK;
}

/**
 * @author unknown, updated by Vilim Trakoštanec
 * @brief Initialize and set up a new observable type instance
 * @return AK_TypeObservable* Pointer to initialized observable type, NULL on failure
 */
AK_TypeObservable * init_observable_type() {
    AK_TypeObservable *self;
    AK_PRO;
    
    self = AK_calloc(1, sizeof(AK_TypeObservable));
    if (self == NULL) {
        AK_dbg_messg(LOW, GLOBAL, "ERROR: Memory allocation failed in init_observable_type");
        AK_EPI;
        return NULL;
    }

    // Initialize function pointers
    self->AK_get_message = AK_get_message;
    self->AK_custom_register_observer = &AK_custom_register_observer;
    self->AK_custom_unregister_observer = &AK_custom_unregister_observer;
    self->AK_set_notify_info_details = &AK_set_notify_info_details;
    
    // Initialize observable
    self->observable = AK_init_observable(self, AK_CUSTOM_FIRST, &AK_custom_action);
    if (self->observable == NULL) {
        AK_dbg_messg(LOW, GLOBAL, "ERROR: Failed to initialize observable in init_observable_type");
        AK_free(self);
        AK_EPI;
        return NULL;
    }
    
    AK_dbg_messg(LOW, GLOBAL, "Observable type initialized successfully");
    AK_EPI;
    return self;
}

/**
 * @author unknown, updated by Vilim Trakoštanec
 * @brief Event handler for processing custom type observable events
 * @param observer Pointer to AK_TypeObserver instance handling the event
 * @param observable Pointer to AK_TypeObservable instance that triggered the event
 */
void handle_AK_custom_type(AK_TypeObserver *observer, AK_TypeObservable *observable) {
    char *message;
    AK_PRO;

    if (observer == NULL || observable == NULL) {
        AK_dbg_messg(LOW, GLOBAL, "ERROR: NULL pointer passed to handle_AK_custom_type");
        AK_EPI;
        return;
    }

    if (observable->notifyDetails == NULL) {
        AK_dbg_messg(LOW, GLOBAL, "ERROR: notifyDetails not set in observable");
        AK_EPI;
        return;
    }

    switch(observable->notifyDetails->type) {
    case ERROR:
        message = observable->AK_get_message(observable);
        if (message) {
            printf("ERROR: %s\n", message);
            AK_dbg_messg(LOW, GLOBAL, "Error message handled: %s", message);
        }
        break;
    case WARNING:
        message = observable->AK_get_message(observable);
        if (message) {
            printf("WARNING: %s\n", message);
            AK_dbg_messg(LOW, GLOBAL, "Warning message handled: %s", message);
        }
        break;
    case INFO:
        message = observable->AK_get_message(observable);
        if (message) {
            AK_dbg_messg(LOW, GLOBAL, "Info message handled: %s", message);
        }
        break;
    default:
        AK_dbg_messg(LOW, GLOBAL, "ERROR: Unknown notification type: %d", observable->notifyDetails->type);
    }
    AK_EPI;
}

/**
 * @author unknown, updated by Vilim Trakoštanec
 * @brief Generic event handler for all observer types
 * @param observer Pointer to observer instance
 * @param observable Pointer to observable instance
 * @param AK_ObservableType_Def Type of the observable event
 */
void custom_observer_event_handler(void *observer, void *observable, AK_ObservableType_Enum AK_ObservableType_Def) {
    AK_PRO;

    if (observer == NULL || observable == NULL) {
        AK_dbg_messg(LOW, GLOBAL, "ERROR: NULL pointer passed to custom_observer_event_handler");
        AK_EPI;
        return;
    }

    switch(AK_ObservableType_Def) {
    case AK_TRANSACTION:
        AK_dbg_messg(LOW, GLOBAL, "Transaction event received");
        break;
    case AK_TRIGGER:
        AK_dbg_messg(LOW, GLOBAL, "Trigger event received");
        break;
    case AK_CUSTOM_FIRST:
        AK_dbg_messg(LOW, GLOBAL, "Custom first event received");
        handle_AK_custom_type((AK_TypeObserver*)observer, (AK_TypeObservable*)observable);
        break;
    case AK_CUSTOM_SECOND:
        AK_dbg_messg(LOW, GLOBAL, "Custom second event received");
        break;
    default:
        AK_dbg_messg(LOW, GLOBAL, "ERROR: Unknown observable type: %d", AK_ObservableType_Def);
    }
    AK_EPI;
}

/**
 * @author unknown, updated by Vilim Trakoštanec
 * @brief Initialize a new observer type with an associated observable
 * @param observable Pointer to observable instance to associate with the observer
 * @return AK_TypeObserver* Pointer to initialized observer type, NULL on failure
 */
AK_TypeObserver * init_observer_type(void *observable) {
    AK_TypeObserver *self;
    AK_PRO;
    
    self = AK_calloc(1, sizeof(AK_TypeObserver));
    if (self == NULL) {
        AK_dbg_messg(LOW, GLOBAL, "ERROR: Memory allocation failed in init_observer_type");
        AK_EPI;
        return NULL;
    }

    self->observable = observable;
    self->observer = AK_init_observer(self, &custom_observer_event_handler);
    if (self->observer == NULL) {
        AK_dbg_messg(LOW, GLOBAL, "ERROR: Failed to initialize observer in init_observer_type");
        AK_free(self);
        AK_EPI;
        return NULL;
    }
    
    AK_dbg_messg(LOW, GLOBAL, "Observer type initialized successfully");
    AK_EPI;
    return self;
}

/**
 * @author unknown, updated by Vilim Trakoštanec
 * @brief Initialize a new observer type without an associated observable
 * @return AK_TypeObserver* Pointer to initialized observer type, NULL on failure
 */
AK_TypeObserver * init_observer_type_second() {
    AK_TypeObserver_Second *self;
    AK_PRO;
    
    self = AK_calloc(1, sizeof(AK_TypeObserver_Second));
    if (self == NULL) {
        AK_dbg_messg(LOW, GLOBAL, "ERROR: Memory allocation failed in init_observer_type_second");
        AK_EPI;
        return NULL;
    }

    self->observer = AK_init_observer(self, custom_observer_event_handler);
    if (self->observer == NULL) {
        AK_dbg_messg(LOW, GLOBAL, "ERROR: Failed to initialize observer in init_observer_type_second");
        AK_free(self);
        AK_EPI;
        return NULL;
    }

    AK_dbg_messg(LOW, GLOBAL, "Observer type second initialized successfully");
    AK_EPI;
    return self;
}

/**
 * @author Vilim Trkakoštanec
 * @brief Function that runs tests for observable pattern
 */
TestResult AK_custom_observable_test() {
    AK_PRO;
    int success = 0, failed = 0;
    AK_TypeObservable *observable = NULL;

    // Operation types for test cases
    typedef enum {
        GET_MSG,
        SET_NOTIFY,
        HANDLE_CUSTOM,
        EVENT_HANDLER,
        CUSTOM_ACTION,
        OBSERVER_REG,
        MSG_HANDLING,
        NOTIFY_TYPE,
        OBSERVER_TYPE,
        INIT_SECOND,
        NULL_INIT
    } Op;

    // Test case structure
    typedef struct {
        Op op;                     // Operation to test
        void *param1;              // First parameter (varies by operation)
        void *param2;              // Second parameter (varies by operation)
        int param3;                // Third parameter (numeric value or enum)
        int exp_result;            // Expected result code
        const char *test_name;     // Test description
        const char *msg;           // Test message if needed
    } TestCase;

    TestCase tests[] = {
        // Test NULL handling and error cases
        { GET_MSG, NULL, NULL, 0, 0, "NULL handling in AK_get_message", NULL },
        { SET_NOTIFY, NULL, NULL, ERROR, 1, "NULL observable in AK_set_notify_info_details", "test message" },
        { SET_NOTIFY, NULL, NULL, 999, 0, "Invalid notify type", "test message" },
        { SET_NOTIFY, NULL, NULL, INFO, 1, "NULL message handling", NULL },
        { HANDLE_CUSTOM, NULL, NULL, 0, 1, "NULL parameters in handle_AK_custom_type", NULL },
        { EVENT_HANDLER, NULL, NULL, 999, 1, "Error handling in custom_observer_event_handler", NULL },
        { CUSTOM_ACTION, (void*)1, NULL, 0, OK, "Custom action with non-NULL data", NULL },
        { CUSTOM_ACTION, (void*)~0, NULL, 0, OK, "Custom action with max pointer value", NULL },

        // Test observer registration
        { OBSERVER_REG, NULL, NULL, MAX_OBSERVABLE_OBSERVERS, 1, "Maximum observer registration", NULL },
        { OBSERVER_REG, NULL, NULL, -1, OBSERVER_REGISTER_FAILURE_MAX_OBSERVERS, "Duplicate registration", NULL },
        { OBSERVER_REG, NULL, NULL, -2, OBSERVER_UNREGISTER_FAILURE_NOT_FOUND, "Unregister non-existent observer", NULL },

        // Test message handling
        { MSG_HANDLING, NULL, NULL, 0, 1, "Message storage and retrieval", "Test notification message" },
        { MSG_HANDLING, NULL, NULL, 1, 1, "Message overwrite", "New test message" },

        // Test notification types
        { NOTIFY_TYPE, NULL, NULL, ERROR, 1, "ERROR notification handling", "Error message" },
        { NOTIFY_TYPE, NULL, NULL, WARNING, 1, "WARNING notification handling", "Warning message" },
        { NOTIFY_TYPE, NULL, NULL, INFO, 1, "INFO notification handling", "Info message" },
        { NOTIFY_TYPE, NULL, NULL, 999, 1, "Invalid notification type handling", "Invalid type message" },

        // Test observer types
        { OBSERVER_TYPE, NULL, NULL, AK_TRANSACTION, 1, "Transaction type handling", NULL },
        { OBSERVER_TYPE, NULL, NULL, AK_TRIGGER, 1, "Trigger type handling", NULL },
        { OBSERVER_TYPE, NULL, NULL, AK_CUSTOM_FIRST, 1, "Custom first type handling", NULL },
        { OBSERVER_TYPE, NULL, NULL, AK_CUSTOM_SECOND, 1, "Custom second type handling", NULL },

        // Test observer type second
        { INIT_SECOND, NULL, NULL, 0, 1, "Initialize observer type second", NULL },
        { INIT_SECOND, NULL, NULL, 1, 1, "Register second type observer", NULL },

        // Test NULL initialization
        { NULL_INIT, NULL, NULL, 0, 1, "Initialize observer with NULL observable", NULL }
    };
    size_t n_tests = sizeof(tests) / sizeof(tests[0]);

    printf("\n========== CUSTOM OBSERVABLE TEST BEGIN ==========\n");

    // Initialize observable for tests that need it
    observable = init_observable_type();

    for (size_t i = 0; i < n_tests; i++) {
        TestCase *t = &tests[i];
        printf("\n--- Test %zu: %s ---\n", i + 1, t->test_name);
        
        int result = EXIT_ERROR;
        
        switch (t->op) {
            case GET_MSG: {
                char *msg = AK_get_message(t->param1);
                result = (msg == NULL) ? 0 : 1;
                if (msg) AK_free(msg);
                break;
            }
            
            case SET_NOTIFY: {
                if (t->param3 == 999) {
                    AK_set_notify_info_details(observable, t->param3, t->msg);
                    result = (observable->notifyDetails == NULL) ? 0 : 1;
                } else {
                    AK_set_notify_info_details(observable, t->param3, t->msg);
                    result = (observable->notifyDetails != NULL) ? 1 : 0;
                }
                break;
            }
            
            case HANDLE_CUSTOM: {
                handle_AK_custom_type(NULL, NULL);
                if (observable) {
                    observable->notifyDetails = NULL;
                    handle_AK_custom_type(NULL, observable);
                }
                result = 1; // Success if no crash
                break;
            }
            
            case EVENT_HANDLER: {
                custom_observer_event_handler(NULL, NULL, t->param3);
                result = 1; // Success if no crash
                break;
            }
            
            case CUSTOM_ACTION: {
                result = AK_custom_action(t->param1);
                break;
            }
            
            case OBSERVER_REG: {
                if (t->param3 == MAX_OBSERVABLE_OBSERVERS) {
                    // Inlined test_max_registration
                    if (!observable) {
                        result = 0;
                        break;
                    }

                    AK_TypeObserver *observers[MAX_OBSERVABLE_OBSERVERS];
                    int filled_count = 0;
                    
                    for (int i = 0; i < MAX_OBSERVABLE_OBSERVERS; i++) {
                        observers[i] = init_observer_type(observable);
                        if (observers[i] != NULL) {
                            if (observable->observable->AK_register_observer(
                                observable->observable, 
                                observers[i]->observer
                            ) == OBSERVER_REGISTER_SUCCESS) {
                                filled_count++;
                            } else {
                                AK_free(observers[i]);
                                break;
                            }
                        }
                    }

                    // Try one more
                    AK_TypeObserver *extra = init_observer_type(observable);
                    int temp_result = 0;
                    
                    if (extra != NULL) {
                        temp_result = (observable->observable->AK_register_observer(
                            observable->observable,
                            extra->observer
                        ) == OBSERVER_REGISTER_FAILURE_MAX_OBSERVERS) ? 1 : 0;
                        AK_free(extra);
                    }

                    // Cleanup
                    for (int i = 0; i < filled_count; i++) {
                        if (observers[i] != NULL) {
                            observable->observable->AK_unregister_observer(
                                observable->observable,
                                observers[i]->observer
                            );
                            AK_free(observers[i]);
                        }
                    }

                    result = (filled_count == MAX_OBSERVABLE_OBSERVERS && temp_result);

                } else if (t->param3 == -1) {
                    // Inlined test_duplicate_registration
                    if (!observable) {
                        result = 0;
                        break;
                    }

                    AK_TypeObserver *observer = init_observer_type(observable);
                    if (!observer) {
                        result = 0;
                        break;
                    }

                    int success = 0;
                    
                    if (observable->observable->AK_register_observer(
                        observable->observable,
                        observer->observer
                    ) == OBSERVER_REGISTER_SUCCESS) {
                        // Try to register again
                        success = (observable->observable->AK_register_observer(
                            observable->observable,
                            observer->observer
                        ) == OBSERVER_REGISTER_FAILURE_MAX_OBSERVERS);
                    }

                    observable->observable->AK_unregister_observer(
                        observable->observable,
                        observer->observer
                    );
                    AK_free(observer);

                    result = success;

                } else {
                    // Inlined test_unregister_nonexistent
                    if (!observable) {
                        result = 0;
                        break;
                    }

                    AK_TypeObserver *observer = init_observer_type(observable);
                    if (!observer) {
                        result = 0;
                        break;
                    }

                    result = (observable->observable->AK_unregister_observer(
                        observable->observable,
                        observer->observer
                    ) == OBSERVER_UNREGISTER_FAILURE_NOT_FOUND);

                    AK_free(observer);
                }
                break;
            }
            
            case MSG_HANDLING: {
                if (observable) {
                    if (t->param3 == 0) {
                        AK_set_notify_info_details(observable, INFO, t->msg);
                        char *retrieved = AK_get_message(observable);
                        result = (retrieved && strcmp(retrieved, t->msg) == 0) ? 1 : 0;
                    } else {
                        AK_set_notify_info_details(observable, ERROR, t->msg);
                        char *retrieved = AK_get_message(observable);
                        result = (retrieved && strcmp(retrieved, t->msg) == 0) ? 1 : 0;
                    }
                }
                break;
            }
            
            case NOTIFY_TYPE: {
                if (observable) {
                    AK_TypeObserver *obs = init_observer_type(observable);
                    if (obs) {
                        observable->observable->AK_register_observer(observable->observable, obs->observer);
                        AK_set_notify_info_details(observable, t->param3, t->msg);
                        handle_AK_custom_type(obs, observable);
                        result = 1;
                        observable->observable->AK_unregister_observer(observable->observable, obs->observer);
                        AK_free(obs);
                    }
                }
                break;
            }
            
            case OBSERVER_TYPE: {
                if (observable) {
                    AK_TypeObserver *obs = init_observer_type(observable);
                    if (obs) {
                        custom_observer_event_handler(obs, observable, t->param3);
                        result = 1;
                        AK_free(obs);
                    }
                }
                break;
            }
            
            case INIT_SECOND: {
                AK_TypeObserver *second = init_observer_type_second();
                if (second) {
                    if (t->param3 == 0) {
                        result = 1;
                    } else if (observable) {
                        result = (observable->observable->AK_register_observer(
                            observable->observable, 
                            second->observer
                        ) == OBSERVER_REGISTER_SUCCESS) ? 1 : 0;
                        observable->observable->AK_unregister_observer(
                            observable->observable,
                            second->observer
                        );
                    }
                    AK_free(second);
                }
                break;
            }
            
            case NULL_INIT: {
                AK_TypeObserver *null_obs = init_observer_type(NULL);
                result = (null_obs != NULL) ? 1 : 0;
                if (null_obs) AK_free(null_obs);
                break;
            }
        }

        if (result == t->exp_result) {
            printf("PASS: %s\n", t->test_name);
            success++;
        } else {
            printf("FAIL: %s (got %d, expected %d)\n", t->test_name, result, t->exp_result);
            failed++;
        }
    }

    // Cleanup
    if (observable != NULL) {
        if (observable->observable) {
            for (int i = 0; i < MAX_OBSERVABLE_OBSERVERS; i++) {
                if (observable->observable->observers[i] != NULL) {
                    observable->observable->AK_unregister_observer(observable->observable, observable->observable->observers[i]);
                }
            }
            AK_free(observable->observable);
        }
        if (observable->notifyDetails) {
            AK_free(observable->notifyDetails);
        }
        AK_free(observable);
    }

    printf("\n========== TEST SUMMARY ==========\n");
    printf("Tests passed: %d\n", success);
    printf("Tests failed: %d\n", failed);
    printf("================================\n\n");

    AK_EPI;
    return TEST_result(success, failed);
}