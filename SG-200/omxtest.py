from omxplayer.player import OMXPlayer
from pathlib import Path
from time import sleep

filename=input()

VIDEO_PATH = Path("/home/pi/project/dataset/{0}.mp4".format(filename))

player = OMXPlayer(VIDEO_PATH)

sleep(20)

player.quit()
