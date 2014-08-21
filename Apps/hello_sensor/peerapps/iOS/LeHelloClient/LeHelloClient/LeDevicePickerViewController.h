//
//  LeDevicePickerControllerViewController.h
//  LeHelloClient
//
//  Copyright (c) 2014 Broadcom. All rights reserved.
//

#import <UIKit/UIKit.h>
#import <CoreBluetooth/CoreBluetooth.h>
#import "LeDevicePickerCallback.h"

@interface LeDevicePickerViewController : UITableViewController<CBCentralManagerDelegate>

@property NSArray* servicesToScan;
@property NSDictionary* scanOptions;
@property (nonatomic, weak) id <LeDevicePickerCallback> callback;
@property CBCentralManager* btManager; //iOS allows only a singleton instance, so one must be passed in...
@property BOOL showDeviceIdentifiers; //If true, shows the device identifiers
@end
