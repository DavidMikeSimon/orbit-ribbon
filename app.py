from __future__ import division

import ode, sys, math, pygame, pickle
from pygame.locals import *
from OpenGL.GL import *
from OpenGL.GLU import *
from OpenGL.GLUT import *

import collision, util, console, resman, camera, joy, sky, titlescreen, ore, avatar
from geometry import *

MODE_GAMEPLAY, MODE_TITLE_SCREEN = range(2)

SKY_CLIP_DIST = 1e12
GAMEPLAY_CLIP_DIST = 50000
FOV = 45

VERSION = "prealpha"

#The ODE simulation
#Objects in static_space do not collide with one another.
#Objects in dyn_space collide with those in static_space, as well as with each other.
odeworld = None
static_space = None
dyn_space = None

#The OREManager for currently active ORE file (Orbit Ribbon Export data, from the 3d editor)
ore_man = None

#The currently active OREArea, or None. The tstart variable is the tick time at which cur_area was set by init_area.
cur_area = None
cur_area_tstart = None

#The currently active OREMission, or None. The tstart variable is the tick time at which cur_mission was set by init_mission.
cur_mission = None
cur_mission_tstart = None

#When an area or mission selected, a list of all the various gameplay objects.
objects = None

#When in gameplay, an instance of mission.MissionControl defining the mission parameters.
mission_control = None

#An instance of sky.SkyStuff with defining the location of Voy and other distant objects
sky_stuff = None

#The current game mode, which determines the virtual interface and the meanining of player input
mode = None

#Input events (from PyGame) which occurred this step
events = []

#Billboard objects which should be drawn this frame
billboards = []

#The result of calls to joy.getAxes and joy.getButtons at the beginning of this simstep
axes = {}
buttons = {}

#Dictionary of collisions between geoms logged this step
#Each key is the id of an ODE geom
#Value is an array of ODE geoms (the geoms themselves, not ids) that the key geom collided with
#The collision is added both ways, so that if A and B collide, B is in A's list, and A is in B's list too
collisions = {}

#A group for momentary joints; the group is emptied each step, so joints only last for one step
contactgroup = ode.JointGroup()

winsize = (800, 600) #Size of the display window in pixels; TODO: should be a user setting
maxfps = 60 #Max frames per second, and absolute sim-steps per second

fade_color = None #If not None, this color is drawn as a rect covering the entire screen. Useful for fade effects.

player_camera = None #A Camera object describing our 3D viewpoint

screen = None #The PyGame screen

clock = None #An instance of pygame.time.Clock() used for timing; use step count, not this for game-logic timing
totalsteps = 0L #Number of simulation steps we've ran

cons = None #An instances of console.Console used for in-game debugging
watchers = [] #A sequence of console.Watchers used for in-game debugging

title_screen_manager = None #An instance of titlescreen.TitleScreenManager, which handles the behavior of pre-gameplay menus

class QuitException:
	"""Raised when something wants the main loop to end."""
	pass


def ui_init():
	global screen, clock, cons, watchers

	pygame.display.init()
	pygame.display.set_caption('Orbit Ribbon')
	pygame.display.set_icon(pygame.image.load(os.path.join('images', 'logo.png')))
	pygame.mouse.set_visible(0)
	screen = pygame.display.set_mode(winsize, DOUBLEBUF | OPENGL)

	pygame.mixer.init(22050, -16, 2, 512)

	clock = pygame.time.Clock()
	
	joy.init()
	
	glutInit(sys.argv) # GLUT is only used for drawing text and basic geometrical objects, not its full rigamarole of app control
	
	glDepthFunc(GL_LEQUAL)
	
	glClearColor(0, 0, 0, 0)
	glClearDepth(1.0)
	
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST)
	
	glEnable(GL_BLEND)
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA)
	
	cons = console.Console()
	watchers = []
	sys.stderr = cons.pseudofile
	sys.stdout = cons.pseudofile
	watchers.append(console.Watcher("q", "a", pygame.Rect(20, winsize[1]/5-30, winsize[0]/6, winsize[1]/5-20)))
	watchers.append(console.Watcher("d", "c", pygame.Rect(20, 2*winsize[1]/5-30, winsize[0]/6, winsize[1]/5-20)))
	watchers.append(console.Watcher("p", "l", pygame.Rect(5*winsize[0]/6 - 20, winsize[1]/5-30, winsize[0]/6, winsize[1]/5-20)))
	watchers.append(console.Watcher("j", "n", pygame.Rect(5*winsize[0]/6 - 20, 2*winsize[1]/5-30, winsize[0]/6, winsize[1]/5-20)))


def sim_init():
	"""Initializes the camera and simulation, including ODE.
	
	You must call this before calling run().
	
	You may call this in order to reset the game.
	"""
	
	global ore_man, odeworld, static_space, dyn_space, objects, totalsteps, player_camera, title_screen_manager, fade_color, sky_stuff
	global cur_area, cur_area_tstart, cur_mission, cur_mission_tstart
	
	totalsteps = 0L
	odeworld = ode.World()
	odeworld.setQuickStepNumIterations(10)
	static_space = ode.HashSpace()
	dyn_space = ode.HashSpace()
	objects = []
	title_screen_manager = titlescreen.TitleScreenManager()
	player_camera = title_screen_manager.camera
	fade_color = None
	sky_stuff = sky.SkyStuff()
	
	fh = file(os.path.join('orefiles', 'main.ore'))
	ore_man = ore.OREManager(fh)
	fh.close()

	cur_area = None
	cur_area_tstart = None
	cur_mission = None
	cur_mission_tstart = None


def init_area(areaname):
	"""Loads the given area, by internal name, into the game state. This includes objects, sky, etc."""
	global objects, sky_stuff, cur_area, cur_area_tstart
	cur_area = ore_man.areas[areaname]
	cur_area_tstart = pygame.time.get_ticks()
	objects = cur_area.objects[:] # FIXME Need to copy the contents of the objects themselves, so that the level can be restarted
	sky_stuff = cur_area.sky_stuff # FIXME Should probably also copy the SkyStuff for the sake of completeness


def init_mission(missionname):
	"""Loads the given mission, by internal name, into the game state. This includes mission objects, objectives, camera, etc.
	
	You must first init the mission's area before calling this."""
	global objects, mission_control, player_camera, cur_mission, cur_mission_tstart
	cur_mission = cur_area.missions[missionname]
	cur_mission_tstart = pygame.time.get_ticks()
	objects.extend(cur_mission.objects[:]) # FIXME Need to copy the contents of the objects themselves, so that the level can be restarted
	mission_control = cur_mission.mission_control # FIXME Should probably also copy the MissionControl for the sake of completeness
	
	# Find the Avatar object, then set the camera to follow it
	avatar_obj = None
	for obj in objects:
		if isinstance(obj, avatar.Avatar):
			avatar_obj = obj
			break
	if avatar_obj is None:
		raise RuntimeError("Unable to find Avatar in mission %s!" % missionname)
	player_camera = camera.FollowCamera(target_obj = avatar_obj)


def _sim_step():
	"""Runs one step of the simulation. This is (1/maxfps)th of a simulated second."""
	
	global collisions, contactgroup
	
	#Update mission status
	mission_control.step()
	
	#Calculate collisions, run ODE simulation
	contactgroup.empty()
	collisions = {}
	dyn_space.collide(contactgroup, collision.collision_cb) #Collisions among dyn_space objects
	ode.collide2(dyn_space, static_space, contactgroup, collision.collision_cb) #Colls between dyn_space objects and static_space objs
	odeworld.quickStep(1/maxfps)
	
	#Load each GameObj's state with the new information ODE calculated
	for o in objects:
		o.sync_ode()
	
	#Have each object do any simulation stuff it needs, then damp its linear and angular velocity to simulate air friction
	for o in objects:
		o.step()
		o.damp()


def _draw_frame():
	# Reset state
	glMatrixMode(GL_MODELVIEW)
	glLoadIdentity()
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT)
	
	# Locate the camera position, and orient to it
	camvals = player_camera.get_camvals()
	campos = player_camera.get_position()
	gluLookAt(*camvals)

	# Sort objects to be drawn into those which are far and need to be drawn as billboards, and those which are close and can be drawn as objects
	near_objs, far_objs = [], [] # Lists of GameObjs
	thresh = GAMEPLAY_CLIP_DIST * 0.9
	for o in objects:
		if o.pos.dist_to(campos) > thresh:
			far_objs.append(o)
		else:
			near_objs.append(o)
	
	# 3D projection mode for sky objects and billboards without depth-testing
	glDisable(GL_LIGHTING)
	glMatrixMode(GL_PROJECTION)
	glLoadIdentity()
	gluPerspective(FOV, winsize[0]/winsize[1], 0.1, SKY_CLIP_DIST)
	glMatrixMode(GL_MODELVIEW)
	
	# Draw the atmosphere
	glDisable(GL_DEPTH_TEST)
	sky_stuff.draw_geometry()

	# Draw the billboards for sky objects and distant gameplay objects
	glEnable(GL_DEPTH_TEST)
	glEnable(GL_TEXTURE_2D)
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE)
	sky_stuff.draw_billboards()
	for o in far_objs:
		o.distdraw()
	glDisable(GL_TEXTURE_2D)
	
	# 3D projection mode for nearby gameplay objects with depth-testing
	glEnable(GL_DEPTH_TEST)
	glEnable(GL_LIGHTING)
	glMatrixMode(GL_PROJECTION)
	glLoadIdentity()
	gluPerspective(FOV, winsize[0]/winsize[1], 0.1, GAMEPLAY_CLIP_DIST)
	glMatrixMode(GL_MODELVIEW)
	
	# Draw all objects in the list
	for o in near_objs:
		o.draw()
	
	# 2D drawing mode
	glMatrixMode(GL_PROJECTION)
	glLoadIdentity()
	gluOrtho2D(0.0, winsize[0], winsize[1], 0.0)
	glMatrixMode(GL_MODELVIEW)
	glLoadIdentity()
	glDisable(GL_DEPTH_TEST)
	glDisable(GL_LIGHTING)
	
	# Draw the virtual interface
	if mode == MODE_GAMEPLAY:
		mission_control.draw()
	else:
		title_screen_manager.draw()
	
	# If we have a fade color, apply it
	if fade_color is not None:
		glColor4f(*fade_color)
		glBegin(GL_QUADS)
		glVertex2f(0,          0)
		glVertex2f(winsize[0], 0)
		glVertex2f(winsize[0], winsize[1])
		glVertex2f(0,          winsize[1])
		glEnd()
	
	# Draw the watchers
	for w in watchers:
		if w.expr != None:
			w.draw()
	
	# Draw the console, if it's up
	cons.draw()
	
	# Output and flip buffers
	glFlush()
	pygame.display.flip()


def _proc_input():
	global events
	events = []
	for event in pygame.event.get():
		cons.handle(event)
		if event.type == pygame.QUIT:
			raise QuitException
		elif event.type == pygame.KEYDOWN and event.key == pygame.K_F4:
			raise QuitException
		else:
			if not cons.active:
				events.append(event)
	
	# Update the watchers
	for w in watchers:
		if w.expr != None:
			w.update()
	
	global axes, buttons
	axes = joy.getAxes()
	buttons = joy.getButtons()


def run():
	"""Runs the game.
	
	You have to call ui_init() and sim_init() before running this.
	
	Optionally, you may call init_area() and init_mission() to jump straight to a mission. If not, the
	game will start at the title screen.
	"""
	global totalsteps, mode
	
	if cur_area is not None and cur_mission is not None:
		mode = MODE_GAMEPLAY
	else:
		mode = MODE_TITLE_SCREEN
	
	try:
		totalms = 0L #Total number of milliseconds passed in gameplay
		while True:
			elapsedms = clock.tick(maxfps)
			
			if cons.active:
				_proc_input()
			elif mode == MODE_GAMEPLAY:
				totalms += elapsedms
				#Figure out how many simulation steps we're doing this frame.
				#In theory, shouldn't be zero, since max frames per second is the same as steps per second
				#However, it's alright to be occasionally zero, since clock.tick is sometimes slightly off
				#FIXME: Do we really need totalms?
				steps = int(math.floor((totalms*maxfps/1000)))-totalsteps
				#Run the simulation the desired number of steps
				for i in range(steps):
					_proc_input()
					_sim_step()
					totalsteps += 1
			elif mode == MODE_TITLE_SCREEN:
				_proc_input()
			
			#Draw everything
			_draw_frame()
	except QuitException:
		pass
