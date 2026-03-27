"""
Cricket 3D - Interactive Cricket Game
England vs Australia at Lord's / MCG
EA Cricket 07-style keyboard controls

Controls:
  BATTING: Space=face ball, then W/A/S/D/Q/E/Z/X/C = shots
  BOWLING: 1-6 = delivery type
  ESC = quit, R = restart
"""
import pygame
from pygame.locals import *
from OpenGL.GL import *
from OpenGL.GLU import *
import math, random, time, sys

# ============================================================
#  CONSTANTS
# ============================================================
WIDTH, HEIGHT = 1280, 800
FOV = 50
PITCH_LEN = 20.0   # metres (z-axis)
PITCH_W = 3.0
FIELD_R = 70.0
BOUNDARY_R = 65.0

# Colors (r,g,b 0-1)
COL_GRASS = (0.18, 0.55, 0.22)
COL_PITCH = (0.76, 0.65, 0.42)
COL_SKY_TOP = (0.35, 0.6, 0.85)
COL_SKY_BOT = (0.7, 0.82, 0.95)
COL_WHITE = (1, 1, 1)
COL_BALL = (0.75, 0.12, 0.1)
COL_STUMP = (0.85, 0.75, 0.45)
COL_ENG_BAT = (0.2, 0.3, 0.7)   # England blue
COL_ENG_BOWL = (0.2, 0.3, 0.7)
COL_AUS_BAT = (0.85, 0.75, 0.1)  # Australia gold
COL_AUS_BOWL = (0.85, 0.75, 0.1)
COL_STAND = (0.55, 0.5, 0.45)
COL_PAVILION = (0.7, 0.55, 0.35)
COL_ROOF = (0.3, 0.3, 0.35)
COL_SEAT_RED = (0.7, 0.15, 0.1)
COL_SEAT_BLUE = (0.15, 0.2, 0.6)
COL_SEAT_GREEN = (0.1, 0.5, 0.15)

# ============================================================
#  PLAYER DATA
# ============================================================
ENGLAND = [
    {"name":"Crawley","bat":78,"bowl":15,"power":0.65,"tech":0.70,"temp":0.55,"type":"BAT"},
    {"name":"Duckett","bat":80,"bowl":10,"power":0.72,"tech":0.68,"temp":0.50,"type":"BAT"},
    {"name":"Root","bat":92,"bowl":20,"power":0.55,"tech":0.95,"temp":0.92,"type":"BAT"},
    {"name":"Brook","bat":85,"bowl":12,"power":0.80,"tech":0.72,"temp":0.60,"type":"BAT"},
    {"name":"Bairstow","bat":76,"bowl":10,"power":0.75,"tech":0.60,"temp":0.50,"type":"BAT"},
    {"name":"Stokes","bat":82,"bowl":72,"power":0.78,"tech":0.70,"temp":0.65,"type":"AR"},
    {"name":"Foakes","bat":68,"bowl":5,"power":0.35,"tech":0.75,"temp":0.80,"type":"WK"},
    {"name":"Wood","bat":25,"bowl":85,"power":0.30,"tech":0.20,"temp":0.40,"type":"FAST"},
    {"name":"Anderson","bat":15,"bowl":90,"power":0.10,"tech":0.15,"temp":0.50,"type":"MF"},
    {"name":"Broad","bat":20,"bowl":86,"power":0.25,"tech":0.18,"temp":0.45,"type":"MF"},
    {"name":"Leach","bat":18,"bowl":78,"power":0.15,"tech":0.20,"temp":0.55,"type":"SPIN"},
]
AUSTRALIA = [
    {"name":"Khawaja","bat":84,"bowl":10,"power":0.45,"tech":0.88,"temp":0.90,"type":"BAT"},
    {"name":"Warner","bat":82,"bowl":15,"power":0.85,"tech":0.60,"temp":0.45,"type":"BAT"},
    {"name":"Labuschagne","bat":88,"bowl":25,"power":0.50,"tech":0.90,"temp":0.88,"type":"BAT"},
    {"name":"Smith","bat":93,"bowl":18,"power":0.48,"tech":0.96,"temp":0.95,"type":"BAT"},
    {"name":"Head","bat":79,"bowl":15,"power":0.72,"tech":0.65,"temp":0.55,"type":"BAT"},
    {"name":"Green","bat":72,"bowl":70,"power":0.65,"tech":0.68,"temp":0.60,"type":"AR"},
    {"name":"Carey","bat":70,"bowl":5,"power":0.60,"tech":0.65,"temp":0.55,"type":"WK"},
    {"name":"Cummins","bat":22,"bowl":92,"power":0.20,"tech":0.15,"temp":0.50,"type":"FAST"},
    {"name":"Starc","bat":30,"bowl":88,"power":0.35,"tech":0.22,"temp":0.40,"type":"FAST"},
    {"name":"Hazlewood","bat":15,"bowl":87,"power":0.10,"tech":0.12,"temp":0.55,"type":"MF"},
    {"name":"Lyon","bat":20,"bowl":82,"power":0.18,"tech":0.20,"temp":0.60,"type":"SPIN"},
]

STADIUMS = {
    "lords": {"name":"Lord's, London","boundary":68,"hard":0.55,"grass":0.50,"humid":0.72},
    "mcg":   {"name":"MCG, Melbourne","boundary":75,"hard":0.72,"grass":0.25,"humid":0.45},
}

# ============================================================
#  3D DRAWING PRIMITIVES
# ============================================================
def draw_cylinder(x, y, z, radius, height, slices, color):
    glColor3f(*color)
    quad = gluNewQuadric()
    glPushMatrix()
    glTranslatef(x, y, z)
    glRotatef(-90, 1, 0, 0)
    gluCylinder(quad, radius, radius, height, slices, 1)
    # Top cap
    glTranslatef(0, 0, height)
    gluDisk(quad, 0, radius, slices, 1)
    glPopMatrix()
    gluDeleteQuadric(quad)

def draw_box(x, y, z, sx, sy, sz, color):
    glColor3f(*color)
    glPushMatrix()
    glTranslatef(x, y, z)
    glBegin(GL_QUADS)
    # Front
    glVertex3f(-sx,-sy, sz); glVertex3f( sx,-sy, sz)
    glVertex3f( sx, sy, sz); glVertex3f(-sx, sy, sz)
    # Back
    glVertex3f(-sx,-sy,-sz); glVertex3f(-sx, sy,-sz)
    glVertex3f( sx, sy,-sz); glVertex3f( sx,-sy,-sz)
    # Top
    glVertex3f(-sx, sy,-sz); glVertex3f(-sx, sy, sz)
    glVertex3f( sx, sy, sz); glVertex3f( sx, sy,-sz)
    # Bottom
    glVertex3f(-sx,-sy,-sz); glVertex3f( sx,-sy,-sz)
    glVertex3f( sx,-sy, sz); glVertex3f(-sx,-sy, sz)
    # Right
    glVertex3f( sx,-sy,-sz); glVertex3f( sx, sy,-sz)
    glVertex3f( sx, sy, sz); glVertex3f( sx,-sy, sz)
    # Left
    glVertex3f(-sx,-sy,-sz); glVertex3f(-sx,-sy, sz)
    glVertex3f(-sx, sy, sz); glVertex3f(-sx, sy,-sz)
    glEnd()
    glPopMatrix()

def draw_sphere(x, y, z, r, color):
    glColor3f(*color)
    glPushMatrix()
    glTranslatef(x, y, z)
    quad = gluNewQuadric()
    gluSphere(quad, r, 12, 8)
    gluDeleteQuadric(quad)
    glPopMatrix()

def draw_ground_circle(cx, cz, radius, color, segments=64):
    glColor3f(*color)
    glBegin(GL_TRIANGLE_FAN)
    glVertex3f(cx, 0.01, cz)
    for i in range(segments + 1):
        a = 2 * math.pi * i / segments
        glVertex3f(cx + math.cos(a)*radius, 0.01, cz + math.sin(a)*radius)
    glEnd()

def draw_ring(cx, cz, r_inner, r_outer, color, segments=64):
    glColor3f(*color)
    glBegin(GL_QUAD_STRIP)
    for i in range(segments + 1):
        a = 2 * math.pi * i / segments
        glVertex3f(cx + math.cos(a)*r_inner, 0.02, cz + math.sin(a)*r_inner)
        glVertex3f(cx + math.cos(a)*r_outer, 0.02, cz + math.sin(a)*r_outer)
    glEnd()

def draw_person(x, y, z, color, height=1.8):
    """Simple person: cylinder body + sphere head"""
    draw_cylinder(x, y, z, 0.25, height*0.65, 8, color)
    draw_sphere(x, y + height*0.8, z, 0.2, (0.9, 0.75, 0.6))  # head
    # Arms
    draw_box(x, y+height*0.45, z, 0.45, 0.08, 0.08, color)
    # Legs
    draw_cylinder(x-0.12, y, z, 0.08, height*0.4, 6, color)
    draw_cylinder(x+0.12, y, z, 0.08, height*0.4, 6, color)

def draw_bat(x, y, z, angle=0):
    """Cricket bat"""
    glPushMatrix()
    glTranslatef(x, y, z)
    glRotatef(angle, 0, 1, 0)
    draw_box(0, 0.5, 0.3, 0.06, 0.35, 0.02, (0.85, 0.75, 0.5))  # blade
    draw_box(0, 0.9, 0.3, 0.03, 0.15, 0.02, (0.7, 0.6, 0.4))    # handle
    glPopMatrix()

def draw_stumps(x, z):
    """Three stumps + bails"""
    for dx in [-0.12, 0, 0.12]:
        draw_cylinder(x+dx, 0, z, 0.02, 0.72, 6, COL_STUMP)
    # Bails
    draw_box(x, 0.72, z, 0.15, 0.015, 0.015, (0.9, 0.8, 0.5))

# ============================================================
#  STADIUM RENDERING
# ============================================================
def draw_stadium_lords():
    """Lord's Cricket Ground - iconic pavilion end"""
    # Stands around the ground
    for angle in range(0, 360, 15):
        a = math.radians(angle)
        dist = BOUNDARY_R + 8
        x = math.cos(a) * dist
        z = math.sin(a) * dist
        h = 8 + random.Random(angle).random() * 4
        w = 6
        # Stand block
        draw_box(x, h/2, z, w, h/2, 3, COL_STAND)
        # Seats (colored tiers)
        col = COL_SEAT_RED if angle < 90 else (COL_SEAT_GREEN if angle < 180 else COL_SEAT_BLUE)
        draw_box(x, h/2 - 1, z + (3.2 if math.sin(a) > 0 else -3.2), w, h/2-1, 0.3, col)

    # Pavilion (behind bowler's end, z < 0)
    px, pz = 0, -(BOUNDARY_R + 12)
    draw_box(px, 6, pz, 18, 6, 5, COL_PAVILION)
    draw_box(px, 12.5, pz, 19, 0.5, 6, COL_ROOF)  # roof
    # Pavilion pillars
    for dx in range(-16, 17, 4):
        draw_cylinder(px+dx, 0, pz+4.5, 0.3, 12, 8, (0.8, 0.7, 0.55))
    # Clock tower
    draw_box(px, 15, pz, 2, 3, 2, COL_PAVILION)
    draw_sphere(px, 18.5, pz, 1.0, (0.9, 0.85, 0.7))

    # Media Centre (opposite end, z > 0) - the egg shape
    mx, mz = 0, BOUNDARY_R + 12
    draw_sphere(mx, 10, mz, 10, (0.85, 0.85, 0.88))
    draw_box(mx, 4, mz, 12, 4, 4, COL_STAND)

    # Scoreboard
    draw_box(BOUNDARY_R + 5, 5, 0, 1, 5, 8, (0.2, 0.2, 0.25))

def draw_stadium_mcg():
    """MCG - massive circular stadium"""
    # Continuous circular stands
    for angle in range(0, 360, 5):
        a = math.radians(angle)
        dist = BOUNDARY_R + 10
        x = math.cos(a) * dist
        z = math.sin(a) * dist
        h = 14 + math.sin(a * 2) * 3  # varying height
        w = 4.5
        # Lower tier
        draw_box(x, 5, z, w, 5, 4, COL_STAND)
        col = COL_SEAT_GREEN if angle < 120 else (COL_SEAT_BLUE if angle < 240 else COL_SEAT_RED)
        draw_box(x, 5, z + (4.2 if math.sin(a) > 0 else -4.2), w, 4.5, 0.3, col)
        # Upper tier
        x2 = math.cos(a) * (dist + 5)
        z2 = math.sin(a) * (dist + 5)
        draw_box(x2, 12, z2, w, 4, 3, COL_STAND)
        draw_box(x2, 12, z2 + (3.2 if math.sin(a) > 0 else -3.2), w, 3.5, 0.3, col)

    # Light towers (6 towers)
    for angle in [30, 90, 150, 210, 270, 330]:
        a = math.radians(angle)
        lx = math.cos(a) * (BOUNDARY_R + 20)
        lz = math.sin(a) * (BOUNDARY_R + 20)
        draw_cylinder(lx, 0, lz, 0.5, 35, 8, (0.6, 0.6, 0.6))
        draw_box(lx, 35, lz, 2, 1, 2, (0.9, 0.9, 0.8))  # light panel

    # Great Southern Stand (one side bigger)
    for dx in range(-20, 21, 5):
        draw_box(dx, 10, -(BOUNDARY_R + 18), 3, 10, 5, COL_STAND)
        draw_box(dx, 20.5, -(BOUNDARY_R + 18), 3.5, 0.5, 6, COL_ROOF)

def draw_field(stadium_key):
    """Draw the cricket field"""
    # Ground (large green circle)
    draw_ground_circle(0, 0, FIELD_R, COL_GRASS)

    # Boundary rope
    draw_ring(0, 0, BOUNDARY_R - 0.3, BOUNDARY_R + 0.3, COL_WHITE)

    # 30-yard circle
    draw_ring(0, 0, 27, 27.3, (0.8, 0.8, 0.8))

    # Pitch
    glColor3f(*COL_PITCH)
    glBegin(GL_QUADS)
    glVertex3f(-PITCH_W/2, 0.02, -PITCH_LEN/2)
    glVertex3f( PITCH_W/2, 0.02, -PITCH_LEN/2)
    glVertex3f( PITCH_W/2, 0.02,  PITCH_LEN/2)
    glVertex3f(-PITCH_W/2, 0.02,  PITCH_LEN/2)
    glEnd()

    # Crease lines
    glColor3f(*COL_WHITE)
    glLineWidth(2)
    for z in [-PITCH_LEN/2 + 1.2, PITCH_LEN/2 - 1.2]:
        glBegin(GL_LINES)
        glVertex3f(-1.5, 0.03, z); glVertex3f(1.5, 0.03, z)
        glEnd()

    # Stumps
    draw_stumps(0, -PITCH_LEN/2 + 0.5)  # bowler end
    draw_stumps(0, PITCH_LEN/2 - 0.5)   # batsman end

    # Stadium
    if stadium_key == "lords":
        draw_stadium_lords()
    else:
        draw_stadium_mcg()

# ============================================================
#  GAME STATE
# ============================================================
class Game:
    def __init__(self):
        self.phase = "menu"  # menu, toss, bat_ready, shot_select, shot_result, bowl_ready, bowl_result, innings_break, over
        self.stadium_key = "lords"
        self.format = "ODI"
        self.teams = [ENGLAND, AUSTRALIA]
        self.team_names = ["England", "Australia"]
        self.bat_team = 0
        self.bowl_team = 1
        self.user_batting = True

        # Innings
        self.runs = 0
        self.wickets = 0
        self.overs = 0
        self.balls = 0
        self.target = -1
        self.max_overs = 50
        self.innings = 0

        # Batting card
        self.bat_scores = []  # [(name, runs, balls, fours, sixes, out, how)]
        self.striker = 0
        self.non_striker = 1
        self.next_bat = 2

        # Bowling
        self.bowler_idx = 7
        self.last_bowler = -1
        self.bowl_figs = {}  # name -> [overs, runs, wickets, balls_in_over]

        # Ball
        self.ball_shine = 1.0
        self.ball_age = 0

        # Ball animation
        self.ball_pos = [0, 0.5, -12]  # current
        self.ball_target = [0, 0.5, 8]
        self.ball_t = 0
        self.ball_active = False
        self.ball_arc_height = 3

        # Shot timer
        self.shot_timer = 0
        self.shot_max = 1.5
        self.selected_shot = -1

        # Result display
        self.result_text = ""
        self.result_timer = 0
        self.result_color = (1,1,1)

        # Commentary
        self.commentary = []
        self.this_over = []  # list of (runs, is_wicket)

        # Camera
        self.cam_mode = "bowler"  # bowler, batsman, side, follow
        self.cam_angle = 0

        # Wagon wheel lines
        self.wagon = []  # (angle, dist, runs)

        # Menu
        self.menu_sel = 0

        # Match result
        self.match_result = ""

    def add_comment(self, s):
        self.commentary.append(s)
        if len(self.commentary) > 10:
            self.commentary.pop(0)

    def striker_p(self):
        return self.teams[self.bat_team][self.striker]

    def bowler_p(self):
        return self.teams[self.bowl_team][self.bowler_idx]

    def get_overs_str(self):
        return f"{self.overs}.{self.balls}" if self.balls > 0 else str(self.overs)

    def is_innings_over(self):
        if self.wickets >= 10: return True
        if self.overs >= self.max_overs: return True
        if self.target > 0 and self.runs >= self.target: return True
        return False

    def init_innings(self):
        self.runs = 0; self.wickets = 0; self.overs = 0; self.balls = 0
        self.striker = 0; self.non_striker = 1; self.next_bat = 2
        self.last_bowler = -1; self.ball_shine = 1.0; self.ball_age = 0
        self.this_over = []; self.wagon = []
        team = self.teams[self.bat_team]
        self.bat_scores = [[p["name"], 0, 0, 0, 0, False, ""] for p in team]
        self.bowl_figs = {}
        for p in self.teams[self.bowl_team]:
            if p["bowl"] >= 40:
                self.bowl_figs[p["name"]] = [0, 0, 0, 0]  # overs, runs, wkts, balls
        # Pick first bowler
        for i, p in enumerate(self.teams[self.bowl_team]):
            if p["bowl"] >= 70:
                self.bowler_idx = i; break

    def select_bowler(self):
        team = self.teams[self.bowl_team]
        eligible = []
        for i, p in enumerate(team):
            if p["bowl"] >= 40 and i != self.last_bowler:
                if p["name"] in self.bowl_figs and self.bowl_figs[p["name"]][0] >= 10:
                    continue
                eligible.append(i)
        if not eligible:
            eligible = [i for i, p in enumerate(team) if p["bowl"] >= 20 and i != self.last_bowler]
        if eligible:
            self.bowler_idx = random.choice(eligible)

    def calc_difficulty(self, delivery_idx=-1):
        b = self.bowler_p()
        diff = (b["bowl"] / 100) * 0.35
        # Swing
        if b["type"] in ("FAST", "MF"):
            swing = (b["bowl"]/100) * 0.3 * (1.3 if self.ball_shine > 0.5 else 0.6)
            stad = STADIUMS[self.stadium_key]
            if stad["humid"] > 0.65: swing *= 1.3
            diff += swing * 0.2
        # Seam
        if b["type"] in ("FAST", "MF"):
            stad = STADIUMS[self.stadium_key]
            diff += (b["bowl"]/100) * 0.15 * (1 + stad["grass"] * 0.5)
        # Spin
        if b["type"] == "SPIN":
            diff += (b["bowl"]/100) * 0.3
        # New ball
        if self.overs < 10: diff += 0.08
        elif self.overs < 30: diff += 0.03
        if self.ball_shine < 0.25 and self.ball_age > 150: diff += 0.07
        return max(0.05, min(0.75, diff))

# ============================================================
#  SHOT DEFINITIONS
# ============================================================
SHOTS = [
    {"key": K_w, "name": "Straight Drive", "angle": 0, "power": 0.85, "risk": 1.0, "loft": False},
    {"key": K_e, "name": "Lofted Drive",   "angle": 0, "power": 0.95, "risk": 2.2, "loft": True},
    {"key": K_a, "name": "Cover Drive",    "angle": 0.5, "power": 0.82, "risk": 1.0, "loft": False},
    {"key": K_q, "name": "Square Cut",     "angle": 1.2, "power": 0.80, "risk": 1.3, "loft": False},
    {"key": K_d, "name": "Flick",          "angle": -0.5, "power": 0.78, "risk": 1.0, "loft": False},
    {"key": K_z, "name": "Sweep",          "angle": -1.0, "power": 0.75, "risk": 1.5, "loft": False},
    {"key": K_x, "name": "Pull Shot",      "angle": -0.8, "power": 0.88, "risk": 1.6, "loft": False},
    {"key": K_c, "name": "Upper Cut",      "angle": 1.4, "power": 0.90, "risk": 2.0, "loft": True},
    {"key": K_s, "name": "Defend",         "angle": 0, "power": 0.20, "risk": 0.3, "loft": False},
    {"key": K_SPACE, "name": "Leave",      "angle": 0, "power": 0.0, "risk": 0.1, "loft": False},
]

DELIVERIES = [
    {"key": K_1, "name": "Good Length", "diff": 0.0},
    {"key": K_2, "name": "Short Ball",  "diff": -0.03},
    {"key": K_3, "name": "Full Ball",   "diff": 0.02},
    {"key": K_4, "name": "Yorker",      "diff": 0.12},
    {"key": K_5, "name": "Bouncer",     "diff": 0.08},
    {"key": K_6, "name": "Slower Ball", "diff": 0.05},
]

# ============================================================
#  RESOLVE DELIVERY
# ============================================================
def resolve_batting(game, shot_idx, difficulty):
    bat = game.striker_p()
    bowl = game.bowler_p()
    # Wide/no-ball
    if random.random() < 0.02:
        return {"runs":1,"four":False,"six":False,"wkt":False,"wide":True,"nb":False,
                "desc":"Wide ball!"}
    if random.random() < 0.01:
        return {"runs":1,"four":False,"six":False,"wkt":False,"wide":False,"nb":True,
                "desc":"No ball!"}

    shot = SHOTS[shot_idx] if shot_idx >= 0 else SHOTS[8]  # default defend

    if shot["power"] == 0:  # leave
        if random.random() < difficulty * 0.015:
            return {"runs":0,"four":False,"six":False,"wkt":True,"wide":False,"nb":False,
                    "desc":f"{bat['name']} BOWLED leaving! b {bowl['name']}"}
        return {"runs":0,"four":False,"six":False,"wkt":False,"wide":False,"nb":False,
                "desc":f"{bat['name']} leaves alone."}

    # Dismissal
    dc = difficulty * 0.07 * shot["risk"]
    dc *= (1 - bat["bat"]/150)
    dc *= (1 - bat["tech"]*0.25)
    bs = game.bat_scores[game.striker]
    if bs[2] < 8: dc *= 1.4
    dc = max(0.004, min(0.14, dc))

    if random.random() < dc:
        modes = ["bowled","caught","caught behind","lbw","c&b"]
        how = random.choice(modes)
        return {"runs":0,"four":False,"six":False,"wkt":True,"wide":False,"nb":False,
                "desc":f"{bat['name']} {how}! b {bowl['name']}"}

    # Runs
    power = shot["power"] * (bat["bat"]/100) * (0.5 + bat["power"]*0.5)
    val = power * random.random()
    stad = STADIUMS[game.stadium_key]

    if val > 0.88 and shot["loft"]:
        return {"runs":6,"four":False,"six":True,"wkt":False,"wide":False,"nb":False,
                "desc":f"{bat['name']} {shot['name']} for SIX!"}
    elif val > 0.68:
        if stad["boundary"] > 72 and random.random() < 0.12:
            return {"runs":3,"four":False,"six":False,"wkt":False,"wide":False,"nb":False,
                    "desc":f"{bat['name']} just short of boundary, 3 runs."}
        return {"runs":4,"four":True,"six":False,"wkt":False,"wide":False,"nb":False,
                "desc":f"{bat['name']} {shot['name']} FOUR!"}
    elif val > 0.45:
        r = random.randint(1,3)
        return {"runs":r,"four":False,"six":False,"wkt":False,"wide":False,"nb":False,
                "desc":f"{bat['name']} {shot['name']} for {r}."}
    elif val > 0.2:
        return {"runs":1,"four":False,"six":False,"wkt":False,"wide":False,"nb":False,
                "desc":f"{bat['name']} taps for one."}
    else:
        return {"runs":0,"four":False,"six":False,"wkt":False,"wide":False,"nb":False,
                "desc":f"{bat['name']} {shot['name']}, dot ball."}

def resolve_ai_batting(game, delivery_idx, difficulty):
    bat = game.striker_p()
    bowl = game.bowler_p()
    if random.random() < 0.02:
        return {"runs":1,"four":False,"six":False,"wkt":False,"wide":True,"nb":False,"desc":"Wide!"}
    if random.random() < 0.01:
        return {"runs":1,"four":False,"six":False,"wkt":False,"wide":False,"nb":True,"desc":"No ball!"}

    aggr = 1 - bat["temp"]
    if game.target > 0:
        need = game.target - game.runs
        ov_left = game.max_overs - (game.overs + game.balls/6)
        if ov_left > 0 and need/ov_left > 7: aggr += 0.25
    aggr = max(0.1, min(0.9, aggr))

    dc = difficulty * 0.07 * (1 - bat["bat"]/150) * (1 - bat["tech"]*0.25)
    if delivery_idx >= 0: dc += DELIVERIES[delivery_idx]["diff"] * 0.3
    bs = game.bat_scores[game.striker]
    if bs[2] < 8: dc *= 1.4
    dc = max(0.004, min(0.13, dc))

    if random.random() < dc:
        return {"runs":0,"four":False,"six":False,"wkt":True,"wide":False,"nb":False,
                "desc":f"{bat['name']} OUT! b {bowl['name']}"}

    val = (bat["bat"]/100) * random.random() * (0.5 + aggr*0.5)
    if val > 0.88 and random.random() < aggr:
        return {"runs":6,"four":False,"six":True,"wkt":False,"wide":False,"nb":False,
                "desc":f"{bat['name']} SIX!"}
    elif val > 0.65:
        return {"runs":4,"four":True,"six":False,"wkt":False,"wide":False,"nb":False,
                "desc":f"{bat['name']} FOUR!"}
    elif val > 0.4:
        r = random.randint(1,3)
        return {"runs":r,"four":False,"six":False,"wkt":False,"wide":False,"nb":False,
                "desc":f"{bat['name']} works it for {r}."}
    elif val > 0.2:
        return {"runs":1,"four":False,"six":False,"wkt":False,"wide":False,"nb":False,
                "desc":f"{bat['name']} pushes for one."}
    else:
        return {"runs":0,"four":False,"six":False,"wkt":False,"wide":False,"nb":False,
                "desc":f"{bat['name']} defends. Dot."}

# ============================================================
#  APPLY RESULT + OVER LOGIC
# ============================================================
def apply_result(game, res):
    game.add_comment(res["desc"])
    bname = game.bowler_p()["name"]
    bf = game.bowl_figs.get(bname)

    if res["wide"] or res["nb"]:
        game.runs += 1
        if bf: bf[1] += 1
        game.this_over.append(("W" if res["wide"] else "NB", False))
        return

    game.balls += 1
    game.ball_age += 1
    game.ball_shine = max(0, game.ball_shine - 0.003)
    bs = game.bat_scores[game.striker]
    bs[2] += 1  # balls faced

    if res["wkt"]:
        game.runs += res["runs"]
        if bf: bf[1] += res["runs"]; bf[2] += 1
        bs[5] = True; bs[6] = res["desc"]
        game.wickets += 1
        game.this_over.append(("W", True))
        if game.next_bat < len(game.teams[game.bat_team]):
            game.striker = game.next_bat; game.next_bat += 1
    else:
        game.runs += res["runs"]
        bs[1] += res["runs"]
        if bf: bf[1] += res["runs"]
        if res["four"]: bs[3] += 1
        if res["six"]: bs[4] += 1
        game.this_over.append((str(res["runs"]), False))
        if res["runs"] % 2 == 1:
            game.striker, game.non_striker = game.non_striker, game.striker
        if res["runs"] > 0:
            angle = random.uniform(-math.pi, math.pi)
            dist = 0.2 + res["runs"]*0.12
            if res["six"]: dist = 0.95
            elif res["four"]: dist = 0.85
            game.wagon.append((angle, dist, res["runs"]))

    # Over complete?
    if game.balls >= 6:
        game.overs += 1
        if bf:
            bf[0] += 1; bf[3] = 0
        game.balls = 0
        game.striker, game.non_striker = game.non_striker, game.striker
        game.last_bowler = game.bowler_idx
        game.select_bowler()
        game.this_over = []
        game.add_comment(f"End of over {game.overs}: {game.runs}/{game.wickets}")

# ============================================================
#  2D HUD OVERLAY
# ============================================================
def draw_hud(game, surface):
    font_big = pygame.font.SysFont("consolas", 22, bold=True)
    font_med = pygame.font.SysFont("consolas", 16)
    font_sm = pygame.font.SysFont("consolas", 13)

    # Scoreboard panel
    panel = pygame.Surface((380, 300), pygame.SRCALPHA)
    panel.fill((10, 10, 40, 200))
    surface.blit(panel, (WIDTH - 390, 10))

    x, y = WIDTH - 380, 18
    # Team
    t = font_big.render(f"{game.team_names[game.bat_team]}", True, (255,255,100))
    surface.blit(t, (x, y)); y += 26
    # Score
    t = font_big.render(f"{game.runs}/{game.wickets}  ({game.get_overs_str()} ov)", True, (255,255,255))
    surface.blit(t, (x, y)); y += 28
    # RR
    ov = game.overs + game.balls/6
    rr = game.runs/ov if ov > 0 else 0
    rr_text = f"RR: {rr:.2f}"
    if game.target > 0:
        need = game.target - game.runs
        if need > 0:
            ol = game.max_overs - ov
            rrr = need/ol if ol > 0 else 99
            rr_text += f"  Need {need} off {ol:.1f} ov"
    t = font_sm.render(rr_text, True, (180,180,180))
    surface.blit(t, (x, y)); y += 20

    # Batsmen
    bs = game.bat_scores[game.striker]
    t = font_med.render(f"* {bs[0]}  {bs[1]}({bs[2]}b {bs[3]}x4 {bs[4]}x6)", True, (255,255,255))
    surface.blit(t, (x, y)); y += 20
    bs2 = game.bat_scores[game.non_striker]
    t = font_sm.render(f"  {bs2[0]}  {bs2[1]}({bs2[2]}b)", True, (180,180,180))
    surface.blit(t, (x, y)); y += 20

    # Bowler
    bp = game.bowler_p()
    bf = game.bowl_figs.get(bp["name"], [0,0,0,0])
    t = font_med.render(f"{bp['name']}  {bf[2]}/{bf[1]} ({bf[0]}.{game.balls} ov)", True, (200,200,255))
    surface.blit(t, (x, y)); y += 22

    # This over
    over_str = "  ".join([b[0] for b in game.this_over])
    t = font_sm.render(f"This over: {over_str}", True, (180,180,180))
    surface.blit(t, (x, y)); y += 20

    # Commentary (last 4)
    for line in game.commentary[-4:]:
        t = font_sm.render(line[:55], True, (170,170,170))
        surface.blit(t, (x, y)); y += 15

    # Result flash
    if game.result_timer > 0:
        c = tuple(int(v*255) for v in game.result_color)
        t = font_big.render(game.result_text, True, c)
        surface.blit(t, (WIDTH//2 - t.get_width()//2, 50))

    # Controls prompt (bottom)
    panel2 = pygame.Surface((WIDTH, 50), pygame.SRCALPHA)
    panel2.fill((10, 10, 30, 200))
    surface.blit(panel2, (0, HEIGHT - 50))

    if game.phase == "bat_ready":
        t = font_med.render("Press SPACE to face the delivery...", True, (100,255,100))
        surface.blit(t, (20, HEIGHT - 42))
    elif game.phase == "shot_select":
        # Timer bar
        pct = game.shot_timer / game.shot_max
        pygame.draw.rect(surface, (60,60,60), (20, HEIGHT-42, 250, 16))
        col = (0,200,0) if pct > 0.3 else (200,50,0)
        pygame.draw.rect(surface, col, (20, HEIGHT-42, int(250*pct), 16))
        t = font_med.render("PICK SHOT! W/A/D/Q/E/Z/X/C/S/Space", True, (255,255,100))
        surface.blit(t, (280, HEIGHT - 42))
    elif game.phase == "bowl_ready":
        t = font_med.render("BOWLING: 1=Good 2=Short 3=Full 4=Yorker 5=Bouncer 6=Slower", True, (100,200,255))
        surface.blit(t, (20, HEIGHT - 42))
    elif game.phase == "over":
        t = font_big.render(game.match_result + "  Press R to restart", True, (255,255,100))
        surface.blit(t, (20, HEIGHT - 42))

    # Stadium name
    stad = STADIUMS[game.stadium_key]
    t = font_sm.render(stad["name"], True, (200,200,200))
    surface.blit(t, (10, 10))

# ============================================================
#  MAIN LOOP
# ============================================================
def main():
    pygame.init()
    pygame.font.init()
    screen = pygame.display.set_mode((WIDTH, HEIGHT), DOUBLEBUF | OPENGL)
    pygame.display.set_caption("Cricket 3D - England vs Australia")

    glEnable(GL_DEPTH_TEST)
    glEnable(GL_COLOR_MATERIAL)
    glEnable(GL_LIGHTING)
    glEnable(GL_LIGHT0)
    glLightfv(GL_LIGHT0, GL_POSITION, [0.5, 1.0, 0.3, 0.0])
    glLightfv(GL_LIGHT0, GL_AMBIENT, [0.4, 0.4, 0.4, 1.0])
    glLightfv(GL_LIGHT0, GL_DIFFUSE, [0.8, 0.8, 0.75, 1.0])

    clock = pygame.time.Clock()
    game = Game()

    # For 2D overlay
    hud_surface = pygame.Surface((WIDTH, HEIGHT), pygame.SRCALPHA)

    running = True
    while running:
        dt = clock.tick(60) / 1000.0
        keys = pygame.key.get_pressed()

        for event in pygame.event.get():
            if event.type == QUIT:
                running = False
            if event.type == KEYDOWN:
                if event.key == K_ESCAPE:
                    running = False

                # ---- MENU ----
                if game.phase == "menu":
                    if event.key == K_1:
                        game.stadium_key = "lords"; game.phase = "toss"
                    elif event.key == K_2:
                        game.stadium_key = "mcg"; game.phase = "toss"

                # ---- BATTING ----
                elif game.phase == "bat_ready":
                    if event.key == K_SPACE:
                        game.shot_timer = game.shot_max
                        game.selected_shot = -1
                        game.ball_active = True
                        game.ball_t = 0
                        game.ball_pos = [0, 1.5, -PITCH_LEN/2 - 3]
                        game.ball_target = [0, 0.8, PITCH_LEN/2 - 1]
                        game.phase = "shot_select"

                elif game.phase == "shot_select":
                    for i, s in enumerate(SHOTS):
                        if event.key == s["key"]:
                            game.selected_shot = i
                            break

                # ---- BOWLING ----
                elif game.phase == "bowl_ready":
                    for i, d in enumerate(DELIVERIES):
                        if event.key == d["key"]:
                            diff = game.calc_difficulty(i)
                            res = resolve_ai_batting(game, i, diff)
                            # Ball anim
                            angle = random.uniform(-math.pi, math.pi)
                            dist = 5 + res["runs"] * 8
                            if res["six"]: dist = BOUNDARY_R * 0.9
                            elif res["four"]: dist = BOUNDARY_R * 0.8
                            game.ball_pos = [0, 1.5, -PITCH_LEN/2 - 3]
                            game.ball_target = [math.sin(angle)*dist, (3 if res["six"] else 0.3), math.cos(angle)*dist]
                            game.ball_t = 0; game.ball_active = True
                            apply_result(game, res)
                            game.result_text = "WICKET!" if res["wkt"] else ("SIX!" if res["six"] else ("FOUR!" if res["four"] else ""))
                            game.result_color = (1,0.2,0.2) if res["wkt"] else ((1,0.9,0) if res["six"] else ((0,0.9,0) if res["four"] else (1,1,1)))
                            game.result_timer = 2.0
                            game.phase = "bowl_result"
                            break

                elif game.phase == "over":
                    if event.key == K_r:
                        game.__init__()

        # ---- UPDATE ----
        if game.phase == "toss":
            game.bat_team = random.randint(0, 1)
            game.bowl_team = 1 - game.bat_team
            game.user_batting = (game.bat_team == 0)
            game.add_comment(f"{game.team_names[game.bat_team]} bat first.")
            game.init_innings()
            game.phase = "bat_ready" if game.user_batting else "bowl_ready"

        if game.phase == "shot_select":
            game.shot_timer -= dt
            game.ball_t = min(1, game.ball_t + dt / game.shot_max)
            if game.selected_shot >= 0 or game.shot_timer <= 0:
                diff = game.calc_difficulty()
                res = resolve_batting(game, game.selected_shot, diff)
                shot = SHOTS[game.selected_shot] if game.selected_shot >= 0 else SHOTS[8]
                angle = shot["angle"] + random.uniform(-0.2, 0.2)
                dist = 5 + res["runs"] * 8
                if res["six"]: dist = BOUNDARY_R * 0.9
                elif res["four"]: dist = BOUNDARY_R * 0.8
                elif res["wkt"]: dist = 2
                game.ball_target = [math.sin(angle)*dist, (4 if res["six"] else 0.3), -math.cos(angle)*dist]
                game.ball_t = 0
                apply_result(game, res)
                game.result_text = "WICKET!" if res["wkt"] else ("SIX!" if res["six"] else ("FOUR!" if res["four"] else ""))
                game.result_color = (1,0.2,0.2) if res["wkt"] else ((1,0.9,0) if res["six"] else ((0,0.9,0) if res["four"] else (1,1,1)))
                game.result_timer = 2.0
                game.phase = "shot_result"

        if game.phase in ("shot_result", "bowl_result"):
            game.ball_t = min(1, game.ball_t + dt * 2)
            game.result_timer -= dt
            if game.result_timer <= 0:
                game.ball_active = False
                if game.is_innings_over():
                    if game.innings == 0:
                        game.target = game.runs + 1
                        game.innings = 1
                        game.bat_team, game.bowl_team = game.bowl_team, game.bat_team
                        game.user_batting = not game.user_batting
                        game.add_comment(f"--- Innings Break --- Target: {game.target}")
                        game.init_innings()
                        game.phase = "bat_ready" if game.user_batting else "bowl_ready"
                    else:
                        if game.runs >= game.target:
                            game.match_result = f"{game.team_names[game.bat_team]} WIN by {10-game.wickets} wickets!"
                        else:
                            game.match_result = f"{game.team_names[game.bowl_team]} WIN by {game.target-1-game.runs} runs!"
                        game.add_comment(game.match_result)
                        game.phase = "over"
                else:
                    game.phase = "bat_ready" if game.user_batting else "bowl_ready"

        if game.result_timer > 0:
            game.result_timer -= dt * 0.5  # slow decay for flash

        # ---- 3D RENDER ----
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT)

        # Sky gradient (simple clear color)
        glClearColor(*COL_SKY_TOP, 1.0)

        glMatrixMode(GL_PROJECTION)
        glLoadIdentity()
        gluPerspective(FOV, WIDTH/HEIGHT, 0.5, 300)

        glMatrixMode(GL_MODELVIEW)
        glLoadIdentity()

        # Camera
        if game.phase == "menu":
            # Orbit camera
            game.cam_angle += dt * 0.3
            cx = math.sin(game.cam_angle) * 80
            cz = math.cos(game.cam_angle) * 80
            gluLookAt(cx, 30, cz, 0, 0, 0, 0, 1, 0)
        else:
            # Behind bowler view (classic TV angle)
            gluLookAt(2, 12, -PITCH_LEN/2 - 25,  0, 1, PITCH_LEN/4,  0, 1, 0)

        # Draw 3D scene
        glEnable(GL_LIGHTING)
        draw_field(game.stadium_key)

        # Players on field
        bat_col = COL_ENG_BAT if game.bat_team == 0 else COL_AUS_BAT
        bowl_col = COL_ENG_BOWL if game.bowl_team == 0 else COL_AUS_BOWL

        # Batsman at crease
        draw_person(0.8, 0, PITCH_LEN/2 - 1.5, bat_col)
        draw_bat(0.8, 0, PITCH_LEN/2 - 1.5, 30)
        # Non-striker
        draw_person(-0.8, 0, -PITCH_LEN/2 + 1.5, bat_col, 1.75)
        # Bowler
        draw_person(0, 0, -PITCH_LEN/2 - 5, bowl_col)
        # Keeper
        draw_person(0, 0, PITCH_LEN/2 + 2, bowl_col, 1.6)
        # Fielders
        for angle_deg in [0, 45, 90, 135, 180, 225, 270, 315, 20]:
            a = math.radians(angle_deg)
            fd = 25 + random.Random(angle_deg).random() * 15
            fx = math.sin(a) * fd
            fz = math.cos(a) * fd
            draw_person(fx, 0, fz, bowl_col, 1.7)

        # Ball
        if game.ball_active:
            t = game.ball_t
            bx = game.ball_pos[0] + (game.ball_target[0] - game.ball_pos[0]) * t
            by = game.ball_pos[1] + (game.ball_target[1] - game.ball_pos[1]) * t + math.sin(t * math.pi) * game.ball_arc_height
            bz = game.ball_pos[2] + (game.ball_target[2] - game.ball_pos[2]) * t
            draw_sphere(bx, by, bz, 0.15, COL_BALL)

        glDisable(GL_LIGHTING)

        # ---- 2D HUD ----
        # Read OpenGL framebuffer, draw HUD with pygame, then blit back
        # Simpler: use OpenGL ortho for text
        glMatrixMode(GL_PROJECTION)
        glPushMatrix()
        glLoadIdentity()
        glOrtho(0, WIDTH, HEIGHT, 0, -1, 1)
        glMatrixMode(GL_MODELVIEW)
        glPushMatrix()
        glLoadIdentity()
        glDisable(GL_DEPTH_TEST)

        # Draw HUD to pygame surface then texture
        hud_surface.fill((0, 0, 0, 0))

        if game.phase == "menu":
            font_big = pygame.font.SysFont("arial", 40, bold=True)
            font_med = pygame.font.SysFont("consolas", 20)
            t = font_big.render("CRICKET 3D", True, (255,255,100))
            hud_surface.blit(t, (WIDTH//2 - t.get_width()//2, 100))
            t = font_med.render("England vs Australia", True, (200,200,200))
            hud_surface.blit(t, (WIDTH//2 - t.get_width()//2, 160))
            t = font_med.render("[1] Lord's, London", True, (200,255,200))
            hud_surface.blit(t, (WIDTH//2 - 120, 240))
            t = font_med.render("[2] MCG, Melbourne", True, (200,255,200))
            hud_surface.blit(t, (WIDTH//2 - 120, 280))
            t = font_med.render("Press 1 or 2 to start", True, (180,180,180))
            hud_surface.blit(t, (WIDTH//2 - 120, 340))
        else:
            draw_hud(game, hud_surface)

        # Convert pygame surface to OpenGL texture and render
        tex_data = pygame.image.tostring(hud_surface, "RGBA", False)
        tex_id = glGenTextures(1)
        glBindTexture(GL_TEXTURE_2D, tex_id)
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, WIDTH, HEIGHT, 0, GL_RGBA, GL_UNSIGNED_BYTE, tex_data)
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR)
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR)

        glEnable(GL_TEXTURE_2D)
        glEnable(GL_BLEND)
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA)
        glColor4f(1, 1, 1, 1)
        glBegin(GL_QUADS)
        glTexCoord2f(0, 0); glVertex2f(0, 0)
        glTexCoord2f(1, 0); glVertex2f(WIDTH, 0)
        glTexCoord2f(1, 1); glVertex2f(WIDTH, HEIGHT)
        glTexCoord2f(0, 1); glVertex2f(0, HEIGHT)
        glEnd()
        glDisable(GL_TEXTURE_2D)
        glDisable(GL_BLEND)
        glDeleteTextures([tex_id])

        glEnable(GL_DEPTH_TEST)
        glMatrixMode(GL_PROJECTION)
        glPopMatrix()
        glMatrixMode(GL_MODELVIEW)
        glPopMatrix()

        pygame.display.flip()

    pygame.quit()

if __name__ == "__main__":
    main()
