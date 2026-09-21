import pygame
import math
import sys
import socket
import time

# ============================================================
# OPENFLIGHT - LIGHTWEIGHT FLIGHT SIMULATOR
# STEP 2
# Simulator + ESP32 TCP Telemetry
# ============================================================

pygame.init()

WIDTH = 1000
HEIGHT = 650

screen = pygame.display.set_mode((WIDTH, HEIGHT))
pygame.display.set_caption("OpenFlight - Lightweight Flight Simulator")

clock = pygame.time.Clock()

# ============================================================
# ESP32 TCP CONFIGURATION
# ============================================================

ESP32_IP = "192.168.149.178"
ESP32_PORT = 5000

tcp_socket = None
esp32_connected = False

# Send telemetry every 100 ms
TELEMETRY_INTERVAL = 0.1
last_telemetry_time = 0


# ============================================================
# COLORS
# ============================================================

BLACK = (10, 10, 15)
WHITE = (245, 245, 245)
GREEN = (50, 220, 120)
YELLOW = (240, 210, 70)
CYAN = (70, 220, 240)
RED = (240, 80, 80)
GRAY = (150, 150, 150)
DARK_GRAY = (30, 35, 42)


# ============================================================
# FONTS
# ============================================================

font_large = pygame.font.Font(None, 42)
font_medium = pygame.font.Font(None, 30)
font_small = pygame.font.Font(None, 24)


# ============================================================
# AIRCRAFT STATE
# ============================================================

altitude = 5000.0
airspeed = 120.0
heading = 90.0

pitch = 0.0
roll = 0.0

vertical_speed = 0.0

throttle = 50.0

gear_down = True
flaps = 0.0

engine_running = False

paused = False


# ============================================================
# LIMITS
# ============================================================

MIN_ALTITUDE = 0
MAX_ALTITUDE = 50000

MIN_SPEED = 0
MAX_SPEED = 400

MIN_PITCH = -20
MAX_PITCH = 20

MIN_ROLL = -45
MAX_ROLL = 45

MIN_THROTTLE = 0
MAX_THROTTLE = 100

MIN_FLAPS = 0
MAX_FLAPS = 30


# ============================================================
# DRAW TEXT
# ============================================================

def draw_text(text, x, y, font, color=WHITE):

    surface = font.render(text, True, color)

    screen.blit(surface, (x, y))


# ============================================================
# ESP32 CONNECTION
# ============================================================

def connect_to_esp32():

    global tcp_socket
    global esp32_connected

    print("==============================================")
    print("Connecting to ESP32...")
    print(f"ESP32 IP   : {ESP32_IP}")
    print(f"TCP PORT   : {ESP32_PORT}")
    print("==============================================")

    try:

        tcp_socket = socket.socket(
            socket.AF_INET,
            socket.SOCK_STREAM
        )

        # Short timeout so simulator doesn't freeze
        tcp_socket.settimeout(3)

        tcp_socket.connect(
            (ESP32_IP, ESP32_PORT)
        )

        tcp_socket.settimeout(None)

        esp32_connected = True

        print("ESP32 CONNECTED!")
        print()

    except Exception as e:

        esp32_connected = False
        tcp_socket = None

        print("ESP32 CONNECTION FAILED")
        print(e)
        print()

        print("Simulator will continue without ESP32.")
        print()


# ============================================================
# SEND TELEMETRY
# ============================================================

def send_telemetry():

    global tcp_socket
    global esp32_connected

    if not esp32_connected:

        return

    try:

        # Create telemetry packet
        telemetry = (
            f"ALT={altitude:.0f},"
            f"SPD={airspeed:.0f},"
            f"HDG={heading:.0f},"
            f"VS={vertical_speed:.0f},"
            f"PITCH={pitch:.1f},"
            f"ROLL={roll:.1f},"
            f"THR={throttle:.0f},"
            f"GEAR={'DOWN' if gear_down else 'UP'},"
            f"FLAPS={flaps:.0f},"
            f"ENGINE={'ON' if engine_running else 'OFF'}"
        )

        # Send packet
        tcp_socket.sendall(
            (telemetry + "\n").encode("utf-8")
        )

    except Exception as e:

        print("ESP32 connection lost.")

        print(e)

        esp32_connected = False

        try:

            tcp_socket.close()

        except:

            pass

        tcp_socket = None


# ============================================================
# DRAW HORIZON
# ============================================================

def draw_horizon():

    center_x = WIDTH // 2
    center_y = 285

    # Sky
    pygame.draw.rect(
        screen,
        (45, 90, 115),
        (50, 70, 600, 430)
    )

    # Ground
    pygame.draw.rect(
        screen,
        (80, 75, 65),
        (50, 285, 600, 215)
    )

    # Horizon
    pygame.draw.line(
        screen,
        WHITE,
        (50, 285),
        (650, 285),
        3
    )

    # Pitch ladder

    for pitch_angle in range(-20, 25, 5):

        y = center_y - (pitch_angle - pitch) * 7

        if 80 < y < 490:

            line_width = (
                60
                if pitch_angle % 10 == 0
                else 35
            )

            pygame.draw.line(
                screen,
                WHITE,
                (
                    center_x - line_width,
                    y
                ),
                (
                    center_x + line_width,
                    y
                ),
                2
            )

            draw_text(
                str(pitch_angle),
                center_x + line_width + 8,
                y - 10,
                font_small
            )

    # Aircraft reference symbol

    pygame.draw.line(
        screen,
        YELLOW,
        (center_x - 55, center_y),
        (center_x - 10, center_y),
        4
    )

    pygame.draw.line(
        screen,
        YELLOW,
        (center_x + 10, center_y),
        (center_x + 55, center_y),
        4
    )

    pygame.draw.line(
        screen,
        YELLOW,
        (center_x, center_y - 10),
        (center_x, center_y + 10),
        4
    )


# ============================================================
# DRAW INSTRUMENT PANEL
# ============================================================

def draw_instruments():

    panel_x = 680

    pygame.draw.rect(
        screen,
        DARK_GRAY,
        (panel_x, 70, 290, 430)
    )

    pygame.draw.rect(
        screen,
        GRAY,
        (panel_x, 70, 290, 430),
        2
    )

    draw_text(
        "FLIGHT DATA",
        panel_x + 65,
        90,
        font_medium,
        CYAN
    )

    pygame.draw.line(
        screen,
        GRAY,
        (panel_x + 15, 125),
        (panel_x + 275, 125),
        1
    )

    y = 145

    data = [

        ("ALTITUDE", f"{altitude:,.0f} ft"),

        ("AIRSPEED", f"{airspeed:.0f} kt"),

        ("HEADING", f"{heading:03.0f} deg"),

        ("VERT SPEED", f"{vertical_speed:+.0f} fpm"),

        ("PITCH", f"{pitch:+.1f} deg"),

        ("ROLL", f"{roll:+.1f} deg"),

        ("THROTTLE", f"{throttle:.0f}%"),

        ("GEAR", "DOWN" if gear_down else "UP"),

        ("FLAPS", f"{flaps:.0f} deg"),

        ("ENGINE", "RUNNING" if engine_running else "OFF"),

    ]

    for label, value in data:

        draw_text(
            label,
            panel_x + 20,
            y,
            font_small,
            GRAY
        )

        draw_text(
            value,
            panel_x + 145,
            y,
            font_small,
            WHITE
        )

        y += 34


# ============================================================
# DRAW HEADER
# ============================================================

def draw_header():

    draw_text(
        "OPENFLIGHT",
        50,
        20,
        font_large,
        CYAN
    )

    draw_text(
        "LIGHTWEIGHT FLIGHT SIMULATOR",
        300,
        30,
        font_small,
        GRAY
    )


# ============================================================
# DRAW STATUS
# ============================================================

def draw_status():

    if paused:

        draw_text(
            "PAUSED",
            50,
            530,
            font_medium,
            RED
        )

    else:

        draw_text(
            "SIMULATION ACTIVE",
            50,
            530,
            font_medium,
            GREEN
        )

    # ESP32 connection status

    if esp32_connected:

        draw_text(
            "ESP32: CONNECTED",
            400,
            530,
            font_medium,
            GREEN
        )

    else:

        draw_text(
            "ESP32: DISCONNECTED",
            400,
            530,
            font_medium,
            RED
        )


# ============================================================
# RESET AIRCRAFT
# ============================================================

def reset_aircraft():

    global altitude
    global airspeed
    global heading
    global pitch
    global roll
    global vertical_speed
    global throttle
    global gear_down
    global flaps
    global engine_running
    global paused

    altitude = 5000.0

    airspeed = 120.0

    heading = 90.0

    pitch = 0.0

    roll = 0.0

    vertical_speed = 0.0

    throttle = 50.0

    gear_down = True

    flaps = 0.0

    engine_running = False

    paused = False


# ============================================================
# UPDATE FLIGHT MODEL
# ============================================================

def update_flight(dt, keys):

    global altitude
    global airspeed
    global heading
    global pitch
    global roll
    global vertical_speed
    global throttle
    global flaps

    # --------------------------------------------------------
    # PITCH
    # --------------------------------------------------------

    if keys[pygame.K_UP]:

        pitch += 12 * dt

    if keys[pygame.K_DOWN]:

        pitch -= 12 * dt

    pitch = max(
        MIN_PITCH,
        min(MAX_PITCH, pitch)
    )

    # --------------------------------------------------------
    # ROLL
    # --------------------------------------------------------

    if keys[pygame.K_LEFT]:

        roll -= 25 * dt

    if keys[pygame.K_RIGHT]:

        roll += 25 * dt

    roll = max(
        MIN_ROLL,
        min(MAX_ROLL, roll)
    )

    # --------------------------------------------------------
    # THROTTLE
    # --------------------------------------------------------

    if keys[pygame.K_w]:

        throttle += 30 * dt

    if keys[pygame.K_s]:

        throttle -= 30 * dt

    throttle = max(
        MIN_THROTTLE,
        min(MAX_THROTTLE, throttle)
    )

    # --------------------------------------------------------
    # HEADING
    # --------------------------------------------------------

    turn_rate = roll * 0.35

    heading += turn_rate * dt

    heading %= 360

    # --------------------------------------------------------
    # AIRSPEED
    # --------------------------------------------------------

    if engine_running:

        target_speed = (
            60
            + throttle * 2.2
            - flaps * 1.5
        )

    else:

        target_speed = 0

    speed_change = (
        target_speed - airspeed
    ) * 0.35 * dt

    airspeed += speed_change

    airspeed = max(
        MIN_SPEED,
        min(MAX_SPEED, airspeed)
    )

    # --------------------------------------------------------
    # VERTICAL SPEED
    # --------------------------------------------------------

    vertical_speed = (
        pitch * 180
        + (throttle - 50) * 1.5
    )

    # --------------------------------------------------------
    # ALTITUDE
    # --------------------------------------------------------

    altitude += (
        vertical_speed * dt / 60
    )

    altitude = max(
        MIN_ALTITUDE,
        min(MAX_ALTITUDE, altitude)
    )


# ============================================================
# CONNECT TO ESP32
# ============================================================

connect_to_esp32()


# ============================================================
# MAIN LOOP
# ============================================================

running = True

while running:

    dt = clock.tick(60) / 1000.0

    # --------------------------------------------------------
    # EVENTS
    # --------------------------------------------------------

    for event in pygame.event.get():

        if event.type == pygame.QUIT:

            running = False

        if event.type == pygame.KEYDOWN:

            # Engine start

            if event.key == pygame.K_e:

                engine_running = True

            # Engine OFF

            if event.key == pygame.K_x:

                engine_running = False

            # Landing gear

            if event.key == pygame.K_g:

                gear_down = not gear_down

            # Pause

            if event.key == pygame.K_SPACE:

                paused = not paused

            # Reset

            if event.key == pygame.K_r:

                reset_aircraft()

            # ------------------------------------------------
            # FLAPS UP
            # F = +5 degrees
            # ------------------------------------------------

            if event.key == pygame.K_f:

                if not (
                    event.mod
                    & pygame.KMOD_SHIFT
                ):

                    flaps += 5

                    if flaps > MAX_FLAPS:

                        flaps = MAX_FLAPS

            # ------------------------------------------------
            # FLAPS DOWN
            # SHIFT + F = -5 degrees
            # ------------------------------------------------

            if event.key == pygame.K_f:

                if (
                    event.mod
                    & pygame.KMOD_SHIFT
                ):

                    flaps -= 5

                    if flaps < MIN_FLAPS:

                        flaps = MIN_FLAPS

    # --------------------------------------------------------
    # UPDATE FLIGHT MODEL
    # --------------------------------------------------------

    if not paused:

        keys = pygame.key.get_pressed()

        update_flight(
            dt,
            keys
        )

    # --------------------------------------------------------
    # SEND TELEMETRY
    # --------------------------------------------------------

    current_time = time.time()

    if (
        current_time - last_telemetry_time
        >= TELEMETRY_INTERVAL
    ):

        send_telemetry()

        last_telemetry_time = current_time

    # --------------------------------------------------------
    # DRAW
    # --------------------------------------------------------

    screen.fill(BLACK)

    draw_header()

    draw_horizon()

    draw_instruments()

    draw_status()

    # --------------------------------------------------------
    # CONTROLS
    # --------------------------------------------------------

    draw_text(
        "E: Engine Start   X: Engine Off   G: Gear   R: Reset",
        50,
        570,
        font_small,
        GRAY
    )

    draw_text(
        "UP/DOWN: Pitch   LEFT/RIGHT: Roll   W/S: Throttle",
        50,
        600,
        font_small,
        GRAY
    )

    draw_text(
        "F: Flaps +5   SHIFT+F: Flaps -5   SPACE: Pause",
        50,
        625,
        font_small,
        GRAY
    )

    pygame.display.flip()


# ============================================================
# CLEANUP
# ============================================================

if tcp_socket:

    try:
        tcp_socket.close()
    except:
        pass


pygame.quit()
sys.exit()