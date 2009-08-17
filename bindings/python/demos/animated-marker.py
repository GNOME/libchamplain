#!/usr/bin/env python
# -*- coding: utf-8 -*-

#This is a demonstration of the libchamplain python binding.
#It will display a blue "pulsating" dot on a given position on the map
#A old legend that this position is the bed of Pierlux. You will have to find out...

import champlain
import clutter
import cairo
import math
import gobject

MARKER_SIZE = 10
MARKER_COLOR = [0.1,0.1,0.9,1.0]

POSITION = [45.528178, -73.563788]
SCREEN_SIZE = [640, 480]

#The AnimatedMarker will extend the champlain.Marker class
class AnimatedMarker(champlain.Marker) :
    def __init__(self,color=None) :
        champlain.Marker.__init__(self)
        
        orange = clutter.Color(0xf3, 0x94, 0x07, 0xbb)
        white = clutter.Color(0xff, 0xff, 0xff, 0xff)
        if not color :
            color = MARKER_COLOR
        
        #Cairo definition of the inner marker
        bg_in = clutter.CairoTexture(MARKER_SIZE, MARKER_SIZE)
        cr_in = bg_in.cairo_create()
        cr_in.set_source_rgb(0, 0, 0)
        cr_in.arc(MARKER_SIZE / 2.0, MARKER_SIZE / 2.0, MARKER_SIZE / 2.0, 0, 2 * math.pi)
        cr_in.close_path()
        cr_in.set_source_rgba(*color)
        cr_in.fill()
        self.add(bg_in)
        bg_in.set_anchor_point_from_gravity(clutter.GRAVITY_CENTER)
        bg_in.set_position(0, 0)
        
        #Cairo definition of the outside circle (that will be animated)
        bg_out = clutter.CairoTexture(2 * MARKER_SIZE, 2 * MARKER_SIZE)
        cr_out = bg_out.cairo_create()
        cr_out.set_source_rgb(0, 0, 0)
        cr_out.arc(MARKER_SIZE, MARKER_SIZE, 0.9 * MARKER_SIZE, 0, 2 * math.pi)
        cr_out.close_path()
        cr_out.set_line_width(2.0)
        cr_out.set_source_rgba(*color)
        cr_out.stroke()
        self.add(bg_out)
        bg_out.lower_bottom()
        bg_out.set_position(0, 0)
        bg_out.set_anchor_point_from_gravity(clutter.GRAVITY_CENTER)

        #The timeline of our animation
        self.timeline = clutter.Timeline()
        self.timeline.set_duration(1000)
        self.timeline.set_loop(True)

        self.alpha = clutter.Alpha(self.timeline, clutter.EASE_OUT_SINE)
        
        #The "growing" behaviour
        self.grow_behaviour = clutter.BehaviourScale(0.5, 0.5, 2.0, 2.0)
        self.grow_behaviour.set_alpha(self.alpha)
        self.grow_behaviour.apply(bg_out)

        #The fade out behaviour
        self.fade_behaviour = clutter.BehaviourOpacity(255, 0)
        self.fade_behaviour.set_alpha(self.alpha)
        self.fade_behaviour.apply(bg_out)

        #If you want the marked to be animated without calling start_animation
        #Uncomment the following line
        #self.timeline.start()
        
    def stop_animation(self) :
        self.timeline.stop()
        
    def start_animation(self) :
        self.timeline.start()
        

def main() :
    gobject.threads_init()
    clutter.init()
    stage = clutter.stage_get_default()
    actor = champlain.View()
    layer = champlain.Layer()
    marker = AnimatedMarker()
    #Starting the animation, that's what we want after all !
    marker.start_animation()
    #Set marker position on the map
    marker.set_position(*POSITION)
    
    stage.set_size(*SCREEN_SIZE)
    actor.set_size(*SCREEN_SIZE)
    stage.add(actor)
    
    layer.show()
    actor.add_layer(layer)
    layer.add(marker)

    # Finish initialising the map view
    actor.set_property("zoom-level", 5)
    actor.set_property("scroll-mode", champlain.SCROLL_MODE_KINETIC)
    actor.center_on(*POSITION)
    
    stage.show()
    
    clutter.main()
        

if __name__ == "__main__":
    main()
