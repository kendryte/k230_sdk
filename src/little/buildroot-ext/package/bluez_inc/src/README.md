# Bluez in C


The goal of this library is to provide a clean C interface to Bluez, without needing to use DBus commands. Using Bluez over the DBus is quite tricky to say the least, and this library does all the hard work under the hood. 
As a result, it looks like a 'normal' C library for Bluetooth!

The library focuses on BLE and supports both **Central** and **Peripheral** roles. Some interesting facts include:
* Manage/Connect multiple peripherals at the same time
* Support for bonding
* Simplified programming interface for easy coding
* Configurable logging with extensive debugging possibilities

## Dependencies

This library uses GLib 2.0. If needed, you can install it using `sudo apt install -y libglib2.0-dev`.

## Building the code

```
cmake .
cmake --build .
```

## Discovering devices

In order to discover devices, you first need to get hold of a Bluetooth *adapter*. 
You do this by calling `binc_adapter_get_default()` with your DBus connection as an argument:

```c
int main(void) {
    GDBusConnection *dbusConnection = g_bus_get_sync(G_BUS_TYPE_SYSTEM, NULL, NULL);
    Adapter *default_adapter = binc_adapter_get_default(dbusConnection);
    
    //...
}
```

The next step is to set any scanfilters and set the callback you want to receive the found devices on:

```c
int main(void) {
    
    // ...
    
    binc_adapter_set_discovery_cb(default_adapter, &on_scan_result);
    binc_adapter_set_discovery_filter(default_adapter, -100, NULL, NULL);
    binc_adapter_start_discovery(default_adapter);
}
```

When you pass 'NULL' as the 3rd argument to `binc_adapter_set_discovery_filter`, you indicate that you don't want to filter on service UUIDs. Otherwise you can pass a GPtrArray with a number of service UUIDs that you want to filter on. The 4th argument allows you to filter on a 'pattern' which is defined in Bluez as the 'prefix of an address or name'. 
The discovery will deliver all found devices on the callback you provided. You typically check if it is the device you are looking for, stop the discovery and then connect to it:

```c
void on_scan_result(Adapter *default_adapter, Device *device) {
    const char* name = binc_device_get_name(device);
    if (name != NULL && g_str_has_prefix(name, "Polar")) {
        binc_adapter_stop_discovery(default_adapter);
        binc_device_set_connection_state_change_cb(device, &on_connection_state_changed);
        binc_device_set_services_resolved_cb(device, &on_services_resolved);
        binc_device_set_read_char_cb(device, &on_read);
        binc_device_set_write_char_cb(device, &on_write);
        binc_device_set_notify_char_cb(device, &on_notify);
        binc_device_connect(device);
    }
}
```
As you can see, just before connecting, you must set up some callbacks for receiving connection state changes and the results of reading/writing to characteristics.

## Connecting, service discovery and disconnecting

You connect by calling `binc_device_connect(device)`. Then the following sequence will happen:
* first, the *connection_state* will change to 'connecting'.
* when the device is connected, the *connection_state* changes to 'connected' and your registered callback will be called. However, you cannot use the device yet because the service discovery has not yet been done.
* Bluez will then start the service discovery automatically and when it finishes, the *services_resolved* callback is called. So the *service_resolved* callback is the right place to start reading and writing characteristics or starting notifications. 

```c
void on_connection_state_changed(Device *device, ConnectionState state, GError *error) {
    if (error != NULL) {
        log_debug(TAG, "(dis)connect failed (error %d: %s)", error->code, error->message);
        return;
    }

    log_debug(TAG, "'%s' (%s) state: %s (%d)", binc_device_get_name(device), binc_device_get_address(device), binc_device_get_connection_state_name(device), state);
    if (state == BINC_DISCONNECTED && binc_device_get_bonding_state(device) != BINC_BONDED) {
        binc_adapter_remove_device(default_adapter, device);
    }
}
```

If a connection attempt fails or times out after 25 seconds, the *connection_state* callback is called with an error.

To disconnect a connected device, call `binc_device_disconnect(device)` and the device will be disconnected. Again, the *connection_state* callback will be called. If you want to remove the device from the DBus after disconnecting, you call `binc_adapter_remove_device(default_adapter, device)`. 

## Reading and writing characteristics

We can start using characteristics once the service discovery has been completed. 
Typically, we read some characteristics like its model and manufacturer. In order to read or write, you first need to get the **Characteristic** by using `binc_device_get_characteristic(device, DIS_SERVICE, DIS_MANUFACTURER_CHAR)`. 
You need to provide the **service UUID** and **characteristic UUID** to find a characteristic:

```c
void on_services_resolved(Device *device) {
    Characteristic *manufacturer = binc_device_get_characteristic(device, DIS_SERVICE, DIS_MANUFACTURER_CHAR);
    if (manufacturer != NULL) {
        binc_characteristic_read(manufacturer);
    }

    binc_device_read_char(device, DIS_SERVICE, DIS_MODEL_CHAR);
}
```

As you can see, there is also a convenience method `binc_device_read_char` that will look up a characteristic and do a read if the characteristic is found.
Like all BLE operations, reading and writing are **asynchronous** operations. So you issue them and they will complete immediately, but you then have to wait for the result to come in on a callback. You register your callback by calling `binc_device_set_read_char_cb(device, &on_read)`. 

```c
void on_read(Device *device, Characteristic *characteristic, GByteArray *byteArray, GError *error) {
    const char* uuid = binc_characteristic_get_uuid(characteristic);
    if (error != NULL) {
        log_debug(TAG, "failed to read '%s' (error %d: %s)", uuid, error->code, error->message);
        return;
    }
    
    if (byteArray == NULL) return;
    
    if (g_str_equal(uuid, DIS_MANUFACTURER_CHAR)) {
        log_debug(TAG, "manufacturer = %s", byteArray->data);
    } else if (g_str_equal(uuid, MODEL_CHAR)) {
        log_debug(TAG, "model = %s", byteArray->data);
    }
}
```

Writing to characteristics works in a similar way. Register your callback using `binc_device_set_write_char_cb(device, &on_write)`. Make sure you check if you can write to the characteristic before attempting it:

```c
void on_services_resolved(Device *device) {

    // ...
    
    Characteristic *current_time = binc_device_get_characteristic(device, CTS_SERVICE, CURRENT_TIME_CHAR);
    if (current_time != NULL) {
        if (binc_characteristic_supports_write(current_time,WITH_RESPONSE)) {
            GByteArray *timeBytes = binc_get_current_time();
            binc_characteristic_write(current_time, timeBytes, WITH_RESPONSE);
            g_byte_array_free(timeBytes, TRUE);
        }
    }
}
```

## Receiving notifications

Bluez treats notifications and indications in the same way, calling them 'notifications'. If you want to receive notifications you have to 'start' them by calling `binc_characteristic_start_notify()`. As usual, first register your callback by calling `binc_device_set_notify_char_cb(device, &on_notify)`. Here is an example:

```c
void on_services_resolved(Device *device) {

    // ...
    
    Characteristic *temperature = binc_device_get_characteristic(device, HTS_SERVICE_UUID, TEMPERATURE_CHAR);
    if (temperature != NULL) {
        binc_characteristic_start_notify(temperature);
    }
}    
```

And then receiving your notifications:

```c
void on_notify(Device *device, Characteristic *characteristic, GByteArray *byteArray) {
    const char* uuid = binc_characteristic_get_uuid(characteristic);
    Parser *parser = parser_create(byteArray, LITTLE_ENDIAN);
    parser_set_offset(parser, 1);
    if (g_str_equal(uuid, TEMPERATURE_CHAR)) {
        float temperature = parser_get_float(parser);
        log_debug(TAG, "temperature %.1f", temperature);
    } 
    parser_free(parser);
}
```

The **Parser** object is a helper object that will help you parsing byte arrays.

## Bonding
Bonding is possible with this library. It supports 'confirmation' bonding (JustWorks) and PIN code bonding (passphrase).
First you need to register an Agent and set the callbacks for these 2 types of bonding. When creating the agent you can also choose the IO capabilities for your applications, i.e. DISPLAY_ONLY, DISPLAY_YES_NO, KEYBOARD_ONLY, NO_INPUT_NO_OUTPUT, KEYBOARD_DISPLAY. Note that this will affect the bonding behavior.

```c
int main(void) {

    // ...
        
    agent = binc_agent_create(default_adapter, "/org/bluez/BincAgent", KEYBOARD_DISPLAY);
    binc_agent_set_request_authorization_cb(agent, &on_request_authorization);
    binc_agent_set_request_passkey_cb(agent, &on_request_passkey);
}
```

When bonding is starting, one of your callbacks will be called and you should handle the request. In the case of 'confirmation' bonding you either accept (TRUE) or reject (FALSE) the request for bonding:

```c
gboolean on_request_authorization(Device *device) {
    log_debug(TAG, "requesting authorization for '%s", binc_device_get_name(device));
    return TRUE;
}
```

In the case of PIN code pairing you will have to provide a PIN code, which you probably need to ask to the user of your app:

```c
guint32 on_request_passkey(Device *device) {
    guint32 pass = 000000;
    log_debug(TAG, "requesting passkey for '%s", binc_device_get_name(device));
    log_debug(TAG, "Enter 6 digit pin code: ");
    fscanf(stdin, "%d", &pass);
    return pass;
}
```

Note that this type of bonding requires a **6 digit pin** code!

If you want to initiate bonding yourself, you can call `binc_device_pair()`. The same callbacks will be called for dealing with authorization or PIN codes.

# Creating your own peripheral
It is also possible with BINC to create your own peripheral, i.e. start advertising and implementing some services and characteristics.

## Advertising
In Bluez an advertisement is an object on the DBus. By using some basic calls BINC will create this object for you and set the right values.
After that, you need to tell the default_adapter to start advertising.

```c
// Build array with services to advertise
GPtrArray *adv_service_uuids = g_ptr_array_new();
g_ptr_array_add(adv_service_uuids, HTS_SERVICE_UUID);
g_ptr_array_add(adv_service_uuids, BLP_SERVICE_UUID);

// Create the advertisement
advertisement = binc_advertisement_create();
binc_advertisement_set_local_name(advertisement, "BINC2");
binc_advertisement_set_services(advertisement, adv_service_uuids);
        
// Start advertising
binc_adapter_start_advertising(default_adapter, advertisement);
g_ptr_array_free(adv_service_uuids, TRUE);
```

The library also supports setting *manufacturer data* and *service data*.

## Adding services and characteristics
In order to make your peripheral work you need to create an 'app' in Bluez terminology. The steps are:
* Create an app
* Add one or more services
* Add one or more characteristics and descriptors
* Implement read/write/notify callbacks for characteristics

Here is how to setup an app with a service and a characteristic:

```c
// Create an app with a service
app = binc_create_application(default_adapter);
binc_application_add_service(app, HTS_SERVICE_UUID);
binc_application_add_characteristic(
                app,
                HTS_SERVICE_UUID,
                TEMPERATURE_CHAR_UUID,
                GATT_CHR_PROP_READ | GATT_CHR_PROP_INDICATE | GATT_CHR_PROP_WRITE);
                
// Set the callbacks for read/write
binc_application_set_char_read_cb(app, &on_local_char_read);
binc_application_set_char_write_cb(app, &on_local_char_write);

// Register your app
binc_adapter_register_application(default_adapter, app);
```

There are callbacks to be implemented where you can update the value of a characteristic just before the read/write is done. 
If you accept the read, return NULL, otherwise return an error.

```c
char* on_local_char_read(const Application *app, const char *address, const char* service_uuid, const char* char_uuid) {
    if (g_str_equal(service_uuid, HTS_SERVICE_UUID) && g_str_equal(char_uuid, TEMPERATURE_CHAR_UUID)) {
        const guint8 bytes[] = {0x06, 0x6f, 0x01, 0x00, 0xff, 0xe6, 0x07, 0x03, 0x03, 0x10, 0x04, 0x00, 0x01};
        GByteArray *byteArray = g_byte_array_sized_new(sizeof(bytes));
        g_byte_array_append(byteArray, bytes, sizeof(bytes));
        binc_application_set_char_value(app, service_uuid, char_uuid, byteArray);
        return NULL;
    }
    return BLUEZ_ERROR_REJECTED;
}

char* on_local_char_write(const Application *app, const char *address, const char *service_uuid,
                          const char *char_uuid, GByteArray *byteArray) {
    // Reject all writes
    return BLUEZ_ERROR_REJECTED;
}
```

In order to notify you can use:

```c
void binc_application_notify(const Application *app, const char *service_uuid, const char *char_uuid,
                             GByteArray *byteArray);
```

## Examples

The repository includes an example for both the **Central** and **Peripheral** role. 

* The *central* example scans for thermometers and reads the thermometer value once it connects.
* The *peripheral* example acts as a thermometer and can be used in combination with the central example.

## Logging

The library contains its own logger that you can also use in your application. 

* Turn logging on/off: `log_enabled(TRUE)`
* Set logging level: `log_set_level(LOG_DEBUG)`
* Log to a file using log rotation: `log_set_filename("mylog.log", 65536, 10)`
* Log something: `log_debug("MyTag", "Hello %s", "world")
`
## Bluez documentation

The official Bluez documentation is a bit sparse but can be found here: 
* [Adapter documentation](https://github.com/bluez/bluez/blob/master/doc/org.bluez.Adapter.rst) (for default_adapter)
* [Characteristic documentation](https://github.com/bluez/bluez/blob/master/doc/org.bluez.GattCharacteristic.rst) (for characteristics)
* [Device documentation](https://github.com/bluez/bluez/blob/master/doc/org.bluez.Device.rst) (for device)
* [Agent documentation](https://github.com/bluez/bluez/blob/master/doc/org.bluez.Agent.rst) (for agent)

You will notice that most original methods and properties are available in this library. In some cases, some adaptations have been done for convenience.


## Installing on Raspberry Pi
Assuming you have a default installation, you will need to install CMake and GLib:
* sudo apt install -y cmake
* sudo apt install -y libglib2.0-dev
