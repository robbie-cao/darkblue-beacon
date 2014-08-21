//
//  CustomNavigationController.m
//  
//
//  Created by fredc on 2/26/14.
//
//

#import "CustomNavigationController.h"

@implementation CustomNavigationController

// Let the top view controller decide what orientations are supported
- (NSUInteger)supportedInterfaceOrientations
{
    return [ self.topViewController supportedInterfaceOrientations];
}

// Let the top view controller decide what orientations are supported
- (BOOL)shouldAutorotateToInterfaceOrientation:(UIInterfaceOrientation)toInterfaceOrientation
{
    return [self.topViewController shouldAutorotate];
}


@end
