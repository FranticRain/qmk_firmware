# Southpaw

Dopre Southpaw Keyboard

Keyboard Maintainer: [Drew Mills](https://github.com/franticrain)
Hardware Supported: STM32F303CCT6

## About

This is the first custom electrostatic capacitive keyboard using QMK. It features a 100% "southpaw" layout. What this means is that the navigation cluster and numpad have been flipped relative to the alpha keys. This is for the purpose of increasing mousing space for right-handed users, while maintaining a 100% layout. Some suggestion has been made that the numpad should be flipped (enter on the left hand side) in order to accommodate this flipped layout. The choice was made to leave the numpad as-is for purposes of keycap compatibility. Despite the "southpaw" naming, this keyboard is designed for right handed mouse users.

## Build

In order to build the software for this keyboard, run this command after you have set up your build environment:

```bash
    make dopre/southpaw:default
```

In order to flash the software for this keyboard, run this command after you have set up your build environment:

```bash
	make dopre/southpaw:default:dfu-util
```
