#!/bin/bash

set -e
set -o pipefail

rm -rf Othello.app

mkdir -p Othello.app/Contents/MacOS Othello.app/Contents/Resources

clang -Wextra -O3 -DNDEBUG main.m AppDelegate.m OthelloView.m ../othello.c \
	-mmacosx-version-min=10.7 -fobjc-arc -fobjc-link-runtime \
	-lobjc -framework Cocoa -o Othello.app/Contents/MacOS/Othello

ibtool --output-format human-readable-text --errors --warnings --notices \
	--module Othello --target-device mac --minimum-deployment-target 10.7 \
	--compile Othello.app/Contents/Resources/MainMenu.nib \
	MainMenu.xib

iconutil -c icns -o Othello.app/Contents/Resources/Othello.icns ../icons/mac.iconset

cp Info.plist Othello.app/Contents/

rm -f MacOthello.zip && zip -qr9 MacOthello.zip Othello.app
