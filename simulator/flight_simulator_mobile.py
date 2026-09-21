import pygame
import socket
import math
import time

# ============================================================
# OPENFLIGHT - MOBILE FLIGHT SIMULATOR
# Android / Pydroid 3 + Pygame
# ============================================================

pygame.init()

# ------------------------------------------------------------
# SCREEN
# ------------------------------------------------------------
info = pygame.display.Info()

SCREEN_W = info.current_w
SCREEN_H = info.current_h

# Prevent extremely large resolution
if SCREEN_W > 1000:
    SCREEN_W = 1000
if SCREEN_H > 800:
    SCREEN_H = 800

screen = pygame.display.set_mode((SCREEN_W, SCREEN_H))
pygame.display.set_caption("OpenFlight Mobile")

clock = pygame.time.Clock()

# ------------------------------------------------------------
# ESP32
# ------------------------------------------------------------
ESP32_IP = "192.168.145.178"
ESP32_PORT = 5000

esp32_socket = None
esp32_connected = False

# ------------------------------------------------------------
# COLORS
# ------------------------------------------------------------
BLACK = (5, 8, 12)
WHITE = (235, 235, 235)
GRAY = (80, 90, 100)
LIGHT_GRAY = (150, 160, 170)
GREEN = (40, 220, 100)
RED = (240, 60, 60)
YELLOW = (240, 210, 50)
BLUE = (50, 150, 240)
DARK_BLUE = (20, 50, 90)
ORANGE = (240, 150, 40)

# ------------------------------------------------------------
# FONTS
# ------------------------------------------------------------
font_small = pygame.font.Font(None, 24)
font_medium = pygame.font.Font(None, 30)
font_large = pygame.font.Font(None, 40)
font_huge = pygame.font.Font(None, 55)

# ------------------------------------------------------------
# FLIGHT VARIABLES
# ------------------------------------------------------------
altitude = 5000.0
airspeed = 120.0
heading = 0.0
vertical_speed = 0.0

pitch = 0.0
roll = 0.0

throttle = 50.0

gear_down = True
flaps = 0.0
engine_on = False

paused = False

# ------------------------------------------------------------
# BUTTON CLASS
# ------------------------------------------------------------
class Button:
    def __init__(self, x, y, w, h, text,
                 color=(35, 45, 55),
                 active_color=(55, 75, 90)):

        self.rect = pygame.Rect(x, y, w, h)
        self.text = text
        self.color = color
        self.active_color = active_color
        self.pressed = False

    def draw(self):
        color = self.active_color if self.pressed else self.color

        pygame.draw.rect(
            screen,
            color,
            self.rect,
            border_radius=10
        )

        pygame.draw.rect(
            screen,
            LIGHT_GRAY,
            self.rect,
            2,
            border_radius=10
        )

        text_surface = font_medium.render(
            self.text,
            True,
            WHITE
        )

        text_rect = text_surface.get_rect(
            center=self.rect.center
        )

        screen.blit(text_surface, text_rect)

    def check_press(self, pos):
        return self.rect.collidepoint(pos)


# ------------------------------------------------------------
# BUTTON LAYOUT
# ------------------------------------------------------------

margin = 15

button_w = 110
button_h = 55

bottom_y = SCREEN_H - 70

# Left/right roll
roll_left = Button(
    margin,
    bottom_y,
    button_w,
    button_h,
    "ROLL <"
)

roll_right = Button(
    margin + button_w + 10,
    bottom_y,
    button_w,
    button_h,
    "ROLL >"
)

# Pitch
pitch_down = Button(
    SCREEN_W - (button_w * 2 + 25),
    bottom_y,
    button_w,
    button_h,
    "PITCH ↓"
)

pitch_up = Button(
    SCREEN_W - button_w - margin,
    bottom_y,
    button_w,
    button_h,
    "PITCH ↑"
)

# ------------------------------------------------------------
# SECONDARY BUTTONS
# ------------------------------------------------------------

small_w = 90
small_h = 45

control_y = SCREEN_H - 130

throttle_minus = Button(
    15,
    control_y,
    small_w,
    small_h,
    "THR -"
)

throttle_plus = Button(
    115,
    control_y,
    small_w,
    small_h,
    "THR +"
)

gear_button = Button(
    215,
    control_y,
    small_w,
    small_h,
    "GEAR"
)

flaps_minus = Button(
    SCREEN_W - 4 * small_w - 45,
    control_y,
    small_w,
    small_h,
    "FLAP -"
)

flaps_plus = Button(
    SCREEN_W - 3 * small_w - 35,
    control_y,
    small_w,
    small_h,
    "FLAP +"
)

engine_button = Button(
    SCREEN_W - 2 * small_w - 25,
    control_y,
    small_w,
    small_h,
    "ENGINE"
)

reset_button = Button(
    SCREEN_W - small_w - 15,
    control_y,
    small_w,
    small_h,
    "RESET"
)

# ------------------------------------------------------------
# ESP32 CONNECTION
# ------------------------------------------------------------
def connect_to_esp32():

    global esp32_socket
    global esp32_connected

    try:

        if esp32_socket:
            esp32_socket.close()

        esp32_socket = socket.socket(
            socket.AF_INET,
            socket.SOCK_STREAM
        )

        esp32_socket.settimeout(2)

        esp32_socket.connect(
            (ESP32_IP, ESP32_PORT)
        )

        esp32_socket.settimeout(None)

        esp32_connected = True

        print("ESP32 Connected!")

    except Exception as e:

        esp32_connected = False

        print("ESP32 connection failed:", e)


# ------------------------------------------------------------
# SEND TELEMETRY
# ------------------------------------------------------------
def send_telemetry():

    global esp32_socket
    global esp32_connected

    if not esp32_connected:
        return

    try:

        message = (
            f"ALT={altitude:.0f},"
            f"SPD={airspeed:.0f},"
            f"HDG={heading:.0f},"
            f"VS={vertical_speed:.0f},"
            f"PITCH={pitch:.1f},"
            f"ROLL={roll:.1f},"
            f"THR={throttle:.0f},"
            f"GEAR={'DOWN' if gear_down else 'UP'},"
            f"FLAPS={flaps:.0f},"
            f"ENGINE={'ON' if engine_on else 'OFF'}\n"
        )

        esp32_socket.sendall(
            message.encode()
        )

    except Exception as e:

        print("Send error:", e)

        esp32_connected = False

        try:
            esp32_socket.close()
        except:
            pass


# ------------------------------------------------------------
# RESET
# ------------------------------------------------------------
def reset_flight():

    global altitude
    global airspeed
    global heading
    global vertical_speed
    global pitch
    global roll
    global throttle
    global gear_down
    global flaps
    global engine_on

    altitude = 5000.0
    airspeed = 120.0
    heading = 0.0
    vertical_speed = 0.0
    pitch = 0.0
    roll = 0.0
    throttle = 50.0
    gear_down = True
    flaps = 0.0
    engine_on = False


# ------------------------------------------------------------
# HORIZON
# ------------------------------------------------------------
def draw_horizon():

    center_x = SCREEN_W // 2
    center_y = 180

    horizon_width = min(500, SCREEN_W - 40)
    horizon_height = 250

    horizon_rect = pygame.Rect(
        center_x - horizon_width // 2,
        45,
        horizon_width,
        horizon_height
    )

    pygame.draw.rect(
        screen,
        (30, 100, 180),
        horizon_rect
    )

    # Ground
    ground_y = int(
        center_y + pitch * 5
    )

    ground_y = max(
        horizon_rect.top,
        min(horizon_rect.bottom, ground_y)
    )

    pygame.draw.rect(
        screen,
        (130, 80, 40),
        (
            horizon_rect.left,
            ground_y,
            horizon_rect.width,
            horizon_rect.bottom - ground_y
        )
    )

    # Horizon line
    pygame.draw.line(
        screen,
        WHITE,
        (horizon_rect.left, ground_y),
        (horizon_rect.right, ground_y),
        3
    )

    # Center aircraft
    pygame.draw.line(
        screen,
        YELLOW,
        (center_x - 45, center_y),
        (center_x - 10, center_y),
        4
    )

    pygame.draw.line(
        screen,
        YELLOW,
        (center_x + 10, center_y),
        (center_x + 45, center_y),
        4
    )

    pygame.draw.line(
        screen,
        YELLOW,
        (center_x, center_y - 10),
        (center_x, center_y + 10),
        3
    )

    pygame.draw.rect(
        screen,
        WHITE,
        horizon_rect,
        2
    )


# ------------------------------------------------------------
# FLIGHT DATA
# ------------------------------------------------------------
def draw_flight_data():

    x = 15
    y = 15

    values = [
        ("ALT", f"{altitude:.0f} ft"),
        ("SPD", f"{airspeed:.0f} kt"),
        ("HDG", f"{heading:03.0f}°"),
        ("VS", f"{vertical_speed:+.0f}"),
        ("PITCH", f"{pitch:+.1f}°"),
        ("ROLL", f"{roll:+.1f}°"),
    ]

    for label, value in values:

        text = font_small.render(
            f"{label}: {value}",
            True,
            WHITE
        )

        screen.blit(
            text,
            (x, y)
        )

        y += 24


# ------------------------------------------------------------
# STATUS
# ------------------------------------------------------------
def draw_status():

    connection_color = (
        GREEN if esp32_connected else RED
    )

    status = (
        "ESP32 CONNECTED"
        if esp32_connected
        else "ESP32 DISCONNECTED"
    )

    text = font_small.render(
        status,
        True,
        connection_color
    )

    screen.blit(
        text,
        (
            SCREEN_W - text.get_width() - 15,
            15
        )
    )

    engine_text = (
        "ENGINE ON"
        if engine_on
        else "ENGINE OFF"
    )

    engine_color = (
        GREEN if engine_on else RED
    )

    text = font_small.render(
        engine_text,
        True,
        engine_color
    )

    screen.blit(
        text,
        (
            SCREEN_W - text.get_width() - 15,
            40
        )
    )


# ------------------------------------------------------------
# BOTTOM INFORMATION
# ------------------------------------------------------------
def draw_info():

    text = font_small.render(
        f"THR {throttle:.0f}%   "
        f"GEAR {'DOWN' if gear_down else 'UP'}   "
        f"FLAPS {flaps:.0f}°",
        True,
        WHITE
    )

    screen.blit(
        text,
        (
            SCREEN_W // 2 - text.get_width() // 2,
            305
        )
    )


# ------------------------------------------------------------
# PHYSICS
# ------------------------------------------------------------
def update_physics(dt):

    global pitch
    global roll
    global heading
    global throttle
    global altitude
    global airspeed
    global vertical_speed
    global flaps

    if not engine_on:
        throttle -= 10 * dt

    throttle = max(
        0,
        min(100, throttle)
    )

    # Speed
    target_speed = (
        40 + throttle * 2.0
    )

    if engine_on:
        airspeed += (
            target_speed - airspeed
        ) * 0.8 * dt
    else:
        airspeed += (
            30 - airspeed
        ) * 0.3 * dt

    airspeed = max(
        0,
        airspeed
    )

    # Pitch effect
    vertical_speed = pitch * 120

    altitude += (
        vertical_speed * dt
    )

    altitude = max(
        0,
        altitude
    )

    # Roll → heading
    heading += (
        roll * 0.12 * dt
    )

    heading %= 360

    # Natural return
    pitch *= (
        1 - 0.25 * dt
    )

    roll *= (
        1 - 0.35 * dt
    )

    # Flaps limits
    flaps = max(
        0,
        min(30, flaps)
    )


# ------------------------------------------------------------
# TOUCH / MOUSE
# ------------------------------------------------------------
def handle_press(pos):

    global throttle
    global gear_down
    global flaps
    global engine_on

    if roll_left.check_press(pos):
        roll_left.pressed = True

    if roll_right.check_press(pos):
        roll_right.pressed = True

    if pitch_down.check_press(pos):
        pitch_down.pressed = True

    if pitch_up.check_press(pos):
        pitch_up.pressed = True

    if throttle_minus.check_press(pos):
        throttle = max(
            0,
            throttle - 5
        )

    if throttle_plus.check_press(pos):
        throttle = min(
            100,
            throttle + 5
        )

    if gear_button.check_press(pos):
        gear_down = not gear_down

    if flaps_minus.check_press(pos):
        flaps = max(
            0,
            flaps - 5
        )

    if flaps_plus.check_press(pos):
        flaps = min(
            30,
            flaps + 5
        )

    if engine_button.check_press(pos):
        engine_on = not engine_on

    if reset_button.check_press(pos):
        reset_flight()


def handle_release():

    roll_left.pressed = False
    roll_right.pressed = False
    pitch_down.pressed = False
    pitch_up.pressed = False


# ------------------------------------------------------------
# INITIAL CONNECTION
# ------------------------------------------------------------
connect_to_esp32()

# ------------------------------------------------------------
# MAIN LOOP
# ------------------------------------------------------------
running = True

last_send = 0

while running:

    dt = clock.tick(60) / 1000.0

    # --------------------------------------------------------
    # EVENTS
    # --------------------------------------------------------

    for event in pygame.event.get():

        if event.type == pygame.QUIT:

            running = False

        elif event.type == pygame.MOUSEBUTTONDOWN:

            handle_press(
                event.pos
            )

        elif event.type == pygame.MOUSEBUTTONUP:

            handle_release()

        elif event.type == pygame.KEYDOWN:

            if event.key == pygame.K_SPACE:

                paused = not paused

            elif event.key == pygame.K_ESCAPE:

                running = False

    # --------------------------------------------------------
    # HOLD BUTTON CONTROL
    # --------------------------------------------------------

    if not paused:

        if roll_left.pressed:
            roll -= 35 * dt

        if roll_right.pressed:
            roll += 35 * dt

        if pitch_down.pressed:
            pitch -= 15 * dt

        if pitch_up.pressed:
            pitch += 15 * dt

        pitch = max(
            -30,
            min(30, pitch)
        )

        roll = max(
            -45,
            min(45, roll)
        )

        update_physics(dt)

    # --------------------------------------------------------
    # AUTO RECONNECT
    # --------------------------------------------------------

    if not esp32_connected:

        if int(time.time()) % 3 == 0:

            connect_to_esp32()

    # --------------------------------------------------------
    # SEND TELEMETRY
    # --------------------------------------------------------

    current_time = time.time()

    if current_time - last_send >= 0.1:

        send_telemetry()

        last_send = current_time

    # --------------------------------------------------------
    # DRAW
    # --------------------------------------------------------

    screen.fill(BLACK)

    draw_horizon()

    draw_flight_data()

    draw_status()

    draw_info()

    # Buttons
    roll_left.draw()
    roll_right.draw()

    pitch_down.draw()
    pitch_up.draw()

    throttle_minus.draw()
    throttle_plus.draw()

    gear_button.draw()

    flaps_minus.draw()
    flaps_plus.draw()

    engine_button.draw()
    reset_button.draw()

    # Pause indicator
    if paused:

        pause_text = font_huge.render(
            "PAUSED",
            True,
            YELLOW
        )

        screen.blit(
            pause_text,
            (
                SCREEN_W // 2 -
                pause_text.get_width() // 2,
                340
            )
        )

    pygame.display.flip()


pygame.quit()

if esp32_socket:
    esp32_socket.close()