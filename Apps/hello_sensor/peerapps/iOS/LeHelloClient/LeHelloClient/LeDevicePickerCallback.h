//
//  LeDevicePickerCallback.h
//  LeHelloClient
//
//  Created by fredc on 3/3/14.
//  Copyright (c) 2014 Broadcom. All rights reserved.
//

#import <Foundation/Foundation.h>
#import <CoreBluetooth/CoreBluetooth.h>
@class LeDevicePickerViewController;

@protocol LeDevicePickerCallback <NSObject>

- (void)onDevicePickerCancelled;
- (void)onDevicePickerSelected:(CBPeripheral*) device;
@end
