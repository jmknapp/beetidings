#!/usr/bin/python

# Simple pygame program

# Import and initialize the pygame library
import pygame
import math
import mysql.connector
import time

import matplotlib
matplotlib.use("Agg")

import matplotlib.backends.backend_agg as agg


import pylab

class Airdata:
    unixtime = 0
    timestamp = ""
    pm1p0 = 0
    pm2p5 = 0
    pm4p0 = 0
    pm10p0 = 0
    humidity = 0
    temperature = 0
    voc_index = 0
    nox_index = 0

def getairdata():
    airrec = Airdata()

    mydb = mysql.connector.connect(
      host="localhost",
      user="root",
      password="R_250108_z",
      database="air"
    )

    mycursor = mydb.cursor()

    mycursor.execute("select * from sen55 order by unixtime desc limit 1")

    latest = mycursor.fetchone()

    airrec.unixtime = latest[0]
    airrec.timestamp = latest[1]
    airrec.pm1p0 = latest[2]
    airrec.pm2p5 = latest[3]
    airrec.pm4p0 = latest[4]
    airrec.pm10p0 = latest[5]
    airrec.humidity = latest[6]
    airrec.temperature = latest[7]
    airrec.voc_index = latest[8]
    airrec.nox_index = latest[9]

    return airrec

def gettrend():
    return raw_data

white = (255, 255, 255)
green = (0, 255, 0)
red = (255, 0, 0)
darkgreen = (0, 75, 0)
blue = (0, 0, 128)
lightgray = (150, 150, 150)
gray = (100, 100, 100)
transparent = (0, 0, 0, 0)

plotbackground = (0, 0.3, 0)

fig = pylab.figure(figsize=[3.5, 3.5], # Inches
                   dpi=100,        # 100 dots per inch, so the resulting buffer is 400x400 pixels
                   )
t = [1, 2, 3]
pm2p5 = [1, 4, 9]
ax = fig.gca()
ax.set_facecolor(plotbackground) 
fig.set_facecolor(plotbackground)
ax.plot(t, pm2p5, 'g')
ax.set_xlabel("Average Pulse")
ax.set_ylabel("Calorie Burnage")
ax.axis('off')
ax.fill_between(x = t, y1 = pm2p5, color="r", alpha=1)

#canvas = agg.FigureCanvasAgg(fig)
#canvas.draw()
#renderer = canvas.get_renderer()
#raw_data = renderer.tostring_rgb()

canvas = agg.FigureCanvasAgg(fig)
canvas.draw()
renderer = canvas.get_renderer()
raw_data = renderer.tostring_rgb()

pygame.init()

info = pygame.display.Info()

W = info.current_w
H = info.current_h
centerW = math.floor(W/2)
centerH = math.floor(H/2)

# Set up the drawing window
screen = pygame.display.set_mode([W, H])

pygame.display.toggle_fullscreen()

# create a font object.
# 1st parameter is the font file
# which is present in pygame.
# 2nd parameter is size of the font
font = pygame.font.Font('freesansbold.ttf', 180)
 
# Run until the user asks to quit
running = True
while running:

    airrec = getairdata()
    #airinfo = str(airrec.timestamp)+": "+str(airrec.pm2p5) ;
    airinfo = str(round(airrec.pm2p5)) ;


    # Did the user click the window close button?
    for event in pygame.event.get():
        if event.type == pygame.MOUSEBUTTONDOWN:
            running = False

    # Fill the background with white
    screen.fill(darkgreen)

    size = canvas.get_width_height()
    surf = pygame.image.fromstring(raw_data, size, "RGB")
    screen.blit(surf, (50,0))

    # Draw a solid blue circle in the center
    #pygame.draw.circle(screen, (0, 0, 255), (centerW, centerH), 500)

    # create a text surface object,
    # on which text is drawn on it.
    text = font.render(airinfo, True, white)

    # create a rectangular object for the
    # text surface object
    textRect = text.get_rect()
 
    # set the center of the rectangular object.
    textRect.center = (W // 2, H // 2)

    screen.blit(text, textRect)


    time.sleep(1)

    # Flip the display
    pygame.display.flip()

# Done! Time to quit.
pygame.quit()
