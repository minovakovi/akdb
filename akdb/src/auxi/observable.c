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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Library General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor Boston, MA 02110-1301,  USA
 */

#include "./observable.h"


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
    for (i = 0; i < MAX_OBSERVABLE_OBSERVERS; ++i) {
        if(self->observers[i] == NULL) {
            // Assigning unique ID to new observer
            observer->observer_id = self->observer_id_counter++;
            self->observers[i] = observer;
            AK_dbg_messg(LOW, GLOBAL, "NEW OBSERVER ADDED");
            AK_EPI;
            return OK;
        }
    }
    AK_dbg_messg(LOW, GLOBAL, "ERROR IN FUNCTION FOR ADDING NEW OBSERVER!");
    AK_EPI;
    return NOT_OK;
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
    for(i = 0; i < MAX_OBSERVABLE_OBSERVERS; ++i) {
        if(observer == self->observers[i]) {
            AK_free(self->observers[i]);
            self->observers[i] = NULL;
            AK_dbg_messg(LOW, GLOBAL, "OBSERVER DELETED");
            AK_EPI;
            return OK;
        }
    }
    
    AK_dbg_messg(LOW, GLOBAL, "ERROR IN FUNCTION FOR DELETING OBSERVER!");
    AK_EPI;    
    return NOT_OK;
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
    for(i = 0; i < MAX_OBSERVABLE_OBSERVERS; ++i) {
        if(self->observers[i] != NULL && self->observers[i] == observer) {
            observer->AK_notify(observer, self->AK_observable_type, self->AK_ObservableType_Def);
            AK_dbg_messg(LOW, GLOBAL, "NOTIFICATION SENT TO OBSERVER");
            AK_EPI;
            return OK;
        }
    }
    
    AK_dbg_messg(LOW, GLOBAL, "ERROR IN FUNCTION FOR SENDING NOTIFICATION TO SPECIFIED OBSERVER!");
    AK_EPI;
    return NOT_OK;
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
    int i;
    AK_PRO;
    for(i = 0; i < MAX_OBSERVABLE_OBSERVERS; ++i) {
        if(self->observers[i] != NULL) {
            // Call AK_notify and pass AK_observer observer and custom observable instances
            self->observers[i]->AK_notify(self->observers[i], self->AK_observable_type, self->AK_ObservableType_Def);
        }
    }
    
    AK_dbg_messg(LOW, GLOBAL, "OBSERVERS NOTIFIED");
    AK_EPI;
    return OK;
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
    for(i = 0; i < MAX_OBSERVABLE_OBSERVERS; ++i) {
        if(self->observers[i] != NULL && self->observers[i]->observer_id == id) {
            AK_dbg_messg(LOW, GLOBAL, "REQUESTED OBSERVER FOUND. RETURNING OBSERVER BY ID");
            AK_EPI;
            return self->observers[i];
        }
    }
    AK_dbg_messg(LOW, GLOBAL, "OBSERVER WITH ID:%d DOESN'T EXIST!", id);
    AK_EPI;
    return NULL;
}

/** 
 * @author Ivan Pusic
 * @brief Function that initializes a observable object
 *
 * @return Pointer to new observable object
 */
AK_observable * AK_init_observable(void *AK_observable_type, AK_ObservableType_Enum AK_ObservableType_Def, void * AK_custom_action)
{
    AK_observable *self;
    AK_PRO;
    self = (AK_observable*) AK_calloc(1, sizeof(*self));
    self->AK_register_observer = &AK_register_observer;
    self->AK_unregister_observer = &AK_unregister_observer;
    self->AK_notify_observer = &AK_notify_observer;
    self->AK_notify_observers = &AK_notify_observers;
    self->AK_get_observer_by_id = AK_get_observer_by_id;
    self->AK_run_custom_action = AK_custom_action;

    memset(self->observers, 0, sizeof self->observers);
           
    
    self->AK_ObservableType_Def = AK_ObservableType_Def;
    self->observer_id_counter = 1;
    self->AK_observable_type = AK_observable_type;
    AK_dbg_messg(LOW, GLOBAL, "NEW OBSERVABLE OBJECT INITIALIZED!");
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
    if(self != NULL) {
        AK_free(self);
        self = NULL;
        AK_dbg_messg(LOW, GLOBAL, "OBSERVER DESTROYED!");    
        AK_EPI;    
        return OK;
    }
    
    AK_dbg_messg(LOW, GLOBAL, "ERROR WHILE DESTROYING OBSERVER");
    AK_EPI;
    return NOT_OK;
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
    observer->AK_observer_type_event_handler(observer->AK_observer_type, observable_type, type);
    AK_EPI;
    return OK;
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
    self = AK_calloc(1, sizeof(*self));
    self->AK_destroy_observer = &AK_destroy_observer;
    self->AK_observer_type = observer_type;
    self->AK_observer_type_event_handler = observer_type_event_handler;
    self->AK_notify = &AK_notify;
    AK_dbg_messg(LOW, GLOBAL, "NEW OBSERVER OBJECT INITIALIZED!");  
    AK_EPI;  
    return self;
}

/******************** OBSERVABLE PATTERN EXAMPLE ********************/

// This is optional. You can define some notifyTypes for your custom observable type
typedef enum {
    ERROR,
    INFO,
    WARMING,
} NotifyType;

// This is also optional. Here we define the structure that holds message, and the notify type
typedef struct _notifyDetails {
    char *message;
    NotifyType type;
} NotifyDetails;

// First define you observable type 
struct TypeObservable {
    // Then specify your type variables
    NotifyDetails *notifyDetails;
    // And your methods
    char* (*AK_get_message) (struct TypeObservable*);
    
    // You should define methods for adding and removing observers. You can also define some other method for achieveing this
    int (*AK_custom_register_observer) (struct TypeObservable*, AK_observer*);
    int (*AK_custom_unregister_observer) (struct TypeObservable*, AK_observer*);
    // If you want you can define method for setting observable type and message for our custom observable type
    void (*AK_set_notify_info_details) (struct TypeObservable*, NotifyType type, char *message);
    // Define observable variable, nedded for observable pattern. Important!
    AK_observable *observable;
};
typedef struct TypeObservable AK_TypeObservable;

// Implement method for getting some message from observable custom type
char * AK_get_message(AK_TypeObservable *self) {
    char *ret;
    AK_PRO;
    ret = self->notifyDetails->message;
    AK_EPI;
    return ret;
}

// Implement method for adding observer to custom observable type
int AK_custom_register_observer(AK_TypeObservable* self, AK_observer* observer) {
    AK_PRO;
    self->observable->AK_register_observer(self->observable, observer);
    AK_EPI;
    return OK;
}

// Implement method for removing observer from custom observable type
int AK_custom_unregister_observer(AK_TypeObservable * self, AK_observer* observer) {
    AK_PRO;
	self->observable->AK_unregister_observer(self->observable, observer);
    AK_EPI;
    return OK;
}

// Helper method for setting notify details
void AK_set_notify_info_details(AK_TypeObservable *self, NotifyType type, char *message) {
    // Info about notify
    NotifyDetails *notifyDetails;
    AK_PRO;
    notifyDetails = AK_calloc(1, sizeof(NotifyDetails));
    notifyDetails->message = message;
    notifyDetails->type = type;
    self->notifyDetails = notifyDetails;
    AK_EPI;
}

// this is definition of a custom action which can be called from custom observable type
int AK_custom_action(void *data) {
    AK_PRO;
    printf ("THIS IS SOME CUSTOM FUNCTION!\n");
    AK_EPI;
    return OK;
}

// You should have some kind of method for initializing observable type
AK_TypeObservable * init_observable_type() {
    AK_TypeObservable *self;
    AK_PRO;
    self = AK_calloc(1, sizeof(AK_TypeObservable));
    self->AK_get_message = AK_get_message;
    self->AK_custom_register_observer = &AK_custom_register_observer;
    self->AK_custom_unregister_observer = &AK_custom_unregister_observer;
    self->AK_set_notify_info_details = &AK_set_notify_info_details;

    // Very important!!! Call method for initializing AK_Observable and pass instance of custom observable type
    // Last parameter is a pointer to function which is custom
    self->observable = AK_init_observable(self, AK_CUSTOM_FIRST, &AK_custom_action);
    AK_EPI;
    return self;
}

struct TypeObserver {
    // This is observable type we will track.
    // You can declare instance of custom observable type here if you want, but it isn't necessary
    AK_TypeObservable *observable;
    
    // Nedded for observable pattern. Important!
    AK_observer *observer;
}; 
typedef struct TypeObserver AK_TypeObserver;
typedef struct TypeObserver AK_TypeObserver_Second;

// Define event handler for our AK_TypeObservable type. You should also define event handlers for other types if you need.
void handle_AK_custom_type(AK_TypeObserver *observer, AK_TypeObservable *observable) {
    char *message;
    AK_PRO;
    switch(observable->notifyDetails->type) {
        // we define possible notify types 
    case ERROR:
        // for every possibility some action
        message = observable->AK_get_message(observable);
        printf ("%s\n", message);
        break;
    case WARMING:
        message = observable->AK_get_message(observable);
        printf ("%s\n", message);
        break;
    case INFO:
        break;
    }
    AK_EPI;
}

// This method will be called by observable type 
void custom_observer_event_handler(void *observer, void *observable, AK_ObservableType_Enum AK_ObservableType_Def) {
    // Handle event, and call some method from observable type. In this case that is method for getting some message from observable type.
    // You can also define your custom methods and call that method in event handler (here).
    AK_PRO;
    switch(AK_ObservableType_Def) {
        // we define possible observable types
    case AK_TRANSACTION:
        break;
    case AK_TRIGGER:
        break;
    case AK_CUSTOM_FIRST:
        // for some type we call specified function
        handle_AK_custom_type((AK_TypeObserver*)observer, (AK_TypeObservable*)observable);
        break;
    case AK_CUSTOM_SECOND:
        break;
    default:
        printf("ERROR! OBSERVABLE TYPE NOT FOUND");
    }
    AK_EPI;
}

// Define some method for init observer type
AK_TypeObserver * init_observer_type(void *observable) {
    AK_TypeObserver *self;
    AK_PRO;
    self = AK_calloc(1, sizeof(AK_TypeObserver));
    self->observable = observable;
    // Init AK_Observer type. This is very important!!! Pass custom type observer instance as first parameter, and
    // pointer to event handler function of custom observer type
    self->observer = AK_init_observer(self, &custom_observer_event_handler);
    AK_EPI;
    return self;
}

// Define some method for init observer type. This method is without passing custom observable type to observer.
// This is also correct, if you don't want to have instance of custom observable type in custom observer type
AK_TypeObserver * init_observer_type_second() {
    AK_TypeObserver_Second *self;
    AK_PRO;
    self = AK_calloc(1, sizeof(AK_TypeObserver_Second));
    // Init AK_Observer type. This is very important!!! Pass custom type observer instance as first parameter, and
    // pointer to event handler function of custom observer type
    self->observer = AK_init_observer(self, custom_observer_event_handler);
    AK_EPI;
    return self;
}

/**
 * @author Ivan Pusic
 * @brief Function that runs tests for observable pattern
 */
TestResult AK_observable_test()
{
    AK_PRO;
    printf ("\n========== OBSERVABLE PATTERN BEGIN ==========\n");

    // Fucntion for init custom observable type
    AK_TypeObservable *observable_type = init_observable_type();
    // Init observer type with passing observable type instance
    AK_TypeObserver *observer_first = init_observer_type(observable_type);
    // You can also initialize second obsever type without passing observable type instance
    AK_TypeObserver_Second *observer_second = init_observer_type_second();

    // Register out observers to observable type
    observable_type->AK_custom_register_observer(observable_type, observer_first->observer);
    observable_type->AK_custom_register_observer(observable_type, observer_second->observer);

    // Set notify type and message
    observable_type->AK_set_notify_info_details(observable_type, ERROR, "THIS IS SOME ERROR FOR ALL OBSERVERS");
    // Notify all observers
    observable_type->observable->AK_notify_observers(observable_type->observable);

    // Set notify type and message
    observable_type->AK_set_notify_info_details(observable_type, WARMING, "THIS IS SOME WARMING FOR SPECIFIED OBSERVER");
    // Notify specified observer
    observable_type->observable->AK_notify_observer(observable_type->observable, observer_first->observer);

    observable_type->observable->AK_run_custom_action(NULL);
    // Search for observer by ID
    AK_observer *requested_observer = observable_type->observable->AK_get_observer_by_id(observable_type->observable, 1);
    int success=0;
    int failed=0;
	if(requested_observer) {
        printf ("Observer was found. Observer adress: %p\n", requested_observer);
		success++;
	}
    else{
        printf ("Requested observer was not found!\n");
		failed++;
	}
    
    printf ("========== OBSERVABLE PATTERN END ==========\n");
    AK_EPI;
    return TEST_result(success,failed);
}

TestResult AK_observable_pattern(){

    AK_PRO;

    int passed_tests=0;
    int failed_tests=0;

    printf ("\n========== OBSERVABLE PATTERN TEST BEGIN ==========\n\n");
    

    char *m1 = "AK_CUSTOM_FIRST proslijedi dalje!";
    char *m2 = "AK_TRANSACTION proslijedi dalje!";



    // Function for init custom observable type
    AK_TypeObservable *observable_type = init_observable_type();

    // Init observer type with passing observable type instance
    AK_TypeObserver *observer_first = init_observer_type(observable_type);

    // Register out observers to observable type
    observable_type->AK_custom_register_observer(observable_type, observer_first->observer);

    // Set notify type and message
    observable_type->AK_set_notify_info_details(observable_type, AK_CUSTOM_FIRST, m1);

    // Notify all observers
    observable_type->observable->AK_notify_observers(observable_type->observable);

    // Set notify type and message
    observable_type->AK_set_notify_info_details(observable_type, AK_TRANSACTION, m2);

    // Notify specified observer
    observable_type->observable->AK_notify_observer(observable_type->observable, observer_first->observer);

    observable_type->observable->AK_run_custom_action(NULL);
    // Search for observer by ID

    AK_observer *requested_observer = observable_type->observable->AK_get_observer_by_id(observable_type->observable, 1);
    
    if(requested_observer!=NULL) 
	{
        printf ("Observer was found. Observer adress: %p\n\n", requested_observer);
	passed_tests++;
	}

    else{
        printf ("Requested observer was not found!\n\n");
	failed_tests++;
	}
    
    printf ("========== OBSERVABLE PATTERN TEST END ==========\n\n");

    AK_EPI;

    return TEST_result(passed_tests,failed_tests);
}

