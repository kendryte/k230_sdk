/*
 *   Copyright (c) 2022 Martijn van Welie
 *
 *   Permission is hereby granted, free of charge, to any person obtaining a copy
 *   of this software and associated documentation files (the "Software"), to deal
 *   in the Software without restriction, including without limitation the rights
 *   to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 *   copies of the Software, and to permit persons to whom the Software is
 *   furnished to do so, subject to the following conditions:
 *
 *   The above copyright notice and this permission notice shall be included in all
 *   copies or substantial portions of the Software.
 *
 *   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 *   AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 *   OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 *   SOFTWARE.
 *
 */

#ifndef BINC_APPLICATION_H
#define BINC_APPLICATION_H

#include <gio/gio.h>
#include "forward_decl.h"

#ifdef __cplusplus
extern "C" {
#endif

// Errors
#define BLUEZ_ERROR_REJECTED "org.bluez.Error.Rejected"
#define BLUEZ_ERROR_FAILED "org.bluez.Error.Failed"
#define BLUEZ_ERROR_INPROGRESS "org.bluez.Error.InProgress"
#define BLUEZ_ERROR_NOT_PERMITTED "org.bluez.Error.NotPermitted"
#define BLUEZ_ERROR_INVALID_VALUE_LENGTH "org.bluez.Error.InvalidValueLength"
#define BLUEZ_ERROR_NOT_AUTHORIZED "org.bluez.Error.NotAuthorized"
#define BLUEZ_ERROR_NOT_SUPPORTED "org.bluez.Error.NotSupported"

// This callback is called just before the characteristic's value is returned.
// Use it to update the characteristic before it is read
// For accepting the read, return NULL, otherwise return an error (BLUEZ_ERROR_*)
typedef const char *(*onLocalCharacteristicRead)(const Application *application, const char *address,
                                          const char *service_uuid, const char *char_uuid);

// This callback is called just before the characteristic's value is set.
// Use it to accept (return NULL), or reject (return BLUEZ_ERROR_*) the byte array
typedef const char *(*onLocalCharacteristicWrite)(const Application *application, const char *address,
                                            const char *service_uuid, const char *char_uuid, GByteArray *byteArray);

// This callback is called after a characteristic's value is set, e.g. because of a 'write' or 'notify'
// Use it to act upon the new value set
typedef void (*onLocalCharacteristicUpdated)(const Application *application, const char *service_uuid,
                                             const char *char_uuid, GByteArray *byteArray);

// This callback is called when notifications are enabled for a characteristic
typedef void (*onLocalCharacteristicStartNotify)(const Application *application, const char *service_uuid,
                                                 const char *char_uuid);

// This callback is called when notifications are disabled for a characteristic
typedef void (*onLocalCharacteristicStopNotify)(const Application *application, const char *service_uuid,
                                                const char *char_uuid);

// This callback is called just before the descriptor's value is returned.
// Use it to update the descriptor before it is read
typedef const char *(*onLocalDescriptorRead)(const Application *application, const char *address,
                                          const char *service_uuid, const char *char_uuid, const char *desc_uuid);

// This callback is called just before the descriptor's value is set.
// Use it to accept (return NULL), or reject (return BLUEZ_ERROR_*) the byte array
typedef const char *(*onLocalDescriptorWrite)(const Application *application, const char *address,
                                            const char *service_uuid, const char *char_uuid,
                                            const char *desc_uuid, const GByteArray *byteArray);

// Methods
Application *binc_create_application(const Adapter *adapter);

void binc_application_free(Application *application);

const char *binc_application_get_path(const Application *application);

int binc_application_add_service(Application *application, const char *service_uuid);

int binc_application_add_characteristic(Application *application, const char *service_uuid,
                                        const char *char_uuid, guint permissions);

int binc_application_add_descriptor(Application *application, const char *service_uuid,
                                    const char *char_uuid, const char *desc_uuid, guint permissions);

void binc_application_set_char_read_cb(Application *application, onLocalCharacteristicRead callback);

void binc_application_set_char_write_cb(Application *application, onLocalCharacteristicWrite callback);

void binc_application_set_char_start_notify_cb(Application *application, onLocalCharacteristicStartNotify callback);

void binc_application_set_char_stop_notify_cb(Application *application, onLocalCharacteristicStopNotify callback);

int binc_application_set_char_value(const Application *application, const char *service_uuid,
                                    const char *char_uuid, GByteArray *byteArray);

GByteArray *binc_application_get_char_value(const Application *application, const char *service_uuid,
                                            const char *char_uuid);

void binc_application_set_desc_read_cb(Application *application, onLocalDescriptorRead callback);

void binc_application_set_desc_write_cb(Application *application, onLocalDescriptorWrite callback);

int binc_application_set_desc_value(const Application *application, const char *service_uuid,
                                    const char *char_uuid, const char *desc_uuid, GByteArray *byteArray);

int binc_application_notify(const Application *application, const char *service_uuid, const char *char_uuid,
                            const GByteArray *byteArray);

gboolean binc_application_char_is_notifying(const Application *application, const char *service_uuid,
                                            const char *char_uuid);

#ifdef __cplusplus
}
#endif

#endif //BINC_APPLICATION_H
