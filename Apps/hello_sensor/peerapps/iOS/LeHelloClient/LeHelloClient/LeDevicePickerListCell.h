//
//  LeDevicePickerListCell.h
//  LeHelloClient
//
//  Created by fredc on 3/3/14.
//  Copyright (c) 2014 Broadcom. All rights reserved.
//

#import <UIKit/UIKit.h>

@interface LeDevicePickerListCell : UITableViewCell
@property (weak, nonatomic) IBOutlet UILabel *deviceName;
@property (weak, nonatomic) IBOutlet UILabel *deviceIdentifier;
@property (weak, nonatomic) IBOutlet UIProgressView *dbmBar;
@property (weak, nonatomic) IBOutlet UILabel *dbm;


@end
