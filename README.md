# USBtoN64v2
Currently in beta testing phase
Utilizes Nucleo-F446RE board with custom header for usb and n64 communication

Currently supported USB devices:
- Keyboard
- Playstation Dualshock 3 Sixaxis
- Wireless Xbox 360 with Microsoft USB wireless receiver
- Wired Xbox 360
- Wired Xbox One

Setting the controls:
1) Press the blue button. A new LED (light) should light up solid when you go into "set controls" mode, and it should turn off when you finish setting the controls correctly.
2) Press the buttons in the following order depending on which device is plugged in (please see below)
3) (optional) Press the blue button again at any time to exit “set controls” mode early if you make a mistake.

Xbox / PS3:
DPAD_UP, DPAD_DOWN, DPAD_LEFT, DPAD_RIGHT, BUTTON_START, BUTTON_B, BUTTON_A, C_UP, C_DOWN, C_LEFT, C_RIGHT, BUTTON_L, BUTTON_R, BUTTON_Z

KB:
A_UP, A_DOWN, A_LEFT, A_RIGHT, DPAD_UP, DPAD_DOWN, DPAD_LEFT, DPAD_RIGHT, BUTTON_START, BUTTON_B, BUTTON_A, C_UP, C_DOWN, C_LEFT, C_RIGHT, BUTTON_L, BUTTON_R, BUTTON_Z

Note that the device saves the controls even when the device is powered off. It saves the Xbox and PS3 controls together as a shared layout and the Keyboard controls separately.

Currently known bugs
- Wireless X360 drops inputs

TODO:
- invent a way to change deadzone/sensitivity on-the-fly without changing the code
- fix bugs

By: Ownasaurus