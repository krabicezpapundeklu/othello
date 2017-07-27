#import "AppDelegate.h"
#import "OthelloViewController.h"

@implementation AppDelegate

- (BOOL)application:(UIApplication*)application
        willFinishLaunchingWithOptions:(nullable NSDictionary*)launchOptions {
    self.window =
        [[UIWindow alloc] initWithFrame:[[UIScreen mainScreen] bounds]];
    return YES;
}

- (BOOL)application:(UIApplication*)application
        didFinishLaunchingWithOptions:(NSDictionary*)launchOptions {
    if (!self.window.rootViewController) {
        self.window.rootViewController = [[OthelloViewController alloc] init];
    }

    [self.window makeKeyAndVisible];
    return YES;
}

- (BOOL)application:(UIApplication*)application
        shouldSaveApplicationState:(nonnull NSCoder*)coder {
    return YES;
}

- (BOOL)application:(UIApplication *)application
        shouldRestoreApplicationState:(nonnull NSCoder*)coder {
    return YES;
}

@end
