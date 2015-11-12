//
//  AppDelegate.h
//  UPnPToolsOSX
//
//  Created by TJ on 2015. 11. 12..
//  Copyright © 2015년 TJ. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@interface AppDelegate : NSObject <NSApplicationDelegate>

@property (weak) IBOutlet NSButton *startButton;
@property (weak) IBOutlet NSButton *stopButton;

- (IBAction)onStartClicked:(id)sender;
- (IBAction)onStopClicked:(id)sender;

@end

