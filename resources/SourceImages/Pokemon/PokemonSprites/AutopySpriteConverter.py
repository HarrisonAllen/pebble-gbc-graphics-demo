from autopy import mouse
from time import sleep
import keyboard
import sys

choices = {
    '1': 1,
    '2': 2,
    '3': 3,
    '4': 4,
    '5': 5,
    '6': 6,
}

def main(index):
    mouse.location()
    root_x, root_y = mouse.location()
    for i in range(index, 252):
        mouse.move(root_x, root_y)
        mouse.click()
        sleep(0.7)
        keyboard.write(f'{i:03d}.png')
        sleep(0.7)
        keyboard.press_and_release('enter')
        sleep(0.5)
        keyboard.press_and_release('tab')
        keyboard.press_and_release('enter')
            
        times_to_press = 0
        while True:
            key_press = keyboard.read_key()
            if key_press == 'esc':
                return
            if key_press in choices:
                times_to_press = choices[key_press]
                break
        for _ in range(times_to_press):
            keyboard.press_and_release('tab')
        keyboard.press_and_release('enter')
        sleep(2)
        keyboard.press_and_release('down')
        sleep(0.3)
        keyboard.press_and_release('enter')
        sleep(0.7)
        keyboard.write(f'{i:03d}.bin')
        sleep(0.7)
        keyboard.press_and_release('enter')
        sleep(1)

        



if __name__ == "__main__":
    num_args = len(sys.argv)
    if num_args > 1:
        start = int(sys.argv[1])
    else:
        start = 0

    main(start)
    # sleep(2)
    # keyboard.press_and_release('down')

    # Move 39.2, 256.8
    # click
    # sleep 0.5
    # type "000.png"
    # press enter
    # sleep 0.5
    # Move 36.8, 280.8
    # click
    # sleep 0.5
    # wait until text in equals "Success!" (or just wait 1-2 seconds)
    # Get list of sizes by scanning the 6 text boxes
    # find the smallest
    # move the mouse to that button 
    # click
    # sleep 0.5
    # press "down" on the keyboard
    # press enter
    # sleep 0.5
    # type "000.bin"
    # press enter
    # sleep 0.5
    # repeat