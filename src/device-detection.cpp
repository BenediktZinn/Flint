//
// Created by Benedikt Zinn on 29.12.24.
//

#include <CoreFoundation/CFNumber.h>
#include <IOKit/usb/IOUSBLib.h>
#include <IOKit/usb/USBSpec.h>
#include <IOKit/IOKitLib.h>
#include <iostream>
#include "device-detection.h"
#include "utils.h"

bool is_android_device(io_service_t usbDevice) {
    uint16_t vendorID = 0;
    uint16_t productID = 0;

    auto vendorIDRef = (CFNumberRef) IORegistryEntryCreateCFProperty(
            usbDevice, CFSTR(kUSBVendorID), kCFAllocatorDefault, 0);
    auto productIDRef = (CFNumberRef) IORegistryEntryCreateCFProperty(
            usbDevice, CFSTR(kUSBProductID), kCFAllocatorDefault, 0);

    if (vendorIDRef) {
        CFNumberGetValue(vendorIDRef, kCFNumberSInt16Type, &vendorID);
        CFRelease(vendorIDRef);
    }

    if (productIDRef) {
        CFNumberGetValue(productIDRef, kCFNumberSInt16Type, &productID);
        CFRelease(productIDRef);
    }

    // Compare vendor ID and product ID with known Android manufacturers
    // Feel free to extend this list :)


    switch (vendorID) {
        case 0x18D1: // Google
        case 0x04E8: // Samsung
        case 0x12D1: // Huawei
            return true;
        default:
            return false;
    }
}

void usbDeviceCallback(void *refCon, io_iterator_t iterator) {
    io_service_t usbDevice;
    while ((usbDevice = IOIteratorNext(iterator))) {

        if(!is_android_device(usbDevice)){
            continue;
        }
        // the usb-device definitely is one of the usb-devices i know...

        // do something here...

        IOObjectRelease(usbDevice); // Release the device after processing
    }
}

int setUpDeviceDetection() {
    CFMutableDictionaryRef matchingDict = IOServiceMatching(kIOUSBDeviceClassName);
    if (!matchingDict) {
        std::cerr << "Failed to create matching dictionary for USB devices." << std::endl;
        die();
    }

    IONotificationPortRef notificationPort = IONotificationPortCreate(kIOMainPortDefault);
    if (!notificationPort) {
        std::cerr << "Failed to create notification port." << std::endl;
        CFRelease(matchingDict);
        die();
    }

    io_iterator_t iterator;
    kern_return_t result = IOServiceAddMatchingNotification(
            notificationPort,
            kIOFirstMatchNotification,
            matchingDict,
            usbDeviceCallback,
            nullptr,  // Optional user-defined context
            &iterator
    );

    if (result != KERN_SUCCESS) {
        std::cerr << "Failed to add matching notification: " << result << std::endl;
        IONotificationPortDestroy(notificationPort);
        CFRelease(matchingDict);
        die();
    }

    // Process already-matched devices
    usbDeviceCallback(nullptr, iterator);

    // Run the run loop to wait for new devices
    CFRunLoopSourceRef runLoopSource = IONotificationPortGetRunLoopSource(notificationPort);
    CFRunLoopAddSource(CFRunLoopGetCurrent(), runLoopSource, kCFRunLoopDefaultMode);
    CFRunLoopRun();

    // Cleanup
    IOObjectRelease(iterator);  // Release the iterator
    IONotificationPortDestroy(notificationPort);
    CFRelease(matchingDict);  // Release matchingDict

    return 0;
}