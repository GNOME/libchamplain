#!/usr/bin/env python
# -*- coding: utf-8 -*-
import gobject
import clutter
import champlain

def main():
    gobject.threads_init()
    clutter.init()

    stage = clutter.Stage(default=True)
    stage.set_size(800, 600)
    
    # Create the map view
    actor = champlain.View()
    actor.set_size(800, 600)
    stage.add(actor)
    
    stage.show()
    clutter.main()

    actor.destroy()


if __name__ == '__main__':
    main()
