#!/usr/bin/env python3

from omxplayer.player import OMXPlayer
from pathlib import Path
from time import sleep
import logging
logging.basicConfig(level=logging.INFO)


VIDEO_1_PATH = "../dataset/minions.mp4"
player_log = logging.getLogger("Player 1")

player = OMXPlayer(VIDEO_1_PATH, 
        dbus_name='org.mpris.MediaPlayer2.omxplayer1',args=['--no-osd'])
player.playEvent += lambda _: player_log.info("Play")
player.pauseEvent += lambda _: player_log.info("Pause")
player.stopEvent += lambda _: player_log.info("Stop")


# it takes about this long for omxplayer to warm up and start displaying a picture on a rpi3
sleep(2)

player.set_position(5)
player.pause()

sleep(1)

player.set_aspect_mode('stretch')
player.set_video_pos(0, 0, 200, 200)
player.play()
a = input('how do yo feel\n')

if a == '3':
    player.quit()
    print('if')
    VIDEO_2_PATH = "../dataset/common_fairi.mp4"
    playerA_log = logging.getLogger("Player 2")

    playerA = OMXPlayer(VIDEO_2_PATH,dbus_name='org.mpris.MediaPlayer2.omxplayer2',args=['-o','local','--no-osd'])
    playerA.playEvent += lambda _: playerA_log.info("Play")
    playerA.pauseEvent += lambda _: playerA_log.info("Pause")
    playerA.stopEvent += lambda _: playerA_log.info("Stop")
    sleep(10)

    playerA.set_position(5)
    playerA.pause()

    sleep(1)

    playerA.set_aspect_mode('stretch')
    playerA.set_video_pos(0, 0, 200, 200)
    playerA.play()

    sleep(2)
    playerA.quit()
else:
    print('no matter what')
    player.quit()
