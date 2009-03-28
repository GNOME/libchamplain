#!/usr/bin/env python

import gobject, clutter, champlain

#Initialise modules
gobject.threads_init()
clutter.init()

#Get the clutter stage
stage = clutter.stage_get_default()
stage.set_size(800, 600)

#Create a Champlain view
view = champlain.View()
view.set_size(800, 600)

#Add the view to the stage
stage.add(view)

#Make the stage visible
stage.show()

#Begin the clutter main loop
clutter.main()
