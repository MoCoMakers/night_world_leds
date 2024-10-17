from ledGrid import LEDGrid

baseline_led_brightness = 127
ip_prefix = "192.168.147."
last_octed_list = [
    '120',
    '121',
    '122',
    '123',
    '124',
    '125',
    '126',
    '127',
    '128',
    '129',
    '130',
    '131',
    '132',
    '133',
    '134',
    '135',
    '136',
    '137',
    '138',
    '139',
    '140',
    '141',
    ]

ip_list = []
for octet in last_octed_list:
    ip_list.append(ip_prefix+octet)

global_grid_set = []


if len(ip_list)>0:
    myGrid0 = LEDGrid(ip=ip_list[0],n_leds=190, current_state_brightness=baseline_led_brightness, rotate_order=0)
    myGrid0.define_row(21,31, direction="forward")
    myGrid0.define_row(37,47, direction="backward")
    myGrid0.define_row(57,67, direction="forward")
    myGrid0.define_row(73,83, direction="backward")
    myGrid0.define_row(92,102, direction="forward")
    myGrid0.define_row(107,117, direction="backward")
    myGrid0.define_row(125,135, direction="forward")
    myGrid0.define_row(140,150, direction="backward")
    myGrid0.define_row(157,167, direction="forward")
    myGrid0.define_row(174,184, direction="backward")
    myGrid0.set_theme("gal")
    myGrid0.set_active("active")
    myGrid0.set_uses_camera(True)
    global_grid_set.append(myGrid0)


if len(ip_list)>1:
    myGrid1 = LEDGrid(ip=ip_list[1],n_leds=190, current_state_brightness=baseline_led_brightness, rotate_order=1)
    myGrid1.define_row(21,31, direction="forward")
    myGrid1.define_row(37,47, direction="backward")
    myGrid1.define_row(57,67, direction="forward")
    myGrid1.define_row(73,83, direction="backward")
    myGrid1.define_row(92,102, direction="forward")
    myGrid1.define_row(107,117, direction="backward")
    myGrid1.define_row(125,135, direction="forward")
    myGrid1.define_row(140,150, direction="backward")
    myGrid1.define_row(157,167, direction="forward")
    myGrid1.define_row(174,184, direction="backward")
    myGrid1.set_theme("gal")
    myGrid1.set_active("active")
    myGrid1.set_uses_camera(True)
    global_grid_set.append(myGrid1)


if len(ip_list)>2:
    myGrid2 = LEDGrid(ip=ip_list[2],n_leds=190, current_state_brightness=baseline_led_brightness, rotate_order=2)
    myGrid2.define_row(21,31, direction="forward")
    myGrid2.define_row(37,47, direction="backward")
    myGrid2.define_row(57,67, direction="forward")
    myGrid2.define_row(73,83, direction="backward")
    myGrid2.define_row(92,102, direction="forward")
    myGrid2.define_row(107,117, direction="backward")
    myGrid2.define_row(125,135, direction="forward")
    myGrid2.define_row(140,150, direction="backward")
    myGrid2.define_row(157,167, direction="forward")
    myGrid2.define_row(174,184, direction="backward")
    myGrid2.set_theme("sub")
    myGrid2.set_active("active")
    myGrid2.set_uses_camera(True)
    global_grid_set.append(myGrid2)


if len(ip_list)>3:
    myGrid3 = LEDGrid(ip=ip_list[3],n_leds=190, current_state_brightness=baseline_led_brightness, rotate_order=3)
    myGrid3.define_row(21,31, direction="forward")
    myGrid3.define_row(37,47, direction="backward")
    myGrid3.define_row(57,67, direction="forward")
    myGrid3.define_row(73,83, direction="backward")
    myGrid3.define_row(92,102, direction="forward")
    myGrid3.define_row(107,117, direction="backward")
    myGrid3.define_row(125,135, direction="forward")
    myGrid3.define_row(140,150, direction="backward")
    myGrid3.define_row(157,167, direction="forward")
    myGrid3.define_row(174,184, direction="backward")
    myGrid3.set_theme("sub")
    myGrid3.set_active("active")
    myGrid3.set_uses_camera(False)
    global_grid_set.append(myGrid3)


if len(ip_list)>4:
    myGrid4 = LEDGrid(ip=ip_list[4],n_leds=190, current_state_brightness=baseline_led_brightness, rotate_order=4)
    myGrid4.define_row(21,31, direction="forward")
    myGrid4.define_row(37,47, direction="backward")
    myGrid4.define_row(57,67, direction="forward")
    myGrid4.define_row(73,83, direction="backward")
    myGrid4.define_row(92,102, direction="forward")
    myGrid4.define_row(107,117, direction="backward")
    myGrid4.define_row(125,135, direction="forward")
    myGrid4.define_row(140,150, direction="backward")
    myGrid4.define_row(157,167, direction="forward")
    myGrid4.define_row(174,184, direction="backward")
    myGrid4.set_theme("life")
    myGrid4.set_active("active")
    myGrid4.set_uses_camera(False)
    global_grid_set.append(myGrid4)


if len(ip_list)>5:
    myGrid5 = LEDGrid(ip=ip_list[5],n_leds=190, current_state_brightness=baseline_led_brightness, rotate_order=5)
    myGrid5.define_row(21,31, direction="forward")
    myGrid5.define_row(37,47, direction="backward")
    myGrid5.define_row(57,67, direction="forward")
    myGrid5.define_row(73,83, direction="backward")
    myGrid5.define_row(92,102, direction="forward")
    myGrid5.define_row(107,117, direction="backward")
    myGrid5.define_row(125,135, direction="forward")
    myGrid5.define_row(140,150, direction="backward")
    myGrid5.define_row(157,167, direction="forward")
    myGrid5.define_row(174,184, direction="backward")
    myGrid5.set_theme("life")
    myGrid5.set_active("active")
    myGrid5.set_uses_camera(False)
    global_grid_set.append(myGrid5)


if len(ip_list)>6:
    myGrid6 = LEDGrid(ip=ip_list[6],n_leds=190, current_state_brightness=baseline_led_brightness, rotate_order=6)
    myGrid6.define_row(21,31, direction="forward")
    myGrid6.define_row(37,47, direction="backward")
    myGrid6.define_row(57,67, direction="forward")
    myGrid6.define_row(73,83, direction="backward")
    myGrid6.define_row(92,102, direction="forward")
    myGrid6.define_row(107,117, direction="backward")
    myGrid6.define_row(125,135, direction="forward")
    myGrid6.define_row(140,150, direction="backward")
    myGrid6.define_row(157,167, direction="forward")
    myGrid6.define_row(174,184, direction="backward")
    myGrid6.set_theme("life")
    myGrid6.set_active("active")
    myGrid6.set_uses_camera(False)
    global_grid_set.append(myGrid6)


if len(ip_list)>7:
    myGrid7 = LEDGrid(ip=ip_list[7],n_leds=190, current_state_brightness=baseline_led_brightness, rotate_order=7)
    myGrid7.define_row(21,31, direction="forward")
    myGrid7.define_row(37,47, direction="backward")
    myGrid7.define_row(57,67, direction="forward")
    myGrid7.define_row(73,83, direction="backward")
    myGrid7.define_row(92,102, direction="forward")
    myGrid7.define_row(107,117, direction="backward")
    myGrid7.define_row(125,135, direction="forward")
    myGrid7.define_row(140,150, direction="backward")
    myGrid7.define_row(157,167, direction="forward")
    myGrid7.define_row(174,184, direction="backward")
    myGrid7.set_theme("life")
    myGrid7.set_active("active")
    myGrid7.set_uses_camera(False)
    global_grid_set.append(myGrid7)


if len(ip_list)>8:
    myGrid8 = LEDGrid(ip=ip_list[8],n_leds=190, current_state_brightness=baseline_led_brightness, rotate_order=8)
    myGrid8.define_row(21,31, direction="forward")
    myGrid8.define_row(37,47, direction="backward")
    myGrid8.define_row(57,67, direction="forward")
    myGrid8.define_row(73,83, direction="backward")
    myGrid8.define_row(92,102, direction="forward")
    myGrid8.define_row(107,117, direction="backward")
    myGrid8.define_row(125,135, direction="forward")
    myGrid8.define_row(140,150, direction="backward")
    myGrid8.define_row(157,167, direction="forward")
    myGrid8.define_row(174,184, direction="backward")
    myGrid8.set_theme("gal")
    myGrid8.set_active("active")
    myGrid8.set_uses_camera(False)
    global_grid_set.append(myGrid8)


if len(ip_list)>9:
    myGrid9 = LEDGrid(ip=ip_list[9],n_leds=190, current_state_brightness=baseline_led_brightness, rotate_order=9)
    myGrid9.define_row(21,31, direction="forward")
    myGrid9.define_row(37,47, direction="backward")
    myGrid9.define_row(57,67, direction="forward")
    myGrid9.define_row(73,83, direction="backward")
    myGrid9.define_row(92,102, direction="forward")
    myGrid9.define_row(107,117, direction="backward")
    myGrid9.define_row(125,135, direction="forward")
    myGrid9.define_row(140,150, direction="backward")
    myGrid9.define_row(157,167, direction="forward")
    myGrid9.define_row(174,184, direction="backward")
    myGrid9.set_theme("gal")
    myGrid9.set_active("active")
    myGrid9.set_uses_camera(False)
    global_grid_set.append(myGrid9)

if len(ip_list)>10:
    myGrid10 = LEDGrid(ip=ip_list[10],n_leds=190, current_state_brightness=baseline_led_brightness, rotate_order=10)
    myGrid10.define_row(21,31, direction="forward")
    myGrid10.define_row(37,47, direction="backward")
    myGrid10.define_row(57,67, direction="forward")
    myGrid10.define_row(73,83, direction="backward")
    myGrid10.define_row(92,102, direction="forward")
    myGrid10.define_row(107,117, direction="backward")
    myGrid10.define_row(125,135, direction="forward")
    myGrid10.define_row(140,150, direction="backward")
    myGrid10.define_row(157,167, direction="forward")
    myGrid10.define_row(174,184, direction="backward")
    myGrid10.set_theme("gal")
    myGrid10.set_active("active")
    myGrid10.set_uses_camera(False)
    global_grid_set.append(myGrid10)



if len(ip_list)>11:
    myGrid11 = LEDGrid(ip=ip_list[11],n_leds=190, current_state_brightness=baseline_led_brightness, rotate_order=11)
    myGrid11.define_row(21,31, direction="forward")
    myGrid11.define_row(37,47, direction="backward")
    myGrid11.define_row(57,67, direction="forward")
    myGrid11.define_row(73,83, direction="backward")
    myGrid11.define_row(92,102, direction="forward")
    myGrid11.define_row(107,117, direction="backward")
    myGrid11.define_row(125,135, direction="forward")
    myGrid11.define_row(140,150, direction="backward")
    myGrid11.define_row(157,167, direction="forward")
    myGrid11.define_row(174,184, direction="backward")
    myGrid11.set_theme("gal")
    myGrid11.set_active("active")
    myGrid11.set_uses_camera(False)
    global_grid_set.append(myGrid11)


if len(ip_list)>12:
    myGrid12 = LEDGrid(ip=ip_list[12],n_leds=190, current_state_brightness=baseline_led_brightness, rotate_order=12)
    myGrid12.define_row(21,31, direction="forward")
    myGrid12.define_row(37,47, direction="backward")
    myGrid12.define_row(57,67, direction="forward")
    myGrid12.define_row(73,83, direction="backward")
    myGrid12.define_row(92,102, direction="forward")
    myGrid12.define_row(107,117, direction="backward")
    myGrid12.define_row(125,135, direction="forward")
    myGrid12.define_row(140,150, direction="backward")
    myGrid12.define_row(157,167, direction="forward")
    myGrid12.define_row(174,184, direction="backward")
    myGrid12.set_theme("terr")
    myGrid12.set_active("active")
    myGrid12.set_uses_camera(False)
    global_grid_set.append(myGrid12)


if len(ip_list)>13:
    myGrid13 = LEDGrid(ip=ip_list[13],n_leds=190, current_state_brightness=baseline_led_brightness, rotate_order=13)
    myGrid13.define_row(21,31, direction="forward")
    myGrid13.define_row(37,47, direction="backward")
    myGrid13.define_row(57,67, direction="forward")
    myGrid13.define_row(73,83, direction="backward")
    myGrid13.define_row(92,102, direction="forward")
    myGrid13.define_row(107,117, direction="backward")
    myGrid13.define_row(125,135, direction="forward")
    myGrid13.define_row(140,150, direction="backward")
    myGrid13.define_row(157,167, direction="forward")
    myGrid13.define_row(174,184, direction="backward")
    myGrid13.set_theme("terr")
    myGrid13.set_active("active")
    myGrid13.set_uses_camera(False)
    global_grid_set.append(myGrid13)


if len(ip_list)>14:
    myGrid14 = LEDGrid(ip=ip_list[14],n_leds=190, current_state_brightness=baseline_led_brightness, rotate_order=14)
    myGrid14.define_row(21,31, direction="forward")
    myGrid14.define_row(37,47, direction="backward")
    myGrid14.define_row(57,67, direction="forward")
    myGrid14.define_row(73,83, direction="backward")
    myGrid14.define_row(92,102, direction="forward")
    myGrid14.define_row(107,117, direction="backward")
    myGrid14.define_row(125,135, direction="forward")
    myGrid14.define_row(140,150, direction="backward")
    myGrid14.define_row(157,167, direction="forward")
    myGrid14.define_row(174,184, direction="backward")
    myGrid14.set_theme("terr")
    myGrid14.set_active("active")
    myGrid14.set_uses_camera(False)
    global_grid_set.append(myGrid14)


if len(ip_list)>15:
    myGrid15 = LEDGrid(ip=ip_list[15],n_leds=190, current_state_brightness=baseline_led_brightness, rotate_order=15)
    myGrid15.define_row(21,31, direction="forward")
    myGrid15.define_row(37,47, direction="backward")
    myGrid15.define_row(57,67, direction="forward")
    myGrid15.define_row(73,83, direction="backward")
    myGrid15.define_row(92,102, direction="forward")
    myGrid15.define_row(107,117, direction="backward")
    myGrid15.define_row(125,135, direction="forward")
    myGrid15.define_row(140,150, direction="backward")
    myGrid15.define_row(157,167, direction="forward")
    myGrid15.define_row(174,184, direction="backward")
    myGrid15.set_theme("led")
    myGrid15.set_active("active")
    myGrid15.set_uses_camera(False)
    global_grid_set.append(myGrid15)

if len(ip_list)>16:
    myGrid16 = LEDGrid(ip=ip_list[16],n_leds=190, current_state_brightness=baseline_led_brightness, rotate_order=16)
    myGrid16.define_row(21,31, direction="forward")
    myGrid16.define_row(37,47, direction="backward")
    myGrid16.define_row(57,67, direction="forward")
    myGrid16.define_row(73,83, direction="backward")
    myGrid16.define_row(92,102, direction="forward")
    myGrid16.define_row(107,117, direction="backward")
    myGrid16.define_row(125,135, direction="forward")
    myGrid16.define_row(140,150, direction="backward")
    myGrid16.define_row(157,167, direction="forward")
    myGrid16.define_row(174,184, direction="backward")
    myGrid16.set_theme("led")
    myGrid16.set_active("active")
    myGrid16.set_uses_camera(False)
    global_grid_set.append(myGrid16)


if len(ip_list)>17:
    myGrid17 = LEDGrid(ip=ip_list[17],n_leds=190, current_state_brightness=baseline_led_brightness, rotate_order=17)
    myGrid17.define_row(21,31, direction="forward")
    myGrid17.define_row(37,47, direction="backward")
    myGrid17.define_row(57,67, direction="forward")
    myGrid17.define_row(73,83, direction="backward")
    myGrid17.define_row(92,102, direction="forward")
    myGrid17.define_row(107,117, direction="backward")
    myGrid17.define_row(125,135, direction="forward")
    myGrid17.define_row(140,150, direction="backward")
    myGrid17.define_row(157,167, direction="forward")
    myGrid17.define_row(174,184, direction="backward")
    myGrid17.set_theme("led")
    myGrid17.set_active("active")
    myGrid17.set_uses_camera(False)
    global_grid_set.append(myGrid17)


if len(ip_list)>18:
    myGrid18 = LEDGrid(ip=ip_list[18],n_leds=190, current_state_brightness=baseline_led_brightness, rotate_order=18)
    myGrid18.define_row(21,31, direction="forward")
    myGrid18.define_row(37,47, direction="backward")
    myGrid18.define_row(57,67, direction="forward")
    myGrid18.define_row(73,83, direction="backward")
    myGrid18.define_row(92,102, direction="forward")
    myGrid18.define_row(107,117, direction="backward")
    myGrid18.define_row(125,135, direction="forward")
    myGrid18.define_row(140,150, direction="backward")
    myGrid18.define_row(157,167, direction="forward")
    myGrid18.define_row(174,184, direction="backward")
    myGrid18.set_theme("led")
    myGrid18.set_active("active")
    myGrid18.set_uses_camera(False)
    global_grid_set.append(myGrid18)

if len(ip_list)>19:
    myGrid19 = LEDGrid(ip=ip_list[19],n_leds=190, current_state_brightness=baseline_led_brightness, rotate_order=19)
    myGrid19.define_row(21,31, direction="forward")
    myGrid19.define_row(37,47, direction="backward")
    myGrid19.define_row(57,67, direction="forward")
    myGrid19.define_row(73,83, direction="backward")
    myGrid19.define_row(92,102, direction="forward")
    myGrid19.define_row(107,117, direction="backward")
    myGrid19.define_row(125,135, direction="forward")
    myGrid19.define_row(140,150, direction="backward")
    myGrid19.define_row(157,167, direction="forward")
    myGrid19.define_row(174,184, direction="backward")
    myGrid19.set_theme("led")
    myGrid19.set_active("active")
    myGrid19.set_uses_camera(False)
    global_grid_set.append(myGrid19)



