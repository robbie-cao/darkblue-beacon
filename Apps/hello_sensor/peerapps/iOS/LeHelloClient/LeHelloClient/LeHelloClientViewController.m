//
//  LeHelloClientViewController.m
//  LeHelloClient
//
//  Created by fredc on 2/20/14.
//  Copyright (c) 2014 Broadcom. All rights reserved.
//

#import "LeHelloClientViewController.h"
#import "LeDevicePickerViewController.h"
#import "CustomNavigationController.h"
#import <CoreBluetooth/CoreBluetooth.h>
#import "Constants.h"



#define DEVICE_PICKER_CONTROLLER_ID @"DevicePickerSegue"

@interface LeHelloClientViewController ()
@property (weak, nonatomic) IBOutlet UIButton *mButtonConnect;
@property (weak, nonatomic) IBOutlet UIButton *mButtonDisconnect;
@property (weak, nonatomic) IBOutlet UILabel *mTextDeviceAddress;
@property (weak, nonatomic) IBOutlet UILabel *mTextDeviceState;
@property (weak, nonatomic) IBOutlet UIView *mButtonSelectDevice;
@property (weak, nonatomic) IBOutlet UILabel *mTextDeviceName;
@property (weak, nonatomic) IBOutlet UILabel *mTextHelloInput;
@property (weak, nonatomic) IBOutlet UISwitch *mSwitchHelloInput;


@property (weak, nonatomic) IBOutlet UIButton *mButtonReadHelloInput;
@property (weak, nonatomic) IBOutlet UITextField *mTextHelloConfig;
@property (weak, nonatomic) IBOutlet UIButton *mButtonReadHelloConfig;
@property (weak, nonatomic) IBOutlet UIButton *mButtonWriteHelloConfig;

@property (weak, nonatomic) IBOutlet UILabel *mTextManufacturerName;
@property (weak, nonatomic) IBOutlet UILabel *mTextModelNumber;
@property (weak, nonatomic) IBOutlet UILabel *mTextSystemId;
@property (weak, nonatomic) IBOutlet UIButton *mButtonReadDeviceInfo;


@property (weak, nonatomic) IBOutlet UILabel *mTextBatteryLevel;
@property (weak, nonatomic) IBOutlet UIButton *mButtonReadBattery;

@property (weak, nonatomic) IBOutlet UITextView *mTextViewNotification;
@property (weak, nonatomic) IBOutlet UIButton *mButtonClearNotification;
@end


@implementation LeHelloClientViewController
    CBCentralManager *btManager;
    CBPeripheral *mPickedDevice = Nil;
    bool mPickedDeviceConnected;
    NSArray *mHelloInputChoices;
    bool mSyncNotifications;


NSData* byteToNSData(int value) {
    unsigned char b= (unsigned char) value;
    NSData* data = [NSData dataWithBytes:&b length:1];
    return data;
}

- (CBService*) getServiceWithUuid: (CBUUID*) uuid {
    NSUInteger count = mPickedDevice.services.count;
    for (NSUInteger s=0; s < count;s++) {
        CBService* service =  [mPickedDevice.services objectAtIndex:s];
        if ([service.UUID isEqual:uuid]) {
            return service;
        }
    }
    return nil;
}

- (CBCharacteristic*) getCharacteristic: (CBUUID*) uuid FromServiceUuid:(CBUUID*)serviceUuid {
    CBService* service = [self getServiceWithUuid:serviceUuid];
    if (service == nil) {
        return nil;
    }
    NSUInteger count = service.characteristics.count;
    for (NSUInteger c=0; c < count;c++) {
        CBCharacteristic* characteristic =  [service.characteristics objectAtIndex:c];
        if ([characteristic.UUID isEqual:uuid]) {
            return characteristic;
        }
    }
    return nil;
}

- (CBCharacteristic*) getCharacteristic: (CBUUID*) uuid FromService:(CBService*)service {
    if (service == nil) {
        return nil;
    }
    NSUInteger count = service.characteristics.count;
    for (NSUInteger c=0; c < count;c++) {
        CBCharacteristic* characteristic =  [service.characteristics objectAtIndex:c];
        if ([characteristic.UUID isEqual:uuid]) {
            return characteristic;
        }
    }
    return nil;
}

- (CBDescriptor*) getDescriptor: (CBUUID*) uuid FromCharacteristic:(CBCharacteristic*) characteristic {
    if (characteristic == nil) {
        return nil;
    }
    NSUInteger count = characteristic.descriptors.count;
    for (NSUInteger d=0; d< count; d++) {
        CBDescriptor* descriptor = [characteristic.descriptors objectAtIndex: d];
        if ([descriptor.UUID isEqual:uuid]){
            return descriptor;
        }
    }
    return nil;
}

// Support only portrait orientation (api for IOS 5 and below)
- (BOOL)shouldAutorotateToInterfaceOrientation:(UIInterfaceOrientation)toInterfaceOrientation{
    return UIInterfaceOrientationMaskPortrait == toInterfaceOrientation;
}


- (void)viewDidLoad {
    [super viewDidLoad];
    btManager = [[CBCentralManager alloc] initWithDelegate:self queue:nil];

	// Do any additional setup after loading the view, typically from a nib.
    [ self updateWidgets];
}

// Support only portrait orientation
- (NSUInteger)supportedInterfaceOrientations{
    return UIInterfaceOrientationMaskPortrait;
}
- (IBAction)onHelloInputChanged:(id)sender {
    BOOL setNotification =[self.mSwitchHelloInput isOn];
    [self setHelloInput:setNotification ];
    [self updateHelloInputDescription ];
}

- (void) updateConnectionStateButtons {
    if (mPickedDeviceConnected) {
        self.mButtonConnect.tintColor= [UIColor grayColor];
        self.mButtonDisconnect.tintColor= nil;
    } else {
        self.mButtonDisconnect.tintColor= [UIColor grayColor           ];
        self.mButtonConnect.tintColor= nil;
    }
}
- (void) updateHelloInputDescription {
    if (self.mSwitchHelloInput.on) {
        self.mTextHelloInput.text =[NSString stringWithFormat:NSLocalizedString(@"VALUE_ALLOW_NOTIFICATIONS", nil),nil ];
    } else {
        self.mTextHelloInput.text =[NSString stringWithFormat:NSLocalizedString(@"VALUE_NO_SEND", nil),nil ];
    }
}
- (void) setHelloInput: (BOOL)sendNotification {
    
    CBCharacteristic* characteristic = [self getCharacteristic: [CBUUID UUIDWithString:HELLO_CHARACTERISTIC_INPUT_UUID]  FromServiceUuid:[CBUUID UUIDWithString:HELLO_SERVICE_UUID]];
    if (characteristic ==nil) {
        return;
    }
    
    CBDescriptor *descriptor= [self getDescriptor:[CBUUID UUIDWithString:CLIENT_CONFIG_DESCRIPTOR_UUID] FromCharacteristic:characteristic];
    if (descriptor == nil) {
        return;
    }
    if (sendNotification){
        [ mPickedDevice setNotifyValue:YES  forCharacteristic:characteristic];
    } else {
        [ mPickedDevice setNotifyValue:NO  forCharacteristic:characteristic];
    }    
}

- (UIView *)pickerView:(UIPickerView *)pickerView viewForRow:(NSInteger)row forComponent:(NSInteger)component reusingView:(UIView *)view {
    UILabel *label = [[UILabel alloc] initWithFrame:CGRectMake(0, 0, pickerView.frame.size.width, 44)];
    label.backgroundColor = [UIColor clearColor];
    label.textColor = [UIColor blackColor];
    label.font = [UIFont fontWithName:@"HelveticaNeue-Bold" size:18];
    label.text = [mHelloInputChoices objectAtIndex:row];
    return label;
}
//Updates the UI widgets based on the connection state of the peer device
- (void) updateWidgets
{
    if (mPickedDevice == NULL ||!mPickedDeviceConnected)
    {
        NSLog(@"updateWidgets:dd state = %d", (int)mPickedDevice.state);
        if (mPickedDevice == NULL)
        {
            //Disable connect/disconnect if device isn't picked yet
            self.mButtonConnect.enabled=NO;
            self.mButtonDisconnect.enabled=NO;
            self.mTextDeviceName.text= [NSString stringWithFormat:NSLocalizedString(@"TEXT_NO_DEVICES", nil),nil ];
            self.mTextDeviceAddress.text= @"";
            self.mTextDeviceState.text=@"";
            
        }
        else
        {
            //Enable connect/disconnect if device picked but not connected yet
            self.mButtonConnect.enabled=YES;
            self.mButtonDisconnect.enabled=YES;
            self.mTextDeviceState.text=[NSString stringWithFormat:NSLocalizedString(@"VALUE_CONNECTION_STATE_DISCONNECTED", nil),nil ];
            self.mTextDeviceName.text=mPickedDevice.name;
            if (mPickedDevice.name.length ==0) {
                self.mTextDeviceName.text=[NSString stringWithFormat:NSLocalizedString(@"UNKNOWN_DEVICE_NAME", nil),nil ];
            }
            self.mTextDeviceAddress.text=[mPickedDevice.identifier UUIDString];
            [self updateConnectionStateButtons];
        }
        
        NSString *noValue = [NSString stringWithFormat:NSLocalizedString(@"VALUE_NO_VALUE", nil),nil ];
        //self.mButtonSelectDevice.enabled=YES;
        //self.mPickerHelloInput.enabled=NO;
        self.mButtonReadHelloInput.enabled=NO;
        self.mSwitchHelloInput.enabled=NO;
        self.mSwitchHelloInput.on=NO;
        [self updateHelloInputDescription];
        
        self.mTextHelloConfig.text=@"";
        self.mTextHelloConfig.enabled=NO;
        self.mButtonReadHelloConfig.enabled=NO;
        self.mButtonWriteHelloConfig.enabled=NO;
        
        self.mButtonReadDeviceInfo.enabled=NO;
        self.mTextManufacturerName.text=noValue;
        self.mTextModelNumber.text=noValue;
        self.mTextSystemId.text=noValue;
        
        self.mButtonReadBattery.enabled=NO;
        self.mTextBatteryLevel.text=noValue;
        
        self.mTextViewNotification.text=@"";
        self.mButtonClearNotification.enabled=NO;
    }
    else
    {
        NSLog(@"updateWidgets: state = %d", (int)mPickedDevice.state);
        //Device is picked and connected.... Set state based on current values
        self.mButtonConnect.enabled=YES;
        self.mButtonDisconnect.enabled=YES;
        
        //Disable selecting new device when connected
        self.mButtonSelectDevice=NO;
        
        //Set device state
        self.mTextDeviceState.text=[NSString stringWithFormat:NSLocalizedString(@"VALUE_CONNECTION_STATE_CONNECTED", nil),nil ];
        [self updateConnectionStateButtons];

        //Update hello service components
        self.mButtonReadHelloInput.enabled=YES;
        self.mButtonReadHelloConfig.enabled=YES;
        self.mButtonWriteHelloConfig.enabled=YES;
        self.mTextHelloConfig.enabled=YES;
        self.mSwitchHelloInput.enabled=YES;
        [self updateHelloInputDescription];
        
        //Update device info components
        self.mButtonReadDeviceInfo.enabled=YES;
        self.mTextDeviceName.text=mPickedDevice.name;
        self.mTextDeviceAddress.text=[mPickedDevice.identifier UUIDString];

        
        //Update battery components
        self.mButtonReadBattery.enabled=YES;
        
        //Update notification components
        self.mButtonClearNotification.enabled=YES;
    }
}


- (void) readBatteryInfo {
    CBCharacteristic *c  = [self getCharacteristic:[CBUUID UUIDWithString:BATTERY_LEVEL_UUID] FromServiceUuid:
                            [CBUUID UUIDWithString:BATTERY_SERVICE_UUID] ];
    if(c !=nil){
        [mPickedDevice readValueForCharacteristic:c];
    }
}

- (void) readHelloConfiguration {
    CBCharacteristic *c  = [self getCharacteristic:[CBUUID UUIDWithString:HELLO_CHARACTERISTIC_CONFIGURATION_UUID] FromServiceUuid:[CBUUID UUIDWithString:HELLO_SERVICE_UUID]];
    if (c !=nil){
        [mPickedDevice readValueForCharacteristic:c];
    }
}

- (void) readHelloInput {
    CBCharacteristic *c  = [self getCharacteristic:[CBUUID UUIDWithString:HELLO_CHARACTERISTIC_INPUT_UUID] FromServiceUuid:[CBUUID UUIDWithString:HELLO_SERVICE_UUID]];
    if (c !=nil){
        CBDescriptor* descriptor = [self getDescriptor:[CBUUID UUIDWithString:CLIENT_CONFIG_DESCRIPTOR_UUID] FromCharacteristic:c];
        if (descriptor != nil) {
            [mPickedDevice readValueForDescriptor:descriptor];
        }
    }
}

- (void) readDeviceInfo {
    CBService *s = [self getServiceWithUuid:[CBUUID UUIDWithString:DEVICE_INFO_SERVICE_UUID]];
    if (s != nil) {
        CBCharacteristic *c  = [self getCharacteristic:[CBUUID UUIDWithString:MANUFACTURER_NAME_UUID] FromService:s];
        if(c !=nil){
            [mPickedDevice readValueForCharacteristic:c];
        }
        c  = [self getCharacteristic:[CBUUID UUIDWithString:MODEL_NUMBER_UUID] FromService:s];
        if(c !=nil){
            [mPickedDevice readValueForCharacteristic:c];
        }
        c  = [self getCharacteristic:[CBUUID UUIDWithString:SYSTEM_ID_UUID] FromService:s];
        if(c !=nil){
            [mPickedDevice readValueForCharacteristic:c];
        }
    }
}

- (void)prepareForSegue:(UIStoryboardSegue *)segue sender:(id)sender
{
    NSLog(@"LeHelloClientViewController.prepareForSegue: segue=%@", segue.identifier);
    
    if ([[segue identifier] isEqualToString: DEVICE_PICKER_CONTROLLER_ID])
    {
        CustomNavigationController* navController = [segue destinationViewController];
        
        LeDevicePickerViewController* devicePicker = (LeDevicePickerViewController*) navController.topViewController;
#ifdef ENABLE_DEVICE_PICKER_FILTERING
        devicePicker.servicesToScan= [NSArray arrayWithObject:[CBUUID UUIDWithString:HELLO_SENSOR_SERVICE_UUID]];
#endif
        devicePicker.scanOptions = [NSDictionary dictionaryWithObjectsAndKeys:[NSNumber numberWithBool:YES],CBCentralManagerScanOptionAllowDuplicatesKey, nil];
        devicePicker.callback= self;
        devicePicker.btManager=btManager;
#ifdef SHOW_DEVICE_IDS
        devicePicker.showDeviceIdentifiers=YES;
#endif
    }
}


- (void)onDevicePickerCancelled {
    
}

- (void)onDevicePickerSelected:(CBPeripheral*) device {
    mPickedDevice= device;
    [self updateWidgets];
}

- (IBAction)onClearNotificationButtonClicked:(UIButton *)sender {
    self.mTextViewNotification.text=@"";
}
- (IBAction)onConnectButtonClicked:(id)sender {
    NSLog(@"onConnectButtonClicked");
    NSDictionary *options = [NSDictionary dictionaryWithObjectsAndKeys:
                             [NSNumber numberWithBool:YES], CBConnectPeripheralOptionNotifyOnDisconnectionKey
                            , [NSNumber numberWithBool:YES], CBConnectPeripheralOptionNotifyOnConnectionKey
                            , [NSNumber numberWithBool:YES], CBConnectPeripheralOptionNotifyOnNotificationKey,nil ];
    
    
    [ btManager connectPeripheral:mPickedDevice  options:options];
}
- (IBAction)onDisconnectButtonClicked:(id)sender {
    NSLog(@"onDisconnectButtonClicked");
    [ btManager cancelPeripheralConnection:mPickedDevice];
}

- (void)centralManagerDidUpdateState:(CBCentralManager *)central {
    
}

- (void)centralManager:(CBCentralManager *)central didConnectPeripheral:(CBPeripheral *)peripheral {
    NSLog(@"didConnectPeripheral: %@", peripheral.name);
    mSyncNotifications= YES;
    [peripheral setDelegate: self];
    [peripheral discoverServices:nil];
    [self updateWidgets];
}


- (void)centralManager:(CBCentralManager *)central didFailToConnectPeripheral:(CBPeripheral *)peripheral error:(NSError *)error {
    NSLog(@"didFailToConnectPeripheral: %@, error=%ld", peripheral.name, (long)error.code);
    [peripheral setDelegate: nil];
    mPickedDeviceConnected=NO;
    [self updateWidgets];
}

-(void)centralManager:(CBCentralManager *)central didDisconnectPeripheral:(CBPeripheral *)peripheral error:(NSError *)error {
    NSLog(@"didDisconnectPeripheral: %@, error=%ld", peripheral.name, (long)error.code);
    [peripheral setDelegate: nil];
    mPickedDeviceConnected=NO;
    [self updateWidgets];

}


- (void)peripheral:(CBPeripheral *)peripheral didDiscoverServices:(NSError *)error {
    if (error != nil) {
        NSLog(@"didDiscoverServices failed: %ld", (long)error.code);
    } else {
        CBService* service = [self getServiceWithUuid:[CBUUID UUIDWithString:HELLO_SERVICE_UUID] ];
        if (service != nil) {
            [peripheral discoverCharacteristics:nil forService:service];
        }
        service = [self getServiceWithUuid:[CBUUID UUIDWithString:BATTERY_SERVICE_UUID]];

        if (service != nil) {
            [peripheral discoverCharacteristics:nil forService:service];
        }
        service = [ self getServiceWithUuid:[CBUUID UUIDWithString:DEVICE_INFO_SERVICE_UUID] ];
        if (service != nil) {
            [peripheral discoverCharacteristics:nil forService:service];
        }
         mPickedDeviceConnected=YES;
    }
    [self updateWidgets];
}

- (void)peripheral:(CBPeripheral *)peripheral didDiscoverCharacteristicsForService:(CBService *)service error:(NSError *)error {
    if (error != nil) {
        NSLog(@"didDiscoverCharacteristicsForService failed: %ld", (long)error.code);
    } else {
        if ([service.UUID isEqual: [CBUUID UUIDWithString: HELLO_SERVICE_UUID]]) {
            CBCharacteristic *characteristic = [self getCharacteristic:[CBUUID UUIDWithString:HELLO_CHARACTERISTIC_INPUT_UUID] FromService:service];
                if (characteristic != nil) {
                    [peripheral discoverDescriptorsForCharacteristic:characteristic];
                }
            [self readHelloConfiguration];
        } else if ([service.UUID isEqual: [CBUUID UUIDWithString: DEVICE_INFO_SERVICE_UUID]]) {
            [self readDeviceInfo];
        } else if ([service.UUID isEqual: [CBUUID UUIDWithString: BATTERY_SERVICE_UUID]]) {
            [self readBatteryInfo];
        }
    }
}

- (void)peripheral:(CBPeripheral *)peripheral didDiscoverDescriptorsForCharacteristic:(CBCharacteristic *)characteristic error:(NSError *)error {
    if (error != nil) {
        NSLog(@"didDiscoverDescriptorsForCharacteristic failed: %ld", (long)error.code);
    } else {
        if ([characteristic.UUID isEqual: [CBUUID UUIDWithString: HELLO_CHARACTERISTIC_INPUT_UUID]]) {
                CBDescriptor *d = [self getDescriptor: [CBUUID UUIDWithString: CLIENT_CONFIG_DESCRIPTOR_UUID] FromCharacteristic:characteristic];
            if (d != nil) {
                [ self readHelloInput];
            }
        }
    }
}
-(void)peripheral:(CBPeripheral *)peripheral didUpdateValueForCharacteristic:(CBCharacteristic *)characteristic error:(NSError *)error {
    NSLog(@"didUpdateValueForCharacterstic: %@, error=%ld", characteristic,(long)error.code);
    
    if ( [characteristic.UUID isEqual:[CBUUID UUIDWithString:HELLO_CHARACTERISTIC_CONFIGURATION_UUID]]) {
        unsigned char b =  ((unsigned char*)[characteristic.value bytes])[0];
        self.mTextHelloConfig.text=[[NSString alloc] initWithFormat:@"%d", b];
    }
    
    else if ( [characteristic.UUID isEqual:[CBUUID UUIDWithString:MANUFACTURER_NAME_UUID]]) {
        self.mTextManufacturerName.text= [[NSString alloc] initWithData:characteristic.value encoding:NSUTF8StringEncoding];
    }
    
    else if ( [characteristic.UUID isEqual:[CBUUID UUIDWithString:MODEL_NUMBER_UUID]]) {
        self.mTextModelNumber.text= [[NSString alloc] initWithData:characteristic.value encoding:NSUTF8StringEncoding];
    }

    else if ( [characteristic.UUID isEqual:[CBUUID UUIDWithString:SYSTEM_ID_UUID]]) {
        unsigned char* bytes = (unsigned char*)[characteristic.value bytes];
        UInt64 systemIdLong = 0;
        NSUInteger shift=0;
        for (NSUInteger i = 0; i < 8;i++){
            systemIdLong = systemIdLong + ((UInt64) (bytes[i] &0xFF) << (8*shift++));
        }
        NSLog(@"64 bit long is %llu", systemIdLong);
        
        self.mTextSystemId.text= [[NSString alloc] initWithFormat:@"%06llX %010llX", 0xFFFFFFL & (systemIdLong >>40) ,0xFFFFFFFFFFL &systemIdLong ];

    }
    
    else if ( [characteristic.UUID isEqual:[CBUUID UUIDWithString:BATTERY_LEVEL_UUID]]) {
        Byte b =  ((Byte*)[characteristic.value bytes])[0];
        self.mTextBatteryLevel.text=[[NSString alloc] initWithFormat:@"%hhu", b];
    }
    
    else if ( [characteristic.UUID isEqual:[CBUUID UUIDWithString:HELLO_CHARACTERISTIC_INPUT_UUID]]) {
        self.mTextViewNotification.text = [[NSString alloc] initWithData:characteristic.value encoding:NSUTF8StringEncoding];
    }
    
}

- (void)peripheral:(CBPeripheral *)peripheral didUpdateValueForDescriptor:(CBDescriptor *)descriptor error:(NSError *)error {
    if (error !=nil) {
        NSLog(@"didUpdateValueForDescriptor failed: %ld", (long)error.code);
    } else {
        if ([descriptor.UUID isEqual:[CBUUID UUIDWithString:CLIENT_CONFIG_DESCRIPTOR_UUID]]) {
            int value = [(NSNumber*) descriptor.value integerValue];
            BOOL notificationOn = value == 1 || value ==2;
            self.mSwitchHelloInput.on =notificationOn;
            [self updateHelloInputDescription];
            if (mSyncNotifications) {
                mSyncNotifications = NO;
                if (notificationOn){
                    [ mPickedDevice setNotifyValue:YES  forCharacteristic:descriptor.characteristic];
                }
            }
        }
    }
}

- (void)peripheral:(CBPeripheral *)peripheral didUpdateNotificationStateForCharacteristic:(CBCharacteristic *)characteristic error:(NSError *)error {
    if (error !=nil){
        NSLog(@"didUpdateNotificationStateForCharacteristic failed: %ld", (long)error.code);
    } else {
        NSLog(@"didUpdateNotificationStateForCharacteristic characteristic %@", characteristic);

    }
}

- (IBAction)onReadHelloInput:(id)sender {
    [self readHelloInput];
}


- (IBAction)onReadHelloConfiguration:(id)sender {
    [self readHelloConfiguration];
}

- (IBAction)onWriteHelloConfiguration:(id)sender {
    CBCharacteristic *c  = [self getCharacteristic:[CBUUID UUIDWithString:HELLO_CHARACTERISTIC_CONFIGURATION_UUID] FromServiceUuid:[CBUUID UUIDWithString:HELLO_SERVICE_UUID]];
    if (c != nil) {
        unsigned char value = (unsigned char)[self.mTextHelloConfig.text intValue];
        NSLog(@"Byte value =%d",value);
        
        NSData* data = byteToNSData([self.mTextHelloConfig.text intValue]);
        [mPickedDevice writeValue:data forCharacteristic:c  type:CBCharacteristicWriteWithResponse];
    }
}

- (IBAction)onReadDeviceInfo:(id)sender {
    [self readDeviceInfo];
    
}



- (IBAction)onReadBatteryInfo:(id)sender {
    [self readBatteryInfo ];
}

                    

@end
