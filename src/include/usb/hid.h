#ifndef _USB_HID_H_
#define _USB_HID_H_ 1



#define _USB_HID_HEADER(type,tag) (((tag)<<4)|((type)<<2))

#define USB_HID_TAG0(header) ((header)|0)
#define USB_HID_TAG1(header,data) ((header)|1),(data)
#define USB_HID_TAG2(header,data) ((header)|2),(data)&0xff,(data)>>8
#define USB_HID_TAG3(header,data) ((header)|3),(data)&0xff,((data)>>8)&0xff,((data)>>16)&0xff,(data)>>24

#define USB_HID_RI_MAIN_INPUT _USB_HID_HEADER(0,8)
#define USB_HID_RI_MAIN_OUTPUT _USB_HID_HEADER(0,9)
#define USB_HID_RI_MAIN_COLLECTION _USB_HID_HEADER(0,10)
#define USB_HID_RI_MAIN_FEATURE _USB_HID_HEADER(0,11)
#define USB_HID_RI_MAIN_COLLECTION_END _USB_HID_HEADER(0,12)

#define USB_HID_RI_GLOBAL_USAGE_PAGE _USB_HID_HEADER(1,0)
#define USB_HID_RI_GLOBAL_LOGICAL_MIN _USB_HID_HEADER(1,1)
#define USB_HID_RI_GLOBAL_LOGICAL_MAX _USB_HID_HEADER(1,2)
#define USB_HID_RI_GLOBAL_PHYSICAL_MIN _USB_HID_HEADER(1,3)
#define USB_HID_RI_GLOBAL_PHYSICAL_MAX _USB_HID_HEADER(1,4)
#define USB_HID_RI_GLOBAL_UNIT_EXPONENT _USB_HID_HEADER(1,5)
#define USB_HID_RI_GLOBAL_UNIT _USB_HID_HEADER(1,6)
#define USB_HID_RI_GLOBAL_REPORT_SIZE _USB_HID_HEADER(1,7)
#define USB_HID_RI_GLOBAL_REPORT_ID _USB_HID_HEADER(1,8)
#define USB_HID_RI_GLOBAL_REPORT_COUNT _USB_HID_HEADER(1,9)
#define USB_HID_RI_GLOBAL_PUSH _USB_HID_HEADER(1,10)
#define USB_HID_RI_GLOBAL_POP _USB_HID_HEADER(1,11)

#define USB_HID_RI_LOCAL_USAGE _USB_HID_HEADER(2,0)
#define USB_HID_RI_LOCAL_USAGE_MIN _USB_HID_HEADER(2,1)
#define USB_HID_RI_LOCAL_USAGE_MAX _USB_HID_HEADER(2,2)
#define USB_HID_RI_LOCAL_DESIGNATOR_INDEX _USB_HID_HEADER(2,3)
#define USB_HID_RI_LOCAL_DESIGNATOR_MIN _USB_HID_HEADER(2,4)
#define USB_HID_RI_LOCAL_DESIGNATOR_MAX _USB_HID_HEADER(2,5)
#define USB_HID_RI_LOCAL_STRING_INDEX _USB_HID_HEADER(2,7)
#define USB_HID_RI_LOCAL_STRING_MIN _USB_HID_HEADER(2,8)
#define USB_HID_RI_LOCAL_STRING_MAX _USB_HID_HEADER(2,9)
#define USB_HID_RI_LOCAL_DELIMITER _USB_HID_HEADER(2,10)

#define USB_HID_CONSTANT 0x001
#define USB_HID_VARIABLE 0x002
#define USB_HID_RELATIVE 0x004
#define USB_HID_WRAP 0x008
#define USB_HID_NONLINEAR 0x010
#define USB_HID_PREFERRED_NO 0x020
#define USB_HID_NULL_STATE 0x040
#define USB_HID_VOLATILE 0x080
#define USB_HID_BUFFERED_BYTES 0x100

#define USB_HID_COLLECTION_PHYSICAL 0
#define USB_HID_COLLECTION_APPLICATION 1
#define USB_HID_COLLECTION_LOGICAL 2
#define USB_HID_COLLECTION_REPORT 3
#define USB_HID_COLLECTION_NAMED_ARRAY 4
#define USB_HID_COLLECTION_USAGE_SWITCH 5
#define USB_HID_COLLECTION_USAGE_MODIFIER 6

#define USB_HID_USAGE_PAGE_DESKTOP 0x01
#define USB_HID_USAGE_PAGE_SIMULATE 0x02
#define USB_HID_USAGE_PAGE_VIRTUAL_REALITY 0x03
#define USB_HID_USAGE_PAGE_SPORT 0x04
#define USB_HID_USAGE_PAGE_GAME 0x05
#define USB_HID_USAGE_PAGE_GENERIC_DEVICE 0x06
#define USB_HID_USAGE_PAGE_KEYBOARD 0x07
#define USB_HID_USAGE_PAGE_LED 0x08
#define USB_HID_USAGE_PAGE_BUTTON 0x09
#define USB_HID_USAGE_PAGE_ORDINAL 0x0a
#define USB_HID_USAGE_PAGE_TELEPHONY 0x0b
#define USB_HID_USAGE_PAGE_CONSUMER 0x0c
#define USB_HID_USAGE_PAGE_DIGITIZER 0x0d
#define USB_HID_USAGE_PAGE_PID 0x0f
#define USB_HID_USAGE_PAGE_UNICODE 0x10
#define USB_HID_USAGE_PAGE_ALPHA_DISPLAY 0x14
#define USB_HID_USAGE_PAGE_MEDICAL 0x40
#define USB_HID_USAGE_PAGE_MONITOR 0x80
#define USB_HID_USAGE_PAGE_POWER 0x84
#define USB_HID_USAGE_PAGE_BARCODE_SCANNER 0x8c
#define USB_HID_USAGE_PAGE_SCALE 0x8d
#define USB_HID_USAGE_PAGE_MSR 0x8e
#define USB_HID_USAGE_PAGE_CAMERA 0x90
#define USB_HID_USAGE_PAGE_ARCADE 0x91

#define USB_HID_USAGE_DESKTOP_POINTER 0x01
#define USB_HID_USAGE_DESKTOP_MOUSE 0x02
#define USB_HID_USAGE_DESKTOP_JOYSTICK 0x04
#define USB_HID_USAGE_DESKTOP_GAMEPAD 0x05
#define USB_HID_USAGE_DESKTOP_KEYBOARD 0x06
#define USB_HID_USAGE_DESKTOP_KEYPAD 0x07
#define USB_HID_USAGE_DESKTOP_MULTI_AXIS_CONTROLLER 0x08
#define USB_HID_USAGE_DESKTOP_TABLET_PC_SYSTEM 0x09
#define USB_HID_USAGE_DESKTOP_X 0x30
#define USB_HID_USAGE_DESKTOP_Y 0x31
#define USB_HID_USAGE_DESKTOP_Z 0x32
#define USB_HID_USAGE_DESKTOP_RX 0x33
#define USB_HID_USAGE_DESKTOP_RY 0x34
#define USB_HID_USAGE_DESKTOP_RZ 0x35
#define USB_HID_USAGE_DESKTOP_SLIDER 0x36
#define USB_HID_USAGE_DESKTOP_DIAL 0x37
#define USB_HID_USAGE_DESKTOP_WHEEL 0x38
#define USB_HID_USAGE_DESKTOP_HAT_SWITCH 0x39
#define USB_HID_USAGE_DESKTOP_COUNTED_BUFFER 0x3a
#define USB_HID_USAGE_DESKTOP_BYTE_COUNT 0x3b
#define USB_HID_USAGE_DESKTOP_MOTION_WAKEUP 0x3c
#define USB_HID_USAGE_DESKTOP_START 0x3d
#define USB_HID_USAGE_DESKTOP_SELECT 0x3e
#define USB_HID_USAGE_DESKTOP_VX 0x40
#define USB_HID_USAGE_DESKTOP_VY 0x41
#define USB_HID_USAGE_DESKTOP_VZ 0x42
#define USB_HID_USAGE_DESKTOP_VBRX 0x43
#define USB_HID_USAGE_DESKTOP_VBRY 0x44
#define USB_HID_USAGE_DESKTOP_VBRZ 0x45
#define USB_HID_USAGE_DESKTOP_VNO 0x46
#define USB_HID_USAGE_DESKTOP_FEATURE_NOTIFICATION 0x47
#define USB_HID_USAGE_DESKTOP_RESOLUTION_MULTIPLIER 0x48
#define USB_HID_USAGE_DESKTOP_SYSTEM_CONTROL 0x80
#define USB_HID_USAGE_DESKTOP_SYSTEM_POWER_DOWN 0x81
#define USB_HID_USAGE_DESKTOP_SYSTEM_SLEEP 0x82
#define USB_HID_USAGE_DESKTOP_SYSTEM_WAKE_UP 0x83
#define USB_HID_USAGE_DESKTOP_SYSTEM_CONTEXT_MENU 0x84
#define USB_HID_USAGE_DESKTOP_SYSTEM_MAIN_MENU 0x85
#define USB_HID_USAGE_DESKTOP_SYSTEM_APP_MENU 0x86
#define USB_HID_USAGE_DESKTOP_SYSTEM_MENU_HELP 0x87
#define USB_HID_USAGE_DESKTOP_SYSTEM_MENU_EXIT 0x88
#define USB_HID_USAGE_DESKTOP_SYSTEM_MENU_SELECT 0x89
#define USB_HID_USAGE_DESKTOP_SYSTEM_MENU_RIGHT 0x8a
#define USB_HID_USAGE_DESKTOP_SYSTEM_MENU_LEFT 0x8b
#define USB_HID_USAGE_DESKTOP_SYSTEM_MENU_UP 0x8c
#define USB_HID_USAGE_DESKTOP_SYSTEM_MENU_DOWN 0x8d
#define USB_HID_USAGE_DESKTOP_SYSTEM_COLD_RESTART 0x8e
#define USB_HID_USAGE_DESKTOP_SYSTEM_WARM_RESTART 0x8f
#define USB_HID_USAGE_DESKTOP_DPAD_UP 0x90
#define USB_HID_USAGE_DESKTOP_DPAD_DOWN 0x91
#define USB_HID_USAGE_DESKTOP_DPAD_RIGHT 0x92
#define USB_HID_USAGE_DESKTOP_DPAD_LEFT 0x93
#define USB_HID_USAGE_DESKTOP_SYSTEM_DOCK 0xa0
#define USB_HID_USAGE_DESKTOP_SYSTEM_UNDOCK 0xa1
#define USB_HID_USAGE_DESKTOP_SYSTEM_SETUP 0xa2
#define USB_HID_USAGE_DESKTOP_SYSTEM_BREAK 0xa3
#define USB_HID_USAGE_DESKTOP_SYSTEM_DEBUGGER_BREAK 0xa4
#define USB_HID_USAGE_DESKTOP_APPLICATION_BREAK 0xa5
#define USB_HID_USAGE_DESKTOP_APPLICATION_DEBUGGER_BREAK 0xa6
#define USB_HID_USAGE_DESKTOP_SYSTEM_SPEAKER_MUTE 0xa7
#define USB_HID_USAGE_DESKTOP_SYSTEM_HIBERNATE 0xa8
#define USB_HID_USAGE_DESKTOP_SYSTEM_DISPLAY_INVERT 0xb0
#define USB_HID_USAGE_DESKTOP_SYSTEM_DISPLAY_INTERNAL 0xb1
#define USB_HID_USAGE_DESKTOP_SYSTEM_DISPLAY_EXTERNAL 0xb2
#define USB_HID_USAGE_DESKTOP_SYSTEM_DISPLAY_BOTH 0xb3
#define USB_HID_USAGE_DESKTOP_SYSTEM_DISPLAY_DUAL 0xb4
#define USB_HID_USAGE_DESKTOP_SYSTEM_DISPLAY_TOGGLE_INT_EXT 0xb5
#define USB_HID_USAGE_DESKTOP_SYSTEM_DISPLAY_SWAP_PRIMARY_SECONDARY 0xb6
#define USB_HID_USAGE_DESKTOP_SYSTEM_DISPLAY_LCD_AUTOSCALE 0xb7



#endif
