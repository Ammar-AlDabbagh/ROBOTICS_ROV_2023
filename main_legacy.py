import serial
import pygame
import pygame.joystick as js
import math
from time import sleep

pygame.init()

WIDTH = 700
HEIGHT = 1050

INTVALS = 6
BOOLVALS = 18

# Class for printing text on the screen


class TextPrint:
    def __init__(self):
        self.reset()
        self.font = pygame.font.Font(None, 25)

    def tprint(self, screen, text):
        text_bitmap = self.font.render(text, True, (0, 0, 0))
        screen.blit(text_bitmap, (self.x, self.y))
        self.y += self.line_height

        # debug
        # print(text)

    def newline(self):
        self.y += self.line_height

    def reset(self):
        self.x = 10
        self.y = 10
        self.line_height = 20
        self.tab = 10

    def indent(self):
        self.x += self.tab

    def unindent(self):
        self.x -= self.tab

# Base class for joystick controllers


class Controller:
    def __init__(self, controller, printer=None, applyOuterDeadZone=True, verbose=True):

        self.verbose = verbose
        self.controller = controller
        self.applyOuterDeadZone = applyOuterDeadZone
        self.printer = printer
        self.model_num = -1  # generic
        self.serial_cache = []  # dummy val

        if self.verbose:
            self.get_meta()

    def get_meta(self):
        self.jid = self.controller.get_instance_id()
        self.name = self.controller.get_name()
        self.guid = self.controller.get_guid()
        self.power_level = self.controller.get_power_level()
        self.numaxes = self.controller.get_numaxes()
        self.numbuttons = self.controller.get_numbuttons()
        self.numhats = self.controller.get_numhats()

    def print_meta(self, screen, printer=None):
        printer = self.printer if not printer else printer

        printer.tprint(screen, f"Joystick: {self.jid}")
        printer.indent()
        printer.tprint(screen, f"Joystick name: {self.name}")
        printer.tprint(screen, f"GUID: {self.guid}")
        printer.tprint(screen, f"Joystick's power level: {self.power_level}")
        printer.indent()

    def get_data(self):
        self.axes = {i: int(100 * round(self.controller.get_axis(i),   2)) for i in range(self.numaxes)}
        self.buttons = {i: int(round(self.controller.get_button(i), 2)) for i in range(self.numbuttons)}
        self.hats = {i: int(100 * round(self.controller.get_hat(i),    2)) for i in range(self.numhats)}

    def print_data(self, screen, printer=None):
        self.get_data()
        printer = self.printer if not printer else printer

        data_map = {
            'axes': self.axes,
            'buttons': self.buttons,
            'hats': self.hats
        }

        for dataType, data in data_map.items():
            printer.tprint(screen, f'Number of {dataType}: {len(data)}')
            printer.indent()
            for key, val in data.items():
                printer.tprint(screen, f'{key}: {val}')
            printer.newline()
            printer.unindent()

    def send_serial(self, com='COM5'):
        if not self.serial_init:
            self.ser = serial.Serial(com, 74880, timeout=1)
            self.serial_init = True
            sleep(3)

        intVals = list(self.axes.values()) + [0] * (INTVALS - len(self.axes.values()))
        boolVals = list(self.buttons.values()) + [0] * (INTVALS - len(self.buttons.values()))

        data = str([self.model_num] + intVals + boolVals)
        if data != self.serial_cache:
            self.ser.write(data.encode())
            self.serial_cache = data
            if (output := self.ser.read_all().decode()):
                print(output)

    def debug(self):

        intVals = list(self.axes.values()) + [0] * (INTVALS - len(self.axes.values()))
        boolVals = list(self.buttons.values()) + [0] * (INTVALS - len(self.buttons.values()))

        data = str([self.model_num] + intVals + boolVals)
        if data != self.serial_cache:
            self.serial_cache = data
            print(data)


# class keyboard

class Keyboard:
    def __init__(self, printer=None,  verbose=True):
        self.verbose = verbose
        self.printer = printer
        self.model_num = -1  # generic
        self.serial_cache = []  # dummy val

        if self.verbose:
            self.get_meta()

    def print_meta(self, screen, printer=None):
        printer = self.printer if not printer else printer

        printer.tprint(screen, f"Keyboard")
        printer.indent()

    def get_data(self):
        pressed_keys = pygame.key.get_pressed()


# Controller subclass for Nintendo Switch Pro Controller


class ProController(Controller):
    def __init__(self, controller, printer=None, applyOuterDeadZone=True):
        super().__init__(controller, printer, applyOuterDeadZone)
        self.numbuttons = 16
        self.model_num = 0  # switch pro controller
        self.serial_init = False

    def get_data(self):
        super().get_data()

        buttonsMap = ['A', 'B', 'X', 'Y', 'Minus', 'Home', 'Plus', 'L_Stick', 'R_Stick', 'L', 'R', 'UP', 'DOWN', 'LEFT', 'RIGHT', 'Capture']

        self.buttons = {buttonsMap[i]: v for i, v in enumerate(self.buttons.values())}

        self.buttons['ZL'] = int((self.axes.pop(4)) > 0)  # type: ignore
        self.buttons['ZR'] = int((self.axes.pop(5)) > 0)  # type: ignore

        self.buttons = {el: self.buttons[el] for el in [
            'A', 'B', 'X', 'Y',
            'UP', 'DOWN', 'LEFT', 'RIGHT',
            'L', 'R', 'ZL', 'ZR', 'Plus',
            'Minus', 'Home', 'Capture', 'L_Stick', 'R_Stick'
        ]}

        axesMap = ['L_V', 'L_H', 'R_V', 'R_H']
        self.axes = {axesMap[i]: v for i, v in enumerate(self.axes.values())}
        for key, val in self.axes.items():

            if val in range(-10, 10):
                val = 0
            if not self.applyOuterDeadZone:

                mltp = 1.4  # 1 + (math.sin(math.pi/4) / 2) (mathimatically correct number, doesn't work because the controller is not perfect)
                if val > 0:
                    val = 100 if (output := val*mltp) > 100 else round(output)
                else:
                    val = -100 if (output := val*mltp) < -100 else round(output)

            val = 5 * round(val/5)  # clip val to nearest multiple of 5
            self.axes[key] = val

        self.axes = {el: self.axes[el] for el in ['L_H', 'L_V', 'R_H', 'R_V']}
        self.axes['L_H'] *= -1
        self.axes['R_H'] *= -1


class PS5Controller(Controller):
    def __init__(self, controller, printer=None, applyOuterDeadZone=True):
        super().__init__(controller, printer, applyOuterDeadZone)
        self.numbuttons = 16
        self.model_num = 1  # ps5 controller
        self.serial_init = False

    def get_data(self):
        super().get_data()

        buttonsMap = ['B', 'A', 'Y', 'X', 'Minus', 'Home', 'Plus', 'L_Stick', 'R_Stick', 'L', 'R', 'UP', 'DOWN', 'LEFT', 'RIGHT', 'Capture']

        self.buttons = {buttonsMap[i]: v for i, v in enumerate(self.buttons.values())}

        self.buttons['ZL'] = int((self.axes[4]) > 0)  # type: ignore
        self.buttons['ZR'] = int((self.axes[5]) > 0)  # type: ignore

        self.buttons = {el: self.buttons[el] for el in [
            'A', 'B', 'X', 'Y',
            'UP', 'DOWN', 'LEFT', 'RIGHT',
            'L', 'R', 'ZL', 'ZR', 'Plus',
            'Minus', 'Home', 'Capture', 'L_Stick', 'R_Stick'
        ]}

        axesMap = ['L_V', 'L_H', 'R_V', 'R_H', 'ZL', 'ZR']
        self.axes = {axesMap[i]: v for i, v in enumerate(self.axes.values())}
        for key, val in self.axes.items():
            if "Z" not in key:
                if val in range(-10, 10):
                    val = 0
                if self.applyOuterDeadZone:

                    mltp = 1.375  # ps5 better val
                    if val > 0:
                        val = 100 if (output := val*mltp) > 100 else round(output)
                    else:
                        val = -100 if (output := val*mltp) < -100 else round(output)

                    val = 2 * round(val/2)  # clip val to nearest multiple of 5
                    self.axes[key] = val

        self.axes = {el: self.axes[el] for el in ['L_H', 'L_V', 'R_H', 'R_V', 'ZL', 'ZR']}
        self.axes['L_H'] *= -1
        self.axes['R_H'] *= -1


def print_numjoys(screen, printer=None):
    printer = TextPrint() if not printer else printer
    printer.tprint(screen, f"Number of joysticks: {js.get_count()}")
    printer.newline()
    printer.indent()

# Main function for printing joystick information


def main_print(com, sendserial):
    screen = pygame.display.set_mode((WIDTH, HEIGHT))
    screen.fill((255, 255, 255))
    pygame.display.set_caption("Joystick example")

    clock = pygame.time.Clock()
    text_print = TextPrint()

    # This dict can be left as-is, since pygame will generate a
    # pygame.JOYDEVICEADDED event for every joystick connected
    # at the start of the program.
    joysticks = {}
    print_numjoys(screen, text_print)
    meta_printedx, meta_printedy = text_print.x, text_print.y

    done = False

    while not done:

        text_print.reset()
        text_print.x += meta_printedx
        text_print.y += meta_printedy
        screen.fill((255, 255, 255), [meta_printedx] + [meta_printedy] + [WIDTH, HEIGHT])

        for event in pygame.event.get():
            if event.type == pygame.QUIT:
                done = True

            # Handle hotplugging
            if event.type == pygame.JOYDEVICEADDED:

                text_print.reset()
                screen.fill((255, 255, 255))
                print_numjoys(screen, text_print)

                joy = js.Joystick(event.device_index).get_name()
                match joy:
                    case 'Nintendo Switch Pro Controller':
                        joy = ProController(js.Joystick(event.device_index), text_print)
                    case 'PS5 Controller':
                        joy = PS5Controller(js.Joystick(event.device_index), text_print)
                    case _:
                        joy = Controller(js.Joystick(event.device_index), text_print)

                joysticks[joy.jid] = joy
                joy.print_meta(screen)
                meta_printedx, meta_printedy = text_print.x, text_print.y
                print(f"Joystick {joy.jid} connected")

            if event.type == pygame.JOYDEVICEREMOVED:
                screen.fill((255, 255, 255))
                text_print.reset()
                print_numjoys(screen, text_print)
                del joysticks[event.instance_id]
                meta_printedx, meta_printedy = text_print.x, text_print.y
                print(f"Joystick {event.instance_id} disconnected")

        for joystick in joysticks.values():
            joystick.print_data(screen)
            if sendserial:
                joystick.send_serial(com)
            joystick.debug()
        # Go ahead and update the screen with what we've drawn.
        pygame.display.update()


def main(com='COM5', sendserial=True, verbose=True):
    if verbose:
        main_print(com, sendserial)
        return

    clock = pygame.time.Clock()
    joysticks = {}

    done = False

    while not done:
        for event in pygame.event.get():
            if event.type == pygame.QUIT:
                done = True

            # Handle hotplugging
            if event.type == pygame.JOYDEVICEADDED:

                joy = js.Joystick(event.device_index).get_name()
                match joy:
                    case 'Nintendo Switch Pro Controller':
                        joy = ProController(js.Joystick(event.device_index))
                    case 'PS5 Controller':
                        joy = PS5Controller(js.Joystick(event.device_index))
                    case _:
                        joy = Controller(js.Joystick(event.device_index))

                joysticks[joy.jid] = joy
                print(f"Joystick {joy.jid} connected")

            if event.type == pygame.JOYDEVICEREMOVED:
                del joysticks[event.instance_id]
                print(f"Joystick {event.instance_id} disconnected")

        for joystick in joysticks.values():
            joystick.get_data()
            if sendserial:
                joystick.send_serial(com)
            # joystick.debug()
if __name__ == "__main__":
    main(com='COM8', verbose=1, sendserial=0)
    pygame.quit()
