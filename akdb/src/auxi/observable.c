/**
@file observable.c File that provides the implementations of functions for observable pattern
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

/******************** OBSERVABLE IMPLEMENTATION ********************/

/** 
 * @author Ivan Pusic
 * @brief Function that registers a new observer. Observer is added to the list of all observers
 * @param self Observable object instance
 * @param observer Observer object instance
 * 
 * @return Exit status 
 */
static inline int AK_register_observer(AK_observable *self, AK_observer *observer)
{
    int i;
    AK_PRO;

    // Add NULL checks for parameters
    if (self == NULL || observer == NULL) {
        AK_dbg_messg(LOW, GLOBAL, "ERROR: NULL pointer passed to AK_register_observer");
        AK_EPI;
        return OBSERVER_REGISTER_FAILURE_MAX_OBSERVERS;
    }

    // Check if observer is already registered
    for (i = 0; i < MAX_OBSERVABLE_OBSERVERS; ++i) {
        if (self->observers[i] == observer) {
            AK_dbg_messg(LOW, GLOBAL, "ERROR: Observer already registered");
            AK_EPI;
            return OBSERVER_REGISTER_FAILURE_MAX_OBSERVERS;
        }
    }

    // Look for empty slot to register the observer
    for (i = 0; i < MAX_OBSERVABLE_OBSERVERS; ++i) {
        if(self->observers[i] == 0) {
            // Assigning unique ID to new observer
            observer->observer_id = self->observer_id_counter++;
            self->observers[i] = observer;
            AK_dbg_messg(LOW, GLOBAL, "NEW OBSERVER ADDED with ID: %d", observer->observer_id);
            AK_EPI;
            return OBSERVER_REGISTER_SUCCESS;
        }
    }
    
    AK_dbg_messg(LOW, GLOBAL, "ERROR: Maximum number of observers (%d) reached!", MAX_OBSERVABLE_OBSERVERS);
    AK_EPI;
    return OBSERVER_REGISTER_FAILURE_MAX_OBSERVERS;
}

/** 
 * @author Ivan Pusic
 * @brief Function that unregisters a observer. Observer will be removed from the list of all observers
 * @param self Observable object instance
 * @param observer Observer object instance
 * 
 * @return Exit status
 */
static inline int AK_unregister_observer(AK_observable *self, AK_observer *observer)
{
    int i;
    AK_PRO;

    // Add NULL checks for parameters
    if (self == NULL || observer == NULL) {
        AK_dbg_messg(LOW, GLOBAL, "ERROR: NULL pointer passed to AK_unregister_observer");
        AK_EPI;
        return OBSERVER_UNREGISTER_FAILURE_NOT_FOUND;
    }

    for(i = 0; i < MAX_OBSERVABLE_OBSERVERS; ++i) {
        if(observer == self->observers[i]) {
            // Found the observer, now free it properly
            if (self->observers[i] != NULL) {
                AK_free(self->observers[i]);
                self->observers[i] = 0;
                AK_dbg_messg(LOW, GLOBAL, "Observer with ID %d successfully unregistered", observer->observer_id);
                AK_EPI;
                return OBSERVER_UNREGISTER_SUCCESS;
            }
        }
    }
    
    AK_dbg_messg(LOW, GLOBAL, "ERROR: Observer not found in the observable's list");
    AK_EPI;    
    return OBSERVER_UNREGISTER_FAILURE_NOT_FOUND;
}

/** 
 * @author Ivan Pusic
 * @brief Function that notifies a certain observer
 * @param self Observable object instance
 * @param observer Observer object to notify
 * 
 * @return Exit status
 */
static inline int AK_notify_observer(AK_observable *self, AK_observer *observer)
{
    int i;
    AK_PRO;

    // Add NULL checks for parameters
    if (self == NULL || observer == NULL) {
        AK_dbg_messg(LOW, GLOBAL, "ERROR: NULL pointer passed to AK_notify_observer");
        AK_EPI;
        return OBSERVER_NOTIFY_FAILURE_NOT_FOUND;
    }

    // Check if observable type is set
    if (self->AK_observable_type == NULL) {
        AK_dbg_messg(LOW, GLOBAL, "ERROR: Observable type not set");
        AK_EPI;
        return OBSERVER_NOTIFY_FAILURE_NOT_FOUND;
    }

    for(i = 0; i < MAX_OBSERVABLE_OBSERVERS; ++i) {
        if(self->observers[i] != 0 && self->observers[i] == observer) {
            // Check if notify function exists
            if (observer->AK_notify == NULL) {
                AK_dbg_messg(LOW, GLOBAL, "ERROR: Observer notify function not set for observer ID %d", observer->observer_id);
                AK_EPI;
                return OBSERVER_NOTIFY_FAILURE_NOT_FOUND;
            }
            
            observer->AK_notify(observer, self->AK_observable_type, self->AK_ObservableType_Def);
            AK_dbg_messg(LOW, GLOBAL, "Notification sent to observer ID: %d", observer->observer_id);
            AK_EPI;
            return OBSERVER_NOTIFY_SUCCESS;
        }
    }
    
    AK_dbg_messg(LOW, GLOBAL, "ERROR: Observer not found in the observable's list");
    AK_EPI;
    return OBSERVER_NOTIFY_FAILURE_NOT_FOUND;
}

/** 
 * @author Ivan Pusic
 * @brief Function that notifies all observers
 * @param self Observable object instance
 * 
 * @return Exit status
 */
static inline int AK_notify_observers(AK_observable *self)
{
    int i, notify_count = 0;
    AK_PRO;

    // Add NULL check for self parameter
    if (self == NULL) {
        AK_dbg_messg(LOW, GLOBAL, "ERROR: NULL pointer passed to AK_notify_observers");
        AK_EPI;
        return OBSERVER_NOTIFY_FAILURE_NOT_FOUND;
    }

    // Check if observable type is set
    if (self->AK_observable_type == NULL) {
        AK_dbg_messg(LOW, GLOBAL, "ERROR: Observable type not set");
        AK_EPI;
        return OBSERVER_NOTIFY_FAILURE_NOT_FOUND;
    }

    for(i = 0; i < MAX_OBSERVABLE_OBSERVERS; ++i) {
        if(self->observers[i] != 0) {
            // Check if notify function exists
            if (self->observers[i]->AK_notify == NULL) {
                AK_dbg_messg(LOW, GLOBAL, "ERROR: Observer notify function not set for observer ID %d", self->observers[i]->observer_id);
                continue;
            }

            // Call AK_notify and pass AK_observer observer and custom observable instances
            self->observers[i]->AK_notify(self->observers[i], self->AK_observable_type, self->AK_ObservableType_Def);
            notify_count++;
        }
    }
    
    if (notify_count > 0) {
        AK_dbg_messg(LOW, GLOBAL, "Successfully notified %d observers", notify_count);
        AK_EPI;
        return OBSERVER_NOTIFY_SUCCESS;
    }
    
    AK_dbg_messg(LOW, GLOBAL, "WARNING: No observers were notified");
    AK_EPI;
    return OBSERVER_NOTIFY_FAILURE_NOT_FOUND;
}

/** 
 * @author Ivan Pusic
 * @brief Function that fetches a observer object from observable type by observer id
 * @param self Observable type instance
 * @param id Observer id
 * 
 * @return Requested observer instance
 */
static inline AK_observer *AK_get_observer_by_id(AK_observable *self, int id)
{
    int i;
    AK_PRO;

    // Add NULL check for self parameter
    if (self == NULL) {
        AK_dbg_messg(LOW, GLOBAL, "ERROR: NULL observable passed to AK_get_observer_by_id");
        AK_EPI;
        return NULL;
    }

    // Validate observer ID
    if (id <= 0 || id >= self->observer_id_counter) {
        AK_dbg_messg(LOW, GLOBAL, "ERROR: Invalid observer ID: %d", id);
        AK_EPI;
        return NULL;
    }

    for(i = 0; i < MAX_OBSERVABLE_OBSERVERS; ++i) {
        if(self->observers[i] != 0 && self->observers[i]->observer_id == id) {
            AK_dbg_messg(LOW, GLOBAL, "Observer with ID %d found successfully", id);
            AK_EPI;
            return self->observers[i];
        }
    }

    AK_dbg_messg(LOW, GLOBAL, "ERROR: Observer with ID %d not found in observable's list", id);
    AK_EPI;
    return NULL;
}

/** 
 * @author Ivan Pusic
 * @brief Function that initializes a observable object
 *
 * @return Pointer to new observable object
 */
AK_observable * AK_init_observable(void *AK_observable_type, AK_ObservableType_Enum AK_ObservableType_Def, void *AK_custom_action)
{
    AK_observable *self;
    AK_PRO;

    // Input validation
    if (AK_observable_type == NULL) {
        AK_dbg_messg(LOW, GLOBAL, "ERROR: NULL observable type passed to AK_init_observable");
        AK_EPI;
        return NULL;
    }

    // Allocate memory for observable
    self = (AK_observable*) AK_calloc(1, sizeof(*self));
    if (self == NULL) {
        AK_dbg_messg(LOW, GLOBAL, "ERROR: Memory allocation failed in AK_init_observable");
        AK_EPI;
        return NULL;
    }

    // Initialize function pointers
    self->AK_register_observer = &AK_register_observer;
    self->AK_unregister_observer = &AK_unregister_observer;
    self->AK_notify_observer = &AK_notify_observer;
    self->AK_notify_observers = &AK_notify_observers;
    self->AK_get_observer_by_id = AK_get_observer_by_id;
    self->AK_run_custom_action = AK_custom_action;

    // Initialize observers array to NULL
    memset(self->observers, 0, sizeof(self->observers));
           
    // Set observable properties
    self->AK_ObservableType_Def = AK_ObservableType_Def;
    self->observer_id_counter = 1;
    self->AK_observable_type = AK_observable_type;

    AK_dbg_messg(LOW, GLOBAL, "NEW OBSERVABLE OBJECT INITIALIZED with type: %d", AK_ObservableType_Def);
    AK_EPI;
    return self;
}

/******************** OBSERVER IMPLEMENTATION ********************/

/** 
 * @author Ivan Pusic
 * @brief Function that destroys a observer object
 *
 * @return Exit status
 */
static inline int AK_destroy_observer(AK_observer *self)
{
    AK_PRO;

    if (self == NULL) {
        AK_dbg_messg(LOW, GLOBAL, "ERROR: NULL pointer passed to AK_destroy_observer");
        AK_EPI;
        return OBSERVER_DESTROY_FAILURE_INVALID_ARGUMENT;
    }

    // Check if observer has valid ID before destroying
    if (self->observer_id <= 0) {
        AK_dbg_messg(LOW, GLOBAL, "ERROR: Invalid observer ID when attempting to destroy");
        AK_EPI;
        return OBSERVER_DESTROY_FAILURE_INVALID_ARGUMENT;
    }

    // Check if observer type is set
    if (self->AK_observer_type == NULL) {
        AK_dbg_messg(LOW, GLOBAL, "WARNING: Observer being destroyed has NULL observer type");
    }

    AK_free(self);
    self = 0;
    AK_dbg_messg(LOW, GLOBAL, "Observer successfully destroyed");    
    AK_EPI;    
    return OBSERVER_DESTROY_SUCCESS;
}

/** 
 * @author Ivan Pusic
 * @brief Function that calls event handler for the observer object
 * @param observer AK_observer type instance
 * @param observable_type Custom observable type instance
 * 
 * @return Exit status
 */
static inline int AK_notify(AK_observer *observer, void *observable_type, AK_ObservableType_Enum type)
{
    AK_PRO;

    // Add NULL checks for parameters
    if (observer == NULL || observable_type == NULL) {
        AK_dbg_messg(LOW, GLOBAL, "ERROR: NULL pointer passed to AK_notify");
        AK_EPI;
        return OBSERVER_NOTIFY_FAILURE_NOT_FOUND;
    }

    // Check if observer has event handler function
    if (observer->AK_observer_type_event_handler == NULL) {
        AK_dbg_messg(LOW, GLOBAL, "ERROR: Observer event handler function not set");
        AK_EPI;
        return OBSERVER_NOTIFY_FAILURE_NOT_FOUND;
    }

    // Call the event handler
    observer->AK_observer_type_event_handler(observer->AK_observer_type, observable_type, type);
    AK_dbg_messg(LOW, GLOBAL, "Successfully called event handler for observer ID: %d", observer->observer_id);
    AK_EPI;
    return OBSERVER_NOTIFY_SUCCESS;
}

/** 
 * @author Ivan Pusic
 * @brief Function that initializes the observer object
 * 
 * @return Pointer to new observer object
 */
AK_observer *AK_init_observer(void *observer_type, void (*observer_type_event_handler)(void*, void*, AK_ObservableType_Enum))
{
    AK_observer *self;
    AK_PRO;

    // Check if event handler is provided
    if (observer_type_event_handler == NULL) {
        AK_dbg_messg(LOW, GLOBAL, "ERROR: NULL event handler passed to AK_init_observer");
        AK_EPI;
        return NULL;
    }

    // Allocate memory for observer
    self = AK_calloc(1, sizeof(*self));
    if (self == NULL) {
        AK_dbg_messg(LOW, GLOBAL, "ERROR: Memory allocation failed in AK_init_observer");
        AK_EPI;
        return NULL;
    }

    self->AK_destroy_observer = &AK_destroy_observer;
    self->AK_observer_type = observer_type;
    self->AK_observer_type_event_handler = observer_type_event_handler;
    self->AK_notify = &AK_notify;
    
    AK_dbg_messg(LOW, GLOBAL, "NEW OBSERVER OBJECT INITIALIZED!");  
    AK_EPI;  
    return self;
}

/**
 * @author Ivan Pusic, refactored by Vilim Trkakoštanec
 * @brief Function that runs tests for observable pattern
 */
TestResult AK_observable_test() {
    AK_PRO;
    int success = 0, failed = 0;
    AK_TypeObservable *observable = NULL;

    // For test operations that need a custom handler
    void (*custom_handler)(void*, void*, AK_ObservableType_Enum) = &custom_observer_event_handler;

    // Operation types for test cases
    typedef enum {
        INIT_OBSERVABLE,
        INIT_OBSERVER,
        REGISTER_OBSERVER,
        NOTIFY_OBSERVER,
        UNREGISTER_OBSERVER,
        GET_OBSERVER,
        NOTIFY_ALL,
        CUSTOM_ACTION
    } Op;

    // Test case structure
    typedef struct {
        Op op;                     // Operation to test
        void *param1;              // First parameter (varies by operation)
        void *param2;              // Second parameter (varies by operation)
        int param3;                // Third parameter (e.g. observer ID)
        int exp_result;            // Expected result code
        const char *test_name;     // Test description
    } TestCase;

    TestCase tests[] = {
        // Error handling tests
        { INIT_OBSERVABLE, NULL, NULL, 0, 0, "NULL parameter handling in AK_init_observable" },
        { INIT_OBSERVER, NULL, NULL, 0, 0, "Observer initialization with NULL handler" },
        { REGISTER_OBSERVER, NULL, NULL, 0, OBSERVER_REGISTER_FAILURE_MAX_OBSERVERS, "Register observer with NULL parameters" },
        { NOTIFY_OBSERVER, NULL, NULL, 0, OBSERVER_NOTIFY_FAILURE_NOT_FOUND, "Notify observer with NULL parameters" },
        { UNREGISTER_OBSERVER, NULL, NULL, 0, OBSERVER_UNREGISTER_FAILURE_NOT_FOUND, "Unregister observer with NULL parameters" },
        { GET_OBSERVER, NULL, NULL, -1, 0, "Negative observer ID handling" },
        { GET_OBSERVER, NULL, NULL, 0, 0, "Zero observer ID handling" },
        { NOTIFY_ALL, NULL, NULL, 0, OBSERVER_NOTIFY_FAILURE_NOT_FOUND, "Notify with no observers" },
        { INIT_OBSERVER, observable, NULL, 0, 0, "Observer with NULL event handler" },
        
        { INIT_OBSERVABLE, observable, NULL, 0, 0, "Observable initialization" },
        { INIT_OBSERVER, observable, custom_handler, 0, 1, "Observer initialization" },
        { REGISTER_OBSERVER, observable, NULL, 0, OBSERVER_REGISTER_SUCCESS, "Observer registration" },
        { NOTIFY_OBSERVER, observable, NULL, 0, OBSERVER_NOTIFY_SUCCESS, "Observer notification" },
        { UNREGISTER_OBSERVER, observable, NULL, 0, OBSERVER_UNREGISTER_SUCCESS, "Observer unregistration" },
        { CUSTOM_ACTION, NULL, NULL, 0, OK, "Custom action execution" }
    };
    size_t n_tests = sizeof(tests) / sizeof(tests[0]);

    printf("\n========== OBSERVABLE PATTERN TEST BEGIN ==========\n");

    // Initialize observable for tests that need it
    observable = init_observable_type();
    AK_observer *current_observer = NULL;

    for (size_t i = 0; i < n_tests; i++) {
        TestCase *t = &tests[i];
        printf("\n--- Test %zu: %s ---\n", i + 1, t->test_name);
        
        int result = EXIT_ERROR;
        
        switch (t->op) {
            case INIT_OBSERVABLE:
                if (t->param1) {
                    // For success test - check if observable is already initialized
                    result = (observable != NULL);
                } else {
                    // For error test - try to initialize with NULL
                    result = (AK_init_observable(NULL, AK_CUSTOM_FIRST, NULL) == NULL) ? 0 : 1;
                }
                break;
                
            case INIT_OBSERVER:
                current_observer = AK_init_observer(t->param1, t->param2);
                result = (current_observer != NULL);
                break;
                
            case REGISTER_OBSERVER:
                if (observable && observable->observable) {
                    result = observable->observable->AK_register_observer(observable->observable, current_observer);
                }
                break;
                
            case NOTIFY_OBSERVER:
                if (observable && observable->observable) {
                    result = observable->observable->AK_notify_observer(observable->observable, current_observer);
                }
                break;
                
            case UNREGISTER_OBSERVER:
                if (observable && observable->observable) {
                    result = observable->observable->AK_unregister_observer(observable->observable, current_observer);
                }
                break;
                
            case GET_OBSERVER:
                if (observable && observable->observable) {
                    result = (observable->observable->AK_get_observer_by_id(observable->observable, t->param3) == NULL) ? 0 : 1;
                }
                break;
                
            case NOTIFY_ALL:
                if (observable && observable->observable) {
                    result = observable->observable->AK_notify_observers(observable->observable);
                }
                break;
                
            case CUSTOM_ACTION:
                if (observable && observable->observable) {
                    result = observable->observable->AK_run_custom_action(NULL);
                }
                break;
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

TestResult AK_observable_pattern() {
    AK_PRO;
    int passed = 0, failed = 0;
    
    // Setup test data
    char *msg1 = "AK_CUSTOM_FIRST proslijedi dalje!";
    char *msg2 = "AK_TRANSACTION proslijedi dalje!";

    // Operation types for test cases
    typedef enum {
        INIT_TYPE,
        REGISTER,
        NOTIFY_ALL,
        NOTIFY_ONE,
        CUSTOM_ACTION,
        GET_BY_ID,
        UNREGISTER,
        DOUBLE_UNREG,
        MAX_REG
    } PatternOp;

    // Test case structure
    typedef struct {
        PatternOp op;
        const char *msg;           // Message for notification tests
        int id;                    // For observer ID tests
        int exp_result;            // Expected result code
        const char *test_name;     // Test description
    } PatternTest;

    PatternTest tests[] = {
        { INIT_TYPE, NULL, 0, 1, "Observable Type Initialization" },
        { REGISTER, NULL, 0, OBSERVER_REGISTER_SUCCESS, "Observer Registration" },
        { NOTIFY_ALL, msg1, 0, OBSERVER_NOTIFY_SUCCESS, "Notify All Observers with Custom Message" },
        { NOTIFY_ONE, msg2, 0, OBSERVER_NOTIFY_SUCCESS, "Notify Single Observer with Transaction Message" },
        { CUSTOM_ACTION, NULL, 0, OK, "Execute Custom Action" },
        { GET_BY_ID, NULL, 1, 1, "Observer Lookup by ID" },
        { UNREGISTER, NULL, 0, OBSERVER_UNREGISTER_SUCCESS, "Observer Unregistration" },
        { DOUBLE_UNREG, NULL, 0, OBSERVER_UNREGISTER_FAILURE_NOT_FOUND, "Double Unregistration (Should Fail)" },
        { MAX_REG, NULL, MAX_OBSERVABLE_OBSERVERS, 1, "Maximum Observer Registration" }
    };
    size_t n_tests = sizeof(tests) / sizeof(tests[0]);

    printf("\n========== OBSERVABLE PATTERN TEST BEGIN ==========\n\n");

    // Initialize observable and observer
    AK_TypeObservable *observable = init_observable_type();
    if (observable == NULL) {
        printf("FAIL: Failed to initialize observable type\n");
        failed++;
        AK_EPI;
        return TEST_result(passed, failed);
    }

    AK_TypeObserver *observer = init_observer_type(observable);
    if (observer == NULL) {
        printf("FAIL: Failed to initialize observer\n");
        failed++;
        if (observable->observable) AK_free(observable->observable);
        AK_free(observable);
        AK_EPI;
        return TEST_result(passed, failed);
    }

    // Run test cases
    for (size_t i = 0; i < n_tests; i++) {
        PatternTest *t = &tests[i];
        printf("\n--- Test %zu: %s ---\n", i + 1, t->test_name);
        
        int result = EXIT_ERROR;
        
        switch (t->op) {
            case INIT_TYPE:
                result = (observable != NULL);
                break;
                
            case REGISTER:
                result = observable->observable->AK_register_observer(
                    observable->observable, 
                    observer->observer
                );
                break;
                
            case NOTIFY_ALL:
                observable->AK_set_notify_info_details(observable, INFO, (char *)t->msg);
                result = observable->observable->AK_notify_observers(observable->observable);
                break;
                
            case NOTIFY_ONE:
                observable->AK_set_notify_info_details(observable, INFO, (char *)t->msg);
                result = observable->observable->AK_notify_observer(
                    observable->observable, 
                    observer->observer
                );
                break;
                
            case CUSTOM_ACTION:
                result = observable->observable->AK_run_custom_action(NULL);
                break;
                
            case GET_BY_ID:
                result = (observable->observable->AK_get_observer_by_id(
                    observable->observable, 
                    t->id
                ) != NULL);
                break;
                
            case UNREGISTER:
                result = observable->observable->AK_unregister_observer(
                    observable->observable,
                    observer->observer
                );
                break;
                
            case DOUBLE_UNREG:
                result = observable->observable->AK_unregister_observer(
                    observable->observable,
                    observer->observer
                );
                break;
                
            case MAX_REG:
                int success_count = 0;
                AK_TypeObserver *temp_observers[MAX_OBSERVABLE_OBSERVERS + 1];
                
                for (int j = 0; j < MAX_OBSERVABLE_OBSERVERS + 1; j++) {
                    temp_observers[j] = init_observer_type(observable);
                    if (temp_observers[j] != NULL) {
                        if (observable->observable->AK_register_observer(
                            observable->observable,
                            temp_observers[j]->observer
                        ) == OBSERVER_REGISTER_SUCCESS) {
                            success_count++;
                        }
                    }
                }
                
                // Cleanup temp observers
                for (int j = 0; j < MAX_OBSERVABLE_OBSERVERS + 1; j++) {
                    if (temp_observers[j] != NULL) {
                        if (temp_observers[j]->observer != NULL) {
                            observable->observable->AK_unregister_observer(
                                observable->observable,
                                temp_observers[j]->observer
                            );
                        }
                        AK_free(temp_observers[j]);
                    }
                }
                
                result = (success_count == t->id); // t->id contains MAX_OBSERVABLE_OBSERVERS
                break;
        }

        if ((result == t->exp_result) || 
            (t->exp_result == 1 && result > 0)) { // Handle cases where 1 means success
            printf("PASS: %s\n", t->test_name);
            passed++;
        } else {
            printf("FAIL: %s (got %d, expected %d)\n", t->test_name, result, t->exp_result);
            failed++;
        }
    }

    // Cleanup
    if (observer != NULL) {
        AK_free(observer);
    }
    if (observable != NULL) {
        if (observable->observable) {
            AK_free(observable->observable);
        }
        if (observable->notifyDetails) {
            AK_free(observable->notifyDetails);
        }
        AK_free(observable);
    }
    
    printf("\n========== OBSERVABLE PATTERN TEST END ==========\n");
    printf("Tests passed: %d\n", passed);
    printf("Tests failed: %d\n", failed);
    printf("====================================\n\n");
    
    AK_EPI;
    return TEST_result(passed, failed);
}