//
//  AppDelegate.m
//  UPnPToolsOSX
//
//  Created by TJ on 2015. 11. 12..
//  Copyright © 2015년 TJ. All rights reserved.
//

#import "AppDelegate.h"
#import "../../upnp-tools/upnp-tools/upnp_tools.h"

@interface AppDelegate ()

@property (weak) IBOutlet NSWindow *window;

@end

@implementation AppDelegate
{
    upnp_tools * tools;
}

- (void)applicationDidFinishLaunching:(NSNotification *)aNotification {
    // Insert code here to initialize your application
    tools = [[upnp_tools alloc] init];
}

- (void)applicationWillTerminate:(NSNotification *)aNotification {
    // Insert code here to tear down your application
}

- (IBAction)onStartClicked:(id)sender {
    NSLog(@"start");
    
    [tools hello];
    [tools startServer];
}

- (IBAction)onStopClicked:(id)sender {
    NSLog(@"stop");
}
@end
