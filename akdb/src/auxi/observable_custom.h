/**
   @file observable_custom.h Header file that provides custom observable type definitions
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

#ifndef OBSERVABLE_CUSTOM
#define OBSERVABLE_CUSTOM

#include "observable.h"

// Notify types for custom observable type
typedef enum {
    ERROR,
    INFO,
    WARMING,
} NotifyType;

// Structure that holds message and notify type
typedef struct _notifyDetails {
    char *message;
    NotifyType type;
} NotifyDetails;

// Custom observable type definition
struct TypeObservable {
    NotifyDetails *notifyDetails;
    char* (*AK_get_message) (struct TypeObservable*);
    int (*AK_custom_register_observer) (struct TypeObservable*, AK_observer*);
    int (*AK_custom_unregister_observer) (struct TypeObservable*, AK_observer*);
    void (*AK_set_notify_info_details) (struct TypeObservable*, NotifyType type, char *message);
    AK_observable *observable;
};
typedef struct TypeObservable AK_TypeObservable;

// Custom observer type definition
struct TypeObserver {
    AK_TypeObservable *observable;
    AK_observer *observer;
}; 
typedef struct TypeObserver AK_TypeObserver;
typedef struct TypeObserver AK_TypeObserver_Second;

// Function declarations
char * AK_get_message(AK_TypeObservable *self);
int AK_custom_register_observer(AK_TypeObservable* self, AK_observer* observer);
int AK_custom_unregister_observer(AK_TypeObservable * self, AK_observer* observer);
void AK_set_notify_info_details(AK_TypeObservable *self, NotifyType type, char *message);
int AK_custom_action(void *data);
void handle_AK_custom_type(AK_TypeObserver *observer, AK_TypeObservable *observable);
void custom_observer_event_handler(void *observer, void *observable, AK_ObservableType_Enum AK_ObservableType_Def);
AK_TypeObservable * init_observable_type();
AK_TypeObserver * init_observer_type(void *observable);
AK_TypeObserver * init_observer_type_second();

// Test function
TestResult AK_custom_observable_test();

#endif