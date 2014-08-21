//
//  LeDevicePickerControllerViewController.m
//  LeHelloClient
//
//  Created by fredc on 2/26/14.
//  Copyright (c) 2014 Broadcom. All rights reserved.
//

#import "LeDevicePickerViewController.h"
#import <CoreBluetooth/CoreBluetooth.h>
#import "LeDevicePickerListCell.h"

//Expected RSSI range is -127 -> 20
#define RSSI_RANGE 147
#define RSSI_MAX 20

@interface LeDevicePickerViewController ()
@property (weak, nonatomic) IBOutlet UIButton *mButtonScan;
@property (weak, nonatomic) IBOutlet UINavigationItem *mButtonCancel;



@end

@implementation LeDevicePickerViewController



bool scanState;
NSMutableArray *discoveredDevices;
NSMutableArray *discoveredRssi;
id<CBCentralManagerDelegate> oldDelegate;


- (id)initWithCoder:(NSCoder *)aDecoder
{
    if ((self = [super initWithCoder:aDecoder]))
	{
		NSLog(@"init PlayerDetailsViewController");
	}
	return self;
}

- (void)viewDidLoad {
    [super viewDidLoad];
    scanState= NO;
    discoveredDevices= [NSMutableArray array];
    discoveredRssi=[NSMutableArray array];
    oldDelegate= self.btManager.delegate;
    [self.btManager setDelegate: self];
    
}


-(void)viewDidLayoutSubviews {
    [super viewDidLayoutSubviews];
    self.tableView.separatorInset = UIEdgeInsetsMake(20, 20, 20,20);
}

-(void) viewDidAppear:(BOOL) animated {
    [super viewDidAppear: animated];
    [self setScanState: YES];
}

     
- (void) setScanState:(BOOL) startScan{
    if (startScan == scanState) {
        return;
    }
    
    scanState = startScan;
    [self updateScanStateWidgets];
    
    if (startScan){
        [discoveredDevices removeAllObjects];
        [self.tableView reloadData];
        [self.btManager scanForPeripheralsWithServices:self.servicesToScan options:self.scanOptions];
    }
    else {
        [self.btManager stopScan];
    }
}


// Update widgets based on the the current scan state
- (void) updateScanStateWidgets {
    if (scanState == NO) {
        [self.mButtonScan setTitle: [NSString stringWithFormat:NSLocalizedString(@"SCAN_STATE_LABEL_STOPPED", nil),nil ] forState: UIControlStateNormal];
    }
    else {
        [self.mButtonScan setTitle: [NSString stringWithFormat:NSLocalizedString(@"SCAN_STATE_LABEL_STARTED", nil),nil ] forState: UIControlStateNormal];
    }
}


//Callback invoked the the cancel button is invoked
- (IBAction)onCancelClicked:(id)sender {
    [self setScanState: NO];
    [self.btManager setDelegate: oldDelegate];
    [self.callback onDevicePickerCancelled];
    [self dismissViewControllerAnimated:YES completion:nil];
}

//Callback invoked when the scan button is invoked
- (IBAction)onScanClicked:(id)sender {
    [self setScanState: !scanState];
}


- (id)initWithStyle:(UITableViewStyle)style {
    self = [super initWithStyle:style];
    if (self) {
        // Custom initialization
    }
    return self;
}
- (void)didReceiveMemoryWarning {
    NSLog(@"didReceiveMemoryWarning");
}


- (NSInteger)numberOfSectionsInTableView:(UITableView *)tableView {
    return 1;
}

- (NSInteger)tableView:(UITableView *)tableView numberOfRowsInSection:(NSInteger)section {
    return [discoveredDevices count];
}

- (UITableViewCell *)tableView:(UITableView *)tableView cellForRowAtIndexPath:(NSIndexPath *)indexPath {
    LeDevicePickerListCell *cell = [tableView dequeueReusableCellWithIdentifier:@"LeDevicePickerListCell"];
    NSUInteger deviceIndex = [indexPath row];
    if (deviceIndex != NSNotFound && deviceIndex < discoveredDevices.count) {
        CBPeripheral *device = [discoveredDevices objectAtIndex:deviceIndex];
        NSNumber *rssi= [discoveredRssi objectAtIndex:deviceIndex];
        NSString* deviceId=[device.identifier UUIDString];
    
        //Set name
        if ([device.name length]==0) {
            cell.deviceName.text=[NSString stringWithFormat:NSLocalizedString(@"UNKNOWN_DEVICE_NAME", nil),nil ];
        } else{
            cell.deviceName.text=device.name;
        }
    
        //Set device id
        if (self.showDeviceIdentifiers) {
            cell.deviceIdentifier.text= deviceId;
        } else {
            cell.deviceIdentifier.text=@"";
        }
             
        //Set the rssi progress bar
        cell.dbmBar.progress =  ((float)(RSSI_RANGE + rssi.intValue - RSSI_MAX))/RSSI_RANGE;

        //Set the rssi label
        cell.dbm.text=[NSString stringWithFormat:NSLocalizedString(@"LABEL_DBM", nil),rssi ];
    }
    return cell;
}



//Callback methods for CBCentralManagerDelegate protocol
- (void)centralManagerDidUpdateState:(CBCentralManager *)central
{
    
}

//Callback invoked when a device is discovered
- (void)centralManager:(CBCentralManager *)central didDiscoverPeripheral:(CBPeripheral *)peripheral advertisementData:(NSDictionary *)advertisementData RSSI:(NSNumber *)RSSI
{
    NSString *name = peripheral.name;
    NSString *id= [peripheral.identifier UUIDString];
    
    NSUInteger index= [discoveredDevices indexOfObject:peripheral];
    if (index != NSNotFound)
    {
        NSLog(@"Found existing device %@  %@  %@: index %ld",id, name, RSSI,(long)index);
        [ discoveredDevices replaceObjectAtIndex:index withObject:peripheral];
        [ discoveredRssi replaceObjectAtIndex:index withObject:RSSI];
    }
    else
    {
        NSLog(@"Found existing device %@  %@  %@",id, name, RSSI);
        [discoveredDevices addObject:peripheral];
        [discoveredRssi addObject:RSSI];
    }
    
    [self.tableView reloadData];
}

- (void)tableView:(UITableView *)tableView didSelectRowAtIndexPath:(NSIndexPath *)indexPath
{
    NSInteger deviceIndex = indexPath.row;
    
    //Get the CBPeriperal device
    if (deviceIndex != NSNotFound && deviceIndex < discoveredDevices.count) {
        [self setScanState:NO];
        CBPeripheral* device = [discoveredDevices objectAtIndex:deviceIndex];
        [self.btManager setDelegate: oldDelegate];
        [self.callback onDevicePickerSelected:device];
        [self dismissViewControllerAnimated:YES completion:nil];
    }
    
}

- (void)prepareForSegue:(UIStoryboardSegue *)segue sender:(id)sender
{
   // if(sender == self.done)
}




@end
